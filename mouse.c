#include <stdio.h>
#include <dos.h>
#include <bios.h>
#include <conio.h>
#include "mouse.h"
#include <graphics.h>

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
int showmouseptr(void)
{
    union REGS i,o;
    i.x.ax=1;
    int86(0x33,&i,&o);
    return(o.x.ax);
}
int hidemouseptr(void)
{
union REGS i,o;
    i.x.ax=2;
    int86(0x33,&i,&o);
    return(o.x.ax);
}

bool getmousepos(int *button,int *x,int *y)
{
union REGS i,o;
    i.x.ax=3;
    int86(0x33,&i,&o);
    *button=o.x.bx;
    *x=o.x.cx;
    *y=o.x.dx;
    return *button;
}
