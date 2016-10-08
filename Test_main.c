#include "Text.h"
#include <stdio.h>


int main()
{
	Text *txt = text_load("E:\\tools\\BC\\DISK_C\\learnc\\test.txt");
	FILE *fp = fopen(".\\core.log", "w");

	text_insert(txt, 2, "(hhhh)", 6);
	text_insert(txt, 2, "[aa]", 4);
	text_delete(txt, 3, 3);

	getchar();

	test_print_buffer(txt, fp);
	test_print_piece(txt, fp);
	test_print_current_action(txt, fp);

	text_free(txt);
	fclose(fp);
	return 0;
}