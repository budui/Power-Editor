#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <errno.h>
#include <bios.h>
#include <conio.h>
#include "view.h"
#include "print.h"
#include "mouse.h"
#include "config.h"
#include "key.h"

#define SUBMENU 1
#define MAINMENU 2

/* flag for func main_menu_change */
#define MAIN_BORDER_IN 1
#define MAIN_BORDER_OUT 2
#define MAIN_BORDER_QUIT 0
/* flag for func sub_menu_change */
#define SUB_DEHIGHLIGHT 0
#define SUB_HIGHLIGHT 1

#define MAIN_MENU_WIDTH 48
#define SUB_MENU_Y 41
#define MAIN_MENU_X 10
#define MAIN_MENU_Y 23

#define SHADOW_IN 1
#define SHADOW_OUT 2

#define INPUTBOX_MINX 200
#define INPUTBOX_MINY 165
#define INPUTBOX_MAXX 420
#define INPUTBOX_MAXY 300
#define INPUTBOX_EDIT_MINX 225
#define INPUTBOX_EDIT_MINY 220
#define INPUTBOX_EDIT_MAXX 395
#define INPUTBOX_EDIT_MAXY 240

static int is_arrow_key_hit(int flag);
static void sub_menu_change(int x1,int choice,int flag);
static void main_menu_change(int choice, int flag);
static void draw_closebutt(int x1,int y1);
static void draw_icon(int x1,int y1);
static void draw_button(int x1,int y1,char *str);
static void draw_shadow(int x1,int y1,int x2,int y2, int flag,int inner);
static void get_sub_menu_choice(menuptr m);
static bool sub_menu_hidden(char far *buf,int x,int y);
static char far *sub_menu_show(menuptr m);
static char far *inputbox_show(char *);

static void draw_shadow(int x1,int y1,int x2,int y2, int flag,int inner)
{
    switch(flag)
    {
    case SHADOW_IN:
        setfillstyle(SOLID_FILL,inner);
        bar(x1+1,y1+1,x2,y2);
        setcolor(DARKGRAY);
        line(x1,y1,x2+2,y1);
        line(x1,y1,x1,y2+2);
        setcolor(BLACK);
        line(x1+1,y1+1,x2+1,y1+1);
        line(x1+1,y1+1,x1+1,y2+1);
        setcolor(WHITE);
        line(x1,y2+3,x2+2,y2+3);
        line(x2+3,y1,x2+3,y2+3);
        break;
    case SHADOW_OUT:
        setfillstyle(SOLID_FILL,inner);
        setcolor(WHITE);
        rectangle(x1-1, y1-1, x2, y2);
        setcolor(DARKGRAY);
        rectangle(x1, y1, x2+1, y2+1);
        bar(x1, y1, x2, y2);
        break;
    }
}

static void draw_closebutt(int x,int y)
{
    //shadow.
    setcolor(WHITE);
    setfillstyle(SOLID_FILL,LIGHTGRAY);
    settextstyle(SMALL_FONT,0,6);
    rectangle(x-5,y-3,x+8,y+8);
    setcolor(DARKGRAY);
    rectangle(x-4,y-2,x+9,y+9);

    bar(x-4,y-2,x+8,y+8);
    setcolor(BLACK);
    outtextxy(x-1,y-9,"x");
    outtextxy(x-2,y-9,"x");
}

static void draw_icon(int x1,int y1)
{
    setcolor(YELLOW);
    settextstyle(SMALL_FONT,0,5);
    outtextxy(x1,y1,"/");
    outtextxy(x1+1,y1,"/");
    outtextxy(x1+2,y1+2,"/");
    outtextxy(x1+3,y1+2,"/");
    outtextxy(x1+4,y1+4,"/");
    outtextxy(x1+5,y1+4,"/");
}

