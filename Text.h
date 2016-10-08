#ifndef __TEXT_H__ 
#define __TEXT_H__

#include "Public.h"
#include <stdlib.h>
#include <stdio.h>

#define EPOS ((size_t)-1)         /* invalid position */

/* This is the core of editor, it provides some functions to complete basic
* editor's work. such as load file, free file, insert and delete.
* Note:
*	1)
*   2) Usage: #include "Text.h"
*   3) After all works completed, remeber to use func text_free to free
*      all memery!
*   4) Function when func return bool, "true" means it run correctly and
*      "false" means it run incorrectly.
*   5) Typically, pos means global position in bytes from start of a file.
* coded by WangRui.
*/

/* These structs are typically used in core.
* but their pointers can be used outside core.
*/
typedef struct Text Text;
typedef struct Piece Piece;
typedef struct TextSave TextSave;

/* Filerange means range in bytes from start of the file to the end. */
typedef struct {
	size_t start; 
	size_t end;
} Filerange;

/* Iterator used to iterate when do something like searching, showing... */
typedef struct {
	const char *start;  /* data of this piece: [start, end) */
	const char *end;    
	const char *text;   /* current position(start <= text < end) */
	const Piece *piece; /* internal state do not touch! */
	size_t pos;         
} Iterator;



/* Functions for reading, writing files. */

/* Create a text instance populated with the given file content, if `filename'
* is NULL the text starts out empty */
Text *text_load(const char *filename);
/* Release all ressources associated with this text instance */
void text_free(Text* txt);
/* Save the whole text to the given `filename'. Return true if succesful.
* In which case an implicit snapshot is taken. The save might associate a
* new inode to file. */
//bool text_save(Text*, const char *filename);
/* write the text content to the given file descriptor `fd'. Return the
* number of bytes written or -1 in case there was an error. */
//ssize_t text_write(Text*, int fd);

/* Functions for editing. */

/* Insert len bytes starting from data at pos which has to be
* in the interval [0, text_size(txt)] */
bool text_insert(Text*, size_t pos, const char *data, size_t len);
/* Delete len bytes starting from pos */
bool text_delete(Text *txt, size_t pos, size_t len);
/* Delete chars within filerange r. */
bool text_delete_range(Text *txt, Filerange *r);


/* Functions for re/undo. */
void text_snapshot(Text *txt);


/* Functions for iterating. */

Iterator text_iterator_get(Text *txt, size_t pos);
bool text_iterator_next(Iterator *it);
bool text_iterator_prev(Iterator *it);
/* Filter out sentinel nodes */
bool text_iterator_valid(const Iterator *it);


#ifdef DEBUG
void test_print_buffer(Text *txt, FILE *log);
void test_print_piece(Text *txt, FILE *log);
void test_print_current_action(Text *txt, FILE *log);
#endif // DEBUG
#endif