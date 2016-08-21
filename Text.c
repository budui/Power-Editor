#include "Text.h"

#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // 用来检测内存泄漏

typedef struct Buffer Buffer;
/* Buffer包含文本内容, 包括只读的缓存区和用malloc分配的堆*/
struct Buffer {
	size_t size;               /* BUFF大小 */
	size_t len;                /* 当前位置 / 插入位置 */
	char *data;                /* 实际数据 */
	enum {                     /* 内存分配类型 */
		MMAP_ORIG,         /* 已经存在的文件的文件映射 */
		MMAP,              /* 只对当前进程可见的暂时文件 */
		MALLOC,            /* 用malloc分配的堆 */
	} type;
	Buffer *next;              /* 下一区块 */
};

/* 一个Piece包括一个索引和数据大小(但不存储数据本身)。所有活动的Piece
* 形成一个包括了整篇文档的链表。开始时只有一个包括了整篇文档的Piece。
* 当插入/删除时，会新形成Piece来代表发生的变化。
* 一般Piece永远不会被释放，用来支持undo/redo操作。
*/
struct Piece {
	Text *text;             /* 这个Piece属于的Text */
	Piece *prev, *next;     /* 指向文本中此Piece前后的Piece */
	Piece *global_prev;     /* 以分配顺序连接的双链表, */
	Piece *global_next;     /* 用来Free个别pieces */
	const char *data;       /* 指向存储着数据的BUFF的指针 */
	size_t len;             /* 数据字节大小 */
};

/* 用来转换全局索引(从文本开头数起的字节数) 到与Piece关联的索引。
*/
typedef struct {
	Piece *piece;           /* 包含指定位置的Piece */
	size_t off;             /* 在此Piece中的索引 */
} Location;

/* 一个Span包括确定范围的Piece。对文档的改变总是通过用新Span取代旧Span实现。
*/
typedef struct {
	Piece *start, *end;     /* Span的开始和结束 */
	size_t len;             /* 组成这个Span的各个Piece长度之和 */
} Span;

/* 一个Change结构体保持着redo/undo删除/插入操作的所有需要的信息
*/
typedef struct Change Change;
struct Change {
	Span old;               /* 被修改/被换出的所有Piece */
	Span new;               /* 新生成/交换的所有Piece */
	size_t pos;             /* 改变发生的位置 */
	Change *next;           /* 下一个同属一个动作的Change */
	Change *prev;           /* 上一个同属一个动作的Change */
};

/* Action是一个change的列表，使得编辑器可以
*  undo/redo所有在最近的一次做快照前的change
*  所有的Action被保存在一个有向图结构中。由于存在
*  undo/redo，Action不是一个按时间顺序的列表，而是一颗树。
*/
typedef struct Action Action;
struct Action {
	Change *change;         /* 当前change */
	Action *next;           /* 在undo树上的子代 */
	Action *prev;           /* 在undo树上的父辈 */
	Action *earlier;        /* 按时间顺序的前一个Action */
	Action *later;          /* 按时间顺序的后一个Action */
	time_t time;            /* 这个Action的第一个change的发生时间*/
	size_t seq;             /* 唯一的，严格递增的标识符 */
};

/* 包含所有给定文件信息的最主要的结构体 */
struct Text {
	Buffer *buf;            /* 在加载文件内容时的不可变的缓存区 */
	Buffer *buffers;        /* 被分配来容纳插入数据的缓存区 */
	Piece *pieces;          /* 所有被申请来的Piece, 被用来free这些Piece */
	Piece *cache;           /* 最近被改变的Piece */
	Piece begin, end;       /* 总是存在但不包括任何信息的哨兵节点 */
	Action *history;        /* undo树 */
	Action *current_action; /* 包括做快照前所有change的Action */
	Action *last_action;    /* 最近被加入到undo树上的Action */
	Action *saved_action;   /* 在保存时的最后一个Action */
	size_t size;            /* 当前文件大小 */
	struct stat info;       /* stat as probed at load time */
};


