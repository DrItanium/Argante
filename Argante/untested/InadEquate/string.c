/*
 * String hash table.
 * 
 * This is for ensuring only one copy of "hit me" is around
 * at the same time, which makes it much more efficient to
 * find functions et al.
 *
 * We'll only ever need to add strings to this.
 */
#include <stdlib.h>
#include <string.h>
#include "string.h"


#define TABSIZE 150

struct _string {
	char *ch;
	string next;
	
};

static string table[TABSIZE];

static int hash(char *s)
{
	return (s[0] << 8 | s[1]) % TABSIZE;
}

static string findString(int x, char *s)
{
	string z=table[x];

	while(z)
	{
		if (!strcmp(z->ch, s)) return z;
		z=z->next;
	}
	return NULL;
}

string String(char *s)
{
	string a=malloc(sizeof(struct _string));
	string b;
	int x;
	
	x=hash(s);

	b=findString(x, s);
	if (b) return b;

	a->ch=strdup(s);
	a->next=table[x];
	table[x]=a;
	
	return a;
}

char *StringToChar(string s)
{
	if (!s) return "(nuLL>";
	return s->ch;
}
