#include <graphics.h>
#include <stdio.h>
void linefeed(int *x,int *y,int width,int height)
{
	if(*x>getmaxx())
	{
		*x=0;
		*y+=height;
	}
	else if(*x<0)
	{
		*x=getmaxx()+1-width;
		*y-=height;
	}
	if(*y<height)
	{

		*y=height;
	}
}