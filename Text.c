#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include "text.h"
#include "config.h"
/* Allocate buffers holding the actual file content in junks of size: */
#define BUFFER_SIZE ((size_t) 1 << 13)

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
};

struct TextSave {                  /* used to hold context between text_save_{begin,commit} calls */
	Text *txt;                 /* text to operate on */
	char *filename;            /* filename to save to as given to text_save_begin */
	char *tmpname;             /* temporary name */
	int fd;                    /* file descriptor to write data to using text_save_write */
};

static int write_all(int fd, const char *buf, size_t count);
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
static bool piece_get_two_intern(Text *txt, size_t start_pos, size_t end_pos, Location *start_loc, Location *end_loc);
static void piece_free(Piece *p);
/* Acthon layer */
static void action_free(Action *a);
static Action *action_alloc(Text *txt);
/* Change layer */
static Change *change_alloc(Text *txt, size_t pos);
static void change_free(Change *c);
/* iterater layer */
static bool iterator_init(Iterator *it, size_t pos, Piece *p, size_t off);
static void span_swap(Text *txt, Span *old_span, Span *new_span);


struct ClipBorad {
	size_t len;
	char *buf;
};


ClipBorad *clipborad_init(void)
{
	ClipBorad *cli = (ClipBorad *)malloc(sizeof(ClipBorad));
	if (!cli)
		return NULL;
	cli->buf = (char *)malloc(CONFIG_CLIPBORAD_SIZE);
	if (!cli->buf) {
		free(cli);
		return NULL;
	}
	cli->len = 0;
	return cli;
}

void clipborad_close(ClipBorad *cli)
{
	if (!cli)
		return;
	free(cli->buf);
	free(cli);
}
bool text_copy(ClipBorad *cli, Text *txt, Filerange *r)
{
	size_t size = text_range_size(r), rem = size;
	Iterator it = iterator_get(txt, r->start);
	if (size > CONFIG_CLIPBORAD_SIZE)
		return false;
	for (; rem > 0 && iterator_valid(&it); iterator_next(&it))
	{
		size_t prem = it.end - it.text;
		if (prem > rem)
			prem = rem;
		memcpy(cli->buf + size - rem, it.text, prem);
		rem -= prem;
	}
	cli->len = size;
	return true;
}

bool text_paste(ClipBorad *cli, Text *txt, size_t pos)
{
	if (!cli->len)
		return false;
	if (!text_insert(txt, pos, cli->buf, cli->len))
		return false;
	cli->len = 0;
	return true;
}

bool text_cut(ClipBorad *cli, Text *txt, Filerange *r)
{
	if (!text_copy(cli, txt, r))
		return false;
	if (!text_delete_range(txt, r))
		return false;
	return true;
}

/* write len chars from buf into fd file. */
static int write_all(int fd, const char *buf, size_t len)
{
	size_t rem = len;
	while (rem > 0)
	{
		int written = write(fd, buf, rem);
		if (written < 0)
			return -1;
		else if (written == 0)
			break;
		rem -= written;
		buf += written;
	}
	return len - rem;
}


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
	// keep buffer size >= BUFFER_SIZE
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
	// if buf == NULL or buffer can't not contains data, alloc a new buffer.
	if (!buf || !buffer_capacity(buf, len))
	{
		if (!(buf = buffer_alloc(txt, len)))
			return NULL;
	}

	return buffer_append(buf, data, len);
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
		int len = read(fd, data, MIN(sizeof(data), size));
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

/* insert data into buffer at an arbitrary position, this should only
* be used with data of the most recently created piece. because the most
* recently created piece's buffer may not contains so many data.
*/
static bool buffer_insert(Buffer *buf, size_t pos, const char *data, size_t len)
{
	char *insert; // insert point.
	if (pos > buf->len || !buffer_capacity(buf, len))
		return false;
	if (buf->len == pos)
		return (bool)buffer_append(buf, data, len);
	insert = buf->data + pos;
	memmove(insert + len, insert, buf->len - pos);
	memcpy(insert, data, len);
	buf->len += len;
	return true;
}