static void main_menu_change(int choice, int flag)
{
    int x1 = MAIN_MENU_X + (choice-1)*MAIN_MENU_WIDTH;
    int y1 = MAIN_MENU_Y-2;
    int x2 = x1 + MAIN_MENU_WIDTH-7;
    int y2 = MAIN_MENU_Y + CONFIG_CN_SIZE+1;
    x1 = x1 - 5;
    switch(flag)
    {
    case MAIN_BORDER_OUT:
        setcolor(WHITE);
        line(x1,y1,x2,y1);
        line(x1,y1,x1,y2);
        setcolor(DARKGRAY);
        line(x2,y2,x2,y1);
        line(x2,y2,x1,y2);
        break;
    case MAIN_BORDER_IN:
        setcolor(DARKGRAY);
        line(x1,y1,x2,y1);
        line(x1,y1,x1,y2);
        setcolor(WHITE);
        line(x2,y2,x2,y1);
        line(x2,y2,x1,y2);
        break;
    case MAIN_BORDER_QUIT:
        setcolor(LIGHTGRAY);
        line(x1,y1,x2,y1);
        line(x1,y1,x1,y2);
        setcolor(LIGHTGRAY);
        line(x2,y2,x2,y1);
        line(x2,y2,x1,y2);
        break;
    }
}

static void sub_menu_change(int x1,int choice,int flag)
{
    int y1 = SUB_MENU_Y + (16+5)*(choice-1);
    int x2,y2;
    x1 = x1 + 3;
    y1 = y1 + 3;
    y2 = y1 + (16+5) - 2;
    x2 = x1 + 5*CONFIG_CN_SIZE-5;
    switch(flag)
    {
    case SUB_HIGHLIGHT:
        setcolor(DARKGRAY);
        line(x1,y1,x2,y1);
        line(x1,y1,x1,y2);
        setcolor(WHITE);
        line(x2,y2,x2,y1);
        line(x2,y2,x1,y2);
        break;
    case SUB_DEHIGHLIGHT:
        setcolor(LIGHTGRAY);
        line(x1,y1,x2,y1);
        line(x1,y1,x1,y2);
        setcolor(LIGHTGRAY);
        line(x2,y2,x2,y1);
        line(x2,y2,x1,y2);
        break;
    default:
        return;
    }
}

bool vga_init(void)
{
    int gd=VGA,gm=VGAHI;
    initgraph(&gd,&gm,"c:\\BORLANDC\\bgi");
    if(graphresult() != grOk)
    {
        return false;
    }
    setbkcolor(BLACK);
    return true;
}

void vga_close(void)
{
    cleardevice();
    closegraph();
}

