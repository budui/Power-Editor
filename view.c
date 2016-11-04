#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <errno.h>

#include "view.h"
#include "print.h"
#include "mouse.h"
#include "config.h"

#define SHADOW_LIGHT 1
#define SHADOW_HARD 2

/* flag for func main_menu_change */
#define MAIN_BORDER_IN 1
#define MAIN_BORDER_OUT 2
#define MAIN_BORDER_QUIT 0
/* flag for func sub_menu_change */
#define SUB_DEHIGHLIGHT 0
#define SUB_HIGHLIGHT 1

#define MAIN_MENU_WIDTH 48

typedef struct{
    int x;
    int y;
} point;

static void sub_menu_change(int x1,int y1,int flag);
static void main_menu_change(int x1, int flag);
//void get_sub_menu_choice(menuptr m);
static void closebutton(int x1,int y1);
static void showicon(int x1,int y1);
static void shadow_inner(point left,point right,colors inner,int flag);

static bool point_init(point *p, int x, int y);
static bool point2int(point p,int *x,int *y);




static void closebutton(int x,int y)
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

static void showicon(int x1,int y1)
{
    setcolor(YELLOW);
    outtextxy(x1,y1,"/");
    outtextxy(x1+1,y1,"/");
    outtextxy(x1+2,y1+2,"/");
    outtextxy(x1+3,y1+2,"/");
    outtextxy(x1+4,y1+4,"/");
    outtextxy(x1+5,y1+4,"/");
}

static void shadow_outter(point left,point right,colors inner,int flag)
{
    int x1,x2,y1,y2;
    point2int(left,&x1,&y1);
    point2int(right,&x2,&y2);
    switch(flag)
    {
    case SHADOW_HARD:
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
    case SHADOW_LIGHT:
        setfillstyle(SOLID_FILL,inner);
        setcolor(WHITE);
        rectangle(x1-1, y1-1, x2, y2);
        setcolor(DARKGRAY);
        rectangle(x1, y1, x2+1, y2+1);
        bar(x1, y1, x2, y2);
        break;
    }

}

static void shadow_inner(point left,point right,colors inner,int flag)
{
    int x1,x2,y1,y2;
    point2int(left,&x1,&y1);
    point2int(right,&x2,&y2);
    switch(flag)
    {
    case SHADOW_HARD:
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
    case SHADOW_LIGHT:
        break;
    }

}

static bool point_init(point *p, int x, int y)
{
    if(!p)
        return false;
    p->x = x;
    p->y = y;
    return true;
}

static bool point2int(point p,int *x,int *y)
{
    if(!x || !y)
        return false;
    *x = p.x;
    *y = p.y;
    return true;
}

