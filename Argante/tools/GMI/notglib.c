#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "notglib.h"

void g_free(void *a)
{
	free(a);
}

char *g_strdup(char *in)
{
	char *out;
	out=strdup(in);
	if (!out)
	{
		perror("g_strdup");
		exit(1);
	}
	return out;
}

GList *g_list_append(GList *g, void *data)
{
	GList *z, *new;

	if (g) {
		z=g;
		while (z->next) z=z->next;
		new=z->next=malloc(sizeof(GList));
	} else {
		g=new=malloc(sizeof(GList));
		z=NULL;
	}

	new->prev=z;
	new->next=NULL;
	new->data=data;
	return g;
}

