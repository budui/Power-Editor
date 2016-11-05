#include "disp.h"
#include "im.h"
Cursor cursor;
void linefeed(int *x,int *y,int width,int height)
{
	if(*x>getmaxx())
	{
		*x=0;
		*y+=height;
	}
	else if(*x<0)
	{
		*x=getmaxx()+1-width;
		*y-=height;
	}
	if(*y<height)
	{

		*y=height;
	}
}
void print_text_xy(int mode,const  char *text,size_t n, bool auto_height_feed, int *x,int *y)//mode 0,dont display,just move the cursor;mode 1:display and move the cursor at the same time
{
	unsigned char c,d;
	unsigned char *cc=(unsigned  char*)text;
	unsigned int innercode,tempx=0;
	struct fonts curfont;
	unsigned int currentx=*x,currenty=*y,i;
	unsigned int xmax=getmaxx();
	FILE*fp=fopen("test.txt","wt");
	for(i=0;i<n;i++)
	{
		c=*cc;
		fwrite(&c,1,1,fp);
		cc++;
		/*if(16<currenty&&currenty<=32)
		getch();*/
		if(c>>7==1)
		{
			tempx=currentx+16-1;
			linefeed(&tempx,&currenty,16,16);
			if(tempx==0)
				currentx=tempx;
			d=*cc;
			cc++;
			innercode=c*256+d;
			if(mode)
				displayhz(1,innercode,2,currentx,currenty,1,15);
			currentx+=16;
			linefeed(&currentx,&currenty,16,16);
			i++;
			if(auto_height_feed)
			{
				cursor.cursor_x=currentx;
				cursor.cursor_y=currenty;
			}

		}
		else
		{
			if(c==10)
			{
				bar(currentx,currenty-16,xmax,currenty);
				currentx=0;
				currenty+=16;
				if(auto_height_feed)
				{
					cursor.cursor_x=currentx;
					cursor.cursor_y=currenty;
				}
				continue;
			}
			if(mode)
				displayen(1,c,0,currentx,currenty,1,15);
			currentx+=8;
			linefeed(&currentx,&currenty,8,16);
			if(auto_height_feed)
			{
				cursor.cursor_x=currentx;
				cursor.cursor_y=currenty;
			}
		}
	}
	*x=currentx;
	*y=currenty;
	fclose(fp);
}




