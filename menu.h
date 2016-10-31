#ifndef __MENU_H__
#define __MENU_H__

#include "util.h"

/* moudle for loading menu config.
* supporting English and Chinese.
*/

typedef struct menu *menuptr;

/* make a menu tree with config. return the root of this tree. */
const menuptr GetMenu(LANGUAGE);
/* Free memery contains menus. */
void FreeMenu(menuptr root);
menuptr NextBroMenu(menuptr m);
menuptr FirsrChildMenu(menuptr m);
const char *MenuName(menuptr m);
int MenuChildNum(menuptr parent,menuptr child);
menuptr GetRootMenu(menuptr m);
/* Draw menu array in shell to debug. count is the number of array. */
void debug_Draw_menu(menuptr root, int count);

#endif // !__MENU_H__