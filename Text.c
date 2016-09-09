#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "Text.h"

/* Allocate buffers holding the actual file content in junks of size: */
#define BUFFER_SIZE ((size_t) 1 << 14)
/* Files smaller than this value will be load into memory, larger ones are 
* truncated.
*/
#define BUFFER_TRUNCATE_SIZE ((size_t) 1 << 15)
// TODO: try to find best size for reading file.
/* buffer size for reading file content into memory in function buffer_read. */
#define READ_BUFF_SIZE ((size_t) 1 << 8)

struct Piece 
{
	Text *text;             /* text to which this piece belongs */
	Piece *prev, *next;     /* pointers to the logical predecessor/successor */
	Piece *global_prev;     /* double linked list in order of allocation, */
	Piece *global_next;     /* used to free individual pieces */
	const char *data;       /* pointer into a Buffer holding the data */
	size_t len;             /* the length in number of bytes of the data */
};

/* used to transform a global position (byte offset starting from the beginning
* of the text) into an offset relative to a piece.
*/
typedef struct {
	Piece *piece;           /* piece holding the location */
	size_t off;             /* offset into the piece in bytes */
} Location;

/* A Span holds a certain range of pieces. Changes to the document are always
* performed by swapping out an existing span with a new one.
*/
typedef struct {
	Piece *start, *end;     /* start/end of the span */
	size_t len;             /* the sum of the lengths of the pieces which form this span */
} Span;

/* A Change keeps all needed information to redo/undo an insertion/deletion. */
typedef struct Change Change;
struct Change {
	Span old;               /* all pieces which are being modified/swapped out by the change */
	Span new;               /* all pieces which are introduced/swapped in by the change */
	size_t pos;             /* absolute position at which the change occured */
	Change *next;           /* next change which is part of the same action */
	Change *prev;           /* previous change which is part of the same action */
};

/* An Action is a list of Changes which are used to undo/redo all modifications
* since the last snapshot operation. Actions are stored in a directed graph structure.
*/
typedef struct Action Action;
struct Action {
	Change *change;         /* the most recent change */
	Action *next;           /* the next (child) action in the undo tree */
	Action *prev;           /* the previous (parent) operation in the undo tree */
	Action *earlier;        /* the previous Action, chronologically */
	Action *later;          /* the next Action, chronologically */
	time_t time;            /* when the first change of this action was performed */
	size_t seq;             /* a unique, strictly increasing identifier */
};


typedef struct Buffer Buffer;
struct Buffer {
	size_t size;               /* maximal capacity */
	size_t len;                /* current used length / insertion position */
	char *data;                /* actual data */
	union 
	{
		char *data;
		char swp;
#ifdef __BORLANDC__
		char far *farmem;
#endif
	} bak;
	enum 
	{                     
		FILESWP,
		FAR,
		HEAP,            
	} type;
	Buffer *next;              /* next junk */
};

typedef struct {
	size_t pos;             /* position in bytes from start of file */
	size_t lineno;          /* line number in file i.e. number of '\n' in [0, pos) */
} LineCache;


/* The main struct holding all information of a given file */
struct Text 
{
	Buffer *buf;            /* Contains original file content */
	Buffer *buffers;        /* buffers allocated to hold insertion data */
	Piece *pieces;          /* all allocated pieces, used to free them */
	Piece *cache;           /* most recently modified piece */
	Piece begin, end;       /* sentinel nodes */
	Action *history;        /* undo tree */
	Action *current_action; /* action holding all file changes until a snapshot is performed */
	Action *last_action;    /* last action added to the tree, chronologically */
	Action *saved_action;   /* last action at the time of the save operation */
	size_t size;            /* current file content size in bytes */
	struct stat info;       /* stat as probed at load time */
	LineCache lines;        /* mapping between absolute pos in bytes and logical line breaks */
	enum TextNewLine newlines; /* which type of new lines does the file use */
};

