/*用于在图形界面显示txt文本，字号问题未解决*/

#include <stdio.h>
#include <graphics.h>
#include<conio.h>
void displayen(unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
displayhz(unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
void displaytxt()
//void main()
{
	int gdriver=DETECT,gmode,row=0,column=16,xmax;
	FILE *fp=fopen("thefile.txt","rt");
	unsigned char c,d;
	unsigned int innercode;
	initgraph(&gdriver,&gmode,"c:\\borlandc\\bgi");
	cleardevice();
	xmax=getmaxx();
	while(1)
	{
		fread(&c,1,1,fp);
		if(feof(fp))
		{
			getch();
			break;
		}
		if(c>>7==1)
		{
			fread(&d,1,1,fp);
			innercode=c*256+d;
			displayhz(innercode,2,row,column,1,15);
			row+=16;
		}
		else
		{
			displayen(c,0,row,column,1,15);
			row+=8;
		}
		if(row>xmax)
		{
			row=0;
			column+=16;
		}
	}
	fclose(fp);
}