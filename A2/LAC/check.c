#include "autocfg.h"
#include "compat/bzero.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lstring.h"
#include "main.h"
#include "function.h"

/* Pretty much do-nothing functions ATM */

typedef struct _prototype prototype;
struct _prototype {
	string id;
	TypeList *ret;
	TypeList *args;
	enum { ProtoCall, ProtoSys } kind;
	prototype *next;
};

static prototype **protohash;
static int protohashsz;
static int protohashused;

static inline unsigned hashfunc(void *p, int size) {
	return ((int) p) % size;
}

static prototype *AddProto(string id) {
	prototype *q, *new;
	if (protohashused >= 0.8 * protohashsz) {
		int s;
		int i;
		prototype **newt;
	
		s=protohashsz + 15;
		newt=calloc(sizeof(prototype *), s);
		/* Rehash */
		
		for(i=0;i<protohashsz;i++) {
			q=protohash[i];
			while (q) {
				new=q->next;
				q->next=newt[hashfunc(q->id, s)];
				newt[hashfunc(q->id, s)]=q;
				q=new;
			}
		}
		if (protohash) free(protohash);
		protohash=newt;
		protohashsz=s;
	}
	protohashused++;
	new=malloc(sizeof(prototype));
	new->id=id;
	new->next=protohash[hashfunc(id, protohashsz)];
	protohash[hashfunc(id, protohashsz)]=new;
	return new;
}

/* a bit lAME! */
int Call_IsSyscall(string id) {
	prototype *q;
	if (!protohashsz) goto dummy;

	q=protohash[hashfunc(id, protohashsz)];
	while (q && q->id != id) q=q->next;
	if (q) return (q->kind == ProtoSys) ? 1 : 0;
dummy:
	EM_Warn("implicit declaration of call %s", StringToChar(id));
	q=AddProto(id);
	q->ret=NULL;
	q->args=NULL;
	q->kind=ProtoCall;
	return 0;
}


void PrototypeT(string id, TypeList *ret, TypeList *args) {
	prototype *x=AddProto(id);
	x->ret=ret;
	x->args=args;
	x->kind=ProtoCall;
}

void PrototypeI(string id, TypeList *ret, ITypeList *args) {
	prototype *x=AddProto(id);
	x->ret=ret;
	x->args=TypeList_fromI(args);
	x->kind=ProtoCall;
}

void PrototypeSCT(string id, TypeList *ret, TypeList *args) {
	prototype *x=AddProto(id);
	x->ret=ret;
	x->args=args;
	x->kind=ProtoSys;
	return;
}

void PrototypeSCI(string id, TypeList *ret, ITypeList *args) {
	prototype *x=AddProto(id);
	x->ret=ret;
	x->args=TypeList_fromI(args);
	x->kind=ProtoSys;
	return;
}

function *global=NULL; /* For easy reference :) */
function *chain=NULL;

mVar *FindVarInFunc(string v, function *f) {
	mVarH *q;
	if (!f->varhashsz) return NULL;
	q=f->varhash[hashfunc(v, f->varhashsz)];
	while(q && q->v->id != v) q=q->next;
	if (q) return q->v;
	else return NULL;
}

static void AddVarToFunc(mVar *v, function *f) {
	mVarH *q, *new;

	if(FindVarInFunc(v->id,f)) {
		EM_Error("Duplicate definition of %s", StringToChar(v->id));
		return;
	}

	if (f->varhashused >= 0.8 * f->varhashsz) {
		int s;
		int i;
		mVarH **newt;
	
		s=f->varhashsz + 10; /* How many vars are you going to use!? */
		newt=calloc(sizeof(mVarH *), s);
		/* Rehash */
		for(i=0;i<f->varhashsz;i++) {
			q=f->varhash[i];
			while (q) {
				new=q->next;
				q->next=newt[hashfunc(q->v->id, s)];
				newt[hashfunc(q->v->id, s)]=q;
				q=new;
			}
		}
		if (f->varhash) free(f->varhash);
		f->varhash=newt;
		f->varhashsz=s;
	}
	new=malloc(sizeof(mVarH));
	new->v=v;
	new->next=f->varhash[hashfunc(v->id, f->varhashsz)];
	f->varhash[hashfunc(v->id, f->varhashsz)]=new;
	f->varhashused++;
}

void Function(string id, TypeList *ret, ITypeList *args, Stm *code) {
	ITypeList *q;
	Stm *head=NULL;
	RegList *rg=NULL;
	int rc; /* Reg Count */
	function *func;
	mVar *var;
	mIType *waste;
	prototype *proto=AddProto(id);
	
	func=malloc(sizeof(function));
	bzero(func, sizeof(function));
	func->id=id;
	
	rc=0;
	/* Stick in the code to grab the args */
	while(args) {
		q=args->next;
		
		rg=Reglist_AddVar(rg, Var_Register(args->itype.id));

		/* Stm_VarDef free's our memory */
		waste=malloc(sizeof(mIType));
		memcpy(waste, &args->itype, sizeof(mIType));
		head=StmList_Join(head, Stm_VarDef(waste));
		args=q;
		rc++;
	}
	head=StmList_Join(head, Stm_ReglistGet(rg));
	code=StmList_Join(head, code);
	func->code=code;
	
	/* This wastes args, but that's OK */
	proto->ret=ret;
	proto->kind=ProtoCall;
	proto->args=TypeList_fromI(args);
	args=NULL;
	
	func->declregs=rc;
	/* How many regs do we clobber by return? */
	rc=0;
	while(ret) {
		rc++; /* Can't free - used in prototype */
		ret=ret->next;
	}
	if (rc > func->declregs) func->declregs=rc;

	/* Now... how many regs do we clobber with ['s and ]'s? */
	func->callregs=func->declregs;
	
	while(code) {
		if (code->kind == StmReglist) {
			rg=code->u.reglist.r;
			rc=0;
			while(rg) {
				rc++;
				rg=rg->next;
			}
			if (rc > func->callregs) func->callregs=rc;
		}
		code=code->next;
	}
	code=func->code;

	/* Okay. Now we can start passing out registers for vars... */
	func->usedregs=func->callregs;
	
	if (id) { /* No point allocating registers for globals! */
		while(code) {
			if (code->kind == StmVarDef) {
				var=Var_Register(code->u.vardef.id);
				memcpy(&var->type, &code->u.vardef.type, sizeof(mType));
				if (func->usedregs < ERRNO_REG) {
					var->u.registr=func->usedregs;
					func->usedregs++;
				} else {
					EM_Error("Too many registers used");
				}
				AddVarToFunc(var, func);
			}
			code=code->next;
		}
	} else {
		func->usedregs=0;
		if (global) {
			EM_Error("internal error? (too many global funcs)");
		} else {
			global=func;
		}
		while(code) {
			if (code->kind == StmVarDef) {
				var=Var_Global(code->u.vardef.id);
				memcpy(&var->type, &code->u.vardef.type, sizeof(mType));
				AddVarToFunc(var, func);
			}
			code=code->next;
		}
	}
	code=func->code;
	/*
	 * We can't yet go through generating code :)
	 * because not all globals have been defined,
	 * prototypes created, etc etc. ad infinitum.
	 * Let it stew for now.
	 */
	func->next=chain;
	chain=func;
}

