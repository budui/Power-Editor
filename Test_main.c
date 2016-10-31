#include <stdio.h>
#include "text.h"


#ifdef DEBUG
FILE *logfile;
#endif // DEBUG

int main(void)
{
	Text *txt = text_load("E:\\tools\\BC\\DISK_C\\learnc\\test_hz.txt");
	Iterator it;
	char c;

	text_insert(txt, 1, "bbb", 3);
	text_insert(txt, 0, "aaa", 3);
	
	it = iterator_get(txt, 0);
	while (iterator_byte_next(&it,&c))
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
	logfile = fopen(".\\editor.log", "w");
	test_print_buffer(txt);
	test_print_piece(txt);
	test_print_current_action(txt);
	fclose(logfile);
#endif // DEBUG

	text_free(txt);
	getchar();
	return 0;
}