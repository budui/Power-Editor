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
*/
bool text_saveas(Text*, const char *filename);
/*
* Save the whole text to txt->filename. Return 1 if succesful.
* return 0 if failed. return 2 if txt->filename = NULL.
*/
int text_save(Text*);

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

/* close clipborad. free all source. */
void clipborad_close(ClipBorad *cli);
/* malloc an area for clipborad, size is CONFIG_CLIPBORAD_SIZE.
* which means you can not copy or cut str longer than CONFIG_CLOPBOAD_SIZE.
*/
ClipBorad *clipborad_init(void);
/* copy text start at r.start and end before r.end, that's [start,end).
*  size_limits is ClipBorad's size.
*/
bool text_copy(ClipBorad *cli, Text *txt, Filerange *r);
/* cut text start at r.start and end before r.end, that's [start,end).
*  size_limits is ClipBorad's size.
*/
bool text_cut(ClipBorad *cli, Text *txt, Filerange *r);
/* paste text at pos. if there is no content in clipboard,return false.
* text content begin at 0.
*/
bool text_paste(ClipBorad *cli, Text *txt, size_t pos);
bool text_null_file(Text* txt);
bool text_modified(Text *txt);

/* text_find_next/prev return pos when not find.
* when finded, return the find pos.
* s must be a string not a char array.
*/
size_t text_find_next(Text *txt, size_t pos, const char *s);
size_t text_find_prev(Text *txt, size_t pos, const char *s);
#ifdef DEBUG
void test_print_buffer(Text *txt);
void test_print_piece(Text *txt);
void test_print_current_action(Text *txt);
#endif // DEBUG
#endif
