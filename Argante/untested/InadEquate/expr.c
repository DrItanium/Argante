#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "tree.h"
#include "i.tab.h"

/*
 * EXPRESSIONS
 */

/*
 * Constant -> Expr
 */
AExpr ExprValuef(float val)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);
	
	out->kind=EValue;
	out->type=Type(TFloat);
	out->u.fval=val;
	return out;
}

AExpr ExprValuei(int val, AType type)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);
	
	out->kind=EValue;
	out->type=type;
	out->u.ival=val;
	return out;
}

/* XXX XXX XXX XXX STRING TABLE */
AExpr ExprString(string val)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);
	
	out->kind=EValue;
	out->type=Type(TString);
	out->u.ival=(int) val;
	return out;
}

/*
 * Please excuse the mammoth complexity of this function.
 * 99% of it is due to the constant optimizations,
 * and the fact they have to deal with types.
 */
#define _EBO2i(n, OP) \
		switch(a2->type->kind) { \
			case TSigned: out->u.ival=n OP s2; break; \
			case TUnsigned: out->u.ival=n OP u2; break; \
			case TFloat: out->u.ival=n OP f2; break; \
			default: EM_ErrorMessage("Can't perform binary operations on argument 2"); \
		}
#define _EBO2f(n, OP) \
		switch(a2->type->kind) { \
			case TSigned: out->u.fval=n OP s2; break; \
			case TUnsigned: out->u.fval=n OP u2; break; \
			case TFloat: out->u.fval=n OP f2; break; \
			default: EM_ErrorMessage("Can't perform binary operations on argument 2"); \
		}
#define _EBO2s(n, OP) \
		switch(a2->type->kind) { \
			case TSigned: out->u.ival=n OP s2; break; \
			case TUnsigned: out->u.ival=n OP u2; break; \
			default: EM_ErrorMessage("Can't perform binary operations on argument 2"); \
		}
#define _EBO1(OP) \
		switch(a1->type->kind) { \
			case TSigned: _EBO2i(s1, OP); break; \
			case TUnsigned: _EBO2i(u1, OP); break; \
			case TFloat: _EBO2f(f1, OP); break; \
			default: EM_ErrorMessage("Can't perform binary operations on argument 1"); \
		}
#define _EBO1s(OP) \
		switch(a1->type->kind) { \
			case TSigned: _EBO2s(s1, OP); break; \
			case TUnsigned: _EBO2s(u1, OP); break; \
			default: EM_ErrorMessage("Can't perform binary operations on argument 1"); \
		}
	
AExpr ExprBinOp(AExpr a1, AExpr a2, int op)
{
	AExpr out;
	signed s1, s2;
	unsigned u1, u2;
	float f1, f2;
	
	out=calloc(sizeof(struct _AExpr), 1);

	/* Because pointers and strings are not EValues,
	 * (we don't know their values until assembly or runtime)
	 * we can't optimize operations on them away. */
	if (a1->kind == EValue && a2->kind== EValue) /* Constant optimization */
	{
		out->kind=EValue;
		out->type=a1->type; /* This had BETTER be ok */

		/* This is truly ghastly. We have umpteen different types and umpteen operators.
		 * But it's all for the greater good. */
		s1=a1->u.ival; s2=a2->u.ival; 
		u1=a1->u.ival; u2=a2->u.ival; 
		f1=a1->u.fval; f2=a2->u.fval;
		switch(op) {
			case '+': _EBO1(+); break;
			case '-': _EBO1(-); break;
			case '/': _EBO1(/); break;
			case '*': _EBO1(*); break;
			/* Here on, ops on floats are undefined */
			case '&': _EBO1s(&); break;
			case '|': _EBO1s(|); break;
			case '%': _EBO1s(%); break;
			/* And Now, the Booleans */
			case '>':
				out->type=Type(TUnsigned);
				_EBO1(>); break;
			case '<':
				out->type=Type(TUnsigned);
				_EBO1(<); break;
			case LEQ:
				out->type=Type(TUnsigned);
				_EBO1(<=); break;
			case GEQ:
				out->type=Type(TUnsigned);
				_EBO1(>=); break;
			case EQ:
				out->type=Type(TUnsigned);
				_EBO1(==); break;
			case NEQ:
				out->type=Type(TUnsigned);
				_EBO1(!=); break;
			default: EM_ErrorMessage("Unknown binary op!");
		}

		free(a1); free(a2);
	} else {
		out->kind=EBinOp;
		out->type=a1->type; /* hmmm.... */
		out->u.binop.a1=a1; out->u.binop.a2=a2; out->u.binop.op=op;
	}
	return out;

}

