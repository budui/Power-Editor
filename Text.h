#ifndef __TEXT_H__ 
#define __TEXT_H__

#include "util.h"
#include <stdlib.h>
#include <stdio.h>

/* This is the core of editor, it provides some functions to complete basic
* editor's work. such as load file, free file, insert and delete.
* Note:
*	1)
*   2) Usage: #include "text.h"
*   3) After all works completed, remeber to use func text_free to free
*      all memery!
*   4) Function when func return bool, "true" means it run correctly and
*      "false" means it run incorrectly.
*   5) Typically, pos means global position in bytes from start of a file.
* coded by WangRui.
*/

#define EPOS ((size_t)-1)         /* invalid position */

/* These structs are typically used in core.
* but their pointers can be used outside core.
*/
typedef struct Text Text;
typedef struct Piece Piece;
typedef struct TextSave TextSave;
typedef struct ClipBorad ClipBorad;

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
bool text_save(Text*, const char *filename);
/* write the text content to the given file descriptor `fd'. Return the
* number of bytes written or -1 in case there was an error. */
int text_write(Text*, int fd);
int text_write_range(Text *txt, Filerange *range, int fd);

/* Functions for editing. */

/* Insert len bytes starting from data at pos which has to be
* in the interval [0, text_size(txt)] */
bool text_insert(Text*, size_t pos, const char *data, size_t len);
/* Delete len bytes starting from pos */
bool text_delete(Text *txt, size_t pos, size_t len);
/* Delete chars within filerange r. */
bool text_delete_range(Text *txt, Filerange *r);

size_t text_range_size(const Filerange *r);
size_t text_size(Text *txt);

/* Functions for re/undo. */
void text_snapshot(Text *txt);
/* undo/redo to the last snapshotted state. returns the position where
* the change occured or EPOS if nothing could be {un,re}done. */
size_t text_undo(Text*);
size_t text_redo(Text*);

/* Functions for iterating. */

Iterator iterator_get(Text *txt, size_t pos);

bool iterator_next(Iterator *it);
bool iterator_prev(Iterator *it);

/* Filter out sentinel nodes */
bool iterator_valid(const Iterator *it);

bool iterator_byte_next(Iterator *it, char *b);
bool iterator_byte_prev(Iterator *it, char *b);


void clipborad_close(ClipBorad *cli);
ClipBorad *clipborad_init(void);
bool text_copy(ClipBorad *cli, Text *txt, Filerange *r);
bool text_paste(ClipBorad *cli, Text *txt, size_t pos);
bool text_cut(ClipBorad *cli, Text *txt, Filerange *r);

#ifdef DEBUG
void test_print_buffer(Text *txt);
void test_print_piece(Text *txt);
void test_print_current_action(Text *txt);
#endif // DEBUG
#endif