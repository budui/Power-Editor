#include "Text.h"


struct Piece {
	Text *text;             /* text to which this piece belongs */
	Piece *prev, *next;     /* pointers to the logical predecessor/successor */
	Piece *global_prev;     /* double linked list in order of allocation, */
	Piece *global_next;     /* used to free individual pieces */
	const char *data;       /* pointer into a Buffer holding the data */
	size_t len;             /* the length in number of bytes of the data */
};

struct Buffer {
	size_t size;               /* maximal capacity */
	size_t len;                /* current used length / insertion position */
	char *data;                /* actual data */
	enum {                     /* type of allocation */
		MMAP_ORIG,         /* mmap(2)-ed from an external file */
		MMAP,              /* mmap(2)-ed from a temporary file only known to this process */
		MALLOC,            /* heap allocated buffer using malloc(3) */
	} type;
	Buffer *next;              /* next junk */
};

struct Text {
	Buffer *buf;            /* original file content at the time of load operation */
	Buffer *buffers;        /* all buffers which have been allocated to hold insertion data */
	Piece *pieces;          /* all pieces which have been allocated, used to free them */
	Piece begin, end;       /* sentinel nodes which always exists but don't hold any data */
	size_t size;            /* current file content size in bytes */
	struct stat info;       /* stat as probed at load time */
};

/* used to transform a global position (byte offset starting from the beginning
* of the text) into an offset relative to a piece.
*/
typedef struct {
	Piece *piece;           /* piece holding the location */
	size_t off;             /* offset into the piece in bytes */
} Location;

static Location piece_get_intern(Text *txt, size_t pos);
static void piece_init(Piece *p, Piece *prev, Piece *next, const char *data, size_t len);
static Piece *piece_alloc(Text *txt);
static void piece_free(Piece *p);
static Buffer *buffer_alloc(Text *txt, size_t size);
static const char *buffer_append(Buffer *buf, const char *data, size_t len);
static Buffer *buffer_read(Text *txt, size_t size, int fd);
static Buffer *buffer_mmap(Text *txt, size_t size, int fd, off_t offset);
static void buffer_free(Buffer *buf);

#ifdef TEST
static void test_print_buf(const char* data, size_t len);
void test(const char *filename);
#endif // TEST



/* returns the piece holding the text at byte offset pos. if pos happens to
* be at a piece boundry i.e. the first byte of a piece then the previous piece
* to the left is returned with an offset of piece->len. this is convenient for
* modifications to the piece chain where both pieces (the returned one and the
* one following it) are needed, but unsuitable as a public interface.
*
* in particular if pos is zero, the begin sentinel piece is returned.
*/
static Location piece_get_intern(Text *txt, size_t pos) {
	size_t cur = 0;
	for (Piece *p = &txt->begin; p->next; p = p->next) {
		if (cur <= pos && pos <= cur + p->len)
			return (Location) { .piece = p, .off = pos - cur };
		cur += p->len;
	}

	return (Location) { 0 };
}

