#include <stdio.h>
#include <stdlib.h>
#include "text.h"

void showfile(Text *txt);
void delenter(char * str);

int main(void)
{
	Text *txt = text_load(".\\thefile.txt");
	Iterator it;
	char c;
	int len = 0;
	int start, end;
	size_t pos = 0,i;
	char str[30] = { 0 };
	Iterator its = iterator_get(txt, text_size(txt));
	ClipBorad *cli = clipborad_init();
	Filerange r;
	freopen(".\\editor.log", "w", stderr);
	
	showfile(txt);
	printf("Power-editor cmd version.\n");
	printf("s for save, ! for quit.\n");
	printf("r for redo, u for undo, i for insert, d for delete a for append.\n");
	printf("c for copy, t for cut, p for paste\n");
	printf("f for search");
	printf("----------------------------\n");
	while ((c = getchar()))
	{
		while(getchar() != '\n');
		switch (c)
		{
		case '!':
			if (text_modified(txt)) {
				printf("has not save file, save?\n");
				printf("y for save, n for not save, c for cancel.\n");
				switch (getchar())
				{
				case 'y':
					text_save(txt,"2.txt");
					break;
				case 'n':
					break;
				case 'c':
					continue;
					break;
				}
			}
			while (getchar() != '\n');
			break;
		case 'f':
			switch (getchar())
			{
			case 'p':
				printf("search start at: ");
				scanf("%d", &pos);
				while (getchar() != '\n');
				printf("find content: ");
				fgets(str, 30, stdin);
				delenter(str);
				printf("find at :%d\n", text_find_prev(txt, pos, str));
				break;
			case 'n':
				printf("search start at: ");
				scanf("%d", &pos);
				while (getchar() != '\n');
				printf("find content: ");
				fgets(str, 30, stdin);
				delenter(str);
				printf("find at :%d\n", text_find_next(txt, pos, str));
				break;
				break;
			}
			break;
		case 'c':
			printf("copy start and end:  ");
			scanf("%d %d", &start, &end);
			r.start = start;
			r.end = end;
			while (getchar() != '\n');
			text_copy(cli, txt, &r);
			break;
		case 't':
			printf("cut start and end:  ");
			scanf("%d %d", &start, &end);
			r.start = start;
			r.end = end;
			while (getchar() != '\n');
			text_cut(cli, txt, &r);
			break;
		case 'p':
			printf("paste at: ");
			scanf("%d", &pos);
			while (getchar() != '\n');
			text_paste(cli, txt, pos);
			break;
		case 's':
			if (!text_saveas(txt, "C:\\Users\\Jinxiapu\\Desktop\\1.txt"))
				printf("save file failed.\n");
			break;
		case 'u':
			text_undo(txt);
			break;
		case 'r':
			text_redo(txt);
			break;
		case 'i':
			printf("insert at: ");
			scanf("%d", &pos);
			while(getchar() != '\n');
			printf("insert content: ");
			fgets(str, 30, stdin);
			delenter(str);
			text_insert(txt, pos, str, strlen(str));
			break;
		case 'd':
			printf("delete at and len: ");
			scanf("%d %d", &pos, &len);
			while(getchar() != '\n');
			text_delete(txt, pos, len);
			test_print_piece(txt);
			break;
		case 'a':
			pos = text_size(txt);
			printf("insert content: ");
			fgets(str, 30, stdin);
			delenter(str);
			text_insert(txt, pos, str, strlen(str));
			break;
		default:
			break;
		}
		printf("---------------");
		printf("file size %d", text_size(txt));
		showfile(txt);
		fflush(stdin);
	}
	
#ifdef DEBUG
	test_print_buffer(txt);
	test_print_piece(txt);
	test_print_current_action(txt);
#endif // DEBUG
	clipborad_close(cli);
	text_free(txt);
	getchar();
	return 0;
}

void showfile(Text *txt)
{
	Iterator it = iterator_get(txt, 0);
	printf("Now the file is :\n");
	do 
	{
		printf("%.*s", it.end - it.start, it.start);
	} while (iterator_next(&it));
	putchar('\n');
}

void delenter(char * str)
{
	size_t i;
	for (i = 0; str[i]; i++)
	{
		if (str[i] == '\n')
		{
			str[i] = '\0';
		}
	}
}