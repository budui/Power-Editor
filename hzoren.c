/*width是英文的宽度*/
#include <stdio.h>
#include <graphics.h>
unsigned char  hzoren(int pixelx,int pixely,unsigned char *text1,int width,int height)
{
	int x=0,y=height,flag=0;
	unsigned char *text=text1;
	while(*text!=0)
	{
			flag=0;
			

			if(*text>>7==0)
			{
				if(*text==10)
				{
					x=0;
					y+=height;
					text++;
					continue;
				}
				if(pixelx<x+width/2&&pixelx>=x&&pixely<=y&&pixely>y-height)//英文左半边
					return 0;
				if(pixelx<x+width&&pixelx>=x+width/2&&pixely<=y&&pixely>y-height)//英文右半边
					return 1;
				x+=width;
				linefeed(&x,&y,width,height);
				text++;
				continue;
			}
			if(*text>>7==1)
			{
				x+=2*width;
				if(x-2*width<getmaxx()&&x-2*width>getmaxx()-2*width+1)
				{
					if(x-2*width<=pixelx&&pixelx<=getmaxx()&&pixely<=y&&pixely>y-height)//调整用的空格
						return 4;
				}
				if(x-2*width<=pixelx&&pixelx<x-width&&pixely<=y&&pixely>y-height)//汉字左半边
					return 2;
				if(x-width<=pixelx&&pixelx<x&&pixely<=y&&pixely>y-height)//汉字右半边
					return 3;
				linefeed(&x,&y,width,height);
				text+=2;

			

				
			}
			if(pixely+height<y){
				//printf("s");
				return 5;}
	}
	return 0;
}