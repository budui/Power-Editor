#ifndef __VIEW_H__
#define __VIEW_H__

#include "util.h"
#include "menu.h"

/* open VGA mode and set back color is BLACK. */
bool VGA_INIT(void);
/* draw the main window of the GUI. */
void view_main_window(const char *name, colors barcolor);
void view_main_menu(const menuptr root,LANGUAGE lang);
char  *view_sub_menu_show(menuptr mainmenu);
bool view_sub_menu_hidden(char *buf,menuptr mainmenu);
#endif