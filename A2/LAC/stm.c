#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lstring.h"
#include "main.h"

static Stm *create_stm(void) {
	Stm *a;
	a=malloc(sizeof(Stm));
	a->last=a;
	a->next=NULL;
	a->file=EM_FileName;
	a->lineno=EM_LineNo;
	return a;
}

Stm *StmList_Join(Stm *a, Stm *b) {
	if (!a) return b;
	if (!b) return a;

	a->last->next=b;
	a->last=b->last;
	return a;
}

Stm *Stm_VarDef(mIType *type)
{
	Stm *n=create_stm();
	n->kind=StmVarDef;
	n->u.vardef.id=type->id;
	memcpy(&n->u.vardef.type, &type->type, sizeof(mType));
	free(type); /* ? */
	return n;
}

Stm *Stm_Assign(mVar *to, AOper op, mVar *from)
{
	Stm *n=create_stm();
	n->kind=StmAssign;
	n->u.assign.to=to;
	n->u.assign.from=from;
	n->u.assign.kind=op;
	return n;
}

Stm *Stm_Label(string label)
{
	Stm *n=create_stm();
	n->kind=StmLabel;
	n->u.label=label;
	return n;
}

Stm *Stm_Goto(string label)
{
	Stm *n=create_stm();
	n->kind=StmGoto;
	n->u.label=label;
	return n;
}

Stm *Stm_Handler(string label)
{
	Stm *n=create_stm();
	n->kind=StmHandler;
	n->u.label=label;
	return n;
}

Stm *Stm_Call(string func)
{
	Stm *n=create_stm();
	n->kind=StmCall;
	n->u.func=func;
	return n;
}

Stm *Stm_Return() {
	Stm *n=create_stm();
	n->kind=StmReturn;
	return n;
}

Stm *Stm_Wait(mVar *a) {
	Stm *n=create_stm();
	n->kind=StmWait;
	n->u.wait.w=a;
	return n;
}

Stm *Stm_Raise(mVar *a) {
	Stm *n=create_stm();
	n->kind=StmRaise;
	n->u.wait.w=a;
	return n;
}

Stm *Stm_If(mVar *a, CmpOper cmp, mVar *b, string label)
{
	Stm *n=create_stm();
	n->kind=StmIf;
	n->u.sif.a1=a;
	n->u.sif.a2=b;
	n->u.sif.kind=cmp;
	n->u.sif.label=label;
	return n;
}

Stm *Stm_ReglistSet(RegList *reg)
{
	Stm *n=create_stm();
	n->kind=StmReglist;
	n->u.reglist.r=reg;
	n->u.reglist.kind=RSet;
	return n;
}

Stm *Stm_ReglistGet(RegList *reg)
{
	Stm *n=create_stm();
	n->kind=StmReglist;
	n->u.reglist.r=reg;
	n->u.reglist.kind=RDeref;
	return n;
}

Stm *Stm_Alloc(mVar *addrdest, mVar *size) {
	Stm *n=create_stm();
	n->kind=StmAlloc;
	n->u.alloc.addrdest=addrdest;
	n->u.alloc.size=size;
	return n;
}

Stm *Stm_Realloc(mVar *addrdest, mVar *size) {
	Stm *n=create_stm();
	n->kind=StmRealloc;
	n->u.alloc.addrdest=addrdest;
	n->u.alloc.size=size;
	return n;
}

Stm *Stm_Finalize(mVar *addr) {
	Stm *n=create_stm();
	n->kind=StmFinalize;
	n->u.fin.addr=addr;
	return n;
}

Stm *Stm_Unfinalize(mVar *addr) {
	Stm *n=create_stm();
	n->kind=StmUnfinalize;
	n->u.fin.addr=addr;
	return n;
}

Stm *Stm_Dealloc(mVar *addr) {
	Stm *n=create_stm();
	n->kind=StmUnfinalize;
	n->u.fin.addr=addr;
	return n;
}

