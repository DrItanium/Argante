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
		 *
		 * (And syntactically very difficult :)
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

static void Argify(AFunc f, AParmList args)
{
	AParmList pl;
	AVar v;
	int cs=0,cf=0,cu=0;

	/* Turn the ParmList into some juicy vars */
	pl=args;
	while(pl)
	{
		v=makeVar(pl->id, f);
		v->kind=VParam;
		v->type=pl->type;
		if (!TypeCmp(v->type, Type(TSigned))) /* Y-iiiiike! */
		{
			v->regType=cs | REG_S;
			cs++;
			if (cs > 15) EM_ErrorMessage("Excessive signed arguments.");
		} else if (!TypeCmp(v->type, Type(TFloat))) {
			v->regType=cf | REG_F;
			cf++;
			if (cf > 15) EM_ErrorMessage("Excessive float arguments.");
		} else {
			v->regType=cu | REG_U;
			cu++;
			if (cu > 15) EM_ErrorMessage("Excessive unsigned/pointer arguments.");
		}

		pl=pl->next;
	}
}

/* Extract function and typedefs */
AFunc Phase1a(AStm from, AParmList args) {
	AFunc f, t;
	AStm next;
	AVar v;

	f=calloc(sizeof(struct _AFunc), 1);
	f->parent=ParentFunc;
	f->code=from;
	if (LastFunc) LastFunc->next=f;
	LastFunc=f;
	ParentFunc=f;

	Argify(f, args);

	while(from)
	{
		/* Produce somewhat informative location information */
		if (from->EMLineNo) EM_LineNo=from->EMLineNo;

		next=from->next;
		switch(from->kind)
		{
			case SFunc:
				t=Phase1a(from->u.func.code, from->u.func.parmlist);
				t->retType=from->u.func.retType;
				t->errhandler.s=from->u.func.errhandler;
				t->id=from->u.func.id;

				if (f->code==from) f->code=next;
				RemoveStm(from); free(from);
				break;
			case SVar:
				v=makeVar(from->u.var.id, f);
				v->kind=VDeclared;
				v->type=from->u.var.type;

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
 * define TID's, define compound types, free the typedef chain 
 */
void Phase1()
{
	AFunc f;
	ANamedType nt, nt2;
	/* Recursively process everything */
	f=NullFunc=Phase1a(prog, NULL);
	/* Resolve ErrHandlers */
	while(f)
	{
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
	/* Resolve type ID's */
	TypeDoTID();	
	/* Free the NamedType chain. After this, FindType won't work... */
	nt=TypeHead;
	while (nt)
	{
		nt2=nt->next;
		free(nt);
		nt=nt2;
	}
	TypeHead=NULL;
	/* Now, go through the CompoundTypes and give them locations */

	/* We're done... for now */
}

/* Phase2:
 * convert -> and [] into *(x+15) style
 * do constant optimization
 * typechecking
 * linearize into temporaries... */

static int tempnum;
static AFunc nowF;
static AStm lastS;
static int sidefx;

static string genTempID()
{
	string s;
	static char buf[30];
	
	tempnum++;
	snprintf(buf, sizeof(buf) - 1, "_T%x", tempnum);
	buf[sizeof(buf) - 1]=0;
	s=String(buf);

	/* Create it */
	return s;
}


/* Construct a temporary */
static AExpr Temporize(AExpr t)
{
	string id;
	AStm news;
	AVar temp;
	AExpr new1, new2;

	if (t->kind == EVar || t->kind == EValue) return t;
	
	id=genTempID();
	temp=makeVar(id, nowF);
	temp->kind=VTemp;
	temp->type=t->type;
	
	new1=calloc(sizeof(struct _AExpr), 1);
	new1->kind=EVar;
	new1->type=t->type;
	new1->u.var=temp;

	new2=calloc(sizeof(struct _AExpr), 1);
	new2->kind=EAssign;
	new2->u.assn.to=new1;
	new2->u.assn.a=t;

	news=calloc(sizeof(struct _AStm), 1);
	news->kind=SExpr;
	news->u.val=new2;

	news->next=lastS->next;
	news->prev=lastS;
	lastS->next=news;
	if (news->next) news->next->prev=news;

	/* We have the potential to get ourselves in a terrible pointer snarl */
	new2=malloc(sizeof(struct _AExpr));
	memcpy(new2, new1, sizeof(struct _AExpr));
	return new1;
}

/* Constant optimization and linearization. Recursively. */
AExpr Phase2aa(AExpr in)
{
	if (!in) {
		EM_ErrorMessage("NULL expression");
		return in;
	}
	switch(in->kind)
	{
		case EVar:
		case EValue:
			break;
		case EID:
			in->kind=EVar;
			in->u.var=findVar(nowF,in->u.id);
			break;
		case EAssign:
			sidefx++;
			in->u.assn.to=Phase2aa(in->u.assn.to);
			if (in->u.assn.to->kind != EVar) /* Can hardly temporize THIS... heh */
			{
				EM_ErrorMessage("You can't assign to this value!");
				yynerrs++;
				break;
			}
			
			in->u.assn.a=Temporize(Phase2aa(in->u.assn.a));
			break;
		case EUnOp: /* TODO - constants */
			in->u.unop.a=Temporize(Phase2aa(in->u.unop.a));
			break;
		case EBinOp:
			in->u.binop.a1=Temporize(Phase2aa(in->u.binop.a1));
			in->u.binop.a2=Temporize(Phase2aa(in->u.binop.a2));
			break;
		case ECast: /* ECast is implemented with a MOV from one reg to another :) */
			in->u.cast=Temporize(Phase2aa(in->u.cast));
		case ECall:
			/* foreach I in in->exprlist do { } */
			sidefx++;
		case ENew: /* BLEH!!! */
		case ESizeof: /* Converted into array[-1]... later :) */
			break;
		default:
			EM_ErrorMessage("Unknown expression type (internal error?)");
	}

	return in;
}

void Phase2a(AStm from)
{
	AStm next;
	while(from)
	{
		/* Produce somewhat informative location information */
		if (from->EMLineNo) EM_LineNo=from->EMLineNo;

		lastS=from;
		next=from->next;
		switch(from->kind)
		{
			case SRaise:
				from->u.val=Phase2aa(from->u.val);
				break;
			case SReturn:
				if (from->u.val) from->u.val=Phase2aa(from->u.val);
				break;
			case SExpr:
				sidefx=0;
				from->u.val=Phase2aa(from->u.val);
				if (!sidefx) EM_ErrorMessage("Statement contains no side effects");
				
				break;
			case SCJump:
				break;
// SDestroy, SResize, 
			case SLabel:
				break;
			case SFunc:
			case SVar:
			case SType:
				EM_ErrorMessage("Internal error (phase1 didn't work?)");
			default:
				fprintf(stderr, "kind = %d", from->kind);
				EM_ErrorMessage("Unknown stmkind in Phase2");
				yynerrs++; /* Internal errors are pretty much fatal :) */
		}
		from=next;
	}
}

void Phase2()
{
	AFunc f;
	f=NullFunc;
	
	while (f)
	{
		nowF=f;
		Phase2a(f->code);
		f=f->next;
	}
}
