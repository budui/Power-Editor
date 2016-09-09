#include "Text.h"
#include <stdio.h>

int main()
{
	Text *txt;
	txt = text_load(".\\1.c");
	text_free(txt);
	return 0;
}