/* buffer 类 */
static Buffer *buffer_alloc(Text *txt, size_t size);
static const char *buffer_append(Buffer *buf, const char *data, size_t len);
static Buffer *buffer_read(Text *txt, size_t size, int fd);
static Buffer *buffer_mmap(Text *txt, size_t size, int fd, off_t offset);
static const char *buffer_store(Text *txt, const char *data, size_t len);
static bool buffer_capacity(Buffer *buf, size_t len);
static void buffer_free(Buffer *buf);
/* piece 类 */
static Location piece_get_intern(Text *txt, size_t pos);
static void piece_init(Piece *p, Piece *prev, Piece *next, const char *data, size_t len);
static Piece *piece_alloc(Text *txt);
static void piece_free(Piece *p);
/* span 类 */
static void span_init(Span *span, Piece *start, Piece *end);
static void span_swap(Text *txt, Span *old, Span *new);
/* change 类 */
static Change *change_alloc(Text *txt, size_t pos);
static void change_free(Change *c);
/* action 类 */
static Action *action_alloc(Text *txt);
static void action_free(Action *a);

#ifdef DEBUG
#include "Draw_List.h"
static void test_print_piece(Piece * p);
//static void test_print_change(Piece * p);

bool piece_iterate(Iter *iter, void* node);
//传给Draw_List的函数
bool piece_iterate(Iter *iter, void* node)
{
	iter->prev = iter->current;
	iter->current = iter->next;
	iter->next = ((Piece*)node)->next;
	iter->data = ((Piece*)node)->data;
	iter->len = ((Piece*)node)->len;
}
static void test_print_piece(Piece * p)
{
	Iter piece_iter;
	Iter_init(&piece_iter, p, p->prev, p->next, p->data, p->len,piece_iterate);
	draw_node(&piece_iter);
}


//static void test_print_change(Change * p)
//{
//	Iter piece_iter;
//	Iter_init(&piece_iter, p, p->prev, p->next, p->data, p->len, piece_iterate);
//	draw_node(&piece_iter);
//}

//外部可见函数
void test_print_buf(const char * data, size_t len)
{
	size_t i;
	printf("\n--------BUFF------------------\n");
	for (i = 0; i < len; i++)
	{
		putchar(*(data + i));
	}
	printf("\n--------CONTENT-------------------\n");
}

void test_show_info(Text *txt)
{
	printf("Size: %ud\n", txt->size);
	test_print_buf(txt->buf->data, txt->size);
	test_print_piece(&(txt->begin));
}


#endif

/* 申请缓存区内存，大小为MAX(size, BUFFER_SIZE) bytes */
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

/* 将给定数据存储至指定缓存区，如果必要的话再申请一个缓存区区块。
*  返回存储位置地址，如果申请内存失败返回NULL。 */
static const char *buffer_store(Text *txt, const char *data, size_t len) {
	Buffer *buf = txt->buffers;
	//(buffers为空 or 现有buffers不能容纳) and 不能分配新内存
	if ((!buf || !buffer_capacity(buf, len)) && !(buf = buffer_alloc(txt, len)))
		return NULL;
	return buffer_append(buf, data, len);
}

/* 假定空间足够，向缓存区增加数据 */
static const char *buffer_append(Buffer *buf, const char *data, size_t len) {
	char *dest = memcpy(buf->data + buf->len, data, len);
	buf->len += len;
	return dest;
}

/* 检查是否有足够空间存储len个字符 */
static bool buffer_capacity(Buffer *buf, size_t len) {
	return buf->size - buf->len >= len;
}

