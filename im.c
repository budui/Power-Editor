#include "im.h"
#include <graphics.h>
#include <bios.h>
#include <dos.h>
void initfonts(struct fonts *hzfont,unsigned char hzk,struct fonts*enfont,unsigned char enzk)
{
	hzfont->hzk=hzk;
	enfont->hzk=enzk;
	hzkchoose(hzfont);
	enchoose(enfont);
}

unsigned char  checkpy(unsigned char* aa)
{
	FILE *fp;
	unsigned int offset,i;
	unsigned char temp[3]={0},c[3]={0},a,pytemp[7]={0},s[7]={0};
	strcpy(s,aa);
	if(strlen(s)<=1)
		return 1;
	fp=fopen("pyvalid.bin","rb");

	for(i=0;i<STRLENTWO;i++)
	{
		c[0]=s[0];
		c[1]=s[1];
		fread(temp,1,2,fp);
		if(strcmp(temp,c)==0)
		{
			if(strlen(s)==2)
				return 1;
			else if(strlen(s)>2)
				{
					fread(temp,1,2,fp);
					offset=temp[0]*256+temp[1];
					fseek(fp,offset,SEEK_SET);
					a=0;
					fread(&a,1,1,fp);
					while(a!='/')
					{

						i=0;
						while(a!=' ')
						{
							pytemp[i]=a;
							i++;
							fread(&a,1,1,fp);
						}
						pytemp[i]=0;
						if(strcmp(pytemp,s)==0)
							return 1;
						fread(&a,1,1,fp);
					}
					return 0;
				}
		}
		else if(strcmp(temp,c)!=0)
		{
			fseek(fp,2,SEEK_CUR);
			continue;
		}

	}
	
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

void hzkchoose(struct fonts *font)
{
	switch(font->hzk)
	{
		case 0:
			font->fp=fopen(".\\fonts\\hzk12","rb");
			font->height=12;font->width=16;
			break;
		case 1:
			font->fp=fopen(".\\fonts\\hzk14","rb");
			font->height=14;font->width=16;
			break;
		case 2:
			font->fp=fopen(".\\fonts\\hzk16","rb");
			font->height=font->width=16;
			break;
		case 3:
			font->fp=fopen(".\\fonts\\hzk16f","rb");
			font->height=font->width=16;
			break;
		case 4:
			font->fp=fopen(".\\fonts\\hzk24f","rb");
			font->height=font->width=24;
			break;
		case 5:
			font->fp=fopen(".\\fonts\\hzk24h","rb");
			font->height=font->width=24;
			break;
		case 6:
			font->fp=fopen(".\\fonts\\hzk24k","rb");
			font->height=font->width=24;
			break;
		case 7:
			font->fp=fopen(".\\fonts\\hzk24s","rb");
			font->height=font->width=24;
			break;
		case 8:
			font->fp=fopen(".\\fonts\\hzk24t","rb");
			font->height=font->width=24;
			break;
		case 9:
			font->fp=fopen(".\\fonts\\hzk40s","rb");
			font->height=font->width=40;
			break;
		case 10:
			font->fp=fopen(".\\fonts\\hzk40t","rb");
			font->height=font->width=40;
			break;
		case 11:
			font->fp=fopen(".\\fonts\\hzk48s","rb");
			font->height=font->width=48;
			break;
		case 12:
			font->fp=fopen(".\\fonts\\hzk48t","rb");
			font->height=font->width=48;
			break;
		default:
			printf("error,no such font file");
			exit(1);
	}
	font->bytes=(font->height)*(font->width)/8;
	return;
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
void displayhz(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor)
{
	unsigned int a=innercode;
	unsigned long offset=0;
	int qh,wh,x=x0,y=y0-16;
    char far *p = (char far *)(0xa0000000 + 80*y + (int)(x / 8)); 
	register int  i,j,k;
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
	//y0-=font.height;
	
    for(i = 0; i < 16; ++i)
    {
        for(j = 0; j < 2; ++j)
        {
            outportb(0x3ce,0x05);
            outportb(0x3cf,0x02);
            outportb(0x3ce,0x08);
	outportb(0x3cf,mat[2 * i + j]);
            *(p + 80 * i + j) = textcolor;
            k = *(p + 80*i + j);
            outportb(0x3ce,0x08);
            outportb(0x3cf,~mat[2 * i + j]);
            *(p + 80 * i + j) = bkcolor;
        }
    }
    i = k; //useless!
    outportb(0x3ce,0x05);
    outportb(0x3ce,0xff08);
	free(mat);
	mat=NULL;
	fclose(font.fp);
	return;
}



void initcharacter(unsigned int *offsetshuju,int **character)
{
	unsigned int *p=offsetshuju,i;
	int offset1[]={0,4,5,6,7,10,11,17,19,24,25,26,27,28,33,37,38,39,41,42,43,47,49,50,51,52};
	inithz(p);
	for(i=0;i<26;i++)
	{
	   character[i] =offset1[i]+p;
	}
	return;
}
void inithz(unsigned int *offsetshuju)
{
	int i=0;
	unsigned char c[2];
	FILE*fp=fopen(".\\shuju.bin","rb");
	if(fp==NULL)
	{
		perror("shuju.bin");
		exit(1);
	}
	while(i<53)
	{
		fread(c,1,2,fp);
		if(c[0]==c[1]&&c[0]=='*')
		{
			offsetshuju[i]=ftell(fp);
			i++;
			
		}
	}
	fclose(fp);
}
unsigned int receivepy(char c,char rank,unsigned int *innercode,unsigned int *innercodeend,unsigned int preinnercode)//接受一个字母并改变innercode和innercodeend的数值，返回该字的二级汉字在文件中的偏移量
{
	unsigned char s[2];
	FILE*fp=fopen(".\\shuju.bin","rb");
	unsigned int offset,offseterji,curinnercode;//shuju.bin中每次指向
	offset=*(character[c-'a']+rank);
	if(fp==NULL)
	{
		perror("receivepy()shuju.bin");
		exit(1);
	}
	fseek(fp,offset,SEEK_SET);
	while(1)
	{
		fread(s,1,2,fp);
		curinnercode=s[0]*256+s[1];
		if(curinnercode<preinnercode)
		{
			while(s[0]!='*'&&s[1]!='/')
			{
				fread(s,1,2,fp);
			}
			continue;
		}
		break;
	}
	*innercode=curinnercode;
	fread(s,1,2,fp);
	*innercodeend=s[0]*256+s[1];
	offseterji=(unsigned int)ftell(fp);
	fclose(fp);
	return offseterji;
}
unsigned int receivenum(char c,unsigned int innercode,unsigned int innercodeend,unsigned int offseterji)//接受一个数字并计算出偏移量
{
	unsigned int i,count=innercodeend-innercode;
	unsigned int dif=0;
	unsigned char temp[2]={0};
	FILE*fp;
	if(innercodeend/256>innercode/256)
			count-=0xa3;
	if(c<0)//c begins from 0
	{
		return 0;
	}
	else if(c<=count)
	{
		return (unsigned int )c;
	}
	else
	{
		fp=fopen(".\\shuju.bin","rb");
		if(fp==NULL)
		{
			perror("receivenum() shuju.bin");
			exit(1);
		}
		fseek(fp,offseterji,SEEK_SET);
		for(i=0;i<c-count;i++)
		{
			fread(temp,1,2,fp);
			if(temp[0]=='*'&&temp[1]=='/')
			{
				fclose(fp);
				return 0xffff;//normal dif's assignment cannot reach 0xffff
			}
		}
		dif=temp[0]*256+temp[1]-innercode;
		fclose(fp);
		return dif;
	}
	
}
unsigned int receivehz(int bs,char*pinyin,int mode)//two modes,0:getch() mode,1:receive pinyin mode,used to debug//if mode 1,pinyin needs to be initialized
{
	char c,rank=0,pyvalid;
	unsigned int innercode=0,innercodeend=0,preinnercode=0,offseterji=0,i;
	unsigned int dif,tempdif;
	void*buf;
	unsigned int tempx,currentx=getx(),currenty=gety();//record current place of painting brush
	if(currentx+9*32>getmaxx())		//in case the input method move out of the screen scale
	{
		currentx=getmaxx()-9*32;
	}
	initcharacter(offsetshuju,character);
	buf=malloc(imagesize(0,0,32*9*3,32));// input method will cover the text,save in advance
	getimage(currentx,currenty,currentx+32*9,currenty+32*3,buf);
	tempx=currentx;
	if(mode==0)
		{
		for(i=0;i<7;i++)
			pinyin[i]=0;
		}
	while(1)
	{
		if(bs==0&&mode==1&&rank+1==strlen(pinyin))
			bs=1;
		if(mode==0||(mode==1&&(pinyin[rank]<'a'||pinyin[rank]>'z')))
			c=bioskey(0)%256;
		else
			c=pinyin[rank];
		if(c<'z'+1&&c>'a'-1)//if(c!='='&&c!='-')
		{
			pinyin[rank]=c;
			displayen(1,c,0,tempx,currenty+16,1,15);
			pyvalid=checkpy(pinyin);/*新加*/
		}
		if(c==8)
		{
			pinyin[rank-1]=0;
			putimage(currentx,currenty,buf,0);
			free(buf);
			return 8;
		}
		tempx+=8;
		if(c<'z'+1&&c>'a'-1)
		{
			dif=0;
			offseterji=receivepy(c,rank,&innercode,&innercodeend,preinnercode);
			for(i=0;i<9;i++)
			{
				if(pyvalid==0)/*新加*/
					break;
				tempdif=receivenum(i,innercode,innercodeend,offseterji);
				if(tempdif==0xffff)//拼音到结尾不再显示
				{
					displayhz(bs,0xa1a1,2,currentx+32*i,currenty+32,1,15);
					displayhz(bs,0xa1a1,2,currentx+32*i+16,currenty+32,1,15);
					continue;				
				}
				if((innercode+tempdif)%256>0xfe||(innercode+tempdif)%256<0xa1)
					tempdif+=0xa3;
				displayen(bs,i+'1',0,currentx+32*i,currenty+32,1,15);
				displayen(bs,'.',0,currentx+32*i+8,currenty+32,1,15);
				displayhz(bs,tempdif+innercode,2,currentx+32*i+16,currenty+32,1,15);
			}
			preinnercode=innercode;
			pinyin[rank]=c;
			rank++;
			continue;//如果c一直都是拼音
		}
		///////rank=0;
		if(c<='9'&&c>='1'||c==' ')
		{
			if(c==' ')
				c='1';
			dif=receivenum(dif+c-'1',innercode,innercodeend,offseterji);
			if((innercode+dif)%256>0xfe||(innercode+dif)%256<0xa1)
				dif+=0xa3;
			putimage(currentx,currenty,buf,0);
			free(buf);
			if(dif==0xffff)
				return 0xa1a1;////////////////如果超过就返回0xa1a1出错处理不显示
			return (innercode+dif);
		}
		else if(c=='='||c=='-')
		{
			if(c=='=')
				dif+=9;
			else if(c=='-'&&dif>=9)
				dif-=9;
			else if(c=='-'&&dif<9)
				dif=0;
			tempdif=receivenum(dif,innercode,innercodeend,offseterji);
			if(tempdif==0xffff)
			{
				dif-=9;
			}
			for(i=0;i<9;i++)
			{

				tempdif=receivenum(dif+i,innercode,innercodeend,offseterji);
				if(tempdif==0xffff)
				{
					displayhz(bs,0xa1a1,2,currentx+32*i+16,currenty+32,1,15);
					displayhz(bs,0xa1a1,2,currentx+32*i,currenty+32,1,15);
					continue;
				}
				if((innercode+tempdif)%256>0xfe||(innercode+tempdif)%256<0xa1)
					tempdif+=0xa3;
				displayen(bs,i+'1',0,currentx+32*i,currenty+32,1,15);
				displayen(bs,'.',0,currentx+32*i+8,currenty+32,1,15);
				displayhz(bs,tempdif+innercode,2,currentx+32*i+16,currenty+32,1,15);
			}
		}
		else 
		{
			putimage(currentx,currenty,buf,0);
			free(buf);
			return 0xa1a1;////////////////////////////////////////////////////此处要改
		}
	}
}