struct TextSave {                  /* used to hold context between text_save_{begin,commit} calls */
	Text *txt;                 /* text to operate on */
	char *filename;            /* filename to save to as given to text_save_begin */
	char *tmpname;             /* temporary name used for atomic rename(2) */
	int fd;                    /* file descriptor to write data to using text_save_write */
	enum {
		TEXT_SAVE_UNKNOWN,
		TEXT_SAVE_ATOMIC,  /* create a new file, write content, atomically rename(2) over old file */
		TEXT_SAVE_INPLACE, /* truncate file, overwrite content (any error will result in data loss) */
	} type;
};

static Buffer *buffer_truncated_read(Text *txt, size_t size, int fd);
static Buffer *buffer_alloc(Text *txt, size_t size);
static const char *buffer_append(Buffer *buf, const char *data, size_t len);
static Buffer *buffer_read(Text *txt, size_t size, int fd);
static void buffer_free(Buffer *buf);
static void lineno_cache_invalidate(LineCache *cache);
static Piece *piece_alloc(Text *txt);
static void piece_init(Piece *p, Piece *prev, Piece *next, const char *data, size_t len);
static void piece_free(Piece *p);
static void action_free(Action *a);
static Action *action_alloc(Text *txt);
static Change *change_alloc(Text *txt, size_t pos);
static void change_free(Change *c);

// TODO: think out a plan to deal with large file.
/* Truncate file and read when file is too large. */
static Buffer *buffer_truncated_read(Text *txt, size_t size, int fd)
{
	Buffer *buf = buffer_alloc(txt, size);
	return buf;
}

/* allocate a new buffer of MAX(size, BUFFER_SIZE) bytes */
static Buffer *buffer_alloc(Text *txt, size_t size) 
{
	Buffer *buf = calloc(1, sizeof(Buffer));
	if (!buf)
		return NULL;
	if (BUFFER_SIZE > size)
		size = BUFFER_SIZE;
	if (!(buf->data = malloc(size)))
	{
		free(buf);
		return NULL;
	}
	buf->type = HEAP;
	buf->size = size;
	buf->next = txt->buffers;
	txt->buffers = buf;
	return buf;
}

/* Append data to buffer, assumes there is enough space available */
static const char *buffer_append(Buffer *buf, const char *data, size_t len) 
{
	char *dest = memcpy(buf->data + buf->len, data, len);
	buf->len += len;
	return dest;
}

static Buffer *buffer_read(Text *txt, size_t size, int fd) 
{
	Buffer *buf = buffer_alloc(txt, size);
	if (!buf)
		return NULL;
	while (size > 0) 
	{
		char data[READ_BUFF_SIZE];
		ssize_t len = read(fd, data, MIN(sizeof(data), size));
		if (len == -1) 
		{
			txt->buffers = buf->next;
			buffer_free(buf);
			return NULL;
		}
		else if (len == 0)
			break;
		else 
		{
			buffer_append(buf, data, len);
			size -= len;
		}
	}
	return buf;
}

static void buffer_free(Buffer *buf) 
{
	if (!buf)
		return;
	if (buf->type == HEAP)
		free(buf->data);
	else if ((buf->type == FAR || buf->type == FILESWP) && buf->data)
		// TODO: think out a plan to free buff when buffer'type is FAR or FILESWP
		;

	free(buf);
}

static void lineno_cache_invalidate(LineCache *cache) 
{
	cache->pos = 0;
	cache->lineno = 1;
}

static Piece *piece_alloc(Text *txt) 
{
	Piece *p = calloc(1, sizeof(Piece));
	if (!p)
		return NULL;
	p->text = txt;
	p->global_next = txt->pieces;
	if (txt->pieces)
		txt->pieces->global_prev = p;
	txt->pieces = p;
	return p;
}

static void piece_init(Piece *p, Piece *prev, Piece *next, const char *data, size_t len) 
{
	p->prev = prev;
	p->next = next;
	p->data = data;
	p->len = len;
}

static void piece_free(Piece *p) 
{
	if (!p)
		return;
	if (p->global_prev)
		p->global_prev->global_next = p->global_next;
	if (p->global_next)
		p->global_next->global_prev = p->global_prev;
	if (p->text->pieces == p)
		p->text->pieces = p->global_next;
	if (p->text->cache == p)
		p->text->cache = NULL;
	free(p);
}

