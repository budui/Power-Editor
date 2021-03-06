#ifndef __MENU_H__
#define __MENU_H__

#include "util.h"

/* moudle for loading menu config.
* supporting English and Chinese.
*/

#define MENU_MAIN_FILE 1
#define MENU_SUB_NEWFILE 2
#define MENU_SUB_OPENFILE 3
#define MENU_SUB_SAVE 4
#define MENU_SUB_SAVEAS 5
#define MENU_SUB_EXIT 6

#define MENU_MAIN_EDIT 7
#define MENU_SUB_UNDO 8
#define MENU_SUB_REDO 9
#define MENU_SUB_FIND 10
#define MENU_SUB_REPLACE 11

#define MENU_MAIN_HELP 12
#define MENU_SUB_ABOUT 13

typedef struct menu *menuptr;

/* make a menu tree with config. return the root of this tree. */
const menuptr GetMenu(LANGUAGE);
/* Free memery contains menus. */
void FreeMenu(menuptr root);
/* return this menu's next bro. */
menuptr NextBroMenu(menuptr m);
/* return this menu's first child. */
menuptr FirsrChildMenu(menuptr m);
/* return this menu's name. */
const char *MenuName(menuptr m);
/* return this menu's id.
* id is the line num of the menu config.
* return -1 if pass NULL.
*/
int MenuID(menuptr m);
/* return the no. of the child. */
int MenuChildNum(menuptr parent,menuptr child);
int MenuChildCount(menuptr root);
/* return the root of the menu tree. */
menuptr GetRootMenu(menuptr m);

menuptr GetMenuByNum(menuptr root,int num);
/* Draw menu array in shell to debug. count is the number of array. */
void debug_Draw_menu(menuptr root, int count);

#endif // !__MENU_H__
