%{
#include "string.h"
#include "tree.h"

AStm prog;
%}
%union {
	int uinum;
	string ustring;
	float ufnum;
	AExpr uexpr;
	AStm ustmt;
	AType utype;
	AExprList uexprlist;
	AParmList uparmlist;
}

%token UNSIGNED SIGNED FLOAT ARRAY SIZE STRING
%left POINTER POINTERARRAY CAST
%token TYPEDEF FUNCDEF VAR ERRHANDLER
%token DO WHILE UNTIL IF ELSE ASM
%token GOTO RETURN RAISE
%token SIZEOF NEW DESTROY RESIZE

%token <uinum> V_INT
%token <ufnum> V_FLOAT
%token <ustring> V_STRING
%nonassoc <ustring> ID

%token '{' '}' '(' ',' ')' ';'
%token '='

%nonassoc '<' '>' LEQ GEQ EQ NEQ
%token BOOL_AND BOOL_OR

%left '='

%left '+' '-'
%left '*' '/'

%left '|' '&' 

// Really, they're left-assoc, but !! expr is a retarded thing to do
%nonassoc UMINUS '~' '!'

// Precedence rules
%left CALL

//%token type
%type <uinum> '+' '-' '*' '/' '|' '&'
// '~' '!' '=' ';' '{' '}' '(' ')'
%type <uexpr> expr
%type <utype> type
%type <ustmt> stm stmlist prog

%type <ustring> ID
%type <uexprlist> arglist
%type <uparmlist> parmlist1 parmlist

%start prog
%%

prog: stm { prog=$1 };

stm: expr ';' { $$=StmExpr($1); }

stmlist: stm { $$=$1; }
	| stmlist stm { $$=StmCompound($1, $2); }

stm: '{' stmlist '}' { $$=$2 }

/* These are stmlists not stms so we don't have to deal
   with the dangling-else problem. */
stmlist: WHILE '(' expr ')' stm { $$=StmWhileHead($3, $5); }
	| UNTIL '(' expr ')' stm { $$=StmUntilHead($3, $5); }
	| DO stm WHILE '(' expr ')' { $$=StmWhileTail($5, $2); }
	| DO stm UNTIL '(' expr ')' { $$=StmWhileTail($5, $2); }
stmlist: IF '(' expr ')' stm ELSE stm { $$=StmIfElse($3, $5, $7); }
	| IF '(' expr ')' stm { $$=StmIfElse($3, $5, NULL); }

/* Built in functions. It would be fairly trying to write a RET or GOTO add-in,
   but exceptions and memory could have been hived out (like C). However, if
   they're important enough for their own opcode...
*/
stm:	DESTROY ID ';' { $$=StmDestroy($2); }
stm:	RESIZE '(' ID ',' type ')' ';' { $$=StmResize($3, $5); }
stm:	RETURN expr ';' { $$=StmReturn($2); }
stm:	RAISE expr ';' { $$=StmRaise($2); }
stm:	GOTO ID ';' { $$=StmGoto($2); }

/* int main(int argc) */
parmlist1: parmlist ',' type ID { $$=ParmListAdd($1, $3, $4); }
	 | type ID { $$=ParmListNew($1, $2); }
parmlist: { $$=NULL; }
	 | parmlist1 { $$=$1; }

/* and main(i) */
arglist: arglist ',' expr { $$=ArgListAdd($1, $3); }
	 | expr { $$=ArgListNew($1); }

/* Eh, well, they're no-ops :) */
stm: ID ':' { $$=LabelGen($1); }

stm:	FUNCDEF type ID parmlist ERRHANDLER ID '=' stm
		{ $$=FuncGen($3, $2, $4, $6, $8); }
	| FUNCDEF type ID parmlist '=' stm
		{ $$=FuncGen($3, $2, $4, NULL, $6); }
	| FUNCDEF ID parmlist ERRHANDLER ID '=' stm
		{ $$=FuncGen($2, NULL, $3, $5, $7); }
	| FUNCDEF ID parmlist '=' stm
		{ $$=FuncGen($2, NULL, $3, NULL, $5); }

type: UNSIGNED { $$=Type(T_UNSIGNED); }
	| SIGNED { $$=Type(T_SIGNED); }
	| FLOAT { $$=Type(T_FLOAT); }
	| STRING { $$=Type(T_STRING); }
	| POINTERARRAY type { $$=TypePointerArray($2); }
	| POINTER type { $$=TypePointer($2); }
	| ARRAY type SIZE V_INT { $$=TypeArray($2, $4); }
	| ID { $$=TypeID($1); }

expr: CAST type expr { $$=ExprCast($3, $2); }
expr: '(' expr ')' { $$=$2; }

expr:	V_INT { $$=ExprValue($1, T_SIGNED); }
	| V_FLOAT { $$=ExprValue($1, T_FLOAT); }
	| V_STRING { $$=ExprString($1); }

expr:	expr '+' expr { $$=ExprBinOp($1, $3, '+'); }
	| expr '-' expr	{ $$=ExprBinOp($1, $3, '-'); }
	| expr '/' expr	{ $$=ExprBinOp($1, $3, '/'); }
	| expr '*' expr	{ $$=ExprBinOp($1, $3, '*'); }
	| expr '&' expr	{ $$=ExprBinOp($1, $3, '&'); }
	| expr '|' expr	{ $$=ExprBinOp($1, $3, '|'); }
expr:	expr '<' expr { $$=ExprBinOp($1, $3, '<'); }
	| expr '>' expr { $$=ExprBinOp($1, $3, '>'); }
	| expr LEQ expr { $$=ExprBinOp($1, $3, LEQ); }
	| expr GEQ expr { $$=ExprBinOp($1, $3, GEQ); }
	| expr EQ expr { $$=ExprBinOp($1, $3, EQ); }
	| expr NEQ expr { $$=ExprBinOp($1, $3, NEQ); }
expr:	ID '=' expr { $$=ExprAssign($1, $3); }

/* Erk. sizeof types may well be out, sorry. */
expr:	SIZEOF '(' ID ')' { $$=ExprSizeof($3); }
expr:	NEW '(' type ')' { $$=ExprNew($3); }

expr:	ID '(' ')' %prec CALL { $$=ExprCall($1, NULL); }
	| ID '(' arglist ')' %prec CALL { $$=ExprCall($1, $3); }

/* Using (-a) is nicer looking code. And, unlike Visual Basic,
   there's no overhead in excess braces. Why not just use it? */
expr:	'(' '-' expr ')' { $$=ExprUnOp($2, '-'); }
	| '!' expr { $$=ExprUnOp($2, '!'); }
	| '~' expr { $$=ExprUnOp($2, '~'); }

expr: ID { $$=ExprID($1); }
