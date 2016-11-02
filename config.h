#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
/* file loaction. */
#define CONFIG_MENU_CN ".\\config\\menu_cn"
#define CONFIG_MENU_EN ".\\config\\menu_en"
#define CONFIG_FONT_HZK16 ".\\fonts\\hzk16"

/* GUI info. */
#define CONFIG_WINDOW_MAXX 638
#define CONFIG_WINDOW_MAXY 478

/* Util number. */
#define CONFIG_CN_SIZE 16
#define CONFIG_EN_SIZE 8
#define CONFIG_MAIN_MENU_X 10
#define CONFIG_MAIN_MENU_Y 23

/* Files smaller than this value will be load into memory, larger one will be
* truncated.
*/
#define CONFIG_FILE_TRUNCATE_SIZE ((size_t) 1 << 15)
#endif // !__CONFIG_H__
