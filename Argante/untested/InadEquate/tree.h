/*
 * Tree structures for handling abstractsyntax.
 * Vaguely based on 'modern compiler implementation in C, andrew appel',
 * which is a extreeeeemely good book.
 */

extern void EM_ErrorMessage(char *x);
extern int EM_LineNo;

typedef struct _AStm *AStm;
typedef struct _AExpr *AExpr;
typedef struct _AType *AType;
typedef struct _AExprList *AExprList;
typedef struct _AParmList *AParmList;
typedef struct _ACompoundField *ACompoundField;
typedef struct _AVar *AVar;

struct _AStm {
	enum { SGoto, SReturn, SRaise, SLabel, SExpr, SFunc, SCJump, SDestroy, SResize, SVar, SType } kind;

	union {
		string to;
		AExpr val;
		struct { AExpr cond; string to; } cjump;
		struct { string id; AType newsize; } resize;
		struct { string id; AType type; } var;
		struct { string id; AType retType; AParmList parmlist; string errhandler; AStm code; } func;
	} u;
	int EMLineNo;
	AStm next;
	AStm prev;
	AStm last; /* Last statement in this StmList */
};

struct _AExpr {
	enum { EID, EAssign, EUnOp, EBinOp, ECast, ESizeof, ENew, ECall, EValue, EVar } kind;
	AType type; /* float/unsigned/signed etc */
	union {
		string id;
		struct { AExpr a1; int op; AExpr a2; } binop;
		struct { AExpr to; AExpr a; } assn;
		struct { AExpr a; int op; } unop;
		struct { string id; AExprList args; } call;
		AVar var;
		AExpr cast;
		int ival;
		float fval;
	} u;
};

struct _AType {
	/* In Argante, we can't manipulate anything other than a dword.
	 * So all these types are either dwords or implicit pointers.
	 *
	 * TArray is just a TPointer that can't be assigned directly.
	 * TCompound is the same.
	 */
	enum { TID, TUnsigned, TSigned, TFloat, TString, TPointer, TPointerArray, TArray, TCompound } kind;
	AExpr size;
	union {
		AType PointerTo;
		ACompoundField FirstRec; /* Sadly I can't do my little CompactList as it reverses list orders */
		ACompoundField LastRec;
		string id;
	} u;
};

struct _ACompoundField {
	string id;
	AType type;
	int location;
	ACompoundField next;
	ACompoundField prev;
};

struct _AExprList {
	AExpr expr;
	AType type;
	AExprList next;
	AExprList last;
};
	
struct _AParmList {
	string id;
	AType type;
	AParmList next;
	AParmList last;
};

/*
 * -- Statements. --
 */
/* generic statement, warn if no side-effects */
extern AStm StmExpr(AExpr in);
/* statement list */
extern AStm StmCompound (AStm list, AStm new);

/* Loops + control exchange statements */
extern AStm StmWhileHead(AExpr expr, AStm code);
extern AStm StmUntilHead(AExpr expr, AStm code);
extern AStm StmWhileTail(AExpr expr, AStm code);
extern AStm StmUntilTail(AExpr expr, AStm code);
extern AStm StmIfElse(AExpr expr, AStm branch1, AStm branch2);

extern AStm StmReturn(AExpr expr);
extern AStm StmRaise(AExpr expr);
extern AStm StmGoto(string label);

/* Memory mixers */
extern AStm StmDestroy(string id);
extern AStm StmResize(string id, AType newsize);

/*
 * -- The Funny Stuff. --
 * Not control statements, not expressions.
 * What is this stuff?
 */
extern AParmList ParmListAdd(AParmList to, AType type, string id);
extern AParmList ParmListNew(AType type, string id);

extern AExprList ArgListAdd(AExprList to, AExpr new);
extern AExprList ArgListNew(AExpr first);

extern AStm LabelGenNamed(string id);
extern AStm LabelGenGeneric(string id);
extern AStm FuncGen(string id, AType retType, AParmList parmlist, string errhandler, AStm code);
extern AStm VarGen(string id, AType type);
extern AStm TypeGen(string id, AType type);

/*
 * -- Types. --
 * Bleh. This is the ugliest stuff to wrap your head
 * around. It'd be nice if everything kept to exactly
 * one register and never got spilled :) 
 */
extern AType Type(int kind);
extern AType TypePointerArray(AType arraytype);
extern AType TypePointer(AType pointertype);
extern AType TypeArray(AType pointertype, AExpr size);
extern AType TypeID(string id);
extern AType TypeCompound(AParmList parms);
extern int TypeCmp(AType a, AType b);

/*
 * -- Expressions. --
 * Need I say it? Nearly all the work in inadEquate
 * is done in expressions.
 */
extern AExpr ExprBinOp(AExpr a1, AExpr a2, int op);
extern AExpr ExprUnOp(AExpr a, int op);

extern AExpr ExprCast(AExpr expr, AType type);
extern AExpr ExprValuef(float val);
extern AExpr ExprValuei(int val, AType type);
extern AExpr ExprString(string val);
extern AExpr ExprAssign(AExpr id, AExpr val);
extern AExpr ExprSizeof(string id);
extern AExpr ExprNew(AType type);
extern AExpr ExprCall(string id, AExprList arglist);
extern AExpr ExprID(string id);

extern int yynerrs;
