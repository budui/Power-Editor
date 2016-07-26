#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
extern void initializecharacter(int *index,int **character);
unsigned int receivehz()
{
	unsigned char i=0,a[5]={0};
	int index[53]={0},*character[26];
	char c,temp;
	FILE*fp;
	unsigned long offsetindex;
	unsigned int offsethzk=0,offsethzkold=0;
	if((fp=fopen("index.txt","rt"))==NULL)
	{
		printf("cannot open index.txt");
		exit(1);
	}

	initializecharacter(index,character);		//每次输入一个汉字就会初始
	c=getch();						//化一次浪费
	while(c<='z'&&c>='a')
	{
		offsetindex=(long)*(character[c-'a']+i);//index.txt偏移量
		fseek(fp,offsetindex,SEEK_SET);
		fread(a,1,4,fp);
		offsethzk=(int)strtol(a,NULL,16);
		while(offsethzk<offsethzkold)
		{
			fread(a,1,4,fp);
			offsethzk=(int)strtol(a,NULL,16);
		}
		offsethzkold=offsethzk;
		i++;
	   //	printf("%d",offsethzk);
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
		}
		else if(c=='-')
		{
			temp-=9;
		}
		else if(c>='0'&&c<='9')
		{
			temp+=c-'1';
			break;
		}
	   /*	else
		{
			cancelinput();
		}*/
		c=getch();
	}
	}
	offsethzk+=temp;
	if((offsethzk/256)==0xa0)
	{
		offsethzk++;
	}
	else if(offsethzk/256==0xff)
	{
		offsethzk+=0xa2;
	}
	return offsethzk;
}