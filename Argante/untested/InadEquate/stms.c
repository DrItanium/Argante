#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "tree.h"

/* -- List manipulations -- */

AStm StmCompound(AStm s1, AStm s2)
{
	AStm oldlast;

	if (!s2) return s1;
	if (!s1) return s2;

	oldlast=s1->last;
	if (s2->last) s1->last=s2->last; else s1->last=s2;
	
	if (!oldlast) oldlast=s1;
	
	oldlast->next=s2;
	s2->prev=oldlast;

	return s1;
}

/* -- Labels -- */
static int labelnum;

AStm LabelGenGeneric(string id)
{
	AStm new;
	new=calloc(sizeof(struct _AStm), 1);
	new->kind=SLabel;
	new->u.to=id;
	return new;
}

static string genLabelID()
{
	string s;
	char buf[30];
	
	labelnum++;
	snprintf(buf, sizeof(buf) - 1, "_L%x", labelnum);
	buf[sizeof(buf) - 1]=0;
	s=String(buf);

	/* Create it */
	return s;
}

/* FIXME: HASH TABLE */
typedef struct _ALabel *ALabel;
struct _ALabel {
	string id;
	string genid;
	ALabel next;
};
ALabel Lhead;

/* The trouble arises when we use GOTO to something
 * declared after we want to jump... */

static ALabel FindByAsked(string id)
{
	ALabel search=Lhead;
	while (search)
	{
		if (search->id==id) return search;
		search=search->next;
	}

	search=calloc(sizeof(struct _ALabel), 1);
	search->id=id;
	search->genid=genLabelID();
	search->next=Lhead;
	Lhead=search;

	return NULL;
}

/* Create a label we're given. It's bad practice to
 * give the user what they actually asked for :) */
AStm LabelGenNamed(string id)
{
	ALabel l;

	l=FindByAsked(id);
	return LabelGenGeneric(l->genid);
}

AStm StmGotoGeneric(string id)
{
	AStm new;
	new=calloc(sizeof(struct _AStm), 1);
	new->kind=SGoto;
	new->u.to=id;
	return new;
}

AStm StmGoto(string label)
{
	ALabel l; 
	/* Look up what was asked for... */
	l=FindByAsked(label);
	return StmGotoGeneric(l->genid);
}

/* -- Loops -- */
AStm StmWhileHead(AExpr expr, AStm code)
{
	AStm out;
	string lname;
	/* Create a label for the head */
	lname=genLabelID();
	out=LabelGenGeneric(lname);
	/* Now, tack a GOTO HEAD onto the end of the code */
	code=StmCompound(code, StmGotoGeneric(lname));
	/* Now, the whole thing's HEAD: IF expr { code; GOTO HEAD; } */
	out=StmCompound(out, StmIfElse(expr, code, NULL));
	return out;
}

AStm StmUntilHead(AExpr expr, AStm code)
{
	AStm out;
	string lname;
	/* Create a label for the head */
	lname=genLabelID();
	out=LabelGenGeneric(lname);
	/* Now, tack a GOTO HEAD onto the end of the code */
	code=StmCompound(code, StmGotoGeneric(lname));
	/* Now, the whole thing's HEAD: IF expr {} ELSE { code; GOTO HEAD; } */
	out=StmCompound(out, StmIfElse(expr, NULL, code));
	return out;
}

/* do {} while () loops... a little simpler */
AStm StmWhileTail(AExpr expr, AStm code)
{
	AStm out;
	string lname;
	/* Create a label for the head */
	lname=genLabelID();
	out=LabelGenGeneric(lname);
	/* Now, tack on the code */
	out=StmCompound(out, code);
	/* Now, tack on the IF expr GOTO HEAD; */
	out=StmCompound(out, StmIfElse(expr, StmGotoGeneric(lname), NULL));
	return out;
}

