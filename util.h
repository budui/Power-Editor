#ifndef __UTIL_H__
#define __UTIL_H__

#ifdef _MSC_VER
#include <stdbool.h>
#endif // if support c99

#ifdef __BORLANDC__
typedef enum color color;
typedef enum { false, true } bool;
typedef int ssize_t;
#endif // if compiled by borland c++ 3.1 in dos

/* check whether a char stores Chinese encoded by GBK. */
#define ISGBK(c) ((0x01) & ((c) >> 7))

/* set debug mode 
* open core.log to write log file.
*/
#define DEBUG

#endif