/* delete data from a buffer at an arbitrary position, this should only be used with
* data of the most recently created piece. */
static bool buffer_delete(Buffer *buf, size_t pos, size_t len)
{
	char *delete; //delete point.

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
	else if ((buf->type == FARMEM) && buf->data)
		//farfree(buf->data);
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
	// set txt->piece with new piece.
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
/* similiar to piece_get_intern but usable as a public API. returns the piece
* holding the text at byte offset pos. never returns a sentinel piece.
* it pos is the end of file (== text_size()) and the file is not empty then
* the last piece holding data is returned.
*/
static Location piece_get_extern(Text *txt, size_t pos)
{
	size_t cur = 0;
	Location loc = { NULL, 0 };
	Piece *p;

	if (pos > 0 && pos == txt->size)
	{
		for (p = txt->begin.next; p->next->next; p = p->next)
			;
		loc.off = p->len;
		loc.piece = p;
		return loc;
	}

	for (p = txt->begin.next; p->next; p = p->next) {
		if (cur <= pos && pos < cur + p->len) {
			loc.off = pos - cur;
			loc.piece = p;
			return loc;
		}
		cur += p->len;
	}

	return loc;
}
/* Get start and end location releted to start_pos and end_pos.
* Note:
*   1) Since piece_get_intern is used in this function,
*	 the notes of piece_get_intern must to be read.
*/
static bool piece_get_two_intern(Text *txt, size_t start_pos, size_t end_pos, Location *start_loc, Location *end_loc)
{
	size_t cur;
	Piece *p;
	Location loc;
	if ((start_pos > end_pos) || (end_pos > txt->size))
		return false;

	loc = piece_get_intern(txt, start_pos);
	start_loc->off = loc.off;
	start_loc->piece = loc.piece;

	cur = start_pos - loc.off; // make sure cur correct.
	for (p = loc.piece; p->next; p = p->next)
	{
		if (cur <= end_pos && end_pos <= cur + p->len)
		{
			end_loc->off = end_pos - cur;
			end_loc->piece = p;
			return true;
		}
		cur += p->len;
	}
	return false;
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

/* undo all change linked with action. */
static size_t action_undo(Text *txt, Action *a)
{
	size_t pos = EPOS;
	Change *c = a->change;
	for (; c; c = c->next)
	{
		span_swap(txt, &c->new, &c->old);
		pos = c->pos;
	}
	return pos;
}


static size_t action_redo(Text *txt, Action *a)
{
	size_t pos = EPOS;
	Change *c = a->change;
	while (c->next)
		c = c->next;
	for (; c; c = c->prev)
	{
		span_swap(txt, &c->old, &c->new);
		pos = c->pos;
		if (c->new.len > c->old.len)
			pos += c->new.len - c->old.len;
	}
	return pos;
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
*	1. adjusts the document size accordingly.
*   2. make sure new_span->start->prev and new_span->end->next point
*      at correct pieces.
*/
static void span_swap(Text *txt, Span *old_span, Span *new_span)
{
	if (old_span->len == 0 && new_span->len == 0) {
		return;
	}
	else if (old_span->len == 0)
	{
		/* insert new span */
		new_span->start->prev->next = new_span->start;
		new_span->end->next->prev = new_span->end;
	}
	else if (new_span->len == 0)
	{
		/* delete old span */
		old_span->start->prev->next = old_span->end->next;
		old_span->end->next->prev = old_span->start->prev;
	}
	else
	{
		/* replace old with new */
		old_span->start->prev->next = new_span->start;
		old_span->end->next->prev = new_span->end;
	}
	txt->size -= old_span->len;
	txt->size += new_span->len;
}

Iterator iterator_get(Text *txt, size_t pos)
{
	Iterator it;
	Location loc = piece_get_extern(txt, pos);
	iterator_init(&it, pos, loc.piece, loc.off);
	return it;
}

static bool iterator_init(Iterator *it, size_t pos, Piece *p, size_t off)
{
	it->pos = pos;
	it->piece = p;
	it->start = p ? p->data : NULL;
	it->end = p ? p->data + p->len : NULL;
	it->text = p ? p->data + off : NULL;
	return iterator_valid(it);
}
/* it->text will be equal to it->start. */
bool iterator_next(Iterator *it)
{
	return iterator_init(it, it->pos, it->piece ? it->piece->next : NULL, 0);
}

bool iterator_prev(Iterator *it)
{
	return iterator_init(it, it->pos, it->piece ? it->piece->prev : NULL, 0);
}

bool iterator_valid(const Iterator *it)
{
	/* filter out sentinel nodes */
	return it->piece && it->piece->text;
}

bool iterator_byte_get(Iterator *it, char *b)
{
	if (iterator_valid(it)) {
		if (it->start <= it->text && it->text < it->end) {
			*b = *it->text;
			return true;
		}
		else if (it->pos == it->piece->text->size) { /* EOF */
			*b = '\0';
			return true;
		}
	}
	return false;
}

bool iterator_byte_next(Iterator *it, char *b)
{
	if (!iterator_valid(it))
		return false;
	it->text++;
	/* special case for advancement to EOF */
	if (it->text == it->end && !it->piece->next->text)
	{
		it->pos++;
		if (b)
			*b = '\0';
		return true;
	}
	/* Case for iterator must be iterated without iterate it->pos. */
	while (it->text >= it->end)
	{
		if (!iterator_next(it))
			return false;
		it->text = it->start;
	}
	it->pos++;
	if (b)
		*b = *it->text;
	return true;
}

bool iterator_byte_prev(Iterator *it, char *b)
{
	if (!iterator_valid(it))
		return false;
	// iterate iterator previously until Iterator->text == Iterator->start
	while (it->text == it->start)
	{
		if (!iterator_prev(it))
			return false;
		it->text = it->end;
	}
	--it->text;
	--it->pos;
	if (b)
		*b = *it->text;
	return true;
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
		if (size < CONFIG_FILE_TRUNCATE_SIZE)
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

bool text_save(Text *txt, const char *filename)
{
	int fd = -1;
	if (filename) {
		if ((fd = open(filename, O_CREAT | O_RDWR | O_TEXT, S_IREAD | S_IWRITE)) == -1)
		{
			perror("open failed.");
			return false;
		}

		if (text_write(txt, fd) == -1)
		{
			perror("write failed.");
			return false;
		}

	}
	if (fd != -1)
		close(fd);
	return true;
}

int text_write(Text *txt, int fd)
{
	Filerange r;
	r.start = 0;
	r.end = text_size(txt);
	return text_write_range(txt, &r, fd);
}

int text_write_range(Text *txt, Filerange *range, int fd)
{
	size_t size = text_range_size(range), rem = size;
	Iterator it = iterator_get(txt, range->start);
	for (; rem > 0 && iterator_valid(&it); iterator_next(&it))
	{
		size_t prem = it.end - it.text;
		int written;
		if (prem > rem)
			prem = rem;
		written = write_all(fd, it.text, prem);
		if (written == -1)
			return -1;
		rem -= written;
		if ((size_t)written != prem)
			break;
	}
	return size - rem;
}

size_t text_undo(Text *txt) {
	size_t pos = EPOS;
	Action *a;
	/* taking a snapshot makes sure that txt->current_action is reset */
	text_snapshot(txt);
	a = txt->history->prev;
	if (!a)
		return pos;
	pos = action_undo(txt, txt->history);
	txt->history = a;
	lineno_cache_invalidate(&txt->lines);
	return pos;
}

size_t text_redo(Text *txt) {
	size_t pos = EPOS;
	Action *a;
	/* taking a snapshot makes sure that txt->current_action is reset */
	text_snapshot(txt);
	a = txt->history->next;
	if (!a)
		return pos;
	pos = action_redo(txt, a);
	txt->history = a;
	lineno_cache_invalidate(&txt->lines);
	return pos;
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
	Piece *p, *new = NULL;
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
	Location start_loc, end_loc;
	Change *c;
	Piece *old_start, *old_end; /* span which is removed */
	Piece *new_start = NULL, *new_end = NULL;
	if (len == 0)
		return true;
	if (pos + len > txt->size)
		return false;
	if (pos < txt->lines.pos)
		lineno_cache_invalidate(&txt->lines);


	if (!piece_get_two_intern(txt, pos, pos + len, &start_loc, &end_loc))
		return false;
	if (cache_delete(txt, start_loc.piece, start_loc.off, len))
		return true;

	if ((start_loc.off == start_loc.piece->len) && (end_loc.off == end_loc.piece->len))
	{
		/* deletion starts at a piece boundry and ends at a piece boundry. */
		old_start = start_loc.piece->next;
		old_end = end_loc.piece;
	}
	else if ((start_loc.off != start_loc.piece->len) && (end_loc.off == end_loc.piece->len))
	{
		/* deletion starts midway through a piece and ends at a piece boundry. */
		old_start = start_loc.piece;
		old_end = end_loc.piece;
		new_start = piece_alloc(txt);
		if (!new_start)
			return false;
		piece_init(new_start, old_start->prev, old_end->next, old_start->data, start_loc.off);
		new_end = new_start;
	}
	else if ((start_loc.off == start_loc.piece->len) && (end_loc.off != end_loc.piece->len))
	{
		/* deletion starts at a piece boundry and ends midway through a piece. */
		old_start = start_loc.piece->next;
		old_end = end_loc.piece;
		new_end = piece_alloc(txt);
		if (!new_end)
			return false;
		piece_init(new_end, old_start->prev, old_end->next, old_end->data + end_loc.off, old_end->len - end_loc.off);
		new_start = new_end;
	}
	else
	{
		old_start = start_loc.piece;
		old_end = end_loc.piece;
		new_end = piece_alloc(txt);
		new_start = piece_alloc(txt);
		if ((!new_end) || (!new_start))
			return false;
		piece_init(new_end, new_start, old_end->next, old_end->data + end_loc.off, old_end->len - end_loc.off);
		piece_init(new_start, old_start->prev, new_end, old_start->data, start_loc.off);
	}


	c = change_alloc(txt, pos);
	if (!c)
		return false;

	span_init(&c->new, new_start, new_end);
	span_init(&c->old, old_start, old_end);
	span_swap(txt, &c->old, &c->new);
	return true;
}

bool text_range_valid(const Filerange *r)
{
	return r->start != EPOS && r->end != EPOS && r->start <= r->end;
}

size_t text_range_size(const Filerange *r)
{
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

size_t text_size(Text *txt)
{
	return txt->size;
}

size_t text_find_next(Text *txt, size_t pos, const char *s) 
{
	char c;
	size_t len = strlen(s), matched = 0;
	Iterator it = iterator_get(txt, pos), sit;
	if (!s)
		return pos;
	while (matched < len && iterator_byte_get(&it, &c)) 
	{
		if (c == s[matched]) 
		{
			if (matched == 0)
				sit = it;
			matched++;
		}
		else if (matched > 0) 
		{
			it = sit;
			matched = 0;
		}
		iterator_byte_next(&it, NULL);
	}
	return matched == len ? it.pos - len : pos;
}

size_t text_find_prev(Text *txt, size_t pos, const char *s) 
{
	size_t len = strlen(s), matched = len - 1;
	Iterator it = iterator_get(txt, pos), sit;
	char c;
	if (!s)
		return pos;
	if (len == 0)
		return pos;
	while(iterator_byte_prev(&it, &c))
	{
		if (c == s[matched]) 
		{
			if (matched == 0)
				return it.pos;
			if (matched == len - 1)
				sit = it;
			matched--;
		}
		else if (matched < len - 1) 
		{
			it = sit;
			matched = len - 1;
		}
	}
	return pos;
}

#ifdef DEBUG

/* if set debug mode, open log file to recode log. */

static bool test_print_span(Piece *start, Piece *end, size_t lim);
static bool test_print_span(Piece *start, Piece *end, size_t lim)
{
	Piece *p = start;
	if (!start || !end)
	{
		fprintf(stderr, "**[NULL]**\n");
		return false;
	}
	while (p != end->next)
	{
		fprintf(stderr, "@ %u %.*s ", p->len, (int)(lim > p->len ? p->len : lim), p->data);
		p = p->next;
	}
	fprintf(stderr, "\n");
	return true;
}

void test_print_buffer(Text *txt)
{
	Buffer *buf;

	for (buf = txt->buffers; buf; buf = buf->next)
	{
		fprintf(stderr, "\n***[BUFFER CONTENT]***\n");
		fprintf(stderr, "%.*s\n", (int)txt->buffers->len, txt->buffers->data);
	}
}
void test_print_piece(Text *txt)
{
	Piece *p;
	fprintf(stderr, "\n***[PIECE CHAINS]***\n");
	fprintf(stderr, "+-------+--------+---------------------+\n");
	fprintf(stderr, "| start | length |       content       |\n");
	for (p = txt->begin.next; p->next; p = p->next)
	{
		fprintf(stderr, "+-------+--------+---------------------+\n");
		fprintf(stderr, "+   %c   +  %3u   +  %.*s\n", *(p->data), p->len, (int)(p->len), p->data);
	}
	fprintf(stderr, "+-------+--------+---------------------+\n");
}
void test_print_current_action(Text *txt)
{
	Change *c;
	fprintf(stderr, "\n***[CHANGE CHAINS]***\n");
	fprintf(stderr, "&*********&*********&*******&************************&\n");
	fprintf(stderr, "|  Type   |   Size  |	 pos  |     Pieces content     |\n");
	fprintf(stderr, "&*********&*********&*******&************************&\n");
	if (!txt->current_action)
		return;
	for (c = txt->current_action->change; c; c = c->next)
	{
		fprintf(stderr, "|   new   |  %5u  |  %3u  |  ", c->new.len, c->pos);
		test_print_span(c->new.start, c->new.end, 10);
		fprintf(stderr, "+---------+---------+-------+------------------------+\n");
		fprintf(stderr, "|   old   |  %5u  |  %3u  |  ", c->old.len, c->pos);
		test_print_span(c->old.start, c->old.end, 10);
		fprintf(stderr, "&*********&*********&*******&************************&\n");
	}
}

#endif // DEBUG