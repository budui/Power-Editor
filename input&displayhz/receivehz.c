/*receivehz is used to input pinyin and display available hanzi so that user can choose the correct hanzi .It will return the innercode of hanzi which is chosen*/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
extern void displayhen(unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
extern void displayhz(unsigned int innercode,unsigned char fontchoose,int x0,int y0,int textcolor,int bkcolor);
extern void initializecharacter(int *index,int **character);
unsigned int receivehz()
{
	unsigned char i=0,m=0,a[5]={0};
	void *buf;
	int index[53]={0},*character[26];
	char c,temp;
	FILE*fp;
	unsigned long offsetindex;
	unsigned int innercode=0,innercodeold=0;
	unsigned int tempx,currentx=getx(),currenty=gety();//record current place of painting brush
	if(currentx+9*32>getmaxx())		//in case of input method move out of the screen scale
	{
		currentx=getmaxx()-9*32;
	}
	tempx=currentx;
	if((fp=fopen("index.txt","rt"))==NULL)
	{
		perror("error index.txt");
		exit(1);
	}

	initializecharacter(index,character);
	buf=malloc(imagesize(0,0,32*9,32));// input method will cover the text,save in advance
	getimage(currentx,currenty,currentx+32*9,currenty+32,buf);
	c=getch();						
	while(c<='z'&&c>='a')
	{
		offsetindex=(long)*(character[c-'a']+i);//offset in index.txt
		fseek(fp,offsetindex,SEEK_SET);
		fread(a,1,4,fp);
		innercode=(int)strtol(a,NULL,16);
		while(innercode<innercodeold)
		{
			fread(a,1,4,fp);
			innercode=(int)strtol(a,NULL,16);
		}
		innercodeold=innercode;
		displayen(c,0,currentx+8*i,currenty+16,1,15);
		for(m=0;m<9;m++)//display nine available hanzi
		{
			displayen(m+'1',0,tempx,currenty+32,1,15);
			tempx+=8;
			displayen('.',0,tempx,currenty+32,1,15);
			tempx+=8;
			displayhz(innercode+m,2,tempx,currenty+32,1,15);
			tempx+=16;
		}
		tempx=currentx;//

		i++;
		c=getch();
	}
	temp=c;
   /*	if(!(c==' '||c<='9'&&c>='0'||c=='-'||c=='='))
	{
		cancelinput();
	}*/
	if(c<='9'&&c>='0')
	{
		temp=c-'1';
	}
	else if(c==' ')
	{
		temp=0;
	}
	else
	{
		temp=0;
	while(1)
	{
		if(c=='=')
		{
			temp+=9;

			for(m=0;m<9;m++)
			{
			displayen(m+'1',0,tempx,currenty+32,1,15);
			tempx+=8;
			displayen('.',0,tempx,currenty+32,1,15);
			tempx+=8;
			displayhz(innercode+m+temp,2,tempx,currenty+32,1,15);
			tempx+=16;
			}
			tempx=currentx;

		}
		else if(c=='-')
		{
			temp-=9;
			for(m=0;m<9;m++)
			{
			displayen(m+'1',0,tempx,currenty+32,1,15);
			tempx+=8;
			displayen('.',0,tempx,currenty+32,1,15);
			tempx+=8;
			displayhz(innercode+m+temp,2,tempx,currenty+32,1,15);
			tempx+=16;
			}
			tempx=currentx;
		}
		else if(c>='0'&&c<='9')
		{
			temp+=c-'1';
			break;
		}
		else if(c==' ')
		{
			break;
		}	
	   /*	else
		{
			cancelinput();
		}*/
		c=getch();
	}
	}
	innercode+=temp;
	if((innercode/256)==0xa0)
	{
		innercode++;
	}
	else if(innercode/256==0xff)
	{
		innercode+=0xa2;
	}
	putimage(currentx,currenty,buf,0);
	free(buf);
	fclose(fp);
	return innercode;
}