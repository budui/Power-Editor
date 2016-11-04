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
//得到鼠标位置和按键状态（取值为上面3个宏）
bool getmousepos(int *button,int *x,int *y);