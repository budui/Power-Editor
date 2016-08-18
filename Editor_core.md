 

# 内核模块

## 主要结构体
### Buffer

```c
/* Buffer包含文本内容, 包括只读的缓存区和用malloc分配的堆
 */
struct Buffer {
	size_t size;               /* BUFF大小 */
	size_t len;                /* 当前位置 / 插入位置 */
	char *data;                /* 实际数据 */
	enum {                     /* 内存分配类型 */
		MMAP_ORIG,         /* mmap(2)-ed from an external file */
		MMAP,              /* mmap(2)-ed from a temporary file only known to this process */
		MALLOC,            /* 用malloc分配的堆 */
	} type;
	Buffer *next;              /* 下一区块 */
};
```

### Piece

```c
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
```

### Location

```c
/* 用来转换全局索引(从文本开头数起的字节数) 到与Piece关联的索引。
 */
typedef struct {
	Piece *piece;           /* 包含指定位置的Piece */
	size_t off;             /* 在此Piece中的索引 */
} Location;
```

### Span

```c
/* 一个Span包括确定范围的Piece。对文档的改变总是通过用新Span取代旧Span实现。
 */
typedef struct {
	Piece *start, *end;     /* Span的开始和结束 */
	size_t len;             /* 组成这个Span的各个Piece长度之和 */
} Span;
```

### Change

```c
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
```

### Action

```c
/* An Action is a list of Changes which are used to undo/redo all modifications
 * since the last snapshot operation. Actions are stored in a directed graph structure.
 */
typedef struct Action Action;
struct Action {
	Change *change;         /* the most recent change */
	Action *next;           /* the next (child) action in the undo tree */
	Action *prev;           /* the previous (parent) operation in the undo tree */
	Action *earlier;        /* the previous Action, 按时间顺序 */
	Action *later;          /* the next Action, 按时间顺序 */
	time_t time;            /* when the first change of this action was performed */
	size_t seq;             /* 唯一的严格递增的标识符 */
};
```

### LineCache

```c
typedef struct {
	size_t pos;             /* position in bytes from start of file */
	size_t lineno;          /* line number in file i.e. number of '\n' in [0, pos) */
} LineCache;
```

### Text

```c
/* The main struct holding all information of a given file */
struct Text {
	Buffer *buf;            /* original file content at the time of load operation */
	Buffer *buffers;        /* all buffers which have been allocated to hold insertion data */
	Piece *pieces;          /* all pieces which have been allocated, used to free them */
	Piece *cache;           /* most recently modified piece */
	Piece begin, end;       /* 总是存在但不包括任何信息的哨兵节点 */
	Action *history;        /* undo tree */
	Action *current_action; /* action holding all file changes until a snapshot is performed */
	Action *last_action;    /* the last action added to the tree, 按时间顺序 */
	Action *saved_action;   /* the last action at the time of the save operation */
	size_t size;            /* current file content size in bytes */
	struct stat info;       /* stat as probed at load time */
	LineCache lines;        /* mapping between absolute pos in bytes and logical line breaks */
	enum TextNewLine newlines; /* which type of new lines does the file use */
};
```

### TextSave

```c
struct TextSave {                  /* used to hold context between text_save_{begin,commit} calls */
	Text *txt;                 /* text to operate on */
	char *filename;            /* filename to save to as given to text_save_begin */
	char *tmpname;             /* temporary name used for atomic rename(2) */
	int fd;                    /* file descriptor to write data to using text_save_write */
	enum {
		TEXT_SAVE_UNKNOWN,
		TEXT_SAVE_ATOMIC,  /* create a new file, write content, atomically rename(2) over old file */
		TEXT_SAVE_INPLACE, /* 截断文件重写内容 (any error will result in data loss) */
	} type;
};
```

### Iterator

```c
typedef struct {
	const char *start;  /* begin of piece's data */
	const char *end;    /* pointer to the first byte after valid data i.e. [start, end) */
	const char *text;   /* current position within piece: start <= text < end */
	const Piece *piece; /* internal state do not touch! */
	size_t pos;         /* global position in bytes from start of file */
} Iterator;
```



## 函数

### Buffer类

#### buffer_alloc

```c
/* allocate a new buffer of MAX(size, BUFFER_SIZE) bytes */
static Buffer *buffer_alloc(Text *txt, size_t size) {
	Buffer *buf = calloc(1, sizeof(Buffer));//申请一个Buffer空间并将该空间全部写为0
	if (!buf)
		return NULL;
	if (BUFFER_SIZE > size)
		size = BUFFER_SIZE;
	if (!(buf->data = malloc(size))) {//检查是否申请到BUFF空间
		free(buf);
		return NULL;
	}
	buf->type = MALLOC;
	buf->size = size;
	buf->next = txt->buffers;
	txt->buffers = buf;
	return buf;
}

```

#### buffer_read

