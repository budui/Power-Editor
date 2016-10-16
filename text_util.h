/* Some utility functions for text editing */
#ifndef __TEXT_UTIL_H
#define __TEXT_UTIL_H

#include "Text.h"
#include "Public.h"

/* Search str form start_pos to end. 
* 1) start position will be return if matched.
* 2) if not have matched anything, 0 will be return.
* 3) substring must be a string!('\0' is nessary.)
*/
size_t text_search(Text *txt, const size_t start_pos, const char *substring); 

bool text_cut(Text *txt, size_t start, size_t end);
bool text_paste(Text *txt, size_t pos);
bool text_copy(Text *txt, size_t start, size_t end);

#endif
