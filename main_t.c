#include <graphics.h>
#include <stdio.h>
#include <conio.h>
#include "util.h"
#include "print.h"
#include "menu.h"
#include "view.h"
#include "config.h"

#ifdef DEBUG
FILE *logfile;
#endif // DEBUG


int main()
{
    const menuptr root =  GetMenu(CHINESE);
    menuptr m = FirsrChildMenu(root);
    char  *buf;

    logfile = fopen(".\\editor.log","wt");
    VGA_INIT();
    print_init();

    view_main_window("Power-editor",BLUE);
    view_main_menu(root,CHINESE);
	
    do
    {
        getch();
        buf = view_sub_menu_show(m);
        getch();
        view_sub_menu_hidden(buf,m);
    } while((m = NextBroMenu(m)) != NULL);
    
	getch();
    print_close();
    return 0;
}

