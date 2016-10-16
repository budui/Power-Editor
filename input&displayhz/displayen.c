#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <conio.h>

struct fonts
{
	unsigned char hzk;
	FILE *fp;
	unsigned char height;
	unsigned char width;
	unsigned int bytes;
};


void enchoose(struct fonts *font);

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