#include "editor.h"
#include "view.h"
#include "print.h"
#include "mouse.h"
#include <conio.h>

extern EDITORSTATE editor;

bool editor_find(void)
{
    messagebox_manager("finding");
    return true;
}

bool editor_raplace(void)
{
    messagebox_manager("raplacing");
    return true;
}

bool editor_run_func(menuptr m,bool iskey,int key)
{
    int func;
    const char *filename;
    if(!iskey)
        func = MenuID(m);
    else
    {
        func = key;
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
            messagebox_manager("open new file failed.");
            return false;
        }
        editor.mode = EDIT;
        break;
    case MENU_SUB_OPENFILE:
        filename = inputbox_manager("");
        if(!filename){
            if(!editor_newfile(filename)){
                messagebox_manager("open file failed.");
                return false;
            }
            editor.mode = EDIT;
        }
        else
            return false;
    case MENU_SUB_SAVE:
        if(editor.mode == EDIT)
            text_save(editor.txt);
        break;
    case MENU_SUB_SAVEAS:
        if(editor.mode == EDIT){
            filename = inputbox_manager("please input filename:");
            if(!filename){
                if(!text_saveas(editor.txt,filename))
                messagebox_manager("save file failed.!");
            }
        }
        break;
    case MENU_SUB_EXIT:
		editor_exit();
		break;
	case MENU_SUB_UNDO:
        text_undo(editor.txt);
		break;
	case MENU_SUB_REDO:
		text_redo(editor.txt);
        break;
    case MENU_SUB_FIND:
        editor_find();
        break;
    case MENU_SUB_REPLACE:
        editor_raplace();
        break;
    case MENU_SUB_ABOUT:
		messagebox_manager("Made by wr&fzy!");
        break;
    default:
        return false;
    }
    return true;
}

void editor_init(void)
{
    Text *txt = text_load(NULL);

    vga_init();
    print_init();
	view_main_window();

    editor.root =  GetMenu(CHINESE);
    if(!(editor.root)){
        messagebox_manager("load menu failed.");
        getch();
        text_free(txt);
        exit(1);
    }
    view_main_menu(editor.root);
    if(!txt){
        messagebox_manager("open empty file failed.");
    }
    editor.cli = clipborad_init();

    initmouse();
    fprintf(stderr,"logging...\n");
    showmouseptr();
    editor.txt = txt;
    editor.mode = MENU;
}
void editor_exit(void)
{
    Text *txt = editor.txt;
    if(text_modified(txt)){
        int choice = judgebox_manager("File not save,save?");
        switch(choice)
        {
        case 0:
            return;
        case 1:
            text_free(txt);
            break;
        case 2:
            if(text_save(txt) == 2){
                char * f = inputbox_manager("please input filename:");
                text_saveas(txt,f);
                free(f);
                text_free(txt);
            }
        }
    }
    FreeMenu(editor.root);
    clipborad_close(editor.cli);
    print_close();
    vga_close();
}
Text *editor_newfile(const char *filename)
{
    Text *newtxt;
    Text *txt = editor.txt;
    if(text_modified(txt)){
        int choice = judgebox_manager("File not save,save?");
        switch(choice)
        {
        case 0:
            return NULL;
        case 1:
            text_free(txt);
            break;
        case 2:
            if(text_save(txt) == 2){
                char * f = inputbox_manager("please input filename:");
                text_saveas(txt,f);
                free(f);
                text_free(txt);
            }
            break;
        }
    }
	newtxt = text_load(filename);
    if(!newtxt)
        messagebox_manager("open new file failed");
    editor.txt = newtxt;
    return newtxt;
}

void editor_menu_mode(void)
{
    while(1)
    {
		get_main_menu_choice(editor.root);
		menu_key_manager(editor.root);
    }
}
void editor_edit_mode(void)
{
    return;
}
