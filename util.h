#ifndef __UTIL_H__
#define __UTIL_H__
 
#ifdef _MSC_VER
#include <stdbool.h>
#endif // if support c99

#ifdef __BORLANDC__
typedef enum COLORS colors;
typedef enum { false, true } bool;
#endif // if compiled by borland c++ 3.1 in dos

/* used to control permission. */
typedef enum { GUEST = 0, USER, SUPERUSER } USERTYPE;
/* used to choose language. */
typedef enum { ENGLISH, CHINESE } LANGUAGE;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/* check whether a char stores Chinese encoded by GBK. 
* Chinese is encoded use the same pattern in GBK, which 
* first bit is 1 and English's is 0. (c)>>7 keep the first
* bit and & with (0x01) make other bit is 0.
*/
#define ISGBK(c) ((0x01) & ((c) >> 7))

/* set debug mode 
* open editor.log to write log file.
*/
#define DEBUG 1


#endif
