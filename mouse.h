// mouse
#include "util.h"
/* mouse moudle. */

//���
#define MOUSE_LEFTPRESS 1
//�Ҽ�
#define MOUSE_RIGHTPRESS 2
//û�а������
#define MOUSE_NOPRESS 0

bool initmouse(void);
//��ʾ���
int showmouseptr(void);
//�������
int hidemouseptr(void);

/* get mouse's state.
* return true when keyboard is hit or button is click.
* (getmousepos(&button,&x,&y) && !button): keyboard is hit.
* (getmousepos(&button,&x,&y) && button): mouse is clicked.
* (!getmousepos): mouse just move.
*/
bool getmousepos(int *button,int *x,int *y);

void draw_cursor(int x,int y);
