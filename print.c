#include "print.h"
#include <graphics.h>
#include <string.h>
#include <malloc.h>
#include <dos.h>

#define HZFONT 2
#define ENFONT 0

struct fonts
{
	unsigned char hzk;
	FILE *fp;
	unsigned char width;
	unsigned char height;
	unsigned int bytes;
};

void hzkchoose(struct fonts *font);
void displayen(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
void displayhz(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
void enchoose(struct fonts *font);
void hzkchoose(struct fonts *font);
static void linefeed(int *x,int *y,int width,int height);

struct fonts enfont,hzfont;

void print_init(int hz, int en)
{

	enfont.hzk=en;
	hzfont.hzk=hz;
	hzkchoose(&hzfont);
	enchoose(&enfont);

}
static void linefeed(int *x,int *y,int width,int height)
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
		*x=0;
		*y=height;
	}
}

void displayen(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor)
{
	unsigned int a=innercode;
	unsigned long offset=0;
	unsigned char i,j,k;
	unsigned char *mat,c;
	struct fonts font;
	font.hzk=fontchoose;
	if(bs==0)
		return;
	enchoose(&font);
	if(( mat=(unsigned char*)malloc(font.bytes*sizeof(char)) )==NULL)
	{
		perror("malloc");
		exit(1);
	}
	offset=(long)innercode*font.bytes;
	if(font.fp==NULL)
	{
		perror("ASCXX");
		exit(1);
	}
	fseek(font.fp,offset,SEEK_SET);

	fread(mat,1,font.bytes,font.fp);
	y0-=font.height;
	for(i=0;i<font.height;i++)
	{
		for(j=0;j<font.width/8;j++)
		{
			for(k=0;k<8;k++)
			{
				c=mat[j+i*font.width/8]&(0x80>>k) ;
				if(c)
				{
					putpixel(j*8+k+x0,y0+i,textcolor);
				}
				else
				{
					putpixel(j*8+k+x0,y0+i,bkcolor);
				}
			}
		}
	}
	free(mat);
	mat=NULL;
	fclose(font.fp);
	return;
}

void enchoose(struct fonts *font)
{
	switch(font->hzk)
	{
		case 0:
			font->fp=fopen(".\\fonts\\asc16","rb");
			font->height=16;font->width=8;
			break;
		case 1:
			font->fp=fopen(".\\fonts\\asc48","rb");
			font->height=48;font->width=24;
			break;

		default:
			printf("error,no such font file");
			exit(1);
	}
	font->bytes=(font->height)*(font->width)/8;
	return;
}

void displayhz(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor)
{
	unsigned int a=innercode;
	unsigned long offset=0;
	int qh,wh;
	unsigned char i,j,k;
	unsigned char *mat,c;
	struct fonts font;
	font.hzk=fontchoose;
	if(bs==0)
		return;
	hzkchoose(&font);
	if(( mat=(unsigned char*)malloc(font.bytes*sizeof(char)) )==NULL)
	{
		perror("malloc");
		exit(1);
	}
	if(font.hzk<4)
	{
		a=innercode-0xa0a0;
	}
	else
	{
		a=innercode-0xafa0;

	}
	qh=a/256;
	wh=a%256;
	offset=(long)font.bytes*(94*(qh-1)+wh-1);
	if(font.fp==NULL)
	{
		perror("error");
		exit(1);
	}
	fseek(font.fp,offset,SEEK_SET);

	fread(mat,1,font.bytes,font.fp);
	y0-=font.height;
	for(i=0;i<font.height;i++)
	{
		for(j=0;j<font.width/8;j++)
		{
			for(k=0;k<8;k++)
			{
				c=mat[j+i*font.width/8]&(0x80>>k) ;
				if(c)
				{
					if(font.width==24)	//hanzi in hzk24x need to be rotated
					{
						putpixel(x0+i,y0+j*8+k,textcolor);
					}
					else			//other hanzi can be output normally
					{
						putpixel(j*8+k+x0,y0+i,textcolor);
					}
				}
				else
				{
					if(font.width==24)
					{
						putpixel(x0+i,j*8+k+y0,bkcolor);
					}
					else
					{
						putpixel(j*8+k+x0,y0+i,bkcolor);
					}
				}
			}
		}
	}
	free(mat);
	mat=NULL;
	fclose(font.fp);
	return;
}

void hzkchoose(struct fonts *font)
{
	switch(font->hzk)
	{
		case 0:
			font->fp=fopen(".\\fonts\\hzk12","rb");
			font->width=12;font->height=16;
			break;
		case 1:
			font->fp=fopen(".\\fonts\\hzk14","rb");
			font->width=14;font->height=16;
			break;
		case 2:
			font->fp=fopen(".\\fonts\\hzk16","rb");
			font->width=font->height=16;
			break;
		case 3:
			font->fp=fopen(".\\fonts\\hzk16f","rb");
			font->width=font->height=16;
			break;
		case 4:
			font->fp=fopen(".\\fonts\\hzk24f","rb");
			font->width=font->height=24;
			break;
		case 5:
			font->fp=fopen(".\\fonts\\hzk24h","rb");
			font->width=font->height=24;
			break;
		case 6:
			font->fp=fopen(".\\fonts\\hzk24k","rb");
			font->width=font->height=24;
			break;
		case 7:
			font->fp=fopen(".\\fonts\\hzk24s","rb");
			font->width=font->height=24;
			break;
		case 8:
			font->fp=fopen(".\\fonts\\hzk24t","rb");
			font->width=font->height=24;
			break;
		case 9:
			font->fp=fopen(".\\fonts\\hzk40s","rb");
			font->width=font->height=40;
			break;
		case 10:
			font->fp=fopen(".\\fonts\\hzk40t","rb");
			font->width=font->height=40;
			break;
		case 11:
			font->fp=fopen(".\\fonts\\hzk48s","rb");
			font->width=font->height=48;
			break;
		case 12:
			font->fp=fopen(".\\fonts\\hzk48t","rb");
			font->width=font->height=48;
			break;
		default:
			printf("error,no such font file");
			exit(1);
	}
	font->bytes=(font->width)*(font->height)/8;
	return;
}

void print_str_xy(const char *c, size_t n, bool auto_height_feed, int x,int y)
{
	int i;
	unsigned int innercode;
	int currentx=x,currenty=y;
	c = (unsigned char*)c;
	n=strlen(c);
	for(i=0;i<n;i++)
	{

		if(ISGBK(*(c+i)))
		{
			if(auto_height_feed)
				linefeed(&currentx,&currenty,hzfont.height,hzfont.width);
			innercode=c[i]*256+c[i+1];
			displayhz(1,innercode,HZFONT,currentx,currenty,1,15);
			i++;
			currentx+=hzfont.width;
		}
		else
		{
			if(auto_height_feed)
				linefeed(&currentx,&currenty,enfont.height,enfont.width);
			displayen(1,c[i],ENFONT,currentx,currenty,1,15);
			currentx+=enfont.width;
		}
	}
}

void print_wchar_xy(const char *c,color bg, color f, int x, int y)
/* first char will be print at (x,y)*/
{
	if(ISGBK(*c))
		displayhz(1,c[0]*256+c[1],HZFONT,x,y,f,bg);
	else
		displayen(1,*c,ENFONT,x,y,f,bg);
}

void print_str(const char *c, size_t n, bool auto_height_feed)
{
	print_str_xy(c, n, auto_height_feed, cursor.x,cursor.y);
}

void print_wchar(const char *c,color bg, color f)
{
	print_wchar_xy(c,bg,f,cursor.x, cursor.y);
}
