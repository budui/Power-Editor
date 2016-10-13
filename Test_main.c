#include "Text.h"
#include <stdio.h>

int main()
{
	Text *txt = text_load("E:\\tools\\BC\\DISK_C\\learnc\\test_hz.txt");
	FILE *fp = fopen(".\\core.log", "w");
	Iterator it;
	char c;

	text_insert(txt, 2, "(hhhh)", 6);
	text_insert(txt, 2, "[aa]", 4);
	text_delete(txt, 6, 3);

	getchar();
	it = text_iterator_get(txt, 0);
	while (text_iterator_byte_next(&it,&c))
	{
		if (ISGBK(c))
		{
			putchar('*');
		}
		else
		{
			putchar(c);
		}
	}
#ifdef DEBUG
	test_print_buffer(txt, fp);
	test_print_piece(txt, fp);
	test_print_current_action(txt, fp);
#endif // DEBUG


	text_free(txt);
	fclose(fp);
	getchar();
	return 0;
}