static void main_menu_change(int x1, int flag)
{
    int y1 = CONFIG_MAIN_MENU_Y-2;
    int x2 = x1 + MAIN_MENU_WIDTH-7;
    int y2 = CONFIG_MAIN_MENU_Y + CONFIG_CN_SIZE+1;
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

static void sub_menu_change(int x1,int y1,int flag)
{
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
bool VGA_INIT(void)
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


bool clickclosebutton(int x,int y,int button)
{
    int x1 = CONFIG_WINDOW_MAXX - 15 - 5;
    int y1 = 0;
    int x2 = CONFIG_WINDOW_MAXX - 15 + 9;
    int y2 = 18;
    if(button != MOUSE_LEFTPRESS)
        return false;
    return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

void view_main_window(const char *name, colors barcolor)
{
    point tpl,tpr; //temp point left and temp point right.
    int x1 = 1,y1 = 1, x2 = CONFIG_WINDOW_MAXX, y2 = CONFIG_WINDOW_MAXY;

    //biggest window.
    point_init(&tpr,x2,y2);
    point_init(&tpl,x1 + 1,x1 + 1);
    shadow_outter(tpl,tpr,LIGHTGRAY,SHADOW_LIGHT);
    //name bar.
    setfillstyle(SOLID_FILL,barcolor);
    bar(x1+3,y1+3,x2-3,y1+19);
    //editor name.
    settextstyle(SMALL_FONT,0,5);
    setcolor(WHITE);
    outtextxy(x1+25,y1+2,name);
    //edit window
    point_init(&tpr,x2 - 7,y2 - 22);
    point_init(&tpl,x1 + 4,y1 + 39);
    shadow_outter(tpl,tpr,WHITE,SHADOW_HARD);
    //button
    closebutton(x2-15,y1+8);
    //icon
    showicon(x1+7,y1);
}

int view_main_menu(const menuptr root)
{
    menuptr m = FirsrChildMenu(root);
    int x = CONFIG_MAIN_MENU_X,y = CONFIG_MAIN_MENU_Y;
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
    case MENU_MAIN_OPTION:
    case MENU_MAIN_HELP:
        get_sub_menu_choice(m);
        break;
    case MENU_SUB_NEWFILE:
		outtextxy(100,100,"aaaa");
        break;
    case MENU_SUB_OPENFILE:
        break;
    case MENU_SUB_SAVE:
        break;
    case MENU_SUB_SAVEAS:
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
    case MENU_SUB_SELECTALL:
        break;
    case MENU_SUB_TIME:
        break;
    case MENU_SUB_AUTOLINE:
        break;
    case MEUN_SUB_FONT:
        break;
    case MENU_SUB_HOTKEY:
        break;
    case MENU_SUB_ABOUT:
        break;
    default:
#ifdef DEBUG
        fprintf(stderr,"select func and run func failed.\n");
#endif
        return false;
    }
	return true;
}

void get_main_menu_choice(menuptr root)
{
    int prevchoice = 0, choice = 1;
    int nSubMenu = MenuChildCount(root);
    int x1 = CONFIG_MAIN_MENU_X;
    int y1 = CONFIG_MAIN_MENU_Y;
    int y2 = CONFIG_MAIN_MENU_Y + CONFIG_CN_SIZE;
    int x2 = x1 + nSubMenu*MAIN_MENU_WIDTH;
    int x,y,button,i;
    bool in = false; // if mouse is above main menu area.

    while(1)
    {
        getmousepos(&button,&x,&y);getmousepos(&button,&x,&y);
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
                if(x <= CONFIG_MAIN_MENU_X + MAIN_MENU_WIDTH*i)
                {
                    choice = i;
                    break;
                }
            }
            if(prevchoice!=choice)
            {
                hidemouseptr();
                if(prevchoice)
                    main_menu_change(CONFIG_MAIN_MENU_X + (prevchoice-1)*MAIN_MENU_WIDTH,MAIN_BORDER_QUIT);
                main_menu_change(CONFIG_MAIN_MENU_X + (choice-1)*MAIN_MENU_WIDTH,MAIN_BORDER_OUT);
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
                    main_menu_change(CONFIG_MAIN_MENU_X + (choice-1)*MAIN_MENU_WIDTH,MAIN_BORDER_IN);
                    if(!view_run_func(GetMenuByNum(root,choice)))
                    {
#ifdef DEBUG
                        outtext("run func failed.");
#endif
                    }
                    main_menu_change(CONFIG_MAIN_MENU_X + (choice-1)*MAIN_MENU_WIDTH,MAIN_BORDER_QUIT);
                    showmouseptr();
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
                main_menu_change(CONFIG_MAIN_MENU_X + (choice-1)*MAIN_MENU_WIDTH,MAIN_BORDER_QUIT);
                showmouseptr();
            }
        }
    }
}

void get_sub_menu_choice(menuptr m)
{
    menuptr root =  GetRootMenu(m);
    int n = MenuChildNum(root,m);
    int prevchoice = 0, choice = 1;
    int nSubMenu = MenuChildCount(m);
    int x1 = CONFIG_MAIN_MENU_X+(n-1)*MAIN_MENU_WIDTH;
    int y1 = CONFIG_SUB_MENU_Y;
    int x2 = x1 + 5*CONFIG_CN_SIZE + 3;
    int y2 = y1 + nSubMenu*(16+5)+3;
    int x,y,button;
    bool in; // if mouse is above main menu area.
    char far *buf;

    hidemouseptr();
    buf = sub_menu_show(m);
    showmouseptr();
    if(!buf)
    {
#ifdef DEBUG
        fprintf(stderr,"Show sub menu may be failed.");
#endif
        return;
    }

    while(1)
    {
        getmousepos(&button,&x,&y);
        if(x<x1 || x>x2 || y<y1 || y>y2)
        {
            if(button == MOUSE_LEFTPRESS)
            {
                hidemouseptr();
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
					sub_menu_change(x1,y1 + (16+5)*(prevchoice-1),SUB_DEHIGHLIGHT);
				sub_menu_change(x1,y1 + (16+5)*(choice-1),SUB_HIGHLIGHT);
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
#ifdef DEBUG
						outtext("run func failed.");
#endif
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
                sub_menu_change(x1,y1 + (16+5)*(choice-1),SUB_DEHIGHLIGHT);
                showmouseptr();
            }
        }
    }
}

char far *sub_menu_show(menuptr m)
{
    menuptr root =  GetRootMenu(m);
    int n = MenuChildNum(root,m);
    int x1 = CONFIG_MAIN_MENU_X+(n-1)*MAIN_MENU_WIDTH;
    int y1 = CONFIG_SUB_MENU_Y;
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

bool sub_menu_hidden(char far *buf,int x,int y)
{
    if(!buf)
        return false;
    hidemouseptr();
	putimage(x,y,buf,COPY_PUT);
    farfree(buf);
    showmouseptr();
    return true;
}