static Buffer *buffer_read(Text *txt, size_t size, int fd) {
	Buffer *buf = buffer_alloc(txt, size);
	if (!buf)
		return NULL;
	while (size > 0) {
		char data[READ_BUFF_SIZE];
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

/*用文本映射方式读取文本内容，暂时还不会写*/
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

/* 返回第pos个字符所在的Piece，如果pos恰好处于Piece和Piece之间，
* 则返回前一个Piece并使得，Location.off为该Piece大小。
*
* 如果pos为0，则返回txt.begin
*/
static Location piece_get_intern(Text *txt, size_t pos) {
	size_t cur = 0;
	Location loc;
	for (Piece *p = &txt->begin; p->next; p = p->next) {
		if (cur <= pos && pos <= cur + p->len) {
			loc.piece = p;
			loc.off = pos - cur;
			return loc;
		}
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

/* 用新span取代旧span
*
*  - 如果旧span是空的，并且不移去任何东西，直接插入新span
*  - 如果新span是空的，并且不插入任何东西，直接移除旧span
*
* 顺便更改txt大小信息
*/
static void span_swap(Text *txt, Span *old, Span *new) {
	if (old->len == 0 && new->len == 0) {
		return;
	}
	else if (old->len == 0) {
		/* 插入新 span */
		new->start->prev->next = new->start;
		new->end->next->prev = new->end;
	}
	else if (new->len == 0) {
		/* 移除旧 span */
		old->start->prev->next = old->end->next;
		old->end->next->prev = old->start->prev;
	}
	else {
		/* 用新span替换掉旧span */
		old->start->prev->next = new->start;
		old->end->next->prev = new->end;
	}
	txt->size -= old->len;
	txt->size += new->len;
}

/* 初始化一个Span并计算其长度 */
static void span_init(Span *span, Piece *start, Piece *end) {
	size_t len = 0;
	span->start = start;
	span->end = end;
	for (Piece *p = start; p; p = p->next) {
		len += p->len;
		if (p == end)
			break;
	}
	span->len = len;
}

static void change_free(Change *c) {
	if (!c)
		return;
	/* 只free change.new的Piece,change.old依旧使用 */
	piece_free(c->new.start);
	if (c->new.start != c->new.end)
		piece_free(c->new.end);
	free(c);
}
/* 申请分配一个新change，将其与当前Action关联，
* 如果当前不存在Acition，就新申请分配一个Action */
static Change *change_alloc(Text *txt, size_t pos) {
	Action *a = txt->current_action;
	if (!a) {
		a = action_alloc(txt);
		if (!a)
			return NULL;
	}
	Change *c = calloc(1, sizeof(Change));
	if (!c)
		return NULL;
	c->pos = pos;
	c->next = a->change;
	if (a->change)
		a->change->prev = c;
	a->change = c;
	return c;
}

static void action_free(Action *a) {
	if (!a)
		return;
	for (Change *next, *c = a->change; c; c = next) {
		next = c->next;
		change_free(c);
	}
	free(a);
}

/* 申请分配一个新Action, 将其指针指向先前的Action,
* 并将其设置为 txt->history. 所有先前的Action将和这个Action关联起来 */
static Action *action_alloc(Text *txt) {
	Action *new = calloc(1, sizeof(Action));
	if (!new)
		return NULL;
	new->time = time(NULL);
	txt->current_action = new;

	/* 设置序列数seq */
	if (!txt->last_action)
		new->seq = 0;
	else
		new->seq = txt->last_action->seq + 1;

	/* 设置 earlier, later 指针 */
	if (txt->last_action)
		txt->last_action->later = new;
	new->earlier = txt->last_action;

	if (!txt->history) {
		txt->history = new;
		return new;
	}

	/* 设置 prev, next 指针 */
	new->prev = txt->history;
	txt->history->next = new;
	txt->history = new;
	return new;
}


/* 插入数据时有两种情况
*
*  - 1. 插入点在一个已存在的Piece的中间，那么这个Piece将会被三个新Piece取代
*
*      /-+ --> +---------------+ --> +-\
*      | |     | existing text |     | |
*      \-+ <-- +---------------+ <-- +-/
*                         ^
*                         "demo " 插入点
*
*      /-+ --> +---------+ --> +-----+ --> +-----+ --> +-\
*      | |     | existing|     |demo |     |text |     | |
*      \-+ <-- +---------+ <-- +-----+ <-- +-----+ <-- +-/
*
*  - 2. 插入点正好在Piece与Piece之间，那么只需要更改前后Piece指针。
*
*      /-+ --> +---------------+ --> +-\
*      | |     | existing text |     | |
*      \-+ <-- +---------------+ <-- +-/
*            ^
*            "short" 插入点
*
*      /-+ --> +-----+ --> +---------------+ --> +-\
*      | |     |short|     | existing text |     | |
*      \-+ <-- +-----+ <-- +---------------+ <-- +-/
*/
bool text_insert(Text *txt, size_t pos, const char *data, size_t len) {
	if (len == 0)
		return true;
	if (pos > txt->size)
		return false;
	//if (pos < txt->lines.pos)
	//	lineno_cache_invalidate(&txt->lines);

	Location loc = piece_get_intern(txt, pos);
	Piece *p = loc.piece;
	if (!p)
		return false;
	size_t off = loc.off;
	/*if (cache_insert(txt, p, off, data, len))
		return true;*/

	Change *c = change_alloc(txt, pos);
	if (!c)
		return false;

	if (!(data = buffer_store(txt, data, len)))
		return false;

	Piece *new = NULL;

	if (off == p->len) {
		/* 在两个Piece中间插入数据,假设没有字符需要移除，
		* 只需要在中间增加一个包括插入数据的Piece */
		if (!(new = piece_alloc(txt)))
			return false;
		piece_init(new, p, p->next, data, len);
		span_init(&c->new, new, new);
		span_init(&c->old, NULL, NULL);
	}
	else {
		/* 插入点在一个已存在的Piece的中间,因此将旧Piece分为三个新Piece。
		* 这三个Piece分别为包括插入点前的Piece内容的before,包括插入点后的Piece内容的after
		* 包括插入数据的new
		*/
		Piece *before = piece_alloc(txt);
		new = piece_alloc(txt);
		Piece *after = piece_alloc(txt);
		if (!before || !new || !after)
			return false;
		piece_init(before, p->prev, new, p->data, off);
		piece_init(new, before, after, data, len);
		piece_init(after, new, p->next, p->data + off, p->len - off);

		span_init(&c->new, before, after);
		span_init(&c->old, p, p);
	}

	//cache_piece(txt, new);
	span_swap(txt, &c->old, &c->new);
	return true;
}

/* 删除操作可能开始/结束于一个Piece的中间或者两个Piece之间，
* 对于前一种情况可以直接在开始/结束点附近新申请一个Piece
* 来表示删除后的文字。
*
*      /-+ --> +---------+ --> +-----+ --> +-----+ --> +-\
*      | |     | existing|     |demo |     |text |     | |
*      \-+ <-- +---------+ <-- +-----+ <-- +-----+ <-- +-/
*                   ^                         ^
*                   |-------- 删除范围 --------|
*
*      /-+ --> +----+ --> +--+ --> +-\
*      | |     | exi|     |t |     | |
*      \-+ <-- +----+ <-- +--+ <-- +-/
*/
bool text_delete(Text *txt, size_t pos, size_t len) {
	if (len == 0)
		return true;
	if (pos + len > txt->size)
		return false;
	//if (pos < txt->lines.pos)
	//	lineno_cache_invalidate(&txt->lines);

	Location loc = piece_get_intern(txt, pos);
	Piece *p = loc.piece;
	if (!p)
		return false;
	size_t off = loc.off;
	//if (cache_delete(txt, p, off, len))
	//	return true;
	Change *c = change_alloc(txt, pos);
	if (!c)
		return false;

	bool midway_start = false, midway_end = false; /* 开始/结束点是否在Piece中间 */
	Piece *before, *after; /* 在开始/结束点处没修改过的Piece */
	Piece *start, *end;    /* 被移去的Span */
	size_t cur;            /* 已被删除数 */

	if (off == p->len) {
		/* 删除开始于两个Piece之间 */
		cur = 0;
		before = p;
		start = p->next;
	}
	else {
		/* 删除开始于某个Piece内 */
		midway_start = true;
		cur = p->len - off;
		start = p;
		before = piece_alloc(txt);
		if (!before)
			return false;
	}

	/* 跳过删除范围内所有Piece */
	while (cur < len) {
		p = p->next;
		cur += p->len;
	}

	if (cur == len) {
		/* 删除结束于两个Piece之间 */
		end = p;
		after = p->next;
	}
	else {
		/* 删除结束于某个Piece内 */
		midway_end = true;
		end = p;
		after = piece_alloc(txt);
		if (!after)
			return false;
		piece_init(after, before, p->next, p->data + p->len - (cur - len), cur - len);
	}

	if (midway_start) {
		/* 现在知道了before应该和新malloc的Piece连接还是和已存在的Piece连接 */
		piece_init(before, start->prev, after, start->data, off);
	}

	Piece *new_start = NULL, *new_end = NULL;
	if (midway_start) {
		new_start = before;
		if (!midway_end)
			new_end = before;
	}
	if (midway_end) {
		if (!midway_start)
			new_start = after;
		new_end = after;
	}

	span_init(&c->new, new_start, new_end);
	span_init(&c->old, start, end);
	span_swap(txt, &c->old, &c->new);
	return true;
}


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

	/* 写一个空Action当做undo树root */
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

void text_free(Text *txt) {
	if (!txt)
		return;

	// free history
	Action *hist = txt->history;
	while (hist && hist->prev)//找到undo 树的root
		hist = hist->prev;
	while (hist) {
		Action *later = hist->later;
		action_free(hist);
		hist = later;
	}

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

/* 保存当前文本内容使得它可以用undo/redo操作还原，简称做个快照 */
void text_snapshot(Text *txt) {
	if (txt->current_action)
		txt->last_action = txt->current_action;
	txt->current_action = NULL;
	txt->cache = NULL;
}
