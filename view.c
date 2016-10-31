#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <errno.h>
#include "config.h"
#include "view.h"
#include "print.h"

#define SHADOW_LIGHT 1
#define SHADOW_HARD 2
#ifdef DEBUG
extern FILE *logfile;
#endif // DEBUG
typedef struct{
    int x;
    int y;
} point;


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

void view_main_menu(const menuptr root,LANGUAGE lang)
{
    menuptr m = FirsrChildMenu(root);
    int x = CONFIG_MAIN_MENU_X,y = CONFIG_MAIN_MENU_Y;
    do
	{
		print_str_xy(MenuName(m),x,y);
        if(lang == CHINESE)
            x += 3 * CONFIG_CN_SIZE;
        else
            x += 3 * CONFIG_EN_SIZE;
    } while((m = NextBroMenu(m)) != NULL);
    FreeMenu(root);
}

char *view_sub_menu_show(menuptr m)
{
    menuptr root =  GetRootMenu(m);
    int n = MenuChildNum(root,m);
    int x = CONFIG_MAIN_MENU_X-2+(n-1)*CONFIG_CN_SIZE*3;
    int y = CONFIG_MAIN_MENU_Y + CONFIG_CN_SIZE + 2;
    int y0 = y ;
    char  *buf;
    size_t size;
    size = imagesize(x,y,x + 5*CONFIG_CN_SIZE + 3,y+(16+5)*7+3);
    buf = malloc(size);
    if(!buf)
    {
		perror("");
        return NULL;
    }
        
    getimage(x,y,x + 5*CONFIG_CN_SIZE + 3,y+(16+5)*7+3,buf);

    m = FirsrChildMenu(m);
    setfillstyle(SOLID_FILL,LIGHTGRAY);
    
    do
    {
        bar(x,y,x + 5*CONFIG_CN_SIZE, y+(16+5));
        print_str_xy(MenuName(m),x + 10,y + 5);
        y += 16 + 5;
    } while((m = NextBroMenu(m)) != NULL);

    rectangle(x,y0,x + 5*CONFIG_CN_SIZE + 3,y+3);
    setcolor(BLACK);
    rectangle(x,y0,x + 5*CONFIG_CN_SIZE + 2,y+2);
    setcolor(WHITE);
    rectangle(x,y0,x + 5*CONFIG_CN_SIZE + 1,y+1);
    return buf;
}

bool view_sub_menu_hidden(char *buf,menuptr m)
{
    menuptr root =  GetRootMenu(m);
	int n = MenuChildNum(root,m);
    int x = CONFIG_MAIN_MENU_X-2+(n-1)*CONFIG_CN_SIZE*3;
    int y = CONFIG_MAIN_MENU_Y + CONFIG_CN_SIZE + 2;

    if(!buf)
        return false;
	putimage(x,y,buf,COPY_PUT);
    free(buf);
    return true;
}