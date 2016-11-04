#ifndef __VIEW_H__
#define __VIEW_H__

#include "util.h"
#include "menu.h"

/* open VGA mode and set back color is BLACK. */
bool VGA_INIT(void);
/* draw the main window of the GUI. */
void view_main_window(const char *name, colors barcolor);
/* draw the main menu of the GUI. */
int view_main_menu(const menuptr root);

/* recover the hiddend area, use the buffer from view_sub_menu_show.
*/
bool clickclosebutton(int x,int y,int button);
void get_main_menu_choice(menuptr root);
void get_sub_menu_choice(menuptr m);

bool sub_menu_hidden(char far *buf,int x,int y);
 char far *sub_menu_show(menuptr m);

#endif
