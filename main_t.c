#include <graphics.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "util.h"
#include "print.h"
#include "menu.h"
#include "view.h"
#include "config.h"
#include "mouse.h"


int main()
{
    const menuptr root =  GetMenu(CHINESE);
    int x,y,button;
    char far *buf;
    freopen(".\\view.log","w",stderr);

    vga_init();
    print_init();
    view_main_window("Power-editor",BLUE);
    view_main_menu(root);
    if(initmouse()==0)
    {
        printf("\n Unable to initialise Mouse");  exit(0);
    }
    fprintf(stderr,"logging...\n");
    showmouseptr();

    while(1)
    {
        get_main_menu_choice(root);
        menu_key_manager(root);
    }

    getch();
    print_close();
    FreeMenu(root);
    return 0;
}
