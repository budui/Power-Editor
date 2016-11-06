#include "indere.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "im.h"
void insert(Text *txt,int x0,int y0,const char*data,size_t len)
{
	Cursor csr_ist_head,csr_ist_tail;
	int height,length,*aa,*bb,tes;
	int ymax=getmaxy();
	int xmax=getmaxx();
	Iterator it;
	struct fonts enfont;
	enfont.hzk=0;
	enchoose(&enfont);
	*aa=0;*bb=16;
	//printf("start");getch();
	csr_ist_tail.it=iterator_get(txt,0);
	csr_ist_head=csr_ist_tail;
	adjustcursor(x0,y0,&csr_ist_head,8,16);
	csr_ist_tail=csr_ist_head;
	print_text_xy(1,data,strlen(data),0,&(csr_ist_tail.cursor_x),&(csr_ist_tail.cursor_y));

	bar(xmax-enfont.width,y0-16,xmax,ymax);
	while(1)
	{
		
		print_text_xy(1,csr_ist_tail.it.text,csr_ist_tail.it.end - csr_ist_tail.it.text,0,&(csr_ist_tail.cursor_x),&(csr_ist_tail.cursor_y));
		//iterator_next(&csr_ist_tail.it);
		tes=iterator_next(&csr_ist_tail.it);
	if(!tes)
		break;
	}
	text_insert(txt, csr_ist_head.it.pos,data,strlen(data));

		//iterator_prev(&csr_ist_tail.it);
	//it=iterator_get(txt,0);
	//print_text_xy(1,it.start,text_size(txt),1,aa,bb);
}
void delete(Text *txt,int x0,int y0,size_t len)
{
	Cursor csr_ist_head,csr_ist_tail;
	int height,length,watchx,watchy,i,xmax=getmaxx(),ymax=getmaxy(),count=0;
	struct fonts enfont;
	Iterator temp_it;
FILE*fp=fopen("che.txt","wt");
	char *data,b;
	enfont.hzk=0;
	enchoose(&enfont);

	csr_ist_tail.it=iterator_get(txt,0);
	csr_ist_head=csr_ist_tail;
	adjustcursor(x0,y0,&csr_ist_head,8,16);watchx=csr_ist_tail.it.pos;watchy=csr_ist_head.it.pos;
	data=csr_ist_head.it.text;
	csr_ist_tail=csr_ist_head;

	print_text_xy(0,data,len,0,&(csr_ist_tail.cursor_x),&(csr_ist_tail.cursor_y));
	csr_ist_tail.it=iterator_get(txt,0);watchx=csr_ist_tail.it.pos;watchy=csr_ist_head.it.pos;
	adjustcursor(csr_ist_tail.cursor_x,csr_ist_tail.cursor_y-2,&csr_ist_tail,8,16);watchx=csr_ist_tail.it.pos;watchy=csr_ist_head.it.pos;
	height=csr_ist_tail.cursor_y-csr_ist_head.cursor_y;
	length=csr_ist_tail.cursor_x-csr_ist_head.cursor_x;
	bar(xmax-enfont.width,y0-height-16,xmax,ymax);	
	temp_it=csr_ist_head.it;
	count=0;
 
	while(csr_ist_tail.it.start!=temp_it.start||csr_ist_tail.it.end!=temp_it.end||csr_ist_tail.it.text!=temp_it.text||csr_ist_tail.it.piece!=temp_it.piece||csr_ist_tail.it.pos!=temp_it.pos)
	{
		iterator_byte_next(&temp_it,&b);
		count++;
	}
	//print_text_xy(1,csr_ist_tail.it.text,text_size(txt)-csr_ist_tail.it.pos,0,&(csr_ist_head.cursor_x),&(csr_ist_head.cursor_y));
	
/*for(i=0;i<15;i++)
fprintf(fp,"%c",*(csr_ist_head.it.text+i));*/
	watchx=csr_ist_tail.it.pos;watchy=csr_ist_head.it.pos;
	watchy=(int)(csr_ist_tail.it.text-csr_ist_head.it.text);

	text_delete(txt, csr_ist_head.it.pos,count);csr_ist_head.it=iterator_get(txt,0);
	do                                      
	{
		print_text_xy(1,csr_ist_tail.it.text,csr_ist_tail.it.end - csr_ist_tail.it.text,0,&(csr_ist_head.cursor_x),&(csr_ist_head.cursor_y));
	} while (iterator_next(&csr_ist_tail.it));//adjustcursor(x0,y0,&csr_ist_head,8,16);fprintf(fp,"%d  %d",csr_ist_head.cursor_x,csr_ist_head.cursor_y);
	bar(csr_ist_head.cursor_x,csr_ist_head.cursor_y-16,xmax,csr_ist_head.cursor_y);
	bar(0,csr_ist_head.cursor_y,xmax,ymax);
/*for(i=0;i<15;i++)
fprintf(fp,"%c",*(csr_ist_head.it.text+i));*/
fclose(fp);
}