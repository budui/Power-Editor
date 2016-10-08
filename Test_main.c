#include "Text.h"
#include <stdio.h>

void  iterate(Text txt, size_t start, size_t end, bool func(Iterator it));


int main()
{
	Text *txt = text_load("E:\\tools\\BC\\DISK_C\\learnc\\test.txt");
	FILE *fp = fopen(".\\core.log", "w");
	Iterator it;

	text_insert(txt, 2, "(hhhh)", 6);
	text_insert(txt, 2, "[aa]", 4);
	text_delete(txt, 3, 3);

	getchar();
	it = text_iterator_get(txt, 0);
	do
	{
		printf("%.*s",  (int)(it.end - it.start), it.start);
	} while (text_iterator_next(&it));

	test_print_buffer(txt, fp);
	test_print_piece(txt, fp);
	test_print_current_action(txt, fp);

	text_free(txt);
	fclose(fp);
	getchar();
	return 0;
}