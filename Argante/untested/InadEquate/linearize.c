/* (c) 2001 JSK - GPL */
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "tree.h"
#include "linearize.h"

extern AStm prog;

/* The Null Function is the great-grandaddy of all functions.
 * It's also the list head. */
AFunc NullFunc=NULL;
static AFunc LastFunc=NULL;
static AFunc ParentFunc=NULL;

typedef struct _ANamedType *ANamedType;
struct _ANamedType {
	string id;
	AType type;
	ANamedType next;
};

static ANamedType TypeHead=NULL;

/*
 * Mental note: a function sees all functions that are immediate children
 * of one of its ancestors.
 */
int CanFunctionSee(AFunc me, AFunc see)
{
	while(me) {
		/* We don't really need to check if
		 * see=me because that gets picked up next round.
		 * Unless you are trying to set the null function
		 * as your errhandler, which would be dumb.
		 */
		if (see->parent==me) return 1;
		me=me->parent;
	}
	return 0;
}

/* Find AFunc, given id */
AFunc FindFunction(AFunc perspective, string id)
{
	AFunc f;
	f=NullFunc;
	while(f)
	{
		if (id == f->id && CanFunctionSee(perspective, f)) return f;
		f=f->next;
	}
	return NULL;
}

/* Removes the given statement from the list */
AStm RemoveStm(AStm s)
{
	if (s->prev) s->prev->next=s->next;
	if (s->next) s->next->prev=s->prev;
	return s;
}

/* Creates a NamedType */
void Phase1a_type(AStm from)
{
	ANamedType t;
	t=calloc(sizeof(struct _ANamedType), 1);

	fprintf(stderr, "Type %s defined\n", StringToChar(from->u.var.id));
	t->id=from->u.var.id;
	t->type=from->u.var.type;
	t->next=TypeHead;
	TypeHead=t;
}

AType FindType(string id)
{
	ANamedType c;
	c=TypeHead;
	while(c)
	{
		if (c->id == id) return c->type;
		c=c->next;
	}
	return NULL;
}

/* Extract function and typedefs */
AFunc Phase1a(AStm from, AParmList args) {
	AFunc f, t;
	AStm next;
	AParmList pl;
	AVar v;

	f=calloc(sizeof(struct _AFunc), 1);
	f->parent=ParentFunc;
	f->code=from;
	if (LastFunc) LastFunc->next=f;
	LastFunc=f;
	ParentFunc=f;

	/* Turn the ParmList into some juicy vars */
	pl=args;
	while(pl)
	{

		pl=pl->next;
	}


	while(from)
	{
		next=from->next;
		switch(from->kind)
		{
			case SFunc:
				t=Phase1a(from->u.func.code);
				t->retType=from->u.func.retType;
				t->parmlist=from->u.func.parmlist;
				t->errhandler.s=from->u.func.errhandler;
				t->id=from->u.func.id;

				if (f->code==from) f->code=next;
				RemoveStm(from); free(from);
				break;
			case SVar:
				
				if (f->code==from) f->code=next;
				RemoveStm(from); free(from);
				break;
			case SType:
				Phase1a_type(from);
				if (f->code==from) f->code=next;
				RemoveStm(from); free(from);
				break;
			default: /* do nichts */
		}
		from=next;
	}
	ParentFunc=f->parent;
	return f;
}

/* Phase1: initial processing, split out functions + typedefs,
 * define TID's, free the typedef chain 
 */
void Phase1()
{
	AFunc f;
	ANamedType nt, nt2;
	/* Recursively process everything */
	f=NullFunc=Phase1a(prog);
	/* Resolve ErrHandlers */
	while(f)
	{
		fprintf(stderr, "Phase1b: %s\n", StringToChar(f->id));
		if (f->errhandler.s) {
		f->errhandler.f=FindFunction(f, f->errhandler.s);
		if (!f->errhandler.f)
		{
			yynerrs++;
			EM_ErrorMessage("Given error handler doesn't exist in this context");
		}
		} else f->errhandler.f=NULL;
		f=f->next;
	}
	/* Resolve types */
	TypeDoTID();	
	/* Free the NamedType chain. After this FindType won't work... */
	nt=TypeHead;
	while (nt)
	{
		nt2=nt->next;
		free(nt);
		nt=nt2;
	}
	TypeHead=NULL;
	/* We're done... for now */
}

/* Phase2:
 * finalize compound types.
 * 	type x = { array unsigned size sizeof(y) } won't work. 
 * convert -> and [] into *(x+15) style
 * elaborate on sizeof's
 * do constant optimization
 * linearize into temporaries... */
