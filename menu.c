#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "menu.h"
#include "config.h"


#define MAX_DEEPTH 4 // max deepth of menu tree.
#define END_OF_TREE -2 // have no bro or no child.
#define FUNC_NOT_SET -1 // have not set the func for menu_id node.
#define MAX_LINE_SIZE 20 

typedef struct menu
{
	char *name; // name of menu.
	int func_id; // function will be run when click this menu.
	USERTYPE permission;
	int child_id; // first child in tree. NULL when no child.
	int bro_id; // next brother in tree.
	int offest;
}menu;


static int linecount(FILE *fp);
static int loadmenu(char *filename, FILE * *menu);
static int readline(FILE *fp, char *buf, int *spcount, USERTYPE *u);
static bool initmenu(menu *m, char * name, USERTYPE u, int child, int bro,int offest);
static bool maketree(FILE *config, menu *root, int size, char *buf);

static bool initmenu(menu *m, char * name, USERTYPE u, int child, int bro,int offest)
{
	if (!m)
		return false;
	m->name = name;
	m->bro_id = bro;
	m->child_id = child;
	m->func_id = FUNC_NOT_SET;
	m->permission = u;
	m->offest = offest;
	return true;
}

/* read one of file. return the count of name which has been deleted useless '\n' and ' '. */
static int readline(FILE *fp, char *buf, int *spcount, USERTYPE *u)
{
	char line[MAX_LINE_SIZE] = {0};
	int i,j;

	if (!buf || !spcount || !u || !fp)
		return -1;

	if (!fgets(line, sizeof(line), fp))
		return EOF;

	// count the number of spaces of this line.
	for (*spcount = 0; line[*spcount] == ' '; (*spcount)++)
		;
	// check whether this line contains # which means permission.
	for (i = *spcount; i < MAX_LINE_SIZE; i++)
	{
		if (line[i] == '#')
		{   
			*u = atoi(&line[i + 1]);
			line[i] = '\0';
			// delete useless space.
			for (j = i - 1; line[j] == ' '; line[j] = '\0',j--)
				;
			strcpy(buf, line + *spcount);
			return i - *spcount;
		}
	}

	strcpy(buf, line + *spcount);
	*u = GUEST; // permission is GUEST if not specially show permission.
	i = strlen(buf);
	//delete '\n' before '\0'
	if (buf[i - 1] == '\n')
	{
		buf[i - 1] = '\0';
		return i - 1;
	}
	else
	{
		return i;
	}
}

/* load menu config and return the size of file. */
static int loadmenu(char *filename, FILE * *menu)
{
	int size;

	if (!(*menu = fopen(filename, "rt")))
	{
#ifdef DEBUG
		fprintf(stderr, "[ERROR] Can not open menu config file. | FILE: %s LINE: %d\n", __FILE__, __LINE__);
		fprintf(stderr, "          Detail: %s\n", strerror(errno));
#endif // DEBUG
		return -1;
	}

	fseek(*menu, 0L, SEEK_END);
	size = ftell(*menu);
	rewind(*menu);
	
	return size;
}

/* count the line size of the file. */
static int linecount(FILE *fp)
{
	int ch, count = 0;
	
	if (!fp)
		return -1;

	do
	{
		ch = fgetc(fp);
		if (ch == '\n')
			count++;
	} while (ch != EOF);
	/* last line doesn't end with a new line!
	 but there has to be a line at least before the last line */
	if (ch != '\n' && count != 0)
		count++;
	rewind(fp);
	return count;
}

