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

/* Error recovery */
stm: error ';' { $$=NULL; yyerrok; }

type1: TYPE { $$=$1; }
type1: '@' type1 { $2->pointerct++; $$=$2; }

itype: type1 ID { $$=IType($2, $1); }

typelistA: '(' type1 { $$=TypeList_AddType(NULL, $2); }
typelistA: typelistA ',' type1 { $$=TypeList_AddType($1, $3); }
typelist: typelistA ')' { $$=$1; }
typelist: '(' ')' { $$=NULL; }

itypelistA: '(' itype { $$=ITypeList_AddType(NULL, $2); }
itypelistA: itypelistA ',' itype { $$=ITypeList_AddType($1, $3); }
itypelist: itypelistA ')' { $$=$1; }

prototype: FUNC typelist ID typelist ';' { PrototypeT($3, $2, $4); $$=NULL; }
prototype: FUNC typelist ID itypelist ';' { PrototypeI($3, $2, $4); $$=NULL; }
prototype: SYSCALL typelist ID typelist ';' { PrototypeSCT($3, $2, $4); $$=NULL; }
prototype: SYSCALL typelist ID itypelist ';' { PrototypeSCI($3, $2, $4); $$=NULL; }
prototype: FUNC typelist ID itypelist stm { Function($3, $2, $4, $5); $$=NULL; }
prototype: FUNC typelist ID '(' ')' stm { Function($3, $2, NULL, $6); $$=NULL; }

stm: prototype { $$=NULL; }

stmlist: stm { $$=$1; }
stmlist: stmlist stm { $$=StmList_Join($1, $2); }
stm: '{' stmlist '}' { $$=$2; }
stm: itype ';' { $$=Stm_VarDef($1);  /* free($1); ? */  }

var: ERRNO { $$=Var_Errno(); }
var: IGNORE { $$=Var_Ignore(); }
var: ID { $$=Var_Register($1); }
var: VALUE { $$=$1; }
var: STRADDR '(' STRING ')' { $$=VarStrAddr($3); }
var: STRLEN '(' STRING ')' { $$=VarStrLen($3); }

var1: var { $$=$1; }
var1: '@' var { $2->ptrderef=1; $$=$2; }

stm: var1 '=' var1 ';' { $$=Stm_Assign($1, AEq, $3); }
stm: var1 AOPER var1 ';' { $$=Stm_Assign($1, $2, $3); }
stm: ID ':' { $$=Stm_Label($1); }
stm: GOTO ID ';' { $$=Stm_Goto($2); }
stm: HANDLER ID ';' { $$=Stm_Handler($2); }
stm: IF var1 CMP var1 GOTO ID ';' { $$=Stm_If($2, $3, $4, $6); }
stm: WAIT var1 ';' { $$=Stm_Wait($2); }
stm: RAISE var1 ';' { $$=Stm_Raise($2); }

stm: RETURN ';' { $$=Stm_Return(); }
stm: RETURN reglist ';' { $$=StmList_Join(Stm_ReglistSet($2), Stm_Return()); }
stm: ID reglist ';' { $$=StmList_Join(Stm_ReglistSet($2), Stm_Call($1)); }
stm: ID '[' ']' ';' { $$=Stm_Call($1); }
stm: reglist ';' { $$=Stm_ReglistSet($1); } 
stm: reglist '=' ';' { $$=Stm_ReglistGet($1); } 

stm: ALLOC '(' var1 ',' var1 ')' ';' { $$=Stm_Alloc($3, $5); }
stm: REALLOC '(' var1 ',' var1 ')' ';' { $$=Stm_Realloc($3, $5); }
stm: FINALIZE '(' var1 ')' ';' { $$=Stm_Finalize($3); }
stm: UNFINALIZE '(' var1 ')' ';' { $$=Stm_Unfinalize($3); }
stm: DEALLOC '(' var1 ')' ';' { $$=Stm_Dealloc($3); }

stm: reglist '=' ID reglist ';' { $$=StmList_Join(StmList_Join(Stm_ReglistSet($4), Stm_Call($3)), Stm_ReglistGet($1)); } 
stm: reglist '=' ID '[' ']' ';' { $$=StmList_Join(Stm_Call($3), Stm_ReglistGet($1)); } 

reglistA: '[' var1 { $$=Reglist_AddVar(NULL, $2); }
reglistA: reglistA ',' var1 { $$=Reglist_AddVar($1, $3); }

reglistA: '[' STRING { $$=Reglist_AddVar(Reglist_AddVar(NULL, VarStrAddr($2)), VarStrLen($2)); }
reglistA:  reglistA ',' STRING { $$=Reglist_AddVar(Reglist_AddVar($1, VarStrAddr($3)), VarStrLen($3)); }

reglist: reglistA ']' { $$=$1; }

