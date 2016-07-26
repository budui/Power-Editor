#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
void main()
{
	int i,gdriver=DETECT,gmode;
	initgraph(&gdriver,&gmode,"c:\\borlandc\\bgi");
	cleardevice();
	for(i=0;i<1;i++)
	displayhz(1+i*16,1,1,15);//画笔移至下一个汉字处
}