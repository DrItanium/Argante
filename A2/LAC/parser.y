%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lstring.h"
#include "main.h"

#define YYERROR_VERBOSE
#define YYDEBUG 1

void yyerror(char *);
int yylex(void);

Stm *prog;

%}
%union {
	mVar *var;
	mType *type;
	mIType *itype;
	string string;
	string id;
	AOper optype;
	CmpOper cmptype;
	Stm *stm;
	RegList *reglist;
	TypeList *typelist;
	ITypeList *itypelist;
}

%token <var> VALUE
%token <type> TYPE
%token <string> STRING
%token <optype> AOPER
%token <cmptype> CMP

%nonassoc <id> ID

%type <var> var var1
%type <type> type1
%type <itype> itype
%type <typelist> typelist typelistA
%type <itypelist> itypelist itypelistA

%type <stm> stm stmlist prog prototype

%type <reglist> reglist reglistA

%token FUNC SYSCALL IGNORE STRADDR STRLEN
%token GOTO HANDLER IF RETURN WAIT ERRNO RAISE
%token ALLOC REALLOC FINALIZE UNFINALIZE DEALLOC

%start prog
%%

prog: stmlist { $$=prog=$1; Function(NULL, NULL, NULL, $1); }
	;

/* Error recovery */
stm: error ';' { $$=NULL; yyerrok; }
	;

type1: TYPE { $$=$1; }
	| '@' type1 { $2->pointerct++; $$=$2; }
	;

itype: type1 ID { $$=IType($2, $1); }
	;

typelistA: '(' type1 { $$=TypeList_AddType(NULL, $2); }
	| typelistA ',' type1 { $$=TypeList_AddType($1, $3); }
	;
typelist: typelistA ')' { $$=$1; }
	| '(' ')' { $$=NULL; }
	;

itypelistA: '(' itype { $$=ITypeList_AddType(NULL, $2); }
	| itypelistA ',' itype { $$=ITypeList_AddType($1, $3); }
	;
itypelist: itypelistA ')' { $$=$1; }
	;

prototype: FUNC typelist ID typelist ';' { PrototypeT($3, $2, $4); $$=NULL; }
	| FUNC typelist ID itypelist ';' { PrototypeI($3, $2, $4); $$=NULL; }
	| SYSCALL typelist ID typelist ';' { PrototypeSCT($3, $2, $4); $$=NULL; }
	| SYSCALL typelist ID itypelist ';' { PrototypeSCI($3, $2, $4); $$=NULL; }
	| FUNC typelist ID itypelist stm { Function($3, $2, $4, $5); $$=NULL; }
	| FUNC typelist ID '(' ')' stm { Function($3, $2, NULL, $6); $$=NULL; }
	;

stmlist: stm { $$=$1; }
	| stmlist stm { $$=StmList_Join($1, $2); }
	;

var: ERRNO { $$=Var_Errno(); }
	| IGNORE { $$=Var_Ignore(); }
	| ID { $$=Var_Register($1); }
	| VALUE { $$=$1; }
	| STRADDR '(' STRING ')' { $$=VarStrAddr($3); }
	| STRLEN '(' STRING ')' { $$=VarStrLen($3); }
	;

var1: var { $$=$1; }
	| '@' var { $2->ptrderef=1; $$=$2; }
	;

stm: prototype { $$=NULL; }
	| '{' stmlist '}' { $$=$2; }
	| itype ';' { $$=Stm_VarDef($1);  /* free($1); ? */  }
	| var1 '=' var1 ';' { $$=Stm_Assign($1, AEq, $3); }
	| var1 AOPER var1 ';' { $$=Stm_Assign($1, $2, $3); }
	| ID ':' { $$=Stm_Label($1); }
	| GOTO ID ';' { $$=Stm_Goto($2); }
	| HANDLER ID ';' { $$=Stm_Handler($2); }
	| IF var1 CMP var1 GOTO ID ';' { $$=Stm_If($2, $3, $4, $6); }
	| WAIT var1 ';' { $$=Stm_Wait($2); }
	| RAISE var1 ';' { $$=Stm_Raise($2); }
	| RETURN ';' { $$=Stm_Return(); }
	| RETURN reglist ';' { $$=StmList_Join(Stm_ReglistSet($2), Stm_Return()); }
	| ID reglist ';' { $$=StmList_Join(Stm_ReglistSet($2), Stm_Call($1)); }
	| ID '[' ']' ';' { $$=Stm_Call($1); }
	| reglist ';' { $$=Stm_ReglistSet($1); } 
	| reglist '=' ';' { $$=Stm_ReglistGet($1); } 
	;

stm: ALLOC '(' var1 ',' var1 ')' ';' { $$=Stm_Alloc($3, $5); }
	| REALLOC '(' var1 ',' var1 ')' ';' { $$=Stm_Realloc($3, $5); }
	| FINALIZE '(' var1 ')' ';' { $$=Stm_Finalize($3); }
	| UNFINALIZE '(' var1 ')' ';' { $$=Stm_Unfinalize($3); }
	| DEALLOC '(' var1 ')' ';' { $$=Stm_Dealloc($3); }
	;

stm: reglist '=' ID reglist ';'	{ $$=StmList_Join(
		StmList_Join(Stm_ReglistSet($4), Stm_Call($3)), Stm_ReglistGet($1) ); }
	| reglist '=' ID '[' ']' ';' { $$=StmList_Join(Stm_Call($3), Stm_ReglistGet($1)); } 
	;

reglistA: '[' var1 { $$=Reglist_AddVar(NULL, $2); }
	| reglistA ',' var1 { $$=Reglist_AddVar($1, $3); }
	| '[' STRING { $$=Reglist_AddVar(Reglist_AddVar(NULL, VarStrAddr($2)), VarStrLen($2)); }
	| reglistA ',' STRING { $$=Reglist_AddVar(
		Reglist_AddVar($1, VarStrAddr($3)), VarStrLen($3) ); }
	;

reglist: reglistA ']' { $$=$1; }
	;

