/*
 * Tree structures for handling abstractsyntax.
 * Very much from 'modern compiler implementation in C, andrew appel'.
 */

typedef struct _AStm *AStm;
typedef struct _AExpr *AExpr;

struct _AStm {
	enum { SGoto, SReturn, SExpr } kind;
	union {
		string to;
		AExpr val;
	}
}

struct _AExpr {
	enum { EID, EBinOp, EValue } kind;
	AType type; /* float/unsigned/signed etc */
	union {
		string id;
		struct { AExpr a1, int op, AExpr a2 } binop;
		int ival;
		float fval;
	}
}
