#include "editor.h"
#include "view.h"

void editor_exit(Text *txt);
void editor_init(Text *txt);
bool editor_newfile(const char *filename);
bool editor_find(void);
bool editor_raplace(void);

bool editor_run_func(int lim,menuptr m,bool iskey,...)
{
    va_list ap;
    va_start(ap,lim);
    int func;
    const char *filename;
    if(!iskey)
        func = MenuID(m);
    else
    {
        func = va_arg(ap,int);
    }
    switch(func)
    {
    case MENU_MAIN_FILE:
    case MENU_MAIN_EDIT:
    case MENU_MAIN_HELP:
        get_sub_menu_choice(m);
        break;
    case MENU_SUB_NEWFILE:
        if(!editor_newfile(NULL)){
            view_messagebox("open new file failed.");
            return false;
        }
        break;
    case MENU_SUB_OPENFILE:
        filename = inputbox_manager("");
        if(!filename){
            if(!editor_newfile(filename)){
                view_messagebox("open file failed.");
                return false;
            }
        }
        else
            return false;
    case MENU_SUB_SAVE:
        text_save(va_arg(ap,Text *),va_arg(ap,const char *));
        break;
    case MENU_SUB_SAVEAS:
        filename = inputbox_manager("");
        if(!filename){
            if(!text_save(va_arg(ap,Text *),filename))
                view_messagebox("save file failed.!");
        }
        break;
    case MENU_SUB_EXIT:
        editor_exit(va_arg(ap,Text *));
        break;
    case MENU_SUB_UNDO:
        text_undo(va_arg(ap,Text *));
        break;
    case MENU_SUB_REDO:
        text_undo(va_arg(ap,Text *));
        break;
    case MENU_SUB_CUT:
        text_cut(va_arg(ap,ClipBorad *),va_arg(ap,Text *),va_arg(ap,Filerange *));
        break;
    case MENU_SUB_COPY:
        text_copy(va_arg(ap,ClipBorad *),va_arg(ap,Text *),va_arg(ap,Filerange *));
        break;
    case MENU_SUB_PASTE:
        text_copy(va_arg(ap,ClipBorad *),va_arg(ap,Text *),va_arg(ap,size_t));
        break;
    case MENU_SUB_DEL:
        text_delete_range(va_arg(ap,Text *),va_arg(ap,Filerange *));
        break;
    case MENU_SUB_FIND:
        editor_find();
        break;
    case MENU_SUB_REPLACE:
        editor_raplace();
        break;
    case MENU_SUB_ABOUT:
        view_about();
        break;
    default:
        return false;
    }
    va_end(ap);
    return true;
}
