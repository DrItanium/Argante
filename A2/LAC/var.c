#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lstring.h"
#include "main.h"

/* Variables and register lists */

static mVar *create_var(string id) {
	mVar *q;
	q=malloc(sizeof(mVar));
	q->strreftype=STRREF_NOT;
	q->id=id;
	q->ptrderef=0;
	return q;
}

mVar *Var_Ignore() {
	mVar *var=create_var(NULL);
	var->type.basetype=TypeIgnore;
	var->type.pointerct=0;
	var->kind=VarConst; /* Good as any */
	return var;
}

mVar *Var_Errno() {
	mVar *var=create_var(NULL);
	var->type.basetype=TypeUnsigned;
	var->type.pointerct=0;
	var->kind=VarErrno;
	return var;
}

mVar *Var_Register(string id) {
	mVar *var=create_var(id);
	/* Don't know what yet... */
	var->type.basetype=TypeIgnore;
	var->type.pointerct=0;
	var->kind=VarRegister;
	return var;
}

mVar *Var_Global(string id) {
	mVar *var=create_var(id);
	/* Don't know what yet... */
	var->type.basetype=TypeIgnore;
	var->type.pointerct=0;
	var->kind=VarGlobal;
	return var;
}

mVar *Var_ConstU(unsigned val) {
	mVar *var=create_var(NULL);
	var->type.basetype=TypeUnsigned;
	var->type.pointerct=0;
	var->kind=VarConst;
	var->u.constnt_u=val;
	return var;
}

mVar *Var_ConstF(float val) {
	mVar *var=create_var(NULL);
	var->type.basetype=TypeFloat;
	var->type.pointerct=0;
	var->kind=VarConst;
	var->u.constnt_f=val;
	return var;
}

mVar *VarStrAddr(string what) {
	mVar *var=create_var(SymbolizeString(what));
	var->type.basetype=TypeUnsigned;
	var->type.pointerct=1;
	var->kind=VarConst;
	var->strreftype=STRREF_ADDR;
	return var;
}

mVar *VarStrLen(string what) {
	mVar *var=create_var(what);
	var->type.basetype=TypeUnsigned;
	var->type.pointerct=0;
	var->kind=VarConst;
	var->strreftype=STRREF_LEN;
	var->u.constnt_u=StringLen(what);
	return var;
}

