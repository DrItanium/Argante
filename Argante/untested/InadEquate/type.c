#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "tree.h"

typedef struct _ATChain *ATChain;

struct _ATChain {
	AType a;
	ATChain next;
};

static ATChain Chead;

/*
 * This module would be much much much simpler (+faster) if I wasn't
 * conservative about memory. (If you've read expr.c you have
 * permission to laugh)
 */
static void AddType(AType toadd)
{
	ATChain new;
	new=malloc(sizeof(struct _ATChain));
	new->a=toadd;
	new->next=Chead;
	Chead=new;
}


/* Type. Returns an AType with the specified kind.
 * Should only be used for TUnsigned, TSigned, TFloat.
 * (and maybe TString.)
 */
AType Type(int kind)
{
	ATChain search=Chead;
	AType n;
	while (search)
	{
		if (search->a->kind == kind) return search->a;
		search=search->next;
	}

	n=calloc(sizeof(struct _AType), 1);
	n->kind=kind;
	AddType(n);
	return n;
}

AType TypePointerArray(AType arraytype)
{
	ATChain search=Chead;
	AType n;
	while (search)
	{
		if (search->a->kind == TPointerArray && search->a->u.PointerTo == arraytype) return search->a;
		search=search->next;
	}

	n=calloc(sizeof(struct _AType), 1);
	n->kind=TPointerArray;
	n->u.PointerTo=arraytype;
	AddType(n);
	return n;
}

AType TypePointer(AType pointertype)
{
	ATChain search=Chead;
	AType n;
	while (search)
	{
		if (search->a->kind == TPointer && search->a->u.PointerTo == pointertype) return search->a;
		search=search->next;
	}

	n=calloc(sizeof(struct _AType), 1);
	n->kind=TPointer;
	n->u.PointerTo=pointertype;
	AddType(n);
	return n;
}

AType TypeArray(AType pointertype, AExpr size)
{
	ATChain search=Chead;
	AType n;

	/* You would use a fval in an ARRAY DIMENSION? */
	if (size->type->kind != TUnsigned && size->type->kind != TSigned)
	{
		EM_ErrorMessage("I don't know how to create an array with _that_ dimension...");
		return NULL; /* EGREGIOUS */
	}
	
	while (search)
	{
		if (search->a->kind == TArray &&
			search->a->u.PointerTo == pointertype &&
			(size == search->a->size /* Unlikely... */ ||
			(search->a->size->kind == EValue && size->kind == EValue &&
			 size->u.ival == search->a->size->u.ival) /* See above */
			)) return search->a;
		search=search->next;
	}

	n=calloc(sizeof(struct _AType), 1);
	n->kind=TArray;
	n->u.PointerTo=pointertype;
	n->size=size;
	AddType(n);
	return n;
}

/* Postpone the lookup */
AType TypeID(string id)
{
	ATChain search=Chead;
	AType n;

	while (search)
	{
		if (search->a->kind == TID && search->a->u.id == id) return search->a;
		search=search->next;
	}

	n=calloc(sizeof(struct _AType), 1);
	n->kind=TID;
	n->u.id=id;
	AddType(n);
	return n;
}
