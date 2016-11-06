#ifndef __VIEW_H__
#define __VIEW_H__

#include "util.h"
#include "menu.h"


#define EDITOR_NAME "Power-editor"

#define WINDOW_MAXX 638
#define WINDOW_MAXY 478
#define WINDOW_MINX 1
#define WINDOW_MINY 1

#define EDIT_WINDOW_MAXX 631
#define EDIT_WINDOW_MAXY 456
#define EDIT_WINDOW_MINX 5
#define EDIT_WINDOW_MINY 40


/* open VGA mode and set back color is BLACK. */
bool vga_init(void);
void vga_close(void);
/* draw the main window of the GUI. */
void view_main_window(void);
/* draw the main menu of the GUI. */
int view_main_menu(const menuptr root);

/* recover the hiddend area, use the buffer from view_sub_menu_show.
*/
bool clickclosebutton(int x,int y,int button);
void get_main_menu_choice(menuptr root);
void get_sub_menu_choice(menuptr m);
void menu_key_manager(menuptr root);
char *inputbox_manager(char *message);
void messagebox_manager(char * message);
int judgebox_manager(char * message);
#endif