/* translate config file to a menu tree in memery. */
static bool maketree(FILE *config, menu *root, int size, char *buf)
{
	int temp[MAX_DEEPTH];
	int sptemp[MAX_DEEPTH];
	int deepth = 0, nodenum = 0;
	int spcount,namecount;
	USERTYPE u;
	int i;
	
	if (!config || !root)
		return false;
	
	temp[deepth] = 0;
	sptemp[deepth] = -1;

	while ((namecount = readline(config, buf, &spcount, &u)) != EOF && nodenum <= size)
	{
		nodenum++;
		if (spcount > sptemp[deepth])
		{
			deepth++;
			temp[deepth] = nodenum;
			(root + temp[deepth - 1])->child_id = nodenum;
			initmenu(root + temp[deepth], buf, u, END_OF_TREE, END_OF_TREE,nodenum);
			sptemp[deepth] = spcount;
		}
		else if(spcount == sptemp[deepth])
		{
			temp[deepth] = nodenum;
			(root + nodenum - 1)->bro_id = nodenum;
			initmenu(root + temp[deepth], buf, u, END_OF_TREE, END_OF_TREE,nodenum);
		}
		else 
		{
			for (i = 0; i <= MAX_DEEPTH+1; i++)
			{
				if (sptemp[i] == spcount)
					break;
			}
			if (i == MAX_DEEPTH+1)
			{
#ifdef DEBUG
				fprintf(stderr, "[WARNING] config file incorrect space number. (%s)",buf);
#endif // DEBUG
				fclose(config);
				return false;
			}
			(root + temp[i])->bro_id = nodenum;
			temp[i] = nodenum;
			deepth = i;
			initmenu(root + temp[i], buf, u, END_OF_TREE, END_OF_TREE,nodenum);
		}
		buf = buf + namecount + 1;
	}
	fclose(config);
	return true;
}

menu *GetMenu(LANGUAGE lang)
{
	FILE *config = NULL;
	menu *root = NULL;
	int filesize, linesize;
	
	switch (lang)
	{
	case CHINESE:
		filesize = loadmenu(CONFIG_MENU_CN, &config);
		break;
	case ENGLISH:
		filesize = loadmenu(CONFIG_MENU_EN, &config);
		break;
	default:
		break;
	}
	
	if (!config)
		return NULL;
	linesize = linecount(config);
	root = (menu *)calloc(linesize + 1, sizeof(menu));
	/* root->name save the buffer which contains all names. */
	root->name = (char *)malloc(filesize);

	if (!maketree(config, root, linesize, root->name))
		return NULL;
	return root;
}

void FreeMenu(menuptr root)
{
	if (!root)
		return;
	free(root->name);
	free(root);
	root = NULL;
}

menuptr GetMenuByNum(menuptr root,int num)
{
	menuptr m;
	int i = 1;
	if(!root)
		return NULL;
	
	for(m = FirsrChildMenu(root);m;i++)
	{
		if(i == num)
			break;
		m = NextBroMenu(m);
	}
		
	return m;
}

int MenuChildCount(menuptr root)
{
	menuptr m;
	int i = 0;
	if(!root)
		return -1;
	
	for(m = FirsrChildMenu(root);m;i++)
		m = NextBroMenu(m);
	return i;
}

int MenuID(menuptr m)
{
	if(!m)
		return -1;
	return m->offest;
}

menuptr NextBroMenu(menuptr m)
{
	menuptr root;
	if(!m)
		return NULL;
	root = m - m->offest;
	if(m->bro_id != END_OF_TREE)
		return (root + m->bro_id);
	return NULL;
}

menuptr FirsrChildMenu(menuptr m)
{
	menuptr root;
	if(!m)
		return NULL;
	root = m - m->offest;
	if(m->child_id != END_OF_TREE)
		return (root + m->child_id);
	return NULL;
}

const char *MenuName(menuptr m)
{
	if(!m)
		return NULL;
	return m->name;
}

int MenuChildNum(menuptr parent,menuptr child)
{
	menuptr m = FirsrChildMenu(parent);
	int i=0;
	do
	{
		i++;
		if(m == child)
			break;
	}while((m = NextBroMenu(m)) != NULL);
	if(m == NULL)
		return -1;
	return i;
}

menuptr GetRootMenu(menuptr m)
{
	if(!m)
		return NULL;
	return m - m->offest;
}

#ifdef DEBUG
void debug_Draw_menu(menuptr root, int linenum)
{
	int i = 1;
	menu node;

	if (!root)
	{
		fprintf(stderr, "[WARNING] root is NULL | FILE: %s LINE: %d\n", __FILE__, __LINE__);
		return;
	}

	for (node = *root; i <= linenum; i++)
	{
		node = *(root + i);
		fprintf(stderr, "[NODE]%s func:%d permission:%d, child:%d, bro:%d\n,offest", node.name, \
			node.func_id, node.permission, node.child_id, node.bro_id,node.offest);
			
	}
}
#endif // DEBUG