bool clickclosebutton(int x,int y,int button)
{
    int x1 = WINDOW_MAXX - 15 - 5;
    int y1 = 0;
    int x2 = WINDOW_MAXX - 15 + 9;
    int y2 = 18;
    if(button != MOUSE_LEFTPRESS)
        return false;
    return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

void view_main_window()
{
    //biggest window.
    draw_shadow(WINDOW_MINX,WINDOW_MINY,WINDOW_MAXX,WINDOW_MAXY,SHADOW_OUT,LIGHTGRAY);
    //name bar.
    setfillstyle(SOLID_FILL,BLUE);
    bar(WINDOW_MINX+3,WINDOW_MINY+3,WINDOW_MAXX-3,WINDOW_MINY+19);
    //editor name.
    settextstyle(SMALL_FONT,0,5);
    setcolor(WHITE);
    outtextxy(WINDOW_MINX+25,WINDOW_MINX+2,EDITOR_NAME);
    //edit window
    draw_shadow(EDIT_WINDOW_MINX,EDIT_WINDOW_MINY,EDIT_WINDOW_MAXX,EDIT_WINDOW_MAXY,SHADOW_IN,WHITE);
    //button
    draw_closebutt(WINDOW_MAXX-15,WINDOW_MINY+8);
    //icon
    draw_icon(WINDOW_MINX+7,WINDOW_MINY-1);
}

int view_main_menu(const menuptr root)
{
    menuptr m = FirsrChildMenu(root);
    int x = MAIN_MENU_X,y = MAIN_MENU_Y;
    do
    {
        print_str_xy(MenuName(m),x,y);
            x += MAIN_MENU_WIDTH;
    } while((m = NextBroMenu(m)) != NULL);
    return x;
}

bool view_run_func(menuptr m)
{
    int func = MenuID(m);
    switch(func)
    {
    case MENU_MAIN_FILE:
    case MENU_MAIN_EDIT:
    case MENU_MAIN_HELP:
        get_sub_menu_choice(m);
        break;
    case MENU_SUB_NEWFILE:
        outtextxy(100,100,"aaaa");
        break;
    case MENU_SUB_OPENFILE:
        inputbox_manager("");
        break;
    case MENU_SUB_SAVE:
        break;
    case MENU_SUB_SAVEAS:
        inputbox_manager("");
        break;
    case MENU_SUB_EXIT:
        break;
    case MENU_SUB_UNDO:
        break;
    case MENU_SUB_REDO:
        break;
    case MENU_SUB_CUT:
        break;
    case MENU_SUB_COPY:
        break;
    case MENU_SUB_PASTE:
        break;
    case MENU_SUB_DEL:
        break;
    case MENU_SUB_FIND:
        break;
    case MENU_SUB_REPLACE:
        break;
    case MENU_SUB_ABOUT:
        messagebox_manager("made by WR&FZY");
        break;
    default:
#ifdef DEBUG
        fprintf(stderr,"select func and run func failed.\n");
#endif
        return false;
    }
    return true;
}

static char far *sub_menu_show(menuptr m)
{
    menuptr root =  GetRootMenu(m);
    int n = MenuChildNum(root,m);
    int x1 = MAIN_MENU_X+(n-1)*MAIN_MENU_WIDTH;
    int y1 = SUB_MENU_Y;
    int x2 = x1 + 5*CONFIG_CN_SIZE + 3;
    int y2 = y1 + MenuChildCount(m)*(16+5)+3;
    int y = y1;
    char far *buf;
    hidemouseptr();
    buf = (char far *)farmalloc(imagesize(x1,y1,x2,y2));
    if(!buf)
    {
        perror("sub menu buffer farmalloc failed!");
        return NULL;
    }
    getimage(x1,y1,x2,y2,buf);

    m = FirsrChildMenu(m);
    setfillstyle(SOLID_FILL,LIGHTGRAY);
    do
    {
        bar(x1,y,x1 + 5*CONFIG_CN_SIZE, y+(16+5));
        print_str_xy(MenuName(m),x1 + 10,y + 5);
        fprintf(stderr, "%s\n", MenuName(m));
        y += 16 + 5;
    } while((m = NextBroMenu(m)) != NULL);

    rectangle(x1,y1,x2,y2);
    setcolor(BLACK);
    rectangle(x1,y1,x2-1,y2-1);
    setcolor(WHITE);
    rectangle(x1,y1,x2-2,y2-2);
    showmouseptr();
    return buf;
}

static bool sub_menu_hidden(char far *buf,int x,int y)
{
    if(!buf)
        return false;
    hidemouseptr();
    putimage(x,y,buf,COPY_PUT);
    farfree(buf);
    showmouseptr();
    return true;
}

void get_main_menu_choice(menuptr root)
{
    int prevchoice = 0, choice = 1;
    int nSubMenu = MenuChildCount(root);
    int x1 = MAIN_MENU_X;
    int y1 = MAIN_MENU_Y;
    int y2 = MAIN_MENU_Y + CONFIG_CN_SIZE;
    int x2 = x1 + nSubMenu*MAIN_MENU_WIDTH;
    int x,y,button,i,key;
    bool in = false; // if mouse is above main menu area.

    while(!kbhit())
    {
        getmousepos(&button,&x,&y);
        if(clickclosebutton(x,y,button))
        {
            cleardevice();
            closegraph();
            exit(1);
        }
        if(x>=x1 && x<=x2 && y>=y1 && y<=y2)
        {
            in = true;
            //calculte the no. of the main menu where mouse above.
            for(i = 1;i<=nSubMenu;i++)
            {
                if(x <= MAIN_MENU_X + MAIN_MENU_WIDTH*i)
                {
                    choice = i;
                    break;
                }
            }
            if(prevchoice!=choice)
            {
                hidemouseptr();
                if(prevchoice)
                    main_menu_change(prevchoice,MAIN_BORDER_QUIT);
                main_menu_change(choice,MAIN_BORDER_OUT);
                prevchoice=choice;
                showmouseptr();
            }
            if(button == MOUSE_LEFTPRESS)
            {
                while(button == MOUSE_LEFTPRESS)
                    /* if keep click main menu for a while.
                     do not show sub menu right now.*/
                    getmousepos(&button,&x,&y);
                if(x>=x1 && x<=x2 && y>=y1 && y<=y2)
                {

                    /* main menu is clicked. */
                    main_menu_change(choice,MAIN_BORDER_IN);
                    //hidemouseptr();
                    if(!view_run_func(GetMenuByNum(root,choice)))
                    {
                        outtext("run func failed.");
                    }
                    main_menu_change(choice,MAIN_BORDER_QUIT);
                    //showmouseptr();
                    return;
                }
            }
        }
        else
        {
            if(in)
            {
                in=false;
                prevchoice=0;
                hidemouseptr();
                main_menu_change(choice,MAIN_BORDER_QUIT);
                showmouseptr();
            }
        }
    }
    main_menu_change(choice,MAIN_BORDER_QUIT);
}

static void get_sub_menu_choice(menuptr m)
{
    menuptr root =  GetRootMenu(m);
    int n = MenuChildNum(root,m);
    int prevchoice = 0, choice = 1;
    int nSubMenu = MenuChildCount(m);
    int x1 = MAIN_MENU_X+(n-1)*MAIN_MENU_WIDTH;
    int y1 = SUB_MENU_Y;
    int x2 = x1 + 5*CONFIG_CN_SIZE + 3;
    int y2 = y1 + nSubMenu*(16+5)+3;
    int x,y,button,key;
    bool in; // if mouse is above main menu area.
    char far *buf;

    hidemouseptr();
    buf = sub_menu_show(m);
    showmouseptr();
    if(!buf)
    {
        fprintf(stderr,"Show sub menu may be failed.");
        return;
    }

    while(!kbhit())
    {
        getmousepos(&button,&x,&y);
        if(x<x1 || x>x2 || y<y1 || y>y2)
        {
            if(button == MOUSE_LEFTPRESS)
            {
                sub_menu_hidden(buf,x1,y1);
                return;
            }
        }
        if(x>=x1 && x<=x2 && y>=y1 && y<=y2)
        {
            int i;
            in = true;
            //calculte the no. of the main menu where mouse above.
            for(i = 1;i<=nSubMenu;i++)
            {
                if(y < y1 + (16+5)*i)
                {
                    choice = i;
                    break;
                }
            }
            if(prevchoice!=choice)
            {
                hidemouseptr();
                if(prevchoice)
                    sub_menu_change(x1,prevchoice,SUB_DEHIGHLIGHT);
                sub_menu_change(x1,choice,SUB_HIGHLIGHT);
                prevchoice=choice;
                showmouseptr();
            }
            if(button == MOUSE_LEFTPRESS)
            {
                while(button == MOUSE_LEFTPRESS)
                    /* if keep click main menu for a while.
                     do not show sub menu right now.*/
                    getmousepos(&button,&x,&y);
                if(x>=x1 && x<=x2 && y>=y1 && y<=y2)
                {
                    sub_menu_hidden(buf,x1,y1);
                    if(!view_run_func(GetMenuByNum(m,choice)))
                    {
                        outtext("run func failed.");
                    }
                    return;
                }
            }
        }
        else
        {
            if(in)
            {
                in=false;
                prevchoice=0;
                hidemouseptr();
                sub_menu_change(x1,choice,SUB_DEHIGHLIGHT);
                showmouseptr();
            }
        }
    }
    sub_menu_change(x1,choice,SUB_DEHIGHLIGHT);
}
void menu_key_manager(menuptr root)
{
    menuptr m;
    int mousex,mousey,button;
    int meuntype = MAINMENU;
    int choice = 1;
    int x1;
    int nSubMenu;
    char far *buf;
    while(getmousepos(&button,&mousex,&mousey) && !button)
    {
        if (bioskey(1))
        {
            int key = bioskey(0);
            if(meuntype == MAINMENU)
            {
                switch(key)
                {
                case LEFT:
                    main_menu_change(choice,MAIN_BORDER_QUIT);
                    if(choice)
                    {
                        if(choice != 1)
                            choice--;
                        main_menu_change(choice,MAIN_BORDER_OUT);
                    }
                    break;
                case RIGHT:
                    main_menu_change(choice,MAIN_BORDER_QUIT);
                    if (choice < CONFIG_MAIN_MEMU_NUM)
                    {
                        choice++;
                    }
                    main_menu_change(choice,MAIN_BORDER_OUT);
                    break;
                case ENTER:
                    main_menu_change(choice,MAIN_BORDER_IN);
                    meuntype = SUBMENU;
                    x1 = MAIN_MENU_X+(choice-1)*MAIN_MENU_WIDTH;
                    m = GetMenuByNum(root,choice);
                    buf = sub_menu_show(m);
                    nSubMenu = MenuChildCount(m);
                    choice = 0;
                    break;
                case ESC:
                    exit(1);
                }
            }
            if(meuntype == SUBMENU)
            {
                switch(key)
                {
                case UP:
                    if(choice){
                        sub_menu_change(x1,choice,SUB_DEHIGHLIGHT);
                        choice--;
                        if(!choice)
                        {
                            sub_menu_hidden(buf,x1,SUB_MENU_Y);
                            meuntype = MAINMENU;
                            choice = MenuChildNum(root,m);
                            main_menu_change(choice,MAIN_BORDER_OUT);
                        }
                        else
                            sub_menu_change(x1,choice,SUB_HIGHLIGHT);
                    }
                    break;
                case DOWN:
                    if(choice)
                        sub_menu_change(x1,choice,SUB_DEHIGHLIGHT);
                    if (choice < nSubMenu)
                    {
                        choice++;
                    }
                    sub_menu_change(x1,choice,SUB_HIGHLIGHT);
                    break;
                case ENTER:
                    if(choice){
                        sub_menu_hidden(buf,x1,SUB_MENU_Y);
                        view_run_func(GetMenuByNum(m,choice));
                        choice = MenuChildNum(root,m);
                        main_menu_change(choice,MAIN_BORDER_QUIT);
                        return;
                    }
                    break;
                case ESC:
                    exit(1);
                }
            }

        }
    }
    if (meuntype == MAINMENU)
    {
        main_menu_change(choice,MAIN_BORDER_QUIT);
    }
    if (meuntype == SUBMENU)
    {
        sub_menu_hidden(buf,x1,SUB_MENU_Y);
        choice = MenuChildNum(root,m);
        main_menu_change(choice,MAIN_BORDER_QUIT);
    }
}
#define MESSAGEBOX_MINX 180
#define MESSAGEBOX_MINY 150
#define MESSAGEBOX_MAXX 480
#define MESSAGEBOX_MAXY 250
static void messagebox_hidden(char far *buf)
{
    if(!buf)
        return;
    hidemouseptr();
    putimage(MESSAGEBOX_MINX,MESSAGEBOX_MINY,buf,COPY_PUT);
    farfree(buf);
    showmouseptr();
}

static char far *messagebox_show(char *message)
{
    char far *buf;
    buf = (char far*)farmalloc(imagesize(MESSAGEBOX_MINX,MESSAGEBOX_MINY,MESSAGEBOX_MAXX+2,MESSAGEBOX_MAXY+2));
    if(!buf)
        return NULL;
    hidemouseptr();
    getimage(MESSAGEBOX_MINX,MESSAGEBOX_MINY,MESSAGEBOX_MAXX+2,MESSAGEBOX_MAXY+2,buf);
    //biggest window.
    draw_shadow(MESSAGEBOX_MINX,MESSAGEBOX_MINY,MESSAGEBOX_MAXX,MESSAGEBOX_MAXY,SHADOW_OUT,LIGHTGRAY);
    //name bar.
    setfillstyle(SOLID_FILL,BLUE);
    bar(MESSAGEBOX_MINX+3,MESSAGEBOX_MINY+3,MESSAGEBOX_MAXX-3,MESSAGEBOX_MINY+19);
    //editor name.
    settextstyle(SMALL_FONT,0,5);
    setcolor(WHITE);
    outtextxy(MESSAGEBOX_MINX+25,MESSAGEBOX_MINY+2,EDITOR_NAME);
    //button
    draw_closebutt(MESSAGEBOX_MAXX-15,MESSAGEBOX_MINY+8);
    draw_button(MESSAGEBOX_MINX+110,MESSAGEBOX_MINY+60,"ȷ?");
    //icon
    draw_icon(MESSAGEBOX_MINX+7,MESSAGEBOX_MINY-1);
    print_str_xy(message,MESSAGEBOX_MINX + 20,MESSAGEBOX_MINY +30);
    showmouseptr();
    return buf;
}


#define BUTTON_SIZE_X 46
#define BUTTON_SIZE_Y 23
void messagebox_manager(char * message)
{
    int button,mousex,mousey;
    char far *buf = messagebox_show(message);
    while(1)
    {
        if(!getmousepos(&button,&mousex,&mousey))
            continue;
        if(bioskey(1) && bioskey(0)==ENTER)
        {
            messagebox_hidden(buf);
            return;
        }
        else if(button == MOUSE_LEFTPRESS)
        {
            if(mousex>=MESSAGEBOX_MINX+110&&mousex<=MESSAGEBOX_MINX+110+BUTTON_SIZE_X\
                &&mousey>=MESSAGEBOX_MINY+60&&mousey<=MESSAGEBOX_MINY+60+BUTTON_SIZE_Y)
            { //yes button
                messagebox_hidden(buf);
                return;
            }
            if(mousex>=MESSAGEBOX_MAXX-20 && mousex<=MESSAGEBOX_MAXX-6 \
                && mousey>=MESSAGEBOX_MINY-1 && mousey<=MESSAGEBOX_MINY+17)
			{ //close button
                messagebox_hidden(buf);
                return;
            }
        }
    }
}



static void inputbox_hidden(char far* buf)
{
    if(!buf)
        return;
    hidemouseptr();
    putimage(INPUTBOX_MINX,INPUTBOX_MINY,buf,COPY_PUT);
    farfree(buf);
    showmouseptr();
}

static void draw_button(int x1,int y1,char *str)
{
    int x2 = x1 + BUTTON_SIZE_X;
    int y2 = y1 + BUTTON_SIZE_Y;

    setcolor(WHITE);
    line(x1,y1,x2,y1);
    line(x1,y1,x1,y2);
    setcolor(DARKGRAY);
    line(x2,y2,x2,y1);
    line(x2,y2,x1,y2);
    print_str_xy(str,x1+8,y1+3);
}

static char far *inputbox_show(char *message)
{
	char far *buf;
    buf = (char far*)farmalloc(imagesize(INPUTBOX_MINX,INPUTBOX_MINY,INPUTBOX_MAXX+2,INPUTBOX_MAXY+2));
    if(!buf)
        return NULL;
    hidemouseptr();
    getimage(INPUTBOX_MINX,INPUTBOX_MINY,INPUTBOX_MAXX+2,INPUTBOX_MAXY+2,buf);
    //biggest window.
    draw_shadow(INPUTBOX_MINX,INPUTBOX_MINY,INPUTBOX_MAXX,INPUTBOX_MAXY,SHADOW_OUT,LIGHTGRAY);
    //name bar.
    setfillstyle(SOLID_FILL,BLUE);
    bar(INPUTBOX_MINX+3,INPUTBOX_MINY+3,INPUTBOX_MAXX-3,INPUTBOX_MINY+19);
    //editor name.
    settextstyle(SMALL_FONT,0,5);
    setcolor(WHITE);
    outtextxy(INPUTBOX_MINX+25,INPUTBOX_MINY+2,EDITOR_NAME);
    //edit window
    draw_shadow(INPUTBOX_EDIT_MINX,INPUTBOX_EDIT_MINY,INPUTBOX_EDIT_MAXX,INPUTBOX_EDIT_MAXY,SHADOW_IN,WHITE);
    //button
    draw_closebutt(INPUTBOX_MAXX-15,INPUTBOX_MINY+8);
    draw_button(INPUTBOX_EDIT_MINX-2,INPUTBOX_EDIT_MAXY+15,"ȷ?");
    draw_button(INPUTBOX_EDIT_MAXX-40,INPUTBOX_EDIT_MAXY+15,"ȡ?");
    //icon
    draw_icon(INPUTBOX_MINX+7,INPUTBOX_MINY-1);
	print_str_xy(message,INPUTBOX_MINX + 5,INPUTBOX_MINY +30);
    showmouseptr();
    return buf;
}
#define INPUTBOX_TEXT_SIZE 30
char *inputbox_manager(char *message)
{
    int button,mousex,mousey;
    char far *buf = inputbox_show(message);
	char* str = (char *)calloc(INPUTBOX_TEXT_SIZE,1);
    int i = 0;
    int x,y;
    int key;
    while(1)
    {
        if(!getmousepos(&button,&mousex,&mousey))
            continue;
        if(bioskey(1) && (key=bioskey(0)%(1<<8)) && (key < 128))
        {
            if(key==0x0d)//ENTER
            {
                inputbox_hidden(buf);
                return str;
            }
            if(key==0x08)//BACKSPACE
            {
                if(i){
                    i--;
                    str[i] = '\0';
                }
            }
            else if (i < INPUTBOX_TEXT_SIZE && key>=0x20)//input common char.
            {
                str[i] = key;
                i++;
            }
            setcolor(BLACK);
            setfillstyle(SOLID_FILL,WHITE);
            bar(INPUTBOX_EDIT_MINX+2,INPUTBOX_EDIT_MINY+3,INPUTBOX_EDIT_MAXX-2,INPUTBOX_EDIT_MAXY-2);
            settextstyle(SMALL_FONT,0,5);
            outtextxy(INPUTBOX_EDIT_MINX+2,INPUTBOX_EDIT_MINY+2,str);
        }
        else if(button == MOUSE_LEFTPRESS)
        {
            if(mousex>=INPUTBOX_EDIT_MINX-2&&mousex<=INPUTBOX_EDIT_MINX-2+BUTTON_SIZE_X\
                &&mousey>=INPUTBOX_EDIT_MAXY+15&&mousey<=INPUTBOX_EDIT_MAXY+15+BUTTON_SIZE_Y)
            { //yes button
                inputbox_hidden(buf);
                return str;
            }
            if(mousex>=INPUTBOX_EDIT_MAXX-40&&mousex<=INPUTBOX_EDIT_MAXX-40+BUTTON_SIZE_X\
                &&mousey>=INPUTBOX_EDIT_MAXY+15&&mousey<=INPUTBOX_EDIT_MAXY+15+BUTTON_SIZE_Y)
            { //cancel button
                free(str);
                inputbox_hidden(buf);
                return NULL;
            }
            if(mousex>=INPUTBOX_MAXX-20 && mousex<=INPUTBOX_MAXX-6 \
                && mousey>=INPUTBOX_MINY-1 && mousey<=INPUTBOX_MINY+17)
            { //close button
                free(str);
                inputbox_hidden(buf);
                return NULL;
            }
        }
    }
}

