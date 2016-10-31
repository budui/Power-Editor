#include <stdio.h>
#include <string.h>
#define STRLENTWO 98
typedef unsigned char  bool;
bool checkpy(unsigned char* s)
{
	FILE *fp;
	unsigned int offset,i;
	unsigned char temp[3]={0},c[3]={0},a,pytemp[7]={0};
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
void main()
{
	unsigned char s[]="zuog";
	printf("%d,",checkpy(s));
}