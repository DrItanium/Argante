/*
 * Structures for handling abstract syntax.
 *
 * Used to be based on 'modern compiler implementation in C,
 * andrew appel', which is a extreeeeemely good book.
 *
 * Then I got smart. Expressions aren't really useful. Assignments
 * are, and they're easier to translate into code too :)
 *
 * James Kehl
 */

extern int EM_LineNo;
#define EM_ErrorMessage(s, rest...) \
	fprintf(stderr, "<+> Line %d: " s "\n", EM_LineNo,## rest)
#define EM_Warn(s, rest...) \
	fprintf(stderr, "<-> Line %d: " s "\n", EM_LineNo,## rest)

typedef int iVarid;
typedef struct _iVarCoded *iVarCoded;
typedef struct _iVar *iVar;
typedef struct _iStm *iStm;
typedef struct _iAssignment *iAssignment;
typedef struct _iComparison *iComparison;
typedef struct _iFunc *iFunc;
typedef int opaque;

struct _iVarCoded {
	iVarid var;
	iStm code;
}

struct _iAssignment {
	iVarid to;
	/* Deref, =, and cast are unary op */
	enum { ABinop, AUnop, AConst } kind;
	union {
		struct { iVarid a1; char op; iVarid a2; } binop;
		struct { iVarid a; char op; } unop;
		opaque constval;
	} u;
};

struct _iComparison {
	iVarid a1;
	char op;
	iVarid a2;
};

struct _iVar {
	int regno; /* 0-31 */
	int regtype;
	string name;
	int exportuses; /* Do we need to store in memory for other funcs? */
	int nonconstuses; /* Constant propagation */
	opaque constval;
	iFunc parent;
};

struct _iStm {
	enum {
		SAssignment, SLabel, SGoto, SReturn, SRaise, SCJump,
		/* Temporary statements, to make parent tracking easier */
		SVarDecl, SFunc
	} kind;
	int EMLineNo;
	iStm next;
	iStm prev;
	iStm last; /* Last statement in this StmList */
	union {
		iAssignment assign;
		string id;
		iVarid retval;
		struct { string truelabel; string falselabel; iComparison criteria; } cjump;
	} u;
};

extern iVar iVaridDeref(iVarid id);
extern iVarid iVar_MakeTemp();
extern iVarid iVar_MakeNamed(string name);
extern iVarid iVar_FindName(string name);
extern iVarid iVar_MakeConst(opaque constval); /* iAssign_Const(iVar_MakeTemp(), val) for simplicity */

extern iVarid iAssign_Binop(iVarid to, iVarid a1, int op, iVarid a2);
extern iVarid iAssign_Unop(iVarid to, iVarid a, int op);
extern iVarid iAssign_Const(iVarid to, opaque constval);

/* statement list */
extern iStm iStm_Compound (iStm list, iStm newstm);

/* Loops + control exchange statements */
extern iStm iStm_Assign(iAssignment a);
extern iStm iStm_WhileHead(iComparison cmp, iStm code);
extern iStm iStm_UntilHead(iComparison cmp, iStm code);
extern iStm iStm_WhileTail(iComparison cmp, iStm code);
extern iStm iStm_UntilTail(iComparison cmp, iStm code);
extern iStm iStm_IfElse(iComparison cmp, iStm branch1, iStm branch2);

extern iStm iStm_NamedLabel(string id);
extern iStm iStm_TempLabel(string id);

extern iStm iStm_Return(iVarid val);
extern iStm iStm_Raise(iVarid val);
extern iStm iStm_Goto(string label);

extern iComparison iCompare(iVarid a1, iVarid a2, int op);

/* opaque values */
extern opaque OVal_i(int i);
extern opaque OVal_f(float f);
extern opaque OVal_str(string s);