```c
static Buffer *buffer_read(Text *txt, size_t size, int fd) {
	Buffer *buf = buffer_alloc(txt, size);
	if (!buf)
		return NULL;
	while (size > 0) {//先把内容读入data做个中转，再将中转区内容移动到buf->data
		char data[4096];//2 << 12
		ssize_t len = read(fd, data, MIN(sizeof(data), size));
		if (len == -1) {//读取出错
			txt->buffers = buf->next;
			buffer_free(buf);
			return NULL;
		} else if (len == 0) {//读取到EOF
			break;
		} else {
			buffer_append(buf, data, len);
			size -= len;
		}
	}
	return buf;
}
```

#### buffer_free

```c
static void buffer_free(Buffer *buf) {
	if (!buf)
		return;
	if (buf->type == MALLOC)
		free(buf->data);
	else if ((buf->type == MMAP_ORIG || buf->type == MMAP) && buf->data)
		munmap(buf->data, buf->size);
	free(buf);
}
```

#### buffer_append

看样子一般是够啊。

```c
/* append data to buffer, assumes there is enough space available */
static const char *buffer_append(Buffer *buf, const char *data, size_t len) {
	char *dest = memcpy(buf->data + buf->len, data, len);
	buf->len += len;
	return dest;
}
```

#### buffer_mmap

这鬼函数得是将大文件映射到内存中，得找找DOS下的解决方案啊。

```c
static Buffer *buffer_mmap(Text *txt, size_t size, int fd, off_t offset) {
	Buffer *buf = calloc(1, sizeof(Buffer));
	if (!buf)
		return NULL;
	if (size) {
		buf->data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, offset);
		if (buf->data == MAP_FAILED) {
			free(buf);
			return NULL;
		}
	}
	buf->type = MMAP_ORIG;
	buf->size = size;
	buf->len = size;
	buf->next = txt->buffers;
	txt->buffers = buf;
	return buf;
}
```

#### buffer_capcacity

```c
/* check whether buffer has enough free space to store len bytes */
static bool buffer_capacity(Buffer *buf, size_t len) {
	return buf->size - buf->len >= len;
}
```

#### buffer_store

```c
/* stores the given data in a buffer, allocates a new one if necessary. returns
 * a pointer to the storage location or NULL if allocation failed. */
static const char *buffer_store(Text *txt, const char *data, size_t len) {
	Buffer *buf = txt->buffers;
	if ((!buf || !buffer_capacity(buf, len)) && !(buf = buffer_alloc(txt, len)))
		return NULL;
	return buffer_append(buf, data, len);
}
```

#### buffer_delete

```c
/* delete data from a buffer at an arbitrary position, this should only be used with
 * data of the most recently created piece. */
static bool buffer_delete(Buffer *buf, size_t pos, size_t len) {
	if (pos + len > buf->len)
		return false;
	if (buf->len == pos) {
		buf->len -= len;
		return true;
	}
	char *delete = buf->data + pos;
	memmove(delete, delete + len, buf->len - pos - len);
	buf->len -= len;
	return true;
}
```

#### buffer_insert

```c
/* insert data into buffer at an arbitrary position, this should only be used with
 * data of the most recently created piece. */
static bool buffer_insert(Buffer *buf, size_t pos, const char *data, size_t len) {
	if (pos > buf->len || !buffer_capacity(buf, len))
		return false;
	if (buf->len == pos)
		return buffer_append(buf, data, len);
	char *insert = buf->data + pos;
	memmove(insert + len, insert, buf->len - pos);
	memcpy(insert, data, len);
	buf->len += len;
	return true;
}
```

### Cache类

#### cache_piece

```c
/* cache the given piece if it is the most recently changed one */
static void cache_piece(Text *txt, Piece *p) {
	Buffer *buf = txt->buffers;
	if (!buf || p->data < buf->data || p->data + p->len != buf->data + buf->len)
		return;
	txt->cache = p;
}
```

#### cache_contains

```c
/* 检查给定的Piece是否为最近修改的 */
static bool cache_contains(Text *txt, Piece *p) {
	Buffer *buf = txt->buffers;
	Action *a = txt->current_action;
	if (!buf || !txt->cache || txt->cache != p || !a || !a->change)
		return false;

	Piece *start = a->change->new.start;
	Piece *end = a->change->new.end;
	bool found = false;
	for (Piece *cur = start; !found; cur = cur->next) {
		if (cur == p)
			found = true;
		if (cur == end)
			break;
	}

	return found && p->data + p->len == buf->data + buf->len;
}
```
####cache_insert
```c
/* try to insert a junk of data at a given piece offset. the insertion is only
 * performed if the piece is the most recenetly changed one. the legnth of the
 * piece, the span containing it and the whole text is adjusted accordingly */
static bool cache_insert(Text *txt, Piece *p, size_t off, const char *data, size_t len) {
	if (!cache_contains(txt, p))
		return false;
	Buffer *buf = txt->buffers;
	size_t bufpos = p->data + off - buf->data;
	if (!buffer_insert(buf, bufpos, data, len))
		return false;
	p->len += len;
	txt->current_action->change->new.len += len;
	txt->size += len;
	return true;
}
```
####cache_delete
```c
/* try to delete a junk of data at a given piece offset. the deletion is only
 * performed if the piece is the most recenetly changed one and the whole
 * affected range lies within it. the legnth of the piece, the span containing it
 * and the whole text is adjusted accordingly */
static bool cache_delete(Text *txt, Piece *p, size_t off, size_t len) {
	if (!cache_contains(txt, p))
		return false;
	Buffer *buf = txt->buffers;
	size_t bufpos = p->data + off - buf->data;
	if (off + len > p->len || !buffer_delete(buf, bufpos, len))
		return false;
	p->len -= len;
	txt->current_action->change->new.len -= len;
	txt->size -= len;
	return true;
}
```
###Span类

