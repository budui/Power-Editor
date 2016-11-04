#include <fcntl.h>
#include <graphics.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>

#include "print.h"
#include "config.h"

#define logfile stdout

static int handle;

static int opencfl(void);
static void closecfl(int handle);
static bool char2mat(int handle, const char *ch, char *mat);
static void showchinese(int x, int y, unsigned char *mat, colors,colors);


/* open Chinese font library. return -1 when open failed. */
static int opencfl(void)
{
    int libfile = open(CONFIG_FONT_HZK16,O_BINARY | O_RDONLY);
    if(libfile == -1)
    {
        fprintf(logfile,"[ERROR]Can't open Chinese lib %s\n",CONFIG_FONT_HZK16);
		perror("hzk16 open failed.");
		exit(1);
    }
    return libfile;
}

/* Inner code to view data. */
static bool char2mat(int handle, const char *ch, char *mat)
{
    unsigned char qh,wh;
    unsigned long offset;

    if(!ch || !mat)
        return false;

    /* change Inner code (2 char) to offset in Chinese lib. */
    qh = ch[0] - 0xa0;
    wh = ch[1] - 0xa0;
    offset = (94 * (qh - 1) + (wh - 1)) * 32L;

    if(lseek(handle,offset,SEEK_SET) == -1)
    {
        switch(errno)
        {
            case EBADF:
                fprintf(logfile,"Incorrect file handle. Line: %s",__LINE__);
                break;
            case EINVAL:
                fprintf(logfile,"Incorrect Chinese offset.");
                break;
        }
        return false;
    }
    read(handle,mat,32);
    return true;
}

static void showchinese(int x, int y, unsigned char *mat, colors color,colors bkcolor)
{
    // calculate the address of the pic of the Chinese.
    char far *p = (char far *)(0xa0000000 + 80*y + x / 8);
    register int i,j,k;
    for(i = 0; i < 16; ++i)
    {
        for(j = 0; j < 2; ++j)
        {
            outportb(0x3ce,0x05);
            outportb(0x3cf,0x02);
            outportb(0x3ce,0x08);
			outportb(0x3cf,mat[2 * i + j]);
            *(p + 80 * i + j) = color;
            k = *(p + 80*i + j);
            outportb(0x3ce,0x08);
            outportb(0x3cf,~mat[2 * i + j]);
            *(p + 80 * i + j) = bkcolor;
        }
    }
    i = k; //useless!
    outportb(0x3ce,0x05);
    outportb(0x3ce,0xff08);
}

bool print_init(void)
{
    if((handle = opencfl())!= -1)
        return true;
    return false;
}
void print_close(void)
{
    close(handle);
}

int print_wchar_xy(const char *c,int x,int y)
{
    unsigned char mat[32];

    if(ISGBK(*c))
    {
        char2mat(handle,c,mat);
        showchinese(x,y,mat,BLACK,WHITE);
        return 2;
    }
    else {
        char s[2] = "";
        sprintf(s,"%c",*c);
        outtextxy(x,y,s);
        return 1;
    }
}

int print_str_xy(const char *s, int x,int y)
{
    unsigned char mat[32];
    int i;

    while(*s != NULL)
    {
        while(x < 640 && (*s != NULL))
        {
			char2mat(handle,s,mat);
            showchinese(x,y,mat,BLACK,LIGHTGRAY);
            x += 16;
            s += 2;
            i++;
        }
        x += 24; 
        y += 20;
    }
    return i;
}