/*
 * Tree structures for handling abstractsyntax.
 * Very much from 'modern compiler implementation in C, andrew appel'.
 */

typedef struct _AStm *AStm;
typedef struct _AExpr *AExpr;
typedef struct _AType *AType;
typedef struct _AExprList *AExprList;
typedef struct _AParmList *AParmList;
typedef struct _ACompoundField *ACompoundField;

struct _AStm {
	enum { SGoto, SReturn, SExpr } kind;
	union {
		string to;
		AExpr val;
	} u;
	AStm next;
	AStm last; /* Last statement in this StmList */
};

struct _AExpr {
	enum { EID, EUnOp, EBinOp, ECast, EValue } kind;
	AType type; /* float/unsigned/signed etc */
	union {
		string id;
		struct { AExpr a1; int op; AExpr a2; } binop;
		struct { AExpr a; int op; } unop;
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
	enum { TUnsigned, TSigned, TFloat, TString, TPointer, TArray, TCompound } kind;
	AExpr size;
	union {
		AType PointerTo;
		ACompoundField *FirstRec;
	} u;
};

struct _ACompoundField {
	string id;
	AType type;
	int location;
	ACompoundField next;
};

struct _AExprList {
	AExpr expr;
	AType type;
	AExprList next;
};
	
struct _AParmList {
	string id;
	AType type;
	AParmList next;
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

extern AStm LabelGen(string id);
extern AStm FuncGen(string id, AType retType, AParmList parmlist, string errhandler, AStm code);

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
extern AExpr ExprAssign(string id, AExpr val);
extern AExpr ExprSizeof(AExpr of);
extern AExpr ExprNew(AType type);
extern AExpr ExprCall(string id, AExprList arglist);
extern AExpr ExprID(string id);

