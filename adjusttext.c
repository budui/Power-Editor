#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <graphics.h>
struct fonts
{
	unsigned char hzk;
	FILE *fp;
	unsigned char height;
	unsigned char width;
	unsigned int bytes;
};

unsigned char  hzoren(int pixelx,int pixely,unsigned char *text1,int width,int height);
void displayen(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
/*用来调节插入删除后后面字位置的变化*/
void adjusttext(int x0,int y0,int length0,int height,struct fonts *font,void *charu,unsigned char *text)//x0,y0为要插入位置的底部坐标
{
	int xmax=getmaxx(),i=0,temp,nextlength,length,flag=0;
	int ymax=getmaxy();
	int mallocsize=imagesize(0,0,xmax,font->height);
	void *a=malloc(mallocsize);
	void *b=malloc(mallocsize);
	void *c=malloc(mallocsize);
	void *d=malloc(mallocsize);
	int length1=xmax-x0-length0;
	int length3=xmax-length0;
	int currenty=y0-font->height;
	int currentx=x0;
	height-=font->height;

	if(currenty<0)
		currenty=0;
	if(currentx<0)
		currentx=0;
	if(length0>=0)
	{
		if(hzoren(x0+length1+2,currenty+2,text,font->width,font->height)==3)////////右汉字
			length1-=font->width;
		getimage(x0,currenty,x0+length1,currenty+font->height,c);
		if(hzoren(xmax-2,currenty+2,text,font->width,font->height)==4)
			temp=xmax-font->width;
		else
			temp=xmax;
		getimage(x0+length1,currenty,temp,currenty+font->height,d);//长度length第一行
		length=temp-x0-length1;
		length3=xmax-temp+x0+length1;
		if(charu!=NULL)
			putimage(x0,currenty,charu,COPY_PUT);
		putimage(x0+length0,currenty,c,COPY_PUT);//第一行处理玩
		memmove(a,d,mallocsize);
		currenty+=font->height;

		while(1)
		{
			if(currenty+font->height>ymax)
			{
				break;
			}
			if(hzoren(length3+2,currenty+2,text,font->width,font->height)==3)
			{
				if(flag==0)
					length3-=font->width;
				else if(flag==1)
					length3+=font->width;
			}

			flag=0;
			getimage(0,currenty,length3,currenty+font->height,c);
			if(hzoren(xmax-2,currenty+2,text,font->width,font->height)==4)
			{//printf("%d %d",xmax-2,currenty+2);
				flag=1;
				temp=xmax-font->width;}

			else
				temp=xmax;
			getimage(length3,currenty,temp,currenty+font->height,d);
			if(hzoren(xmax-font->width-2,currenty+2,text,font->width,font->height)==5)
			{
				length3=temp;
				getimage(0,0,0,0,d);
			}
			nextlength=temp-length3;
			putimage(0,currenty,a,COPY_PUT);
			memmove(a,d,mallocsize);//memmove(void *dest,void *b,int bytes);//把b指向的大小为bytes的内容移至dest
			putimage(length,currenty,c,COPY_PUT);//第二行处理完
			currenty+=font->height;
			length=nextlength;
		}

	}
	else if(length0<0)
	{
		getimage(0,y0-font->height,xmax,y0,b);
		length1=xmax-x0;
		if(hzoren(xmax-2,currenty+2,text,font->width,font->height)==4)
		{
			length1-=font->width;
			flag=1;
		}
		getimage(x0,currenty,x0+length1,currenty+font->height,c);
		length3=xmax-(x0+length1);
		if(hzoren(length3+2,currenty+font->height+2,text,font->width,font->height)==3)
		{
			if(flag==0)
				length3-=font->width;
			else if(flag==1)
				length3+=font->width;
		}
		flag=0;
		getimage(0,currenty+font->height,length3,currenty+2*font->height,d);
		putimage(x0+length0,currenty,c,COPY_PUT);
		putimage(x0+length0+length1,currenty,d,COPY_PUT);
		currenty+=font->height;

		while(1)
		{
			if(currenty+font->height>ymax)
			{
				break;
			}
			if(hzoren(xmax-2,currenty+2,text,font->width,font->height)==4)
			{
				temp=xmax-font->width;
				flag=1;
			}
			else
			{
				temp=xmax;
				flag=0;
			}
			getimage(length3,currenty,temp,currenty+font->height,c);
			putimage(0,currenty,c,COPY_PUT);
			length1=length3;
			if(hzoren(length3+2,currenty+2,text,font->width,font->height)==3)
			{
				if(flag==1)
					length1+=font->width;
				else if(flag==0)
					length1-=font->width;
			}
			flag=0;
			getimage(0,currenty+font->height,length1,currenty+2*font->height,d);
			putimage(temp-length3,currenty,d,COPY_PUT);
			currenty+=font->height;
		}
		
	}
	currenty=y0;
	i=0;
	while(i<=height)
	{
		currenty=y0+i;
		getimage(0,currenty-font->height,xmax,currenty,a);
		while(currenty<ymax)
		{
		
			currenty+=height;
			if(currenty>ymax)
				break;
			getimage(0,currenty-font->height,xmax,currenty,c);
			putimage(0,currenty-font->height,a,COPY_PUT);
			memmove(a,c,mallocsize);
		}
			i+=font->height;
	}
	if(length<0&&height>0)
		putimage(0,y0-font->height,b,COPY_PUT);
	free(a);
	free(b);
	free(c);
	free(d);
}
/*void main()
{
	int gdriver=DETECT,gmode,j,i;
	struct fonts font;
	void *charu;
	font.height=16;
	initgraph(&gdriver,&gmode,"c:\\borlandc\\bgi");
	setbkcolor(WHITE);
	charu=malloc(imagesize(1,1,129,17));
	for(j=0;j<4;j++)
	for(i=0;i<80;i++)
	displayen(1,'1'+i,0,8*i,16*j,1,15);
	getimage(0,240,128,240+16,charu);
	getch();
	adjusttext(23,16,128,16,&font,charu);
	getch();
	free(charu);
	closegraph();
}*/