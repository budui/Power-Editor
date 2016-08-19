#ifndef __TEXT_H__ 
#define __TEXT_H__


#include "Public.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include <malloc.h>
#include <io.h>
#include <stdio.h>
#include <memory.h>

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif


#define stat  _stat
#define fstat _fstat
#define open  _open
#define close _close
#define read _read

#define TEST_ON

/* Allocate buffers holding the actual file content in junks of size: */
#define BUFFER_SIZE (1 << 20)
/* Files smaller than this value are copied on load, larger ones are mmap(2)-ed
* directely. Hence the former can be truncated, while doing so on the latter
* results in havoc. */
#define BUFFER_MMAP_SIZE (1 << 23)

typedef struct Text Text;
typedef struct Piece Piece;
typedef struct Buffer Buffer;



//bool text_insert(Text *txt, size_t pos, const char *data, size_t len);
Text *text_load(const char *filename);
//bool text_delete(Text *txt, size_t pos, size_t len);
void text_free(Text *txt);

#define TEST

//FOR TEST
#ifdef TEST
void test(const char* filename);
#endif

#endif
