#include "Draw_List.h"

#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // 用来检测内存泄漏

FILE *gv_init();
static bool gv_close(FILE *fp);
static void write_nodes_by_array(FILE *fp, const char *node_name[], const char* node_data[], int n, size_t data_size);
static void write_egdes_by_array(FILE *fp, const char *prev[], const char *next[], int n);
static bool draw_node_info(FILE *fp, Iter *p, size_t limits);


static FILE *gv_init()
{
	FILE *fp = fopen("Graph.gv", "w");
	if (!fp)
	{
		fprintf(stderr, "%s\n", "couldn't open the file");
		return NULL;
	}
	fprintf(fp, "digraph List {\n    graph[rankdir = \"LR\"];\n    node[shape=record];\n    edge[arrowhead=vee, arrowtail=dot, dir=both, tailclip=false];\n");

	return fp;
}

static bool gv_close(FILE *fp)
{
	fputc('}', fp);
	if (fclose(fp) != 0)
	{
		fprintf(stderr, "%s\n", "Error in closing files\n");
		return false;
	}
	return true;
}

static void write_nodes_by_array(FILE *fp, const char *node_name[], const char* node_data[], int n, size_t data_size)
{
	int i;
	for (i = 0; i < n; ++i)
	{
		if (i == 0)
			write_single_node(fp, node_name[i], node_data[i], "NULL", "", data_size);
		else if (i == n - 1)
			write_single_node(fp, node_name[i], node_data[i], "", "NULL", data_size);
		else
			write_single_node(fp, node_name[i], node_data[i], "", "", data_size);
	}
}

static void write_egdes_by_array(FILE *fp, const char *prev[], const char *next[], int n)
{
	int i;
	for (i = 0; i < n; ++i)
		write_single_egde(fp, prev[i], next[i],true);
		write_single_egde(fp, prev[i], next[i],false);
}

static bool draw_node_info(FILE *fp, Iter *p, size_t limits)
{
	char num_temp[20] = { 'N' };
	char num_temp_t[20] = { 'N' };

	limits = limits < p->len ? limits : p->len;
	//bool node_prev = false, node_next = false;//Node前后是否为NULL
	if (!p->prev && !p->next)
	{
		write_single_node(fp, itoa((int)p->current, &num_temp[1], 16) - 1, p->data, "NULL", "NULL", limits);
	}
	if (!p->prev && p->next)
	{
		write_single_node(fp, itoa((int)p->current, &num_temp[1], 16) - 1, p->data, "NULL", "", limits);
		write_single_egde(fp, itoa((int)p->current, &num_temp[1], 16) - 1, itoa((int)p->next, &num_temp_t[1], 16) - 1, true);
	}
	if (p->prev && !p->next)
	{
		write_single_node(fp, itoa((int)p->current, &num_temp[1], 16) - 1, p->data, "", "NULL", limits);
		write_single_egde(fp, itoa((int)p->prev, &num_temp[1], 16) - 1, itoa((int)p->current, &num_temp_t[1], 16) - 1, false);
	}
	if (p->prev && p->next)
	{
		write_single_node(fp, itoa((int)p->current, &num_temp[1], 16) - 1, p->data, "", "", limits);
		write_single_egde(fp, itoa((int)p->prev, &num_temp[1], 16) - 1, itoa((int)p->current, &num_temp_t[1], 16) - 1, false);
		write_single_egde(fp, itoa((int)p->current, &num_temp[1], 16) - 1, itoa((int)p->next, &num_temp_t[1], 16) - 1, true);
	}
	return true;
}


bool write_single_node(FILE *fp, const char *name, const char *data, const char *f0, const char *f1, size_t data_size)
{
	if (name && f0 && f1)
	{
		if (data) {
			void *temp = malloc(data_size + 1);
			temp = memcpy(temp, (const void*)data, data_size);
			*((char*)temp + data_size) = '\0';
			fprintf(fp, "    %s[label=\"<f0>%s|<f1>%s|%s\"];\n", name, f0, f1, (char*)temp);
			free((void*)temp);
		}
		else
			fprintf(fp, "    %s[label=\"<f0>%s|<f1>%s|%s\"];\n", name, f0, f1, "");
		return true;
	}
	return false;
}

bool write_single_egde(FILE *fp, const char *prev, const char *next, bool flag)
{
	if (prev && next) {
		if (flag) {
			fprintf(fp, "    %s:f1:c -> %s;\n", prev, next);
		}
		else {
			fprintf(fp, "    %s:f0:c -> %s;\n", next, prev);
		}
		return true;
	}
	return false;
}

int draw_node(Iter *iter)
{
	FILE *fp = gv_init();
	if (!fp)
		return 1;

	for (; iter->next; iter->iterate(iter,iter->next)) {
		draw_node_info(fp, iter, DATA_LIMITS);
	}
	//链表最后一个节点
	draw_node_info(fp, iter, DATA_LIMITS);

	if (!gv_close(fp))
		return 2;

	return 0;
}

void Iter_init(Iter *iter, void *current, void *prev, void *next, const char *data, size_t len, bool(*iterate)(Iter *iter, void* node))
{
	iter->current = current;
	iter->prev = prev;
	iter->next = next;
	iter->data = data;
	iter->len = len;
	iter->iterate = iterate;
}

int draw_list(const char *name[], const char *data[], int n, size_t data_size)
{
	FILE *fp = gv_init();
	if (!fp)
	{
		return 1;
	}
	write_nodes_by_array(fp, name, data, n, data_size);
	write_egdes_by_array(fp, name, name + 1, n);

	if (!gv_close(fp))
	{
		return 2;
	}

	return 0;
}