extern AExpr ExprUnOp(AExpr a, int op)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);
	out->type=a->type;

	if (a->kind == EValue) /* Constant optimization */
	{
		out->kind=EValue;
		switch(a->type->kind) {
		case TSigned:
		{
			switch(op) {
			case '-': out->u.ival= -((signed) a->u.ival); break;
			case '!': out->u.ival= ~((signed) a->u.ival); break; 
			default: EM_ErrorMessage("Bad UnOp on signed");
			}
		} break;
		case TUnsigned:
		{
			switch(op) { /* Hmm... Is this 100% desired? */
			case '-': out->u.ival= -((unsigned) a->u.ival); break;
			case '!': out->u.ival= ~((unsigned) a->u.ival); break;
			default: EM_ErrorMessage("Bad UnOp on unsigned");
			}
		} break;
		case TFloat:
		{
			switch(op) { /* What on earth would ~(0.1) be? */
			case '-': out->u.fval= -((float) a->u.fval); break;
			default: EM_ErrorMessage("Bad UnOp on float");
			}
		} break;
		default:
		EM_ErrorMessage("Unary operands are not defined for this type.");
		}

		free(a);
	} else {
		out->kind=EUnOp;
		out->u.unop.a=a;
		out->u.unop.op=op;
	}
	return out;
}

AExpr ExprCast(AExpr expr, AType type)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);

	/* After about three of these damned constant
	 * optimizations you really want to say "to hell with
	 * efficiency". Please don't :)
	 */
	if (expr->kind==EValue)
	{
		out->kind=EValue;
		out->type=type;
		switch(type->kind) {
		case TSigned:
			switch(expr->type->kind) {
			case TSigned:
				out->u.ival = expr->u.ival;
				break;
			case TUnsigned:
				out->u.ival = (signed int) expr->u.ival;
				break;
			case TFloat:
				out->u.ival = (signed int) expr->u.fval;
				break;
			default:
				EM_ErrorMessage("That's one strange value!!");
				goto NoCastOptim;
			}
			break;
		case TUnsigned:
			switch(expr->type->kind) {
			case TSigned:
				out->u.ival = (unsigned int) expr->u.ival;
				break;
			case TUnsigned:
				out->u.ival = expr->u.ival;
				break;
			case TFloat:
				out->u.ival = (unsigned int) expr->u.fval;
				break;
			default:
				EM_ErrorMessage("That's one strange value!!");
				goto NoCastOptim;
			}

			break;
		case TFloat:
			switch(expr->type->kind) {
			case TSigned:
				out->u.fval = (signed) expr->u.ival;
				break;
			case TUnsigned:
				out->u.fval = (unsigned) expr->u.ival;
				break;
			case TFloat:
				out->u.fval = expr->u.fval;
				break;
			default:
				EM_ErrorMessage("That's one strange value!!");
				goto NoCastOptim;
			}
			break;
		default:
			EM_ErrorMessage("What on earth are you casting a value to this for!?");
			goto NoCastOptim;
		}
	}

	NoCastOptim:
	out->kind=ECast;
	out->type=type;
	out->u.cast=expr;
	return out;
}

AExpr ExprAssign(string id, AExpr val)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);
	
	/* Do we want to unchain e=a=1 expressions? */
//	if (val->type->kind == EAssign) val=val->u.assn.a;

	out->kind=EAssign;
	out->type=val->type;
	out->u.assn.id=id;
	out->u.assn.a=val;
	return out;
}

AExpr ExprSizeof(string id)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);
	
	out->kind=ESizeof;
	out->type=Type(TUnsigned);
	out->u.id=id;
	return out;
}

AExpr ExprNew(AType type)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);
	
	out->kind=ENew;
	out->type=TypePointer(type);
	return out;
}

AExpr ExprCall(string id, AExprList arglist)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);
	
	out->kind=ECall;
	out->u.call.id=id;
	out->u.call.args=arglist;
	return out;
}

AExpr ExprID(string id)
{
	AExpr out;
	out=calloc(sizeof(struct _AExpr), 1);
	
	out->kind=EID;
	out->u.id=id;
	return out;
}

