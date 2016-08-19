#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Public.h"



int draw_list(const char *name[], const char *data[], int n,size_t data_size);
bool write_single_node(FILE *fp, const char *name, const char *data, const char *f0, const char *f1, size_t data_size);
void write_single_egde(FILE *fp, const char *prev, const char *next);
FILE *gv_init();
bool gv_close(FILE *fp);
void write_nodes_by_array(FILE *fp, const char *node_name[], const char* node_data[], int n, size_t data_size);
void write_egdes_by_array(FILE *fp, const char *prev[], const char *next[], int n);