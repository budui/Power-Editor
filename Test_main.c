#include <stdio.h>
#include <stdlib.h>
#include "text.h"

void showfile(Text *txt);
void delenter(char * str);

int main(void)
{
	Text *txt = text_load("E:\\tools\\BC\\DISK_C\\learnc\\test_hz.txt");
	Iterator it;
	char c;
	int len = 0;
	size_t pos = 0,i;
	char str[30] = { 0 };
	Iterator its = iterator_get(txt, text_size(txt));
	freopen(".\\editor.log", "w", stderr);
	
	showfile(txt);
	printf("Power-editor cmd version.\n");
	printf("s for save, ! for quit.\n");
	printf("r for redo, u for undo, i for insert, d for delete a for append.\n");
	printf("----------------------------\n");
	while ((c = getchar())!='!')
	{
		while(getchar() != '\n');
		switch (c)
		{
		case 's':
			if (!text_save(txt, "C:\\Users\\Jinxiapu\\Desktop\\1.txt"))
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
		showfile(txt);
		fflush(stdin);
	}
	
#ifdef DEBUG
	test_print_buffer(txt);
	test_print_piece(txt);
	test_print_current_action(txt);
#endif // DEBUG
	
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