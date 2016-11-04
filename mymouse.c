#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <conio.h>
#include <graphics.h>
#include "mouse.h"



int hx,hy;
bool hidden = false;

static void draw_cursor(int x,int y);

bool initmouse(void)
{
    _AX = 0x00;
    geninterrupt(0x33);
    if(!_AX)
    {
        fprintf(stderr,"mouse init failed.\n");
        return false;                
    }
    return true;
}


bool getmousepos(int *button,int *mx, int *my)
{
  union REGS i,o;
	int x0=*mx, y0=*my, button0=*button;
    int xnew, ynew;
    do // if mouse state not change, keep in loop.
    {
        i.x.ax=3;
        int86(0x33,&i,&o);
        xnew=o.x.cx;
        ynew=o.x.dx;
        *button = o.x.bx;
    }while(xnew == x0 && ynew == y0 && *button == button0);
    *mx=xnew;
    *my=ynew;

    /*            
    i.x.ax=3;
    int86(0x33,&i,&o);
    *button=o.x.bx;
    *mx=o.x.cx;
    *my=o.x.dx;*/
    if(hidden)
        draw_cursor(hx,hy);
    draw_cursor(x0,y0);
	draw_cursor(*mx,*my);
	if((*button & 1)==1)
            *button = 1;
    if(*button)
        return true;
    else
        return false;
}

void static draw_cursor(int x,int y)
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
/*
int main()
{
    int driver=VGA;
    int mode=VGAHI;
    int x,y,button=0;
    initgraph(&driver,&mode,"C:\\BORLANDC\\BGI");
    cleardevice();
    initmouse();
    
    setbkcolor(BLUE);
    setcolor(WHITE);
    bar(100,100,200,200);
    while(1)
    {
        getmousepos(&button,&x,&y);
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
	    default:
	        putpixel(x,y,7);
	        break;
        }
        if(button)
        {
            char str[5];
		    sprintf(str,"%d",button);
		    outtext(str);
        }
		if(x>=330&& x<=640 &&y>=12&&y<=480&& button)
        {
            hidemouseptr();
            getch();
            showmouseptr();
            getch();
        }
        if(x>=0&& x<=10 &&y>=0&&y<=12&& button)
        {
            cleardevice();
            closegraph();
            exit(1);
        }
    }
}*/

int showmouseptr(void)
{
    union REGS i,o;
    int x,y,button;
    i.x.ax=1;
    int86(0x33,&i,&o);
    //while(!getmousepos(&button,&x,&y));
    i.x.ax=3;
    int86(0x33,&i,&o);
    x=o.x.cx;
    y=o.x.dx;
    draw_cursor(hx,hy);
   
    return(o.x.ax);
}
int hidemouseptr(void)
{
    union REGS i,o;
    int x,y;
    i.x.ax=3;
    int86(0x33,&i,&o);
    x=o.x.cx;
    y=o.x.dx;
    draw_cursor(x,y);
    hx = x;
    hy = y;
    i.x.ax=2;
    int86(0x33,&i,&o);
    return(o.x.ax);
}
