# 编辑器内核

- 使用方式： `#include "Text.h"`
- 所有可用的函数接口均以text_开头
- 函数返回值为bool类型时，返回true表示该函数执行成功，返回flase表示函数执行失败。

## 可复用结构体

### Filerange

```c
typedef struct {
	size_t start, end;        /* 从文件开头数起的byte范围 */
} Filerange;
```

### Iterator

```c
typedef struct {
	const char *start;  /* Piece结构体数据的开始 */
	const char *end;    /* 指向有效数据后第一个byte的指针，即：[start, end) */
	const char *text;   /* 当前位置（一定在Piece内，即: start <= text < end） */
	const Piece *piece; /* 和内部结构体关联的数据，不要改变 */
	size_t pos;         /* 从文件开始数起的位置 */
} Iterator;
```

## 函数

### 文件读写部分

#### text_load

```c
Text *text_load(const char *filename);
```

创建一个与给定文件相关联的Text结构体。Text是本模块最重要的结构体，模块外对文件的操作全由操作此结构体完成。

此函数可以处理三种情况：

- 如果传入参数 `filename`为空，即`filename`为NULL，那么相当于新建文件，将以空文件开始。

- 如果文件过大，将用分块加载的方式处理。

- 对于其情况，所有文件内容都将被加载至内存中。
#### text_free

```c
void text_free(Text*);
```

释放与Text关联的所有数据。

>注意，text_load函数与text_free函数是共轭的，出现text_load就一定要出现text_free。

#### text_save

```c
bool text_save(Text*, const char *filename);
```



#### text_write

```c
ssize_t text_write(Text*, int fd);
```



###  文件编辑部分

#### text_insert

```c
bool text_insert(Text*, size_t pos, const char *data, size_t len);
```

将从`data`开始的`len`个 byte 插入到第`pos`个字节（从文件开始数起）。`pos`必须在文件大小以内。

#### text_delete

```c
bool text_delete(Text*, size_t pos, size_t len);
```

删除从 `pos`开始的 `len` 个bytes 。

#### text_delete_range

```c
bool text_delete_range(Text*, Filerange*);
```

删除给定范围内字符。

#### text_insert_newline

```c
size_t text_insert_newline(Text*, size_t pos);
```

插入适合于文件的行结束符（\n或者\r\n）。

### 重做/撤销部分

### 查找替代部分

### 复制剪切部分












  ​

