#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lstring.h"
#include "main.h"

mType *Type(TypeBase base) {
	mType *r;
	r=malloc(sizeof(mType));
	r->basetype=base;
	r->pointerct=0;
	return r;
}

mIType *IType(string id, mType *t) {
	mIType *r;
	if(!t) {
		EM_Error("semval lost");
		exit(-1);
	}

	r=malloc(sizeof(mIType));
	r->id=id;
	memcpy(&r->type, t, sizeof(mType));
	free(t); /* ? */
	return r;
}

TypeList *TypeList_AddType(TypeList *to, mType *next) {
	TypeList *noo;
	noo=malloc(sizeof(TypeList));
	noo->next=NULL;
	noo->last=noo;
	memcpy(&noo->type, next, sizeof(mType));
	free(next); /* ? */
	if (to) {
		to->last->next=noo;
		to->last=noo;
		return to;
	}
	return noo;
}

ITypeList *ITypeList_AddType(ITypeList *to, mIType *next) {
	ITypeList *noo;
	noo=malloc(sizeof(ITypeList));
	noo->next=NULL;
	noo->last=noo;
	memcpy(&noo->itype, next, sizeof(mIType));
	free(next); /* ? */
	if (to) {
		to->last->next=noo;
		to->last=noo;
		return to;
	}
	return noo;
}

TypeList *TypeList_fromI(ITypeList *from) {
	TypeList *noo=NULL;
	ITypeList *f;
	mType *t;

	while(from) {
		f=from->next;
		t=malloc(sizeof(mType));
		memcpy(t, &from->itype.type, sizeof(mType));
		TypeList_AddType(noo, t);
		free(from);
		from=f;
	}
	return noo;
}

RegList *Reglist_AddVar(RegList *to, mVar *next) {
	RegList *noo;
	noo=malloc(sizeof(RegList));
	noo->next=NULL;
	noo->last=noo;
	noo->var=next;
	if (to) {
		to->last->next=noo;
		to->last=noo;
		return to;
	}
	return noo;
}

