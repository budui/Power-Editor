#include <stdio.h>
#include <stdlib.h>
void initializeindex(int *a);
void initializecharacter(int *index,int **character)
{
	int *p=index,i;

	int offset1[]={0,4,5,6,7,10,11,17,19,24,25,
	26,27,28,33,37,38,39,41,42,43,47,49,50,51,52};
	initializeindex(p);
	for(i=0;i<26;i++)
	{
	   character[i] =offset1[i]+index;
	}
}





void initializeindex(int *index)          //53 elements
{
	char c;
	int i,j;
   //	int index[53]={0};                //tiao shi yuju
	FILE*fp=NULL;
   fp=	fopen("SHUJU.txt","rt");
   if(fp==NULL)
   {
	printf("error,cannot open the file SHUJU.txt");
	exit(1);
   }
   c='2';
   j=0;
   i=0;
   while(c!=EOF)
   {
	c=fgetc(fp);
	if(c=='*')
	{
		index[i]=ftell(fp)-j-1;
		j++;
		i++;
	}
	if(i>53)
	{
		printf("error:shuzu yue jie");
		exit(1);
	}
   }
   fclose(fp);
   return;
   }