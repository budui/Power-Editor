#include <stdio.h>
#include <dos.h>
#include <bios.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include "mouse.h"
static bool ishidden = false;
static int oldx = 900,oldy = 900;

void draw_cursor(int x,int y)
{
    setcolor(WHITE);
    setwritemode(XOR_PUT);
    setlinestyle(0,0,1);

    setcolor(BLACK);

    line(x,y,x+11,y+11);
    line(x+10,y+11,x+8,y+11);
    line(x+7,y+11,x+10,y+19);
    line(x+9,y+20,x+8,y+20);
    line(x+7,y+19,x+4,y+12);
    line(x+3,y+13,x+1,y+15);
    line(x,y+16,x,y+1);

    setcolor(WHITE);

    line(x+1,y+2,x+1,y+14);
    line(x+2,y+3,x+2,y+13);
    line(x+3,y+4,x+3,y+12);
    line(x+4,y+5,x+4,y+11);
    line(x+5,y+6,x+5,y+13);
    line(x+6,y+7,x+6,y+15);
    line(x+7,y+8,x+7,y+10);
    line(x+7,y+14,x+7,y+17);
    line(x+8,y+9,x+8,y+10);
    line(x+8,y+16,x+8,y+19);
    line(x+9,y+10,x+9,y+10);
    line(x+9,y+18,x+9,y+19);

    setwritemode(COPY_PUT);
}

bool initmouse(void)
{
    freopen("gui.log","w",stdout);
    _AX = 0x00;
    geninterrupt(0x33);
    if(!_AX)
    {
        fprintf(stderr,"mouse init failed.\n");
        return false;
    }
    //showmouseptr();
    return true;
}
int showmouseptr(void)
{
    union REGS i,o;
    i.x.ax=1;
    int86(0x33,&i,&o);
    draw_cursor(oldx,oldy);
    ishidden = false;
    return(o.x.ax);
}
int hidemouseptr(void)
{
    union REGS i,o;
    i.x.ax=2;
    int86(0x33,&i,&o);
    draw_cursor(oldx,oldy);
    ishidden = true;
    return(o.x.ax);
}

bool getmousepos(int *button,int *x,int *y)
{
    union REGS i,o;
    int x0 = *x ,y0 = *y,button0 = *button;

    do // if mouse state not change, keep in loop.
    {
        i.x.ax=3;
        int86(0x33,&i,&o);
        *x=o.x.cx;
        *y=o.x.dx;
        *button = o.x.bx;
        if(kbhit())
            return true;
    }while(*x == x0 && *y == y0 && *button == button0 || ishidden);

    if(!ishidden){
        draw_cursor(oldx,oldy);
        oldx = *x;
        oldy = *y;
        draw_cursor(*x,*y);
    }
    return *button;
}

/*
int main()
{
    int driver=VGA;
    int mode=VGAHI;
    int x = 320,y = 240,button=0;
    int hidden = 0;
    freopen("gui.log","w",stdout);
    initgraph(&driver,&mode,"C:\\BORLANDC\\BGI");
    cleardevice();
    initmouse();
    setbkcolor(BLACK);
    setcolor(WHITE);
    bar(100,100,200,200);
    while(1)
    {
        getmousepos(&button,&x,&y);
		if(bioskey(1))
		{
			bioskey(0);
            if(!hidden)
            {
                hidemouseptr();
                hidden = 1;
                outtext("hidden");
            }
            else
            {
                showmouseptr();
                hidden = 0;
                outtext("show");
            }
        }
        switch(button)
        {
        case MOUSE_NOPRESS:
	        break;
        case MOUSE_LEFTPRESS:
	        circle(x,y,6);
	        break;
	    case MOUSE_RIGHTPRESS:
	        rectangle(x,y,x+12,y+12);
	        break;
        }
        if(button)
        {
            char str[5];
		    sprintf(str,"%d",button);
		    outtext(str);
        }
        if(x>=0&& x<=10 &&y>=0&&y<=12&& button)
        {
            cleardevice();
            closegraph();
            exit(1);
        }
    }
}
*/
