#include <stdio.h>
#include <graphics.h>
#include <stdlib.h>
#include <bios.h>
#include <string.h>

struct fonts				//adjust the function according to different hzk
{
	unsigned char hzk;
	FILE *fp;
	unsigned char height;
	unsigned char width;
	unsigned int bytes;// bytes every hanzi occupys in hzk file
};
void enchoose(struct fonts *font);	
void displayhz(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
unsigned int receivehz(int bs,char*pinyin,int mode);
void displayen(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
void hzkchoose(struct fonts *font);
void adjusttext(int x0,int y0,int length,int height,struct fonts *font,void *charu);

void displaytext(unsigned char hzk,unsigned char enzk,int x0,int y0,int textcolor,int bkcolor)//返回1说明按下esc，退出
{
	int spckey,c,currentx=x0,currenty=y0;
	char curlshift,prelshift,language,temp,pinyin[7]={0};
	struct fonts font,enfont,*allfont,*curfont;
	FILE*fq=fopen("hzoren.bin","w+");
	font.hzk=hzk;
	enfont.hzk=enzk;
	hzkchoose(&font);
	enchoose(&enfont);
	language=0;//默认language为汉字输入法
	while(1)
	{
		spckey=bioskey(2);
		curlshift=spckey/2%2;/////////////左shift控制中英文
		if(curlshift&&prelshift==0)
		language=!language;
		prelshift=curlshift;
		if(!bioskey(1))
		{
			continue;
		}//没有输入继续循环
		c=bioskey(1);
		if(c==283)//esc退出
		{
			bioskey(0);
			break;
		}
		if(language==0)//汉字输入法
		{
			curfont=&font;
			c=receivehz(1,pinyin,0);
			while(c==8&&strlen(pinyin)!=0)
			{
				c=receivehz(0,pinyin,1);
				if(strlen(pinyin)==0)///////如果拼音退到头
				{
					c=0;
					break;		
				}	
			}
			if(c==0)///////c值可能要改
				continue;
		}
		else if(language==1)
		{
			curfont=&enfont;
			c=bioskey(0);
		}
		if(language==0&&c==8||language==1&&c==3592)//backspace的处理
		{
			fseek(fq,-1,SEEK_END);///判断backspace的是英文还是汉字，要改
			fread(&temp,1,1,fq);///判断backspace的是英文还是汉字，要改
			if(temp==0)
				allfont=&font;
			else
				allfont=&enfont;
			currentx-=allfont->width;
			linefeed(&currentx,&currenty,allfont->width,allfont->height);
			moveto(currentx,currenty);
			if(temp==1)
				displayen(1,' ',enfont.hzk,currentx,currenty,textcolor,bkcolor);
			if(temp==0)
				displayhz(1,0xa1a1,font.hzk,currentx,currenty,textcolor,bkcolor);
			continue;
			
		}
		if(language==0)
			displayhz(1,c,font.hzk,currentx,currenty,textcolor,bkcolor);

		else if(language==1)
			displayen(1,c%256,enfont.hzk,currentx,currenty,textcolor,bkcolor);	

		currentx+=curfont->width;
		linefeed(&currentx,&currenty,curfont->width,curfont->height);
		moveto(currentx,currenty);

		fwrite(&language,1,1,fq);

	}	
			fclose(fq);
}
void main()
{
	int gdriver=DETECT,gmode;
	struct fonts font;
	void *charu;
	font.height=16;
	initgraph(&gdriver,&gmode,"c:\\borlandc\\bgi");
	setbkcolor(WHITE);
	charu=malloc(imagesize(0,0,128,16));
	displaytext(2,0,0,16,1,15);
	getimage(0,0,128,16,charu);
	adjusttext(15,16,128,16,&font,charu);
bioskey(0);
	free(charu);
	closegraph();
}/*
void main()
{
	int gdriver=DETECT,gmode,j,i;
	struct fonts font;
	void *charu;
	font.height=16;
	initgraph(&gdriver,&gmode,"c:\\borlandc\\bgi");
	charu=malloc(imagesize(1,1,129,17));
	for(j=0;j<4;j++)
	for(i=0;i<80;i++)
	displayen(1,'1'+i,0,8*i,16*j,1,15);
	getimage(0,0,128,16,charu);
	getch();
	adjusttext(23,16,128,16,&font,charu);
	getch();
	closegraph();
}*/