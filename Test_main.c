#include "Text.h"
#include <stdio.h>
#include <string.h>

#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // 用来检测内存泄漏

int main()
{

	Text *txt = text_load("C:\\Users\\Jinxiapu\\Desktop\\Temp\\1.txt");
	
	//while (1)
	//{
	//	size_t pos, len;
	//	char temp[10];
	//	printf("pos = ");
	//	scanf("%d", &pos);
	//	while (getchar() != '\n')
	//		continue;
	//	printf("contents: ");
	//	gets_s(temp,10);
	//	if (strcmp(temp,"quit") == 0)
	//	{
	//		break;
	//	}
	//	len = strlen(temp);
	//	text_insert(txt, pos, temp, len);
	//}
	text_insert(txt, 12, "fjdskl", 6);
	text_delete(txt, 0, 5);
	text_delete(txt, 1, 1);
	text_insert(txt, 1, "---", 3);
	test_show_info(txt);
	text_free(txt);

#ifdef DEBUG
	_CrtDumpMemoryLeaks();
#endif // 用来检测内存泄漏

}

