#include "Draw_List.h"

#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // 用来检测内存泄漏





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

FILE *gv_init()
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
bool gv_close(FILE *fp)
{
	fputc('}', fp);
	if (fclose(fp) != 0)
	{
		fprintf(stderr, "%s\n", "Error in closing files\n");
		return false;
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
void write_nodes_by_array(FILE *fp, const char *node_name[], const char* node_data[], int n, size_t data_size)
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
void write_egdes_by_array(FILE *fp, const char *prev[], const char *next[], int n)
{
	int i;
	for (i = 0; i < n; ++i)
		write_single_egde(fp, prev[i], next[i]);
}
void write_single_egde(FILE *fp, const char *prev, const char *next)
{
	fprintf(fp, "    %s:f1:c -> %s;\n", prev, next);
	fprintf(fp, "    %s:f0:c -> %s;\n", next, prev);
}

