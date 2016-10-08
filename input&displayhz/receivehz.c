#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <conio.h>
#include <string.h>

struct fonts				//adjust the function according to different hzk
{
	unsigned char hzk;
	FILE *fp;
	unsigned char line;
	unsigned char row;
	unsigned int bytes;// bytes every hanzi occupys in hzk file
};
static unsigned int offsetshuju[53];
static int *character[26];
void inithz(unsigned int *offsetshuju);
int check(char *pinyin);
void initcharacter(unsigned int *offsetshuju,int **character);void displayhz(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
void displayen(int bs,unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
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
			pyvalid=check(pinyin);/*新加*/
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
/*void main()
{
	char pinyin[7]={0};
	int gdriver=DETECT,gmode;
	unsigned int cc,i=0,dif=0;
	unsigned char a[2];
	initgraph(&gdriver,&gmode,"c:\\borlandc\\bgi");
	moverel(0,16);
	for(i=0;i<20;i++)
	{
		cc=receivehz(1,pinyin,0);
		while(cc==8)
			cc=receivehz(0,pinyin,1);
		displayhz(1,cc,2,16*i,16,1,15);
		moverel(16,0);
	}
	closegraph();
	getch();
}*/
int check(char *pinyin)
{
return 1;
}