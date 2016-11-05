#include <stdio.h>
#include <graphics.h>
#include "util.h"
#include "text.h"
typedef struct Cursor
{
	int cursor_x;
	int cursor_y;
	Iterator it;
}Cursor;

void linefeed(int *x,int *y,int width,int height);//
unsigned char hzoren(int pixelx,int pixely,Iterator *it,int width,int height);//
//void displaytext(unsigned char hzk,unsigned char enzk,int x0,int y0,int textcolor,int bkcolor);
void print_text_xy(int mode,const char *text,size_t n, bool auto_height_feed, int *x,int *y);//void print_text_xy(unsigned char *text,size_t n, bool auto_height_feed, int x,int y);//
void adjusttext(int x0,int y0,int length0,int height,struct fonts *font,void *charu,Text *text);//

void adjustcursor(int x,int y,Cursor *cursor,int width,int height);//