#### span_init

```c
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
```

#### span_swap

```c
/* swap out an old span and replace it with a new one.
 *
 *  - if old is an empty span do not remove anything, just insert the new one
 *  - if new is an empty span do not insert anything, just remove the old one
 *
 * adjusts the document size accordingly.
 */
static void span_swap(Text *txt, Span *old, Span *new) {
	if (old->len == 0 && new->len == 0) {
		return;
	} else if (old->len == 0) {
		/* insert new span */
		new->start->prev->next = new->start;
		new->end->next->prev = new->end;
	} else if (new->len == 0) {
		/* delete old span */
		old->start->prev->next = old->end->next;
		old->end->next->prev = old->start->prev;
	} else {
		/* replace old with new */
		old->start->prev->next = new->start;
		old->end->next->prev = new->end;
	}
	txt->size -= old->len;
	txt->size += new->len;
}
```

### Action类

#### action_alloc

```c
/* allocate a new action, set its pointers to the other actions in the history,
 * and set it as txt->history. All further changes will be associated with this action. */
static Action *action_alloc(Text *txt) {
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
```
#### action_free
```c
static void action_free(Action *a) {
	if (!a)
		return;
	for (Change *next, *c = a->change; c; c = next) {
		next = c->next;
		change_free(c);
	}
	free(a);
}
```
### Piece类
#### piece_alloc
```c
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
```
#### piece_free
```c
static void piece_free(Piece *p) {
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
```
#### piece_init
```c
static void piece_init(Piece *p, Piece *prev, Piece *next, const char *data, size_t len) {
	p->prev = prev;
	p->next = next;
	p->data = data;
	p->len = len;
}
```
#### piece_get_intern
```c
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
			return (Location){ .piece = p, .off = pos - cur };
		cur += p->len;
	}

	return (Location){ 0 };
}
```
#### piece_get_extern
```c
/* similiar to piece_get_intern but usable as a public API. returns the piece
 * holding the text at byte offset pos. never returns a sentinel piece.
 * it pos is the end of file (== text_size()) and the file is not empty then
 * the last piece holding data is returned.
 */
static Location piece_get_extern(Text *txt, size_t pos) {
	size_t cur = 0;

	if (pos > 0 && pos == txt->size) {
		Piece *p = txt->begin.next;
		while (p->next->next)
			p = p->next;
		return (Location){ .piece = p, .off = p->len };
	}

	for (Piece *p = txt->begin.next; p->next; p = p->next) {
		if (cur <= pos && pos < cur + p->len)
			return (Location){ .piece = p, .off = pos - cur };
		cur += p->len;
	}

	return (Location){ 0 };
}
```
### Change类
#### change_alloc
```c
/* allocate a new change, associate it with current action or a newly
 * allocated one if none exists. */
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
```
#### change_free
```c
static void change_free(Change *c) {
	if (!c)
		return;
	/* only free the new part of the span, the old one is still in use */
	piece_free(c->new.start);
	if (c->new.start != c->new.end)
		piece_free(c->new.end);
	free(c);
}
```
### lines类
#### lines_skip_forward
```c
/* skip n lines forward and return position afterwards */
static size_t lines_skip_forward(Text *txt, size_t pos, size_t lines, size_t *lines_skipped) {
	size_t lines_old = lines;
	text_iterate(txt, it, pos) {
		const char *start = it.text;
		while (lines > 0 && start < it.end) {
			size_t n = it.end - start;
			const char *end = memchr(start, '\n', n);
			if (!end) {
				pos += n;
				break;
			}
			pos += end - start + 1;
			start = end + 1;
			lines--;
		}

		if (lines == 0)
			break;
	}
	if (lines_skipped)
		*lines_skipped = lines_old - lines;
	return pos;
}
```
#### lines_count
```c
/* count the number of new lines '\n' in range [pos, pos+len) */
static size_t lines_count(Text *txt, size_t pos, size_t len) {
	size_t lines = 0;
	text_iterate(txt, it, pos) {
		const char *start = it.text;
		while (len > 0 && start < it.end) {
			size_t n = MIN(len, (size_t)(it.end - start));
			const char *end = memchr(start, '\n', n);
			if (!end) {
				len -= n;
				break;
			}
			lines++;
			len -= end - start + 1;
			start = end + 1;
		}

		if (len == 0)
			break;
	}
	return lines;
}
```
### lineno类
#### lineno_cache_invalidate
```c
static void lineno_cache_invalidate(LineCache *cache) {
	cache->pos = 0;
	cache->lineno = 1;
}
```
