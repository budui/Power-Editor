#ifndef __TEXT_UTIL_H__
#define __TEXT_UTIL_H__

#include "util.h"
#include "text.h"
#include <stdio.h>
#include <stdlib.h>

/* Some utility functions for text editing 
* provide some regex search function for highlight.
* This moudel need a large char buf for save char.
* to use this moudel first need to malloc a large buf.
*/


/* Search str form start_pos to end. 
* 1) start position will be return if matched.
* 2) if not have matched anything, 0 will be return.
* 3) substring must be a string!('\0' is nessary.)
*/
size_t text_search(Text *txt, size_t start_pos, const char *substring); 

bool text_cut(Text *txt, size_t start, size_t end);
bool text_paste(Text *txt, size_t pos, const char * start, size_t len);
bool text_copy(Text *txt, size_t start, size_t end);

#endif
