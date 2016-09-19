#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "Text.h"

/* Allocate buffers holding the actual file content in junks of size: */
#define BUFFER_SIZE ((size_t) 1 << 13)
/* Files smaller than this value will be load into memory, larger one will be
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

/* used to transform a global position into an offset relative to a piece. */
typedef struct {
	Piece *piece;           /* piece holding the location */
	size_t off;             /* offset into the piece in bytes */
} Location;

/* A Span holds a certain range of pieces. Changes to the document are always
* performed by swapping out an existing span with a new one.
*/
typedef struct {
	Piece *start, *end;     /* start/end of the span */
	size_t len;             /* the sum of this span */
} Span;

/* A Change keeps all needed information to redo/undo an insertion/deletion. 
* Changes make up A Change list which is alaways related to the same Action.
*/
typedef struct Change Change;
struct Change 
{
	Span old;               /* all pieces which are being modified/swapped */
	Span new;               /* all pieces which are introduced/swapped */
	size_t pos;             /* absolute position at which the change occured */
	Change *next;          
	Change *prev;           
};

/* An Action is a list of Changes which are used to undo/redo 
* all modifications since the last snapshot operation.
* Actions are stored in a directed graph structure.
*/
typedef struct Action Action;
struct Action {
	Change *change;         /* the most recent change */
	Action *next;           /* the child action in the undo tree */
	Action *prev;           /* the parent operation in the undo tree */
	Action *earlier;        /* the previous Action, chronologically */
	Action *later;          /* the next Action, chronologically */
	time_t time;            /* when the first change of this action appears. */
	size_t seq;             /* a unique, strictly increasing identifier */
};

/* buffer has three types. FILESWP for very large file. 
* FARMEM for large file. HEAP for current part of file.
* Make sure being edited buffer' type is HEAP.
*/
typedef struct Buffer Buffer;
struct Buffer {
	size_t size;               /* maximal capacity */
	size_t len;                /* current used length / insertion position */
	char *data;                /* actual data */
	union 
	{
		char fileswp; /* keep swap file seq.*/
#ifdef __BORLANDC__
		char far *farmem; /* keep address where data moved to. */
#endif
	} bak;
	enum 
	{                     
		FILESWP,   /* Buf after writing data to file. */
		FARMEM,    /* Buf after moving data to far memery. */
		HEAP,      /* Buf in heap use near pointer. */         
	} type;
	Buffer *next;  /* next junk */
};

