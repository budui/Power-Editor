#ifndef __PUBLIC_H__
#define __PUBLIC_H__

// bool
typedef enum { false, true } bool;
typedef int ssize_t;
typedef int color;

#define MIN(a, b)  ((a) > (b) ? (b) : (a))
#define MAX(a, b)  ((a) < (b) ? (b) : (a))
#define ISGBK(c) ((0x01) & ((c) >> 7))
#define DEBUG

#endif