/* allocate a new change, associate it with current action or a newly
* allocated one if none exists. */
static Change *change_alloc(Text *txt, size_t pos) 
{
	Action *a = txt->current_action;
	Change *c;
	if (!a) {
		a = action_alloc(txt);
		if (!a)
			return NULL;
	}
	c = calloc(1, sizeof(Change));
	if (!c)
		return NULL;
	c->pos = pos;
	c->next = a->change;
	if (a->change)
		a->change->prev = c;
	a->change = c;
	return c;
}

static void change_free(Change *c) 
{
	if (!c)
		return;
	/* only free the new part of the span, the old one is still in use */
	piece_free(c->new.start);
	if (c->new.start != c->new.end)
		piece_free(c->new.end);
	free(c);
}

static void action_free(Action *a) 
{
	Change *next, *c;
	if (!a)
		return;
	for (c = a->change; c; c = next) {
		next = c->next;
		change_free(c);
	}
	free(a);
}

/* allocate a new action, set its pointers to the other actions in the history,
* and set it as txt->history. All further changes will be associated with this action. */
static Action *action_alloc(Text *txt) 
{
	Action *new = calloc(1, sizeof(Action));
	if (!new)
		return NULL;
	new->time = time(NULL);
	txt->current_action = new;

	/* set sequence number */
	if (!txt->last_action)
		new->seq = 0;
	else
		new->seq = txt->last_action->seq + 1;

	/* set earlier, later pointers */
	if (txt->last_action)
		txt->last_action->later = new;
	new->earlier = txt->last_action;

	if (!txt->history) {
		txt->history = new;
		return new;
	}

	/* set prev, next pointers */
	new->prev = txt->history;
	txt->history->next = new;
	txt->history = new;
	return new;
}

/* load the given file as starting point for further editing operations.
* to start with an empty document, pass NULL as filename. */
Text *text_load(const char *filename) 
{
	int fd = -1;
	size_t size;
	Piece *p;
	Text *txt = calloc(1, sizeof(Text));
	if (!txt)
		return NULL;
	piece_init(&txt->begin, NULL, &txt->end, NULL, 0);
	piece_init(&txt->end, &txt->begin, NULL, NULL, 0);
	lineno_cache_invalidate(&txt->lines);
	if (filename) {
		if ((fd = open(filename, O_RDONLY)) == -1)
			goto out;
		// file not found or fd is invalid.
		if (fstat(fd, &txt->info) == -1)
			goto out;
		size = txt->info.st_size;
		if (size < BUFFER_TRUNCATE_SIZE)
			txt->buf = buffer_read(txt, size, fd);
		else
			txt->buf = buffer_truncated_read(txt, size, fd);
		if (!txt->buf)
			goto out;
		p = piece_alloc(txt);
		if (!p)
			goto out;
		piece_init(&txt->begin, NULL, p, NULL, 0);
		piece_init(p, &txt->begin, &txt->end, txt->buf->data, txt->buf->len);
		piece_init(&txt->end, p, NULL, NULL, 0);
		txt->size = txt->buf->len;
	}
	/* write an empty action */
	change_alloc(txt, EPOS);
	text_snapshot(txt);
	txt->saved_action = txt->history;

	if (fd != -1)
		close(fd);
	return txt;
out:
	if (fd != -1)
		close(fd);
	text_free(txt);
	return NULL;
}

void text_free(Text *txt) 
{
	if (!txt)
		return;

	// free history
	{
		Action *hist = txt->history;
		while (hist && hist->prev)
			hist = hist->prev;
		while (hist)
		{
			Action *later = hist->later;
			action_free(hist);
			hist = later;
		}
	}
	// free pieces
	{
		Piece *next, *p;
		for (p = txt->pieces; p; p = next)
		{
			next = p->global_next;
			piece_free(p);
		}
	}
	// free buffers
	{
		Buffer *next, *buf;
		for (buf = txt->buffers; buf; buf = next)
		{
			next = buf->next;
			buffer_free(buf);
		}
	}

	free(txt);
}

/* preserve the current text content such that it can be restored by
* means of undo/redo operations */
void text_snapshot(Text *txt) 
{
	if (txt->current_action)
		txt->last_action = txt->current_action;
	txt->current_action = NULL;
	txt->cache = NULL;
}