typedef struct {
	size_t pos;             
	size_t lineno;  /* line number in file i.e. number of '\n' in [0, pos) */
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

/* buffer layer */
static Buffer *buffer_truncated_read(Text *txt, size_t size, int fd);
static Buffer *buffer_alloc(Text *txt, size_t size);
static const char *buffer_append(Buffer *buf, const char *data, size_t len);
static bool buffer_capacity(Buffer *buf, size_t len);
static const char *buffer_store(Text *txt, const char *data, size_t len);
static bool buffer_insert(Buffer *buf, size_t pos, const char *data, size_t len);
static Buffer *buffer_read(Text *txt, size_t size, int fd);
static void buffer_free(Buffer *buf);
/* Location layer */
static void lineno_cache_invalidate(LineCache *cache);
/* Cache layer */
static bool cache_contains(Text *txt, Piece *p);
static void cache_piece(Text *txt, Piece *p);
static bool cache_insert(Text *txt, Piece *p, size_t off, const char *data, size_t len);
/* Piece layer*/
static Piece *piece_alloc(Text *txt);
static void piece_init(Piece *p, Piece *prev, Piece *next, const char *data, size_t len);
static Location piece_get_intern(Text *txt, size_t pos);
static void piece_free(Piece *p);
/* Acthon layer */
static void action_free(Action *a);
static Action *action_alloc(Text *txt);
/* Change layer */
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

/* check whether buffer has enough free space to store len bytes */
static bool buffer_capacity(Buffer *buf, size_t len) 
{
	return buf->size - buf->len >= len;
}

/* stores the given data in a buffer, allocates a new one if necessary.
* returns a pointer to the storage location or NULL if allocation failed. 
*/
static const char *buffer_store(Text *txt, const char *data, size_t len) 
{
	Buffer *buf = txt->buffers;
	// when buf == NULL or buffer can't not contains data, run buffer_alloc.
	if ((!buf || !buffer_capacity(buf, len)) && !(buf = buffer_alloc(txt, len)))
		return NULL;
	return buffer_append(buf, data, len);
}

/* insert data into buffer at an arbitrary position, this should only 
* be used with data of the most recently created piece. 
*/ 
static bool buffer_insert(Buffer *buf, size_t pos, const char *data, size_t len) 
{
	char *insert;
	if (pos > buf->len || !buffer_capacity(buf, len))
		return false;
	if (buf->len == pos)
		return (bool) buffer_append(buf, data, len);
	insert = buf->data + pos;
	memmove(insert + len, insert, buf->len - pos);
	memcpy(insert, data, len);
	buf->len += len;
	return true;
}

/* Read content of file to memery with buffer.*/
static Buffer *buffer_read(Text *txt, size_t size, int fd) 
{
	Buffer *buf = buffer_alloc(txt, size);
	if (!buf)
		return NULL;
	while (size > 0) 
	{
		char data[READ_BUFF_SIZE];
		ssize_t len = read(fd, data, MIN(sizeof(data), size));
		// read failed.
		if (len == -1) 
		{
			txt->buffers = buf->next;
			buffer_free(buf);
			return NULL;
		}
		// read over.
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

/* delete data from a buffer at an arbitrary position, this should only be used with
* data of the most recently created piece. */
static bool buffer_delete(Buffer *buf, size_t pos, size_t len) 
{
	char *delete;

	if (pos + len > buf->len)
		return false;
	if (buf->len == pos) 
	{
		buf->len -= len;
		return true;
	}
	delete = buf->data + pos;
	memmove(delete, delete + len, buf->len - pos - len);
	buf->len -= len;

	return true;
}

static void buffer_free(Buffer *buf) 
{ 
	if (!buf)
		return;
	if (buf->type == HEAP)
		free(buf->data);
	else if ((buf->type == FARMEM || buf->type == FILESWP) && buf->data)
		// TODO: think out a plan to free buff when buffer'type is FAR or FILESWP
		;

	free(buf);
}

static void lineno_cache_invalidate(LineCache *cache) 
{
	cache->pos = 0;
	cache->lineno = 1;
}


/* Check whether the given piece was the most recently modified one. 
* There are three conditions required:
*	1) txt->cache == p;
*	2) this piece is one of the most recently change's new span's piece.
*   3) the piece contains the last piece of buffer.
*/
static bool cache_contains(Text *txt, Piece *p) 
{
	Buffer *buf = txt->buffers;
	Action *a = txt->current_action;
	Piece *cur;
	bool found;
	if (!buf || !txt->cache || txt->cache != p || !a || !a->change)
		return false;

	// look up piece in most recently change's new span.
	found = false;
	for (cur = a->change->new.start; !found; cur = cur->next)
	{
		if (cur == p)
			found = true;
		if (cur == a->change->new.end)
			break;
	}
	//means the piece is the last piece, so buffer can be append.
	return found && p->data + p->len == buf->data + buf->len;
}

/* cache the given piece if it is the most recently changed one */
static void cache_piece(Text *txt, Piece *p) 
{
	Buffer *buf = txt->buffers;
	if (!buf || p->data < buf->data || p->data + p->len != buf->data + buf->len)
		return;
	txt->cache = p;
}

/* Try to insert a junk of data at a given piece offset. the insertion 
* is only performed if the piece is the most recenetly changed one. 
*/
static bool cache_insert(Text *txt, Piece *p, size_t off, const char *data, size_t len) 
{
	Buffer *buf;
	size_t bufpos;
	if (!cache_contains(txt, p))
		return false;
	buf = txt->buffers;
	bufpos = p->data + off - buf->data;
	if (!buffer_insert(buf, bufpos, data, len))
		return false;
	p->len += len;
	txt->current_action->change->new.len += len;
	txt->size += len;
	return true;
}

/* try to delete a junk of data at a given piece offset. the deletion is only
* performed if the piece is the most recenetly changed one and the whole
* affected range lies within it. the legnth of the piece, the span containing it
* and the whole text is adjusted accordingly */
static bool cache_delete(Text *txt, Piece *p, size_t off, size_t len) 
{
	Buffer *buf;
	size_t bufpos;
	if (!cache_contains(txt, p))
		return false;
	buf = txt->buffers;
	bufpos = p->data + off - buf->data;
	if (off + len > p->len || !buffer_delete(buf, bufpos, len))
		return false;
	p->len -= len;
	txt->current_action->change->new.len -= len;
	txt->size -= len;
	return true;
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

/* Returns the piece holding the text at byte offset pos. 
* Note: 
*	1) In particular if pos is zero, the begin sentinel piece is returned.
*   2) if pos happens to be at a piece boundry, return the previous piece.
*      i.e. when the byte is the first byte of a piece, the previous that
*      don't contains this byte is been returned with an offest of 
*      piece->len.
*      This is make modificate piece chain more conveniently when returned
*      piece are both needed.
*/
static Location piece_get_intern(Text *txt, size_t pos)
{
	size_t cur = 0;
	Location loc;
	Piece *p;

	for (p = &txt->begin; p->next; p = p->next)
	{
		if (cur <= pos && pos <= cur + p->len) 
		{
			loc.off = pos - cur;
			loc.piece = p;
			return loc;
		}
		cur += p->len;
	}
	loc.off = 0;
	loc.piece = NULL;
	return loc;
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

/* allocate a new action, set its pointers 
* to the other actions in the history,
* and set it as txt->history. All further 
* changes will be associated with this action. 
*/
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


/* initialize a span and calculate its length */
static void span_init(Span *span, Piece *start, Piece *end) 
{
	size_t len = 0;
	Piece *p;
	span->start = start;
	span->end = end;
	for (p = start; p; p = p->next) {
		len += p->len;
		if (p == end)
			break;
	}
	span->len = len;
}

/* swap out an old span and replace it with a new one.
*
*  - if old is an empty span do not remove anything, just insert the new one
*  - if new is an empty span do not insert anything, just remove the old one
*
* adjusts the document size accordingly.
*/
static void span_swap(Text *txt, Span *old, Span *new) 
{
	if (old->len == 0 && new->len == 0) {
		return;
	}
	else if (old->len == 0) 
	{
		/* insert new span */
		new->start->prev->next = new->start;
		new->end->next->prev = new->end;
	}
	else if (new->len == 0) 
	{
		/* delete old span */
		old->start->prev->next = old->end->next;
		old->end->next->prev = old->start->prev;
	}
	else 
	{
		/* replace old with new */
		old->start->prev->next = new->start;
		old->end->next->prev = new->end;
	}
	txt->size -= old->len;
	txt->size += new->len;
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

/* When inserting new data there are 2 cases to consider.
*
*  1) the insertion point falls into the middle of an exisiting
*    piece which will be replaced by three new pieces:
*
*      /-+ --> +---------------+ --> +-\
*      | |     | existing text |     | |
*      \-+ <-- +---------------+ <-- +-/
*                         ^
*                         Insertion point for "demo "
*
*      /-+ --> +---------+ --> +-----+ --> +-----+ --> +-\
*      | |     | existing|     |demo |     |text |     | |
*      \-+ <-- +---------+ <-- +-----+ <-- +-----+ <-- +-/
*
*  2) the insertion point falls into a piece boundry:
*
*      /-+ --> +---------------+ --> +-\
*      | |     | existing text |     | |
*      \-+ <-- +---------------+ <-- +-/
*            ^
*            Insertion point for "short"
*
*      /-+ --> +-----+ --> +---------------+ --> +-\
*      | |     |short|     | existing text |     | |
*      \-+ <-- +-----+ <-- +---------------+ <-- +-/
*/
bool text_insert(Text *txt, size_t pos, const char *data, size_t len) 
{
	Location loc;
	Piece *p, *new = NULL ; 
	size_t off;
	Change *c;

	// make sure pos meaningful.
	if (len == 0)
		return true;
	if (pos > txt->size)
		return false;
	if (pos < txt->lines.pos)
		lineno_cache_invalidate(&txt->lines);
	// check whether insert can be completed by cache.
	loc = piece_get_intern(txt, pos);
	p = loc.piece;
	off = loc.off;
	if (!p)
		return false;
	if (cache_insert(txt, p, off, data, len))
		return true;
	// store data first.
	if (!(data = buffer_store(txt, data, len)))
		return false;
	
	// recoding change for re/undo.
	c = change_alloc(txt, pos);
	if (!c)
		return false;
	if (off == p->len) 
	{
		/* insert between two existing pieces, hence there is nothing to
		* remove, just add a new piece holding the extra text */
		if (!(new = piece_alloc(txt)))
			return false;
		piece_init(new, p, p->next, data, len);
		span_init(&c->new, new, new);
		span_init(&c->old, NULL, NULL);
	}
	else 
	{
		/* insert into middle of an existing piece, therfore split the old
		* piece. that is we have 3 new pieces one containing the content
		* before the insertion point then one holding the newly inserted
		* text and one holding the content after the insertion point.
		*/
		Piece *before = piece_alloc(txt);
		Piece *after = piece_alloc(txt);
		new = piece_alloc(txt);
		if (!before || !new || !after)
			return false;
		piece_init(before, p->prev, new, p->data, off);
		piece_init(new, before, after, data, len);
		piece_init(after, new, p->next, p->data + off, p->len - off);

		span_init(&c->new, before, after);
		span_init(&c->old, p, p);
	}

	cache_piece(txt, new);
	span_swap(txt, &c->old, &c->new);
	return true;
}

/* A delete operation can either start/stop midway through a piece or at
* a boundry. In the former case a new piece is created to represent the
* remaining text before/after the modification point.
*
*      /-+ --> +---------+ --> +-----+ --> +-----+ --> +-\
*      | |     | existing|     |demo |     |text |     | |
*      \-+ <-- +---------+ <-- +-----+ <-- +-----+ <-- +-/
*                   ^                         ^
*                   |------ delete range -----|
*
*      /-+ --> +----+ --> +--+ --> +-\
*      | |     | exi|     |t |     | |
*      \-+ <-- +----+ <-- +--+ <-- +-/
*/
bool text_delete(Text *txt, size_t pos, size_t len) 
{
	Location loc;
	Piece *p;
	size_t off,cur; /* how much has already been deleted */
	Change *c;
	Piece *before, *after; /* unmodified pieces before/after deletion point */
	Piece *old_start, *old_end; /* span which is removed */
	// flag for whether split pieces in start and end. 
	bool midway_start = false, midway_end = false; 
	Piece *new_start = NULL, *new_end = NULL;
	if (len == 0)
		return true;
	if (pos + len > txt->size)
		return false;
	if (pos < txt->lines.pos)
		lineno_cache_invalidate(&txt->lines);

	loc = piece_get_intern(txt, pos);
	p = loc.piece;
	if (!p)
		return false;
	off = loc.off;
	if (cache_delete(txt, p, off, len))
		return true;
	
	if (off == p->len)
	{
		/* deletion starts at a piece boundry */
		cur = 0;
		before = p;
		old_start = p->next;
	}
	else 
	{
		/* deletion starts midway through a piece */
		midway_start = true;
		cur = p->len - off;
		old_start = p;
		before = piece_alloc(txt);
		if (!before)
			return false;
	}

	/* skip all pieces which fall into deletion range */
	while (cur < len) 
	{
		p = p->next;
		cur += p->len;
	}

	if (cur == len)
	{
		/* deletion stops at a piece boundry */
		old_end = p;
		after = p->next;
	}
	else
	{
		/* cur > len: deletion stops midway through a piece */
		midway_end = true;
		old_end = p;
		after = piece_alloc(txt);
		if (!after)
			return false;
		piece_init(after, before, p->next, p->data + p->len - (cur - len), cur - len);
	}

	if (midway_start) 
	{
		/* we finally know which piece follows our newly allocated before piece */
		piece_init(before, old_start->prev, after, old_start->data, off);
	}

	
	if (midway_start) 
	{
		new_start = before;
		if (!midway_end)
			new_end = before;
	}
	if (midway_end)
	{
		if (!midway_start)
			new_start = after;
		new_end = after;
	}

	c = change_alloc(txt, pos);
	if (!c)
		return false;

	span_init(&c->new, new_start, new_end);
	span_init(&c->old, old_start, old_end);
	span_swap(txt, &c->old, &c->new);
	return true;
}

bool text_range_valid(const Filerange *r) {
	return r->start != EPOS && r->end != EPOS && r->start <= r->end;
}

size_t text_range_size(const Filerange *r) {
	return text_range_valid(r) ? r->end - r->start : 0;
}

bool text_delete_range(Text *txt, Filerange *r) 
{
	if (!text_range_valid(r))
		return false;
	return text_delete(txt, r->start, text_range_size(r));
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

#ifdef DEBUG
void test_print_pieces_chains(Piece *start, Piece *end, size_t lim, FILE *log);
void test_print_pieces_chains(Piece *start, Piece *end, size_t lim, FILE *log)
{
	Piece *p = start;
	while (p != end->next)
	{
		fprintf(log, "@ %u %.*s ", p->len, lim > p->len ? p->len : lim, p->data);
		p = p->next;
	}
	fprintf(log, "\n");
}

void test_print_buffer(Text *txt, FILE *log)
{
	Buffer *buf;
	
	for (buf = txt->buffers; buf ; buf = buf->next)
	{
		fprintf(log, "\n***[BUFFER CONTENT]***\n");
		fprintf(log, "%.*s\n", txt->buffers->len, txt->buffers->data);
	}
}
void test_print_piece(Text *txt, FILE *log)
{
	Piece *p;
	fprintf(log, "\n***[PIECE CHAINS]***\n");
	fprintf(log, "+-------+--------+---------------------+\n");
	fprintf(log, "| start | length |       content       |\n");
	for (p = txt->begin.next; p->next; p = p->next)
	{
		fprintf(log, "+-------+--------+---------------------+\n");
		fprintf(log, "+   %c   +  %3u   +  %.*s\n", *(p->data), p->len, (size_t)19 > p->len ? p->len : 19,p->data);
	}
	fprintf(log, "+-------+--------+---------------------+\n");
}
void test_print_current_action(Text *txt, FILE *log)
{
	Change *c;
	fprintf(log, "\n***[CHANGE CHAINS]***\n");
	fprintf(log, "&*********&*********&*******&************************&\n");
	fprintf(log, "|  Type   |   Size  |	 pos  |     Pieces content     |\n");
	fprintf(log, "&*********&*********&*******&************************&\n");
	for (c = txt->current_action->change; c; c = c->next)
	{
		fprintf(log, "|   new   |  %5u  |  %3u  |  ", c->new.len, c->pos);
		test_print_pieces_chains(c->new.start, c->new.end, 10, log);
		fprintf(log, "+---------+---------+-------+------------------------+\n");
		fprintf(log, "|   old   |  %5u  |  %3u  |  ", c->old.len, c->pos);
		test_print_pieces_chains(c->old.start, c->old.end, 10, log);
		fprintf(log, "&*********&*********&*******&************************&\n");
	}
}

#endif // DEBUG