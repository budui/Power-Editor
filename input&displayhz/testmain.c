#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
extern unsigned int receivehz();
extern void displayhz(unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
void main()
{
	int i,gdriver=DETECT,gmode;
	unsigned int a;
	initgraph(&gdriver,&gmode,"c:\\borlandc\\bgi");
	cleardevice();
	moverel(1,16);
	for(i=0;i<40;i++)
	{
		a=receivehz();
		displayhz(a,2,1+i*16,16,1,15);//painting brush moves to next hanzi
		moverel(24,0);
	}
	closegraph();
}