#ifndef __TEXT_H__ 
#define __TEXT_H__

/* Power Editor核心，提供给外部函数均为text类（以text_开头的函数），暂时包括方便DEBUG的test类函数
*
*  1) 使用直接#include "Text.h"即可。
*	  外部可用接口只有函数，任何结构体内部信息外部均无法使用，只可使用定义在此头文件的结构体的指针。
*	  如：
*	     Text *txt = text_load("C:\\Users\\Jinxiapu\\Desktop\\Temp\\1.txt");
*		 text_insert(txt, 12, "fjdskl", 6);
*		 text_free(txt);
*     注意，使用text_load后必须text_free
*
*  2) 为了跨平台测试方便，根据宏__BORLANDC__，WIN32区分DOS,WINDOWS系统
*/

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
#include <time.h>
#include "Public.h"



//判断是否是文件夹
#ifndef S_ISDIR 
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

//判断是否是一个常规文件
#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

//将DOS/WINODWS库函数转换为POSIX风格，方便跨平台，方便阅读
#define stat  _stat
#define fstat _fstat
#define open  _open
#define close _close
#define read _read

#ifdef __BORLANDC__
/* 被申请来存储文件内容的缓存区区块大小，1 << 15 == 32KB ，并不知道在DOS下多少合适*/
#define BUFFER_SIZE (1 << 15)
/* 文件大小小于这个数字时直接将文件拷贝至内存中，大于时应该用文件映射的方式 
* 因此前一种形式可以截断，但后一种如果截断会导致文件损坏*/
#define BUFFER_MMAP_SIZE (1 << 20)
/* buffer_read函数读取文本时缓存区大小*/
#define READ_BUFF_SIZE 4096
#endif

#ifdef  WIN32
#define BUFFER_MMAP_SIZE (1 << 23)
#define BUFFER_SIZE (1 << 20)
#define READ_BUFF_SIZE 4096
#endif


typedef struct Text Text;
typedef struct Piece Piece;

#define EPOS ((size_t)-1) 

bool text_insert(Text *txt, size_t pos, const char *data, size_t len);
bool text_delete(Text *txt, size_t pos, size_t len);
Text *text_load(const char *filename);
void text_snapshot(Text *txt);
void text_free(Text *txt);



#ifdef DEBUG
void test_print_buf(const char* data, size_t len);
void test_show_info(Text *txt);
#endif

#endif
