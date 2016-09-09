#ifndef __PUBLIC_H__ 
#define __PUBLIC_H__


typedef enum { false, true } bool;
typedef int ssize_t;

#define MIN(a, b)  ((a) > (b) ? (b) : (a))
#define MAX(a, b)  ((a) < (b) ? (b) : (a))

#define DEBUG

#endif