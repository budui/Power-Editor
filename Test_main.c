#include "Text.h"
#include <stdio.h>

int main()
{
	Text *txt;
	FILE *fp = fopen(".\\editorcore.log", "w");
	txt = text_load(".\\test.txt");
	text_insert(txt, 5, "test", 4);
	text_insert(txt, 2, "****", 3);
	text_insert(txt, 3, "----", 4);
	text_insert(txt, 23, "||||", 4);
	test_print_buffer(txt, fp);
	test_print_piece(txt, fp);
	test_print_current_action(txt, fp);
	text_free(txt);
	fclose(fp);
	return 0;
}