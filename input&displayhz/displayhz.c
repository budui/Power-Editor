/*displayhz(unsigned int innercode,,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor),以(x0,y0)为左下角（考虑不同字号的字底部对齐）fontchoose选定的字库画出内码为innercode的汉字
(x0,y0) as bottom left corner(cosidering different fonts need to be bottom-justified),fontchoose as chosen hzk,draw hanzi according to its innercode*/
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <conio.h>

struct fonts				//adjust the function according to different hzk
{
	unsigned char hzk;
	FILE *fp;
	unsigned char line;
	unsigned char row;
	unsigned int bytes;// bytes every hanzi occupys in hzk file
};


void hzkchoose(struct fonts *font);

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
	y0-=font.line;
	for(i=0;i<font.line;i++)			
	{
		for(j=0;j<font.row/8;j++)
		{
			for(k=0;k<8;k++)
			{
				c=mat[j+i*font.row/8]&(0x80>>k) ;
				if(c)
				{	
					if(font.row==24)	//hanzi in hzk24x need to be rotated 
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
					if(font.row==24)
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
			font->line=12;font->row=16;
			break;
		case 1:
			font->fp=fopen(".\\fonts\\hzk14","rb");
			font->line=14;font->row=16;
			break;
		case 2:
			font->fp=fopen(".\\fonts\\hzk16","rb");
			font->line=font->row=16;
			break;
		case 3:
			font->fp=fopen(".\\fonts\\hzk16f","rb");
			font->line=font->row=16;
			break;
		case 4:
			font->fp=fopen(".\\fonts\\hzk24f","rb");
			font->line=font->row=24;
			break;
		case 5:
			font->fp=fopen(".\\fonts\\hzk24h","rb");
			font->line=font->row=24;
			break;
		case 6:
			font->fp=fopen(".\\fonts\\hzk24k","rb");
			font->line=font->row=24;
			break;
		case 7:
			font->fp=fopen(".\\fonts\\hzk24s","rb");
			font->line=font->row=24;
			break;
		case 8:
			font->fp=fopen(".\\fonts\\hzk24t","rb");
			font->line=font->row=24;
			break;
		case 9:
			font->fp=fopen(".\\fonts\\hzk40s","rb");
			font->line=font->row=40;
			break;
		case 10:
			font->fp=fopen(".\\fonts\\hzk40t","rb");
			font->line=font->row=40;
			break;
		case 11:
			font->fp=fopen(".\\fonts\\hzk48s","rb");
			font->line=font->row=48;
			break;
		case 12:
			font->fp=fopen(".\\fonts\\hzk48t","rb");
			font->line=font->row=48;
			break;
		default:
			printf("error,no such font file");
			exit(1);
	}
	font->bytes=(font->line)*(font->row)/8;
	return;
}