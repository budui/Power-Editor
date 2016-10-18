/* Functions for print. */
#ifndef __PEINT_H__
#define __PRINT_H__
#include <stdio.h>
#include <stdlib.h>
#include "text.h"
#include "public.h"

typedef struct
{
    Iterator *it; // keep the connection between text and cursor.
    int x, y; //cursor position.
    //some useful
} Cursor;

//print_wchar, print_str print from (cursor.x,cursor.y).
extern Cursor cursor;
//initizal. choose hz and english font. typecially, print_init(2,0)
void print_init(int hz, int en);

// functions below all print char start from position at where cursor is.
/* print a char. if ISHZ(c) == true,
* *c is set as qh and *(c+1) will be set as wh.
*/
void print_wchar(const char *c,color bg, color f);
/* print an str, chinese available.
* set auto_line_feed to true means
* auto line feed is needed. */
void print_str(const char *c, size_t n, bool auto_line_feed);

// functions below all print char start from position at where arg point.
void print_wchar_xy(const char *c,color bg, color f, int x, int y);
/* first char will be print at (x,y)*/
void print_str_xy(const char *c, size_t n, bool auto_line_feed, int x,int y);

#endif
