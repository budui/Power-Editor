#include "Text.h"
#include <stdio.h>

#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // DEBUG

int main()
{
	test("C:\\Users\\Jinxiapu\\Desktop\\Temp\\1.txt");	
#ifdef DEBUG
	_CrtDumpMemoryLeaks();
#endif // DEBUG

	
}

