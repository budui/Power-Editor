#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Public.h"
//typedef enum { false, true } bool;

#define DATA_LIMITS 20

typedef struct Iter Iter;
struct Iter
{
	void *current;
	void *prev;
	void *next;
	const char *data;
	size_t len;
	bool(*iterate)(Iter *iter, void* node);
};


int draw_list(const char *name[], const char *data[], int n,size_t data_size);
bool write_single_node(FILE *fp, const char *name, const char *data, const char *f0, const char *f1, size_t data_size);
bool write_single_egde(FILE *fp, const char *prev, const char *next, bool flag);

int draw_node(Iter *iter);
void Iter_init(Iter *iter, void *current, void *prev, void *next, const char *data, size_t len, bool(*iterate)(Iter *iter, void* node));