static Piece *piece_alloc(Text *txt) {
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

static void piece_init(Piece *p, Piece *prev, Piece *next, const char *data, size_t len) {
	p->prev = prev;
	p->next = next;
	p->data = data;
	p->len = len;
}

static void piece_free(Piece *p) {
	if (!p)
		return;
	if (p->global_prev)
		p->global_prev->global_next = p->global_next;
	if (p->global_next)
		p->global_next->global_prev = p->global_prev;
	if (p->text->pieces == p)
		p->text->pieces = p->global_next;
	free(p);
}

/* allocate a new buffer of MAX(size, BUFFER_SIZE) bytes */
static Buffer *buffer_alloc(Text *txt, size_t size) {
	Buffer *buf = calloc(1, sizeof(Buffer));
	if (!buf)
		return NULL;
	if (BUFFER_SIZE > size)
		size = BUFFER_SIZE;
	if (!(buf->data = malloc(size))) {
		free(buf);
		return NULL;
	}
	buf->type = MALLOC;
	buf->size = size;
	buf->next = txt->buffers;
	txt->buffers = buf;
	return buf;
}

/* append data to buffer, assumes there is enough space available */
static const char *buffer_append(Buffer *buf, const char *data, size_t len) {
	char *dest = memcpy(buf->data + buf->len, data, len);
	buf->len += len;
	return dest;
}

static Buffer *buffer_read(Text *txt, size_t size, int fd) {
	Buffer *buf = buffer_alloc(txt, size);
	if (!buf)
		return NULL;
	while (size > 0) {
		char data[4096];
		ssize_t len = read(fd, data, MIN(sizeof(data), size));
		if (len == -1) {
			txt->buffers = buf->next;
			buffer_free(buf);
			return NULL;
		}
		else if (len == 0) {
			break;
		}
		else {
			buffer_append(buf, data, len);
			size -= len;
		}
	}
	return buf;
}

static Buffer *buffer_mmap(Text *txt, size_t size, int fd, off_t offset) {
	Buffer* buf = NULL;
	return buf;
}

static void buffer_free(Buffer *buf) {
	if (!buf)
		return;
	if (buf->type == MALLOC)
		free(buf->data);
	//else if ((buf->type == MMAP_ORIG || buf->type == MMAP) && buf->data)
		//munmap(buf->data, buf->size);
	free(buf);
}



/* When inserting new data there are 2 cases to consider.
*
*  - in the first the insertion point falls into the middle of an exisiting
*    piece which is replaced by three new pieces:
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
*  - the second case deals with an insertion point at a piece boundry:
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
//bool text_insert(Text *txt, size_t pos, const char *data, size_t len) {
//	if (len == 0)
//		return true;
//	if (pos > txt->size)
//		return false;
//
//	Location loc = piece_get_intern(txt, pos);
//	Piece *p = loc.piece;
//	if (!p)
//		return false;
//	size_t off = loc.off;
//	if (cache_insert(txt, p, off, data, len))
//		return true;
//
//	if (!(data = buffer_store(txt, data, len)))
//		return false;
//
//	Piece *new = NULL;
//
//	if (off == p->len) {
//		/* insert between two existing pieces, hence there is nothing to
//		* remove, just add a new piece holding the extra text */
//		if (!(new = piece_alloc(txt)))
//			return false;
//		piece_init(new, p, p->next, data, len);
//	}
//	else {
//		/* insert into middle of an existing piece, therfore split the old
//		* piece. that is we have 3 new pieces one containing the content
//		* before the insertion point then one holding the newly inserted
//		* text and one holding the content after the insertion point.
//		*/
//		Piece *before = piece_alloc(txt);
//		new = piece_alloc(txt);
//		Piece *after = piece_alloc(txt);
//		if (!before || !new || !after)
//			return false;
//		piece_init(before, p->prev, new, p->data, off);
//		piece_init(new, before, after, data, len);
//		piece_init(after, new, p->next, p->data + off, p->len - off);
//	}
//
//	return true;
//}
//
///* A delete operation can either start/stop midway through a piece or at
//* a boundry. In the former case a new piece is created to represent the
//* remaining text before/after the modification point.
//*
//*      /-+ --> +---------+ --> +-----+ --> +-----+ --> +-\
//*      | |     | existing|     |demo |     |text |     | |
//*      \-+ <-- +---------+ <-- +-----+ <-- +-----+ <-- +-/
//*                   ^                         ^
//*                   |------ delete range -----|
//*
//*      /-+ --> +----+ --> +--+ --> +-\
//*      | |     | exi|     |t |     | |
//*      \-+ <-- +----+ <-- +--+ <-- +-/
//*/
//bool text_delete(Text *txt, size_t pos, size_t len) {
//	if (len == 0)
//		return true;
//	if (pos + len > txt->size)
//		return false;
//
//
//	Location loc = piece_get_intern(txt, pos);
//	Piece *p = loc.piece;
//	if (!p)
//		return false;
//	size_t off = loc.off;
//	if (cache_delete(txt, p, off, len))
//		return true;
//
//
//	bool midway_start = false, midway_end = false; /* split pieces? */
//	Piece *before, *after; /* unmodified pieces before/after deletion point */
//	Piece *start, *end;    /* span which is removed */
//	size_t cur;            /* how much has already been deleted */
//
//	if (off == p->len) {
//		/* deletion starts at a piece boundry */
//		cur = 0;
//		before = p;
//		start = p->next;
//	}
//	else {
//		/* deletion starts midway through a piece */
//		midway_start = true;
//		cur = p->len - off;
//		start = p;
//		before = piece_alloc(txt);
//		if (!before)
//			return false;
//	}
//
//	/* skip all pieces which fall into deletion range */
//	while (cur < len) {
//		p = p->next;
//		cur += p->len;
//	}
//
//	if (cur == len) {
//		/* deletion stops at a piece boundry */
//		end = p;
//		after = p->next;
//	}
//	else {
//		/* cur > len: deletion stops midway through a piece */
//		midway_end = true;
//		end = p;
//		after = piece_alloc(txt);
//		if (!after)
//			return false;
//		piece_init(after, before, p->next, p->data + p->len - (cur - len), cur - len);
//	}
//
//	if (midway_start) {
//		/* we finally know which piece follows our newly allocated before piece */
//		piece_init(before, start->prev, after, start->data, off);
//	}
//
//	Piece *new_start = NULL, *new_end = NULL;
//	if (midway_start) {
//		new_start = before;
//		if (!midway_end)
//			new_end = before;
//	}
//	if (midway_end) {
//		if (!midway_start)
//			new_start = after;
//		new_end = after;
//	}
//
//	span_init(&c->new, new_start, new_end);
//	span_init(&c->old, start, end);
//	span_swap(txt, &c->old, &c->new);
//	return true;
//}


Text *text_load(const char *filename) {
	Text *txt = calloc(1, sizeof(Text));
	int fd = -1;
	if (!txt)
		return NULL;
	
	piece_init(&txt->begin, NULL, &txt->end, NULL, 0);
	piece_init(&txt->end, &txt->begin, NULL, NULL, 0);

	if (filename) {
		if ((fd = open(filename, O_RDONLY | O_BINARY)) == -1)
			goto out;
		if (fstat(fd, &txt->info) == -1)
			goto out;
		if (!S_ISREG(txt->info.st_mode)) {
			errno = S_ISDIR(txt->info.st_mode) ? EISDIR : ENOTSUP;
			goto out;
		}
		// XXX: use lseek(fd, 0, SEEK_END); instead?
		size_t size = txt->info.st_size;
		if (size < BUFFER_MMAP_SIZE)
			txt->buf = buffer_read(txt, size, fd);
		else
			txt->buf = buffer_mmap(txt, size, fd, 0);
		if (!txt->buf)
			goto out;
		Piece *p = piece_alloc(txt);
		if (!p)
			goto out;
		piece_init(&txt->begin, NULL, p, NULL, 0);
		piece_init(p, &txt->begin, &txt->end, txt->buf->data, txt->buf->len);
		piece_init(&txt->end, p, NULL, NULL, 0);
		txt->size = txt->buf->len;
	}


	if (fd != -1)
		close(fd);
	return txt;
out:
	if (fd != -1)
		close(fd);
	text_free(txt);
	return NULL;
}

void text_free(Text *txt) {
	if (!txt)
		return;

	for (Piece *next, *p = txt->pieces; p; p = next) {
		next = p->global_next;
		piece_free(p);
	}

	for (Buffer *next, *buf = txt->buffers; buf; buf = next) {
		next = buf->next;
		buffer_free(buf);
	}

	free(txt);
}


#ifdef TEST
static void test_print_buf(const char * data, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++)
	{
		putchar(*(data + i));
	}
}

void test(const char *filename)
{
	Text *txt = text_load(filename);
	printf("Size: %d\n", txt->size);
	printf("------------\nBUFF content:\n");
	test_print_buf(txt->buf->data, txt->size);

	text_free(txt);

}
#endif // TEST
