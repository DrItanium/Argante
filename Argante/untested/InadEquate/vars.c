#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "tree.h"
#include "linearize.h"

/*
 * Note that everything works well even if two things hash
 * to the same value; we just go through the next pointers.
 * 
 * However, if two things have exactly the same ID anyway,
 * they get shadowed - and they should be!
 */
static int hash(void *id)
{
	return ((int) id) % VARTABSIZE;
}

AVar findVar_i(AFunc in, int x, void *id)
{
	AVar z;
	z=in->vars[x];

	while(z)
	{
		if (z->id == id) return z;
		z=z->next;
	}
	return NULL;
}

/* Wooeeep! Don't use this unless the code REALLY wants to
 * find a variable 'koz it might escape a lower level one */
AVar findVar(AFunc in, void *id)
{
	AVar z;
	int x=hash(id);

	z=findVar_i(in, x, id);
	if (z) return z;
	
	while(1)
	{
		in=in->parent;
		if (!in) break;

		z=findVar_i(in, x, id);
		if (z) {
			z->escaperefs++;
			return z;
		}
	}
		
	return NULL;

}

AVar makeVar(string id, AFunc in)
{
	AVar a;
	int x;
	
	x=hash(id);
	a=findVar_i(in, x, id);

	fprintf(stderr, "defining %s\n", StringToChar(id));
	/* Note this doesn't warn about shadowed globals. */
	if (a)
		EM_ErrorMessage("Variable shadowed / redeclared");

	a=calloc(sizeof(struct _AVar), 1);

	a->id=id;
	a->next=in->vars[x];
	in->vars[x]=a;
	
	return a;
}

