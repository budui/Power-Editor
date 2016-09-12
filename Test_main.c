#include "Text.h"
#include <stdio.h>



int main()
{
	Text *txt = text_load(".\\test.txt");
	FILE *fp = fopen(".\\core.log", "w");
	char *insert[] = { "jjjj", "****", "||", "--------" };
	size_t pos[] = { 2, 6, 1, 3 };
	size_t len[] = { 4, 4, 2, 7 };
	size_t i;
#ifdef DEBUG
	fprintf(fp, "***[OPERATION]***\n");
#endif // DEBUG
	for (i = 0; i < 4; i++)
	{
#ifdef DEBUG
		fprintf(fp, "[INSERT] %u :  %.*s\n", pos[i], len[i], insert[i]);
#endif // DEBUG
		text_insert(txt, pos[i], insert[i], len[i]);
	}
	test_print_buffer(txt, fp);
	test_print_piece(txt, fp);
	test_print_current_action(txt, fp);
	text_free(txt);
	fclose(fp);
	return 0;
}