void adjusttext(int x0,int y0,int length0,int height,struct fonts *font,void *charu,Text *text)//x0,y0为要插入位置的底部坐标
{
	int xmax=getmaxx(),i=0,temp=0,heighttemp=0,nextlength,length,lengthc,flag=0;
	int ymax=getmaxy(),watch2;
	int yend=0;
	Iterator it;
	int mallocsize=imagesize(0,0,xmax,font->height);
	void *a=malloc(mallocsize);
	void *b=malloc(mallocsize);
	void *c=malloc(mallocsize);
	void *d=malloc(mallocsize);
	int length1=xmax-x0-length0;
	int length3=xmax-length0;
	int currenty=y0-font->height;
	int currentx=x0;
	unsigned char watch;
	while(temp!=6&&yend<ymax)
	{
		it=iterator_get(text,0);
		temp=hzoren(2,yend,&it,font->width,font->height);
		yend+=font->height;
	}
	if(yend<ymax-3*font->height)
	yend+=3*font->height;
	heighttemp=0;
	if(height>=font->height)
		height-=font->height;
	if(currenty<0)
		currenty=0;
	if(currentx<0)
		currentx=0;
	if(length0>=0)
	{
		it=iterator_get(text,0);
		if(hzoren(x0+length1+2,currenty+2,&it,font->width,font->height)==3)////////右汉字
			length1-=font->width;
		getimage(x0,currenty,x0+length1,currenty+font->height,c);
		it=iterator_get(text,0);
		if(hzoren(xmax-2,currenty+2,&it,font->width,font->height)==4)
			temp=xmax-font->width;
		else
			temp=xmax;
		getimage(x0+length1,currenty,temp,currenty+font->height,d);//长度length第一行
		bar(xmax-font->width,currenty,xmax,currenty+font->height);
		length=temp-x0-length1;
		length3=xmax-temp+x0+length1;
		if(charu!=NULL)
			putimage(x0,currenty,charu,COPY_PUT);
		putimage(x0+length0,currenty,c,COPY_PUT);//第一行处理玩
		it=iterator_get(text,0);
		if(hzoren(xmax-font->width-2,currenty+2,&it,font->width,font->height)==5)
		{
			length3=temp;
			getimage(0,0,0,0,d);
		}
		memmove(a,d,mallocsize);
		currenty+=font->height;
		
		while(1)
		{
			if(currenty+font->height>yend)
			{
				break;
			}
			it=iterator_get(text,0);
			if(hzoren(length3+2,currenty+2,&it,font->width,font->height)==3)
			{
				if(flag==0)
					length3-=font->width;
				else if(flag==1)
					length3+=font->width;
			}

			flag=0;
			getimage(0,currenty,length3,currenty+font->height,c);
			it=iterator_get(text,0);
			if(hzoren(xmax-2,currenty+2,&it,font->width,font->height)==4)
			{//printf("%d %d",xmax-2,currenty+2);
				flag=1;
				temp=xmax-font->width;}

			else
				temp=xmax;
			getimage(length3,currenty,temp,currenty+font->height,d);

			nextlength=temp-length3;
			it=iterator_get(text,0);
			if(hzoren(xmax-font->width-2,currenty+2-font->height,&it,font->width,font->height)==5)
			{	//bar(length3,currenty,xmax,currenty+font->height);
				break;
			}
			putimage(0,currenty,a,COPY_PUT);
			bar(xmax-font->width,currenty,xmax,currenty+font->height);
			memmove(a,d,mallocsize);//memmove(void *dest,void *b,int bytes);//把b指向的大小为bytes的内容移至dest
			putimage(length,currenty,c,COPY_PUT);//第二行处理完
			currenty+=font->height;
			length=nextlength;
		}

	}
	else if(length0<0)
	{
		if(height>=0)
			getimage(0,y0-font->height,xmax,y0,b);
		if(height<0)
			getimage(0,y0+height-font->height,x0+length0,y0+height,b);
		length1=xmax-x0;
		it=iterator_get(text,0);
		if(hzoren(xmax-2,currenty+2,&it,font->width,font->height)==4)
		{
			length1-=font->width;
			flag=1;
		}
		getimage(x0,currenty,x0+length1,currenty+font->height,c);
		length3=-length0;
		it=iterator_get(text,0);
		if(hzoren(length3+2,currenty+font->height+2,&it,font->width,font->height)==3)
		{
			if(flag==0)
				length3-=font->width;
			else if(flag==1)
				{
				//printf("%d  ",length3);
				length3+=font->width;
				}
		}
		flag=0;
		getimage(0,currenty+font->height,length3,currenty+2*font->height,d);
		bar(xmax-font->width,currenty,xmax,currenty+font->height);
		putimage(x0+length0,currenty,c,COPY_PUT);
		it=iterator_get(text,0);
		if(hzoren(xmax-font->width-2,currenty+2,&it,font->width,font->height)!=5)
			{
				putimage(x0+length0+length1,currenty,d,COPY_PUT);
			}
		else
			currenty=ymax;
		currenty+=font->height;

		while(1)
		{
			if(currenty+font->height>yend||currenty+font->height>=ymax)
			{
				break;
			}
			it=iterator_get(text,0);
			if(hzoren(xmax-2,currenty+2,&it,font->width,font->height)==4)
			{
				temp=xmax-font->width;
				flag=1;
			}
			else
			{
				temp=xmax;
				flag=0;
			}
			getimage(length3,currenty,temp,currenty+font->height,c);
			putimage(0,currenty,c,COPY_PUT);
			lengthc=temp-length3;
			length1=length3;
			it=iterator_get(text,0);
		watch=hzoren(length3+2,currenty+2+font->height,&it,font->width,font->height);
		watch2=currenty;
			if(watch==3)
			{
				if(flag==1)
					length1+=font->width;
				else if(flag==0)
					length1-=font->width;
			}
			flag=0;
			getimage(0,currenty+font->height,length1,currenty+2*font->height,d);
			it=iterator_get(text,0);
			if(hzoren(xmax-font->width-2,currenty+2,&it,font->width,font->height)==5)
			{//bar(lengthc,currenty,xmax,currenty+font->height);
				break;
			}
			bar(xmax-font->width,currenty,xmax,currenty+font->height);
			putimage(lengthc,currenty,d,COPY_PUT);
			length3=length1;
			currenty+=font->height;
			
		}
		
	}
	currenty=y0;
	i=0;
	if(height<0)
	{
        while(i<-height)
        {
            currenty=yend-height-i;//currenty=y0+i;
            getimage(0,currenty-font->height,xmax,currenty,a);
           while(currenty>=y0)// while(currenty<ymax)
            {
            
                currenty+=height;
               if(currenty+font->height<y0)// if(currenty>ymax)
                    break;
                getimage(0,currenty-font->height,xmax,currenty,c);
                putimage(0,currenty-font->height,a,COPY_PUT);
                memmove(a,c,mallocsize);
            }
                i+=font->height;
        }
	}
	else if(height>=0)
	{
        while(i<height)
        {
            currenty=y0+i;
            getimage(0,currenty-font->height,xmax,currenty,a);
            while(currenty<ymax)
            {
            
                currenty+=height;
                if(currenty>ymax)
                    break;
                getimage(0,currenty-font->height,xmax,currenty,c);
                putimage(0,currenty-font->height,a,COPY_PUT);
                memmove(a,c,mallocsize);
            }
                i+=font->height;
        }
	}
        if(length0<0)
	{
		if(height>0)
			putimage(0,y0-font->height,b,COPY_PUT);
		else if(height<0)
			putimage(0,y0-font->height+height,b,COPY_PUT);
	}
	free(a);
	free(b);
	free(c);
	free(d);
}
unsigned char hzoren(int pixelx,int pixely,Iterator *it,int width,int height)
{
	int x=0,y=height,flag=0;
	char b,*temp;
	int xmax=getmaxx();
	while(1)
	{
			flag=0;
			

			if(*(it->text)>>7==0)
			{
				if(*(it->text)==10)//回车
				{
					x=0;
					y+=height;
					temp=it->text;
				if(pixely+height<y)
				{
					//printf("s");
					if(*(it->text)==10)
					{
						return 5;
					}
				}
				
					iterator_byte_next(it,&b);
					if(b==NULL)
						break;
					continue;
				}
				if(pixelx<x+width/2&&pixelx>=x&&pixely<=y&&pixely>y-height)//英文左半边
					return 0;
				if(pixelx<x+width&&pixelx>=x+width/2&&pixely<=y&&pixely>y-height)//英文右半边
					return 1;
				x+=width;
				x+=2;
				linefeed(&x,&y,width,height);
				if(x!=0)
					x-=2;
				iterator_byte_next(it,&b);
				if(b==NULL)
				break;
				continue;
			}
			if(*(it->text)>>7==1)
			{
				x+=2*width;
				if(x<xmax+2*width-2&&x>xmax+2)
				{
					if(xmax-width<=pixelx&&pixelx<=xmax&&pixely<=y&&pixely>y-height)//调整用的空格
						return 4;
				}

				if(x-width<=pixelx&&pixelx<x&&pixely<=y&&pixely>y-height)//汉字右半边
					{
					return 3;}
				if(x-2*width<=pixelx&&pixelx<x-width&&pixely<=y&&pixely>y-height)//汉字左半边
					{
					return 2;}
				x+=8;
				linefeed(&x,&y,width,height);
				if(x!=0)
					x-=8;
				iterator_byte_next(it, &b);
				if(b==NULL)
					break;
				iterator_byte_next(it, &b);
				if(b==NULL)
					break;

			

				
			}
			if(pixely+height<y)
			{
				//printf("s");
				if(*(it->text)==10)
				{
					it->text=temp;
					return 5;
				}
				else 
					return 4;
				}
			}
	return 6;
}
void adjustcursor(int x,int y,Cursor *cursor,int width,int height)//指向光标下一个
{
	Cursor cursortemp=*cursor;
	unsigned char c,b;
	int xmax=getmaxx();
	if(y%height==0)
		y-=1;
	c=hzoren(x,y,&(cursor->it),width,height);
	printf("%d ",c);
	switch (c)
	{
		case 0://english left
		case 2://chinese character left
		case 4://space used to adjust hz
			cursor->cursor_x=x/width*width;
			cursor->cursor_y=y/height*height+height;
			break;
		case 1:
			cursor->cursor_x=x/width*width+width;
			iterator_byte_next(&cursor->it,&b);
			cursor->cursor_y=y/height*height+height;
			break;
		case 3:

			cursor->cursor_x=x/width*width+width;
			iterator_byte_next(&cursor->it,&b);
			iterator_byte_next(&cursor->it,&b);
			cursor->cursor_y=y/height*height+height;
			break;
		case 5:
			*cursor=cursortemp;
			b=hzoren(x,y,&(cursor->it),width,height);
			while(b==5&&x>0)
			{	
				
				
				*cursor=cursortemp;
				b=hzoren(x,y,&(cursor->it),width,height);
				x-=width;
			}
			if(x<0)
				cursor->cursor_x=0;
			
			else 
			cursor->cursor_x=x/width*width+width+width;
			cursor->cursor_y=y/height*height+height;
			if(x>=0)
			{
				cursor->it.text++;
				if(*((cursor->it).text)>>7==1)
					cursor->it.text++;
			}
			/*if(*((cursor->it).text)>>7==1)
			{
				cursor->cursor_x-=width;
				iterator_byte_prev(&cursor->it,&b);//如果汉字再往前进一个
			}*///指向回车还是指向回车前一个字符？
			
			break;
		case 6:
			*cursor=cursortemp;
			b=hzoren(x,y,&(cursor->it),width,height);
			while(b==6&&y>=0)
			{	
				
				y-=height;
				*cursor=cursortemp;
				b=hzoren(x,y,&(cursor->it),width,height);
			}
			x=1;
			while(b!=6)
			{
				*cursor=cursortemp;
				b=hzoren(x,y,&(cursor->it),width,height);
				x+=width;
				linefeed(&x,&y,width,height);
			}
			cursor->it.text=cursor->it.end;
			cursor->cursor_x=x/width*width-width;
			cursor->cursor_y=y/height*height+height;
				//printf("%d",*(cursor->it.text));
			break;

	}
	if(cursor->cursor_x>xmax)
	{
		cursor->cursor_x=0;
		cursor->cursor_y+=height;
	}
}
