#include "text_util.h"

#include <stdio.h>


size_t text_search(Text *txt, size_t start_pos, const char *substring)
{
	return 1;
}

bool text_cut(Text *txt, size_t start, size_t end)
{
	return true;
}
bool text_paste(Text *txt, size_t pos, const char * start, size_t len)
{
	return text_insert(txt, pos, start, len);
}
bool text_copy(Text *txt, size_t start, size_t end)
{
	return true;
}