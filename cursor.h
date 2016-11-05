#include "text.h"
typedef struct Cursor
{
	int cursor_x;
	int cursor_y;
	Iterator it;
}Cursor;
	Cursor cursor;

void adjustcursor(int x,int y,Cursor *cursor,int width,int height);
