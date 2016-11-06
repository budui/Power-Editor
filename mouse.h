// mouse
#include "util.h"
/* mouse moudle. */

//左键
#define MOUSE_LEFTPRESS 1
//右键
#define MOUSE_RIGHTPRESS 2
//没有按动鼠标
#define MOUSE_NOPRESS 0

bool initmouse(void);
//显示鼠标
int showmouseptr(void);
//隐藏鼠标
int hidemouseptr(void);

/* get mouse's state.
* return true when keyboard is hit or button is click.
* (getmousepos(&button,&x,&y) && !button): keyboard is hit.
* (getmousepos(&button,&x,&y) && button): mouse is clicked.
* (!getmousepos): mouse just move.
*/
bool getmousepos(int *button,int *x,int *y);

void draw_cursor(int x,int y);
