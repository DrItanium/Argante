%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"
#include "tree.h"

#define YYERROR_VERBOSE
#define YYDEBUG 1

void yyerror(char *);
int yylex();


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

%type <uexpr> expr
%token <uinum> V_INT
%token <ufnum> V_FLOAT
%token <ustring> V_STRING
%nonassoc <ustring> ID

%token '='

%nonassoc '<' '>' LEQ GEQ EQ NEQ
%token BOOL_AND BOOL_OR

%token '{' '}' '(' ',' ')' ';'
%left '='

%left '%'
%left '+' '-'
%left '*' '/'

%left '|' '&' 

// Really, they're left-assoc, but !! expr is a retarded thing to do
%nonassoc UMINUS '!'

//%token type
%type <uinum> '+' '-' '*' '/' '|' '&'
// '~' '!' '=' ';' '{' '}' '(' ')'
%type <uexpr> expr
%type <utype> type
%type <ustmt> stm stmlist stmlist1 prog

%type <uexprlist> arglist
%type <uparmlist> parmlist1 parmlist

/* If ( isn't the highest class token, then a(x)
   becomes two expressions instead of a call. */
%left '('

%start prog
%%

/* Error recovery */
stm: error ';' { $$=NULL; yyerrok; }
expr: '(' error ')' { $$ = NULL; }

/*
Word of warning:
All statements are to be assumed to be lists.
The difference between stmlist and stm is that a
stm is considered a single block.
*/

prog: stmlist { prog=$1 };

stm: expr ';' { $$=StmExpr($1); }

stmlist: stmlist stmlist1 { $$=StmCompound($1, $2); }
	| stmlist1 { $$=$1 }
stmlist1: stm { $$=$1; }

stm: '{' stmlist '}' { $$=$2 }

/* These are stmlists not stms so we don't have to deal
   with the dangling-else problem. */
stmlist1: WHILE '(' expr ')' stm { $$=StmWhileHead($3, $5); }
	| UNTIL '(' expr ')' stm { $$=StmUntilHead($3, $5); }
	| DO stm WHILE '(' expr ')' { $$=StmWhileTail($5, $2); }
	| DO stm UNTIL '(' expr ')' { $$=StmWhileTail($5, $2); }
stmlist1: IF '(' expr ')' stm ELSE stm { $$=StmIfElse($3, $5, $7); }
	| IF '(' expr ')' stm { $$=StmIfElse($3, $5, NULL); }

/* Built in functions. It would be fairly trying to write a RET or GOTO add-in,
   but exceptions and memory could have been hived out (like C). However, if
   they're important enough for their own opcode...
*/
stm:	DESTROY ID ';' { $$=StmDestroy($2); }
stm:	RESIZE '(' ID ',' type ')' ';' { $$=StmResize($3, $5); }

stm:	GOTO ID ';' { $$=StmGoto($2); }
stm:	RETURN expr ';' { $$=StmReturn($2); }
	| RETURN ';' { $$=StmReturn(NULL); }
stm:	RAISE expr ';' { $$=StmRaise($2); }

/* int main(int argc) */
parmlist1: parmlist ',' type ID { $$=ParmListAdd($1, $3, $4); }
	 | type ID { $$=ParmListNew($1, $2); }
parmlist: { $$=NULL; }
	 | parmlist1 { $$=$1; }

/* and main(i) */
arglist: arglist ',' expr { $$=ArgListAdd($1, $3); }
	 | expr { $$=ArgListNew($1); }

/* A label IS a statement. It goes into the asm code
   just as much as any other statement... */
stm: ID ':' { $$=LabelGenNamed($1); }

/* What the hell. Nested functions look harder to avoid than
   to accept. */
stm:	FUNCDEF type ID '(' parmlist ')' ERRHANDLER ID '=' stm
		{ $$=FuncGen($3, $2, $5, $8, $10); }
	| FUNCDEF type ID '(' parmlist ')' '=' stm
		{ $$=FuncGen($3, $2, $5, NULL, $8); }
	| FUNCDEF ID '(' parmlist ')' ERRHANDLER ID '=' stm
		{ $$=FuncGen($2, NULL, $4, $7, $9); }
	| FUNCDEF ID '(' parmlist ')' '=' stm
		{ $$=FuncGen($2, NULL, $4, NULL, $7); }
/* Var decls. We have to wait until later to isolate the decl
   from the code, so they're statements like everything else... */
stm:	VAR type ID '=' expr ';'
		/* The decl will be scooped away later. The assignment sticks
		   where it's put. */
		{ $$=StmCompound(VarGen($3, $2), StmExpr(ExprAssign(ExprID($3), $5))); }
	| VAR type ID ';'
		{ $$=VarGen($3, $2); }
stm:	TYPEDEF ID '=' '{' parmlist '}' { $$=TypeGen($2, TypeCompound($5)); }
stm:	TYPEDEF ID '=' type ';' { $$=TypeGen($2, $4); }

type: UNSIGNED { $$=Type(TUnsigned); }
	| SIGNED { $$=Type(TSigned); }
	| FLOAT { $$=Type(TFloat); }
	| STRING { $$=Type(TString); }
	| POINTERARRAY type { $$=TypePointerArray($2); }
	| POINTER type { $$=TypePointer($2); }
	| ARRAY type SIZE expr { $$=TypeArray($2, $4); }
	| ID { $$=TypeID($1); }

expr: CAST type expr { $$=ExprCast($3, $2); }
expr: '(' expr ')' { $$=$2; }

expr:	V_INT { $$=ExprValuei($1, Type(TSigned)); }
	| V_FLOAT { $$=ExprValuef($1); }
	| V_STRING { $$=ExprString($1); }

expr:	expr '+' expr { $$=ExprBinOp($1, $3, '+'); }
	| expr '-' expr	{ $$=ExprBinOp($1, $3, '-'); }
	| expr '/' expr	{ $$=ExprBinOp($1, $3, '/'); }
	| expr '*' expr	{ $$=ExprBinOp($1, $3, '*'); }
	| expr '&' expr	{ $$=ExprBinOp($1, $3, '&'); }
	| expr '|' expr	{ $$=ExprBinOp($1, $3, '|'); }
	| expr '%' expr	{ $$=ExprBinOp($1, $3, '%'); }
expr:	expr '<' expr { $$=ExprBinOp($1, $3, '<'); }
	| expr '>' expr { $$=ExprBinOp($1, $3, '>'); }
	| expr LEQ expr { $$=ExprBinOp($1, $3, LEQ); }
	| expr GEQ expr { $$=ExprBinOp($1, $3, GEQ); }
	| expr EQ expr { $$=ExprBinOp($1, $3, EQ); }
	| expr NEQ expr { $$=ExprBinOp($1, $3, NEQ); }
expr:	expr '=' expr { $$=ExprAssign($1, $3); }

/* This can get put in later. ugh 
expr:	expr '[' expr ']'
*/

/* We can't separate type and expr */
expr:	SIZEOF '(' ID ')' { $$=ExprSizeof($3); }

expr:	NEW '(' type ')' { $$=ExprNew($3); }

expr:	ID '(' ')' { $$=ExprCall($1, NULL); }
	| ID '(' arglist ')' { $$=ExprCall($1, $3); }

/* Using (-a) is nicer looking code. And, unlike Visual Basic,
   there's no overhead in excess braces. Why not just use it? */
expr:	'(' '-' expr ')' { $$=ExprUnOp($3, '-'); }
	| '!' expr { $$=ExprUnOp($2, '!'); }
/* ~ removed as per Adam's request. ! and ~ are now identical. */

expr: ID { $$=ExprID($1); }
