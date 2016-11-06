#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "menu.h"
#include <stdarg.h>
#include "util.h"
#include "text.h"

typedef enum{EDIT,MENU} EDITORMODE;
typedef struct
{
    EDITORMODE mode;
    Text *txt;
    ClipBorad *cli;
    menuptr root;
} EDITORSTATE;

void editor_exit(void);
void editor_init(void);
Text *editor_newfile(const char *filename);
bool editor_find(void);
bool editor_raplace(void);
void editor_menu_mode(void);
void editor_edit_mode(void);

bool editor_run_func(menuptr m,bool iskey,int key);
#endif