AStm StmUntilTail(AExpr expr, AStm code)
{
	AStm out;
	string lname;
	/* Create a label for the head */
	lname=genLabelID();
	out=LabelGenGeneric(lname);
	/* Now, tack on the code */
	out=StmCompound(out, code);
	/* Now, tack on the IF expr {} ELSE GOTO HEAD; */
	out=StmCompound(out, StmIfElse(expr, NULL, StmGotoGeneric(lname)));
	return out;
}

/* -- Conditionals -- */
AStm StmIfElse(AExpr expr, AStm branch1, AStm branch2)
{
	AStm out;
	string to1, to2;

	out=calloc(sizeof(struct _AStm), 1);
	out->kind=SCJump;
	out->u.cjump.cond=expr;
	out->u.cjump.to=(to1=genLabelID()); /* TRUE branch */
	to2=genLabelID(); /* end */

	/* Note this isn't as effective as it could be,
	 * if there's no FALSE branch. In that case it
	 * would be better to invert expr and cjump end */
	
	/* Tack on FALSE branch, and JMP to end */
	out=StmCompound(out, branch2);
	out=StmCompound(out, StmGotoGeneric(to2));
	/* Tack on TRUE label + code */
	out=StmCompound(out, LabelGenGeneric(to1));
	out=StmCompound(out, branch1);
	out=StmCompound(out, LabelGenGeneric(to2));
	return out;
}

/* -- Boring conventional stuff -- */
AStm StmReturn(AExpr expr)
{
	AStm new;
	new=calloc(sizeof(struct _AStm), 1);
	new->kind=SReturn;
	new->u.val=expr;
	return new;
}

AStm StmRaise(AExpr expr)
{
	AStm new;
	new=calloc(sizeof(struct _AStm), 1);
	new->kind=SRaise;
	new->u.val=expr;
	return new;

}

AStm StmExpr(AExpr in)
{
	AStm new;
	new=calloc(sizeof(struct _AStm), 1);
	new->kind=SExpr;
	new->u.val=in; /* Check for side-effects? */
	return new;

}

/* -- Memory mixers -- */
AStm StmDestroy(string id)
{
	AStm new;
	new=calloc(sizeof(struct _AStm), 1);
	new->kind=SDestroy;
	new->u.to=id;
	return new;
}

AStm StmResize(string id, AType newsize)
{
	AStm new;
	new=calloc(sizeof(struct _AStm), 1);
	new->kind=SResize;
	new->u.resize.id=id;
	new->u.resize.newsize=newsize;
	return new;
}

/* The Temporary Additions
 * These are isolated from the stms on first pass.
 */
AStm FuncGen(string id, AType retType, AParmList parmlist, string errhandler, AStm code)
{
	AStm new;
	new=calloc(sizeof(struct _AStm), 1);
	new->kind=SFunc;

	new->u.func.id=id;
	new->u.func.retType=retType;
	new->u.func.parmlist=parmlist;
	new->u.func.errhandler=errhandler;
	new->u.func.code=code;
	return new;
}

AStm VarGen(string id, AType type)
{
	AStm new;
	new=calloc(sizeof(struct _AStm), 1);
	new->kind=SVar;

	new->u.var.id=id;
	new->u.var.type=type;

	return new;
}

/* ParmList / ArgList; yeuch */

AParmList ParmListAdd(AParmList to, AType type, string id)
{
	AParmList add=ParmListNew(type, id);
	
	if (!to->last) to->last=to;
	to->last->next=add;
	to->last=add;
	
	return to;
}

AParmList ParmListNew(AType type, string id)
{
	AParmList new;
	new=calloc(sizeof(struct _AParmList), 1);
	new->id=id;
	new->type=type;
	return new;
}

AExprList ArgListAdd(AExprList to, AExpr new)
{
	AExprList add=ArgListNew(new);
	
	if (!to->last) to->last=to;
	to->last->next=add;
	to->last=add;
	
	return to;
}

AExprList ArgListNew(AExpr first)
{
	AExprList new;
	new=calloc(sizeof(struct _AExprList), 1);
	new->expr=first;
	new->type=first->type; /* hmm? */
	return new;
}


