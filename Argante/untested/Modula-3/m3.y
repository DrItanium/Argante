%{

#include "m3common.h"
#include "m3tree.h"
#include "m3types.h"
void	ldebug(char *rule);
void 	yyerror(char *s);
void	ydebug(const char *rule);

#define ids_difference(a,b) \
	fatal_error("IDs do not match %s(%d) != %s(%d) at line %d", \
	give_literal(a), (a), give_literal(b), (b), yylineno)

/*
*/
#define split_var_decls(idlist,type,expr) \
	{	t_list *idli=idlist; \
		t_node *n; \
		while (idli) { \
			n=new_node(NT_VARDECL); \
			n->x.vardecl.Id=(t_id)idli->node; \
			n->x.vardecl.Type=type; \
			n->x.vardecl.Expr=expr; \
			varlist = list_add(varlist,n); \
			idli = idli->next; \
		} \
	}
#define split_form_decls(mode,idlist,type,expr) \
	{	t_list *idli=idlist; \
		t_node *n; \
		while (idli) { \
			n=new_node(NT_FORMAL); \
			n->x.formal.Mode=mode; \
			n->x.formal.Id=(t_id)idli->node; \
			n->x.formal.Type=type; \
			n->x.formal.ConstExpr=expr; \
			formlist = list_add(formlist,n); \
			idli = idli->next; \
		} \
	}
	
extern t_node *m3tree;
%}

%union
{
	t_node	*node;
	t_list	*list;
	long 	number;
	float	fnumber;
	long 	key ;
};

%token	ANY ARRAY AS BEGINB BITS 
%token	BRANDED BY CASE CONST DO 
%token	ELSE ELSIF END EVAL EXCEPT EXCEPTION 
%token	EXIT EXPORTS FINALLY FOR FROM GENERIC 
%token	IF IMPORT INTERFACE LOCK LOOP 
%token	METHODS MODULE OBJECT OF 
%token	OVERRIDES PROCEDURE RAISE RAISES READONLY 
%token	RECORD REF REPEAT RETURN REVEAL ROOT 
%token	SET THEN TO TRY TYPE TYPECASE 
%token	UNSAFE UNTIL UNTRACED VALUE VAR WHILE 
%token	WITH 

%token	NEW

%token	xxx_WRITEINT

%token <key> OR AND NOT IN DIV MOD

%token <key> KEY_PLUS "+"
%token <key> KEY_MINUS "-"
%token <key> KEY_MUL "*"
%token <key> KEY_DIV "/"
%token <key> KEY_LT "<"
%token <key> KEY_GT ">"
%token <key> KEY_LEQ "<="
%token <key> KEY_GEQ ">="
// %token <key> KEY_HASH "#"
%token <key> KEY_LBRA "{"
%token <key> KEY_LPAR "("
%token <key> KEY_LSPAR "["
%token <key> KEY_EQ "="
%token <key> KEY_RBRA "}"
%token <key> KEY_RPAR ")"
%token <key> KEY_RSPAR "]"
%token <key> KEY_SEMI ";"
%token <key> KEY_PIPE "|"
%token <key> KEY_UP "^"
%token <key> KEY_DOT "."
%token <key> KEY_DOTDOT ".."
%token <key> KEY_ASSIGN ":="
%token <key> KEY_COMMA ","
%token <key> KEY_AND "&"
%token <key> KEY_COL ":"
%token <key> KEY_SUB "<:"
%token <key> KEY_IMP "=>"
%token <key> KEY_NEQ "<>"

%token	<number>	ID 
%token	<number>	INTEGER 
%token	<number>	STRING 
%token	<fnumber>	FLOAT

%type	<node>	xxx_WriteIntSt

%type	<node>	Program Compilation Interface Module GenInf GenMod Block 
%type	<node>	Import AsImport FromImport ImportItem Block
%type	<node>	ConstDecl TypeDecl ExceptionDecl RevealDecl
%type	<node>	ProcedureDecl ConstExpr Expr Type Signature
%type	<node>	QualId 
%type	<node>	Stmt AssignSt CallSt CaseSt ExitSt EvalSt ForSt
%type	<node>	IfSt LockSt LoopSt RaiseSt RepeatSt ReturnSt
%type	<node>	TCaseSt TryXptSt TryFinSt WhileSt WithSt
%type	<node>	Actual Case IfCase TCase Handler Binding Label 
%type	<node>	TypeName ArrayType PackedType EnumType ObjectType
%type	<node>	ProcedureType RecordType RefType SetType SubrangeType
%type	<node>	ObjectSuper Brand Field Method Override
%type	<node>	E0 E1 E2 E3 E4 E5 E6 E7 E8 Selector Constructor Cons
%type	<node>	ConsType
	
%type	<list>	Imports Decls Decl Exports GenFmls GenActls ImportItems
%type	<list>	IdList S 
%type	<list>	ConstDecls TypeDecls ExceptionDecls 
%type	<list>	VariableDecl VariableDecls RevealDecls
%type	<list>	QualIds Raises Formals Formal
%type	<list>	Actuals Cases IfCases TCases Handlers Bindings Labels Types
%type	<list>	Fields Methods Overrides MethodList OverrideList
%type	<list>	ConsList
%type	<list>	Exprs

%type	<number> Mode
%type	<number> ID
%type	<key> RelOp AddOp MulOp

/* xman */
//%expect 2


%%

/*****************************************************************************\
*                        Compilation Unit Productions                         * 
\*****************************************************************************/

Program
	: Compilation
		{	m3tree=$1; }
	;

Compilation 
	: Interface 
		{	ydebug("Compilation[1]: Interface");
			$$->x.interface.unsafe=0; }
	| UNSAFE Interface 
		{	ydebug("Compilation[2]: Unsafe Interface");
			$$->x.interface.unsafe=1; }
	| Module 
		{	ydebug("Compilation[3]: Module");
			$$->x.module.unsafe=0; }
	| UNSAFE Module 
		{	ydebug("Compilation[4]: UnsafeModule");
			$$->x.module.unsafe=1; }
	| GenInf
		{	ydebug("Compilation[5]: GenInf"); }
	| GenMod 
		{	ydebug("Compilation[6]: GenMod"); }
	;

Interface
	: INTERFACE ID ";" Imports Decls END ID "."
		{	ydebug("Interface[1]: INTERFACE ID ; Imports Decls END ID .");
			$$=new_node(NT_INTERFACE) ;
			$$->x.interface.Id=$2 ;
			$$->x.interface.Imports = $4 ;
			$$->x.interface.Decls = $5 ;
			if ($2 != $7) ids_difference($2,$7);
		}
	| INTERFACE ID "=" ID GenActls END ID "."
		{	ydebug("Interface[2]: INTERFACE ID = ID GenActls END ID .");
			$$=new_node(NT_INST_INTERFACE);
			$$->x.instance.Id=$2;
			$$->x.instance.Id_generic=$4;
			$$->x.instance.GenActls = $5 ;
			if ($2 != $7) ids_difference($2,$7);
		}
	;

Module
	: MODULE ID Exports ";" Imports Block ID "."
		{	ydebug("Module[1]: MODULE ID Exports ; Imports Block ID .");
			$$=new_node(NT_MODULE);
			$$->x.module.Id=$2;
			$$->x.module.Exports=$3;
			$$->x.module.Imports=$5;
			$$->x.module.Block=$6;
			if ($2 != $7) ids_difference($2,$7);
		}
	| MODULE ID Exports "=" ID GenActls END ID "."
		{	ydebug("Module[2]: MODULE ID Exports = ID GenActls END ID .");
			$$=new_node(NT_INST_MODULE);
			$$->x.instance.Id=$2;
			$$->x.instance.Exports=$3;
			$$->x.instance.Id_generic=$5;
			$$->x.instance.GenActls=$6;
			if ($2 != $8) ids_difference($2,$8);
		}
	;

GenInf
	: GENERIC INTERFACE ID GenFmls ";" Imports Decls END ID "."
		{	$$=new_node(NT_INTERFACE);
			$$->x.interface.Id=$3;
			$$->x.interface.GenFmls=$4;
			$$->x.interface.Imports=$6;
			$$->x.interface.Decls=$7;
			if ($3 != $9) ids_difference($3,$9);
		}
	;
	
GenMod
	: GENERIC MODULE ID GenFmls ";" Imports Block ID "."
		{	ydebug("GenMod[1]: GENERIC MODULE ID GenFmls ; Imports Block ID .");
			$$=new_node(NT_MODULE);
			$$->x.module.Id=$3;
			$$->x.module.GenFmls=$4;
			$$->x.module.Imports=$6;
			$$->x.module.Block=$7;
			if ($3 != $8) ids_difference($3,$8);
		}
	;
Imports
	: Imports Import 
		{	ydebug("Imports[1]: _filled_");
			$$=list_add($1,$2); }
	| 	{	ydebug("Imports[2]: _empty_");
			$$=0; }
	;
Import
	: AsImport
	| FromImport
	;
AsImport
	: IMPORT ImportItems ";" 
		{	ydebug("AsImport[1]: IMPORT ImportItems ;");
			$$=new_node(NT_ASIMPORT);
			$$->x.List=$2; 
		}
	;
ImportItems
	: ImportItems "," ImportItem
		{	$$=list_add($1,$3); }
	| ImportItem 
		{	$$=list_new($1); }
	;
ImportItem
	: ID 	
		{	$$=new_node(NT_ID);
			$$->x.Id=$1;
		}
	| ID AS ID
		{	$$=new_node(NT_ALIAS);
			$$->x.alias.Id=$1;
			$$->x.alias.Alias=$3;
		}
	;
FromImport
	: FROM ID IMPORT IdList ";"
		{	$$=new_node(NT_FROMIMPORT);
			$$->x.fromimport.Id_from=$2;
			$$->x.fromimport.IdList=$4;
		}
	;
Exports
	: EXPORTS IdList 
		{	$$=$2; }
	|	{	$$=0; }
GenFmls
	: "(" ")"
		{	ydebug("GenFmls[1]: _empty_");
			$$=0; }
	| "(" IdList ")"
		{	ydebug("GenFmls[2]: _filled_");
			$$=$2; }
	;
GenActls 
	: GenFmls 
	;
Block
	: Decls BEGINB S END
		{	ydebug("Block[1]: Decls BEGIN S END");
			$$=new_node(NT_BLOCK);
			$$->x.block.Decls=$1;
			$$->x.block.S=$3;
		}
	;
Decls
	: Decls Decl
		{	$$=list_join($1,$2); }
	| 	{	$$=0; }
Decl
	: CONST ConstDecls
		{	$$=$2; }
	| TYPE TypeDecls
		{	$$=$2; }
	| EXCEPTION ExceptionDecls
		{	$$=$2; }
	| VAR VariableDecls
		{	$$=$2; }
	| ProcedureDecl
		{	$$=list_new($1); }
	| REVEAL RevealDecls 
		{	$$=$2; }
	;
ConstDecls
	: ConstDecls ConstDecl ";"
		{	ydebug("ConstDecls[1]: ConstDecls ConstDecl ;");
			$$=list_add($1,$2); }
	|	{	ydebug("ConstDecls[2]: _empty_");
			$$=0; }
	;
ConstDecl
	: ID "=" ConstExpr
		{	ydebug("ConstDecl[1]: ID = ConstExpr");
			$$=new_node(NT_CONSTDECL);
			$$->x.vardecl.Id=$1;
			$$->x.vardecl.Expr=$3;
		}
	| ID ":" Type "=" ConstExpr
		{	ydebug("ConstDecl[2]: ID : Type = ConstExpr");
			$$=new_node(NT_CONSTDECL);
			$$->x.vardecl.Id=$1;
			$$->x.vardecl.Type=$3;
			$$->x.vardecl.Expr=$5;
		}
TypeDecls
	: TypeDecls TypeDecl ";"
		{	$$=list_add($1,$2); }
	|	{	$$=0; }
	;
TypeDecl
	: ID "=" Type
		{	$$=new_node(NT_TYPEDECL);
			$$->x.typedecl.Id=$1;
			$$->x.typedecl.Type=$3;
			$$->x.typedecl.subtyping=0;
		}
	| ID "<:" Type
		{	$$=new_node(NT_TYPEDECL);
			$$->x.typedecl.Id=$1;
			$$->x.typedecl.Type=$3;
			$$->x.typedecl.subtyping=1;
		}
	;
ExceptionDecls
	: ExceptionDecls ExceptionDecl ";"
		{	$$=list_add($1,$2); }
	|	{	$$=0; }
	;
ExceptionDecl
	: ID "(" Type ")"
		{	$$=new_node(NT_EXCEPTIONDECL);
			$$->x.exceptiondecl.Id=$1;
			$$->x.exceptiondecl.Type=$3;
		}
	| ID
		{	$$=new_node(NT_EXCEPTIONDECL);
			$$->x.exceptiondecl.Id=$1;
		}
	;
VariableDecls
	: VariableDecls VariableDecl ";"
		{	$$=list_join($1,$2); }
	|	{	$$=0; }
	;
VariableDecl
	: IdList ":" Type ":=" Expr 
		{	t_list *varlist=0;
			split_var_decls($1,$3,$5);
			$$=varlist;
		}
	| IdList ":" Type
		{	t_list *varlist=0;
			split_var_decls($1,$3,0);
			$$=varlist;
		}
	| IdList ":=" Expr 
		{	t_list *varlist=0;
			split_var_decls($1,0,$3);
			$$=varlist;
		}
	;
ProcedureDecl	
	: PROCEDURE ID Signature ";"
		{	$$=new_node(NT_PROCEDUREDECL);
			$$->x.proceduredecl.Id=$2;
			$$->x.proceduredecl.Signature=$3;
		}
	| PROCEDURE ID Signature "=" Block ID ";"
		{	$$=new_node(NT_PROCEDUREDECL);
			$$->x.proceduredecl.Id=$2;
			$$->x.proceduredecl.Signature=$3;
			$$->x.proceduredecl.Block=$5;
			if ($2 != $6) ids_difference($2,$6);
		}
	;
RevealDecls
	: RevealDecls RevealDecl ";"
		{	$$=list_add($1,$2); }
	|	{	$$=0; } 
	;
RevealDecl	
	: QualId "=" Type
		{	ydebug("RevealDecl[1]: QualId = Type");
			$$=new_node(NT_REVEALDECL);
			$$->x.revealdecl.QualId=$1;
			$$->x.revealdecl.Type=$3;
			$$->x.revealdecl.subtyping=0;
		}
	| QualId "<:" Type
		{	ydebug("RevealDecl[2]: QualId <: Type");
			$$=new_node(NT_REVEALDECL);
			$$->x.revealdecl.QualId=$1;
			$$->x.revealdecl.Type=$3;
			$$->x.revealdecl.subtyping=1;
		}
	;		
Signature	
	: "(" Formals ")" ":" Type Raises 
		{	$$=new_node(NT_SIGNATURE);
			$$->x.signature.Formals=$2;
			$$->x.signature.Type=$5;
			$$->x.signature.Raises=$6;
		}
	| "(" Formals ")" Raises {}
		{	$$=new_node(NT_SIGNATURE);
			$$->x.signature.Formals=$2;
			$$->x.signature.Raises=$4;
		}
	;
Raises	
	: RAISES "{" QualIds "}"
		{	$$=$3; }
	| RAISES "{" "}"
		{	$$=0; }
	| RAISES ANY 
		{	$$=list_new(new_node(NT_ANY)); }
	| 	{	$$=0; }
	;
Formals
	: Formal
		{	$$=$1; }
	| Formal ";" Formals 
		{	$$=list_join($1,$3); }
	| 	{	$$=0; } 
	;
Formal
	: Mode IdList ":" Type ":=" ConstExpr 
		{	t_list *formlist=0;
			ydebug("Formal[1]: Mode IdList : Type := Constexpr");
			split_form_decls($1,$2,$4,$6);
			$$=formlist;
/*
			$$=new_node(NT_FORMAL);
			$$->x.formal.IdList=$2;
			$$->x.formal.Mode=$1 ;
			$$->x.formal.Type=$4 ;
			$$->x.formal.ConstExpr=$6 ;
*/
		}
	| Mode IdList ":=" ConstExpr 
		{	t_list *formlist=0;
			ydebug("Formal[2]: Mode IdList := Constexpr");
			split_form_decls($1,$2,0,$4);			
			$$=formlist;
		}
	| Mode IdList ":" Type  
		{	t_list *formlist=0;
			ydebug("Formal[3]: Mode IdList : Type");
			split_form_decls($1,$2,$4,0);
			$$=formlist;
		}
	;
Mode
	:	{	ydebug("Mode[1]: _empty_");
			$$=MODE_VALUE; }
	| VALUE	
		{	ydebug("Mode[2]: VALUE");
			$$=MODE_VALUE; }
	| VAR
		{	ydebug("Mode[2]: VAR");
			$$=MODE_VAR; }
	| READONLY
		{	ydebug("Mode[2]: READONLY");
			$$=MODE_READONLY; }
	;

/*****************************************************************************\
*                           Statement Productions                             *
\*****************************************************************************/
S
	: Stmt ";" S
		{	$$=list_join(list_new($1),$3); } 
	| Stmt 
		{	$$=list_new($1); }
	|	{	$$=0; }
	;
Stmt
	: AssignSt
	| Block
	| CallSt
	| CaseSt
	| ExitSt
	| EvalSt
	| ForSt
	| IfSt
	| LockSt
	| LoopSt
	| RaiseSt
	| RepeatSt
	| ReturnSt
	| TCaseSt
	| TryXptSt
	| TryFinSt
	| WhileSt
	| WithSt
	| xxx_WriteIntSt
	;

xxx_WriteIntSt
	: xxx_WRITEINT "(" Expr ")"
		{	$$=new_node(NT_xxx_WRITEINTST);
			$$->x.expr.Expr=$3;
		}
	;

AssignSt
	: Expr ":=" Expr
		{	$$=new_node(NT_ASSIGNST);
			$$->x.assignst.Expr_left=$1;
			$$->x.assignst.Expr_right=$3;
		}
	;

CallSt
	: Expr 
/* Assume any Expr can be CallSt */
/* 
	"(" ")" 
		{	$$=new_node(NT_CALLST);
			$$->x.callst.Expr=$1;
		}
	| Expr "(" Actuals ")"
		{	$$=new_node(NT_CALLST);
			$$->x.callst.Expr=$1;
			$$->x.callst.Actuals=$3;
		}
*/
	;
Actuals
	: Actuals "," Actual
		{	$$=list_add($1,$3); }
	| Actual
		{	$$=list_new($1); }
	;
Actual
	: ID ":=" Expr
		{	ydebug("Actual[1]: ID := Expr");
			$$=new_node(NT_ACTUAL);
			$$->x.actual.Id=$1;
			$$->x.actual.Expr=$3;
		}
	| Expr
		{	ydebug("Actual[2]: Expr");
			$$=new_node(NT_ACTUAL);
			$$->x.actual.Expr=$1;
		}
/* xman - Causes 2 r/r conflicts, leads to special NEW handling */
/*
	| Type
		{	ydebug("Actual[3]: Type");
			$$=new_node(NT_ACTUAL);
			$$->x.actual.Type=$1; 
		}
*/
	;

CaseSt
	: CASE Expr OF Cases ELSE S END 
		{	$$=new_node(NT_CASEST);
			$$->x.casest.Expr=$2;
			$$->x.casest.Cases=$4;
			$$->x.casest.S_else=$6;
		}
	| CASE Expr OF Cases END	
		{	$$=new_node(NT_CASEST);
			$$->x.casest.Expr=$2;
			$$->x.casest.Cases=$4;
		}
	;
Cases
	:	{	$$=0; }
	|	Cases "|" Case
		{	$$=list_add($1,$3); }
	|	Case
		{	$$=list_new($1); }
	;
Case
	: Labels "=>" S
		{	$$=new_node(NT_CASE);
			$$->x.case_.Labels=$1;
			$$->x.case_.S=$3;
		}
	;
Labels 
	: Labels "," Label
		{	$$=list_add($1,$3); }
	| Label
		{	$$=list_new($1); }
	;
Label
	: ConstExpr ".." ConstExpr
		{	$$=new_node(NT_LABEL);
			$$->x.label.ConstExpr=$1;
			$$->x.label.ConstExpr_to=$3;
		}
	| ConstExpr
		{	$$=new_node(NT_LABEL);
			$$->x.label.ConstExpr=$1;
		}
	;

ExitSt
	: EXIT
		{	$$=new_node(NT_EXITST); }
	;

EvalSt
	: EVAL Expr
		{	$$=new_node(NT_EVALST); 
			$$->x.expr.Expr=$2;
		}
	;

ForSt
	: FOR ID ":=" Expr TO Expr BY Expr DO S END
		{	$$=new_node(NT_FORST);
			$$->x.forst.Id=$2;
			$$->x.forst.Expr_from=$4;
			$$->x.forst.Expr_to=$6;
			$$->x.forst.Expr_by=$8;
			$$->x.forst.S=$10;
		}
	| FOR ID ":=" Expr TO Expr DO S END
		{	$$=new_node(NT_FORST);
			$$->x.forst.Id=$2;
			$$->x.forst.Expr_from=$4;
			$$->x.forst.Expr_to=$6;
			$$->x.forst.S=$8;
		}
	;

IfSt
	: IF IfCases ELSE S END
		{	$$=new_node(NT_IFST);
			$$->x.ifst.IfCases=$2;
			$$->x.ifst.S_else=$4;
		}
	| IF IfCases END
		{	$$=new_node(NT_IFST);
			$$->x.ifst.IfCases=$2;
		}
	;
IfCases
	: IfCases ELSIF IfCase
		{	$$=list_add($1,$3); }
	| IfCase
		{	$$=list_new($1); }
	;
IfCase 
	: Expr THEN S
		{	$$=new_node(NT_IFCASE);
			$$->x.expr_S.Expr=$1;
			$$->x.expr_S.S=$3;
		}
	;

LockSt
	: LOCK Expr DO S END
		{	$$=new_node(NT_LOCKST);
			$$->x.expr_S.Expr=$2;
			$$->x.expr_S.S=$4;
		}
	;

LoopSt
	: LOOP S END
		{	$$=new_node(NT_LOOPST);
			$$->x.expr_S.S=$2;
		}
	;

RaiseSt
	: RAISE QualId "(" Expr ")"
		{	$$=new_node(NT_RAISEST);
			$$->x.raisest.QualId=$2;
			$$->x.raisest.Expr=$4;
		}
	| RAISE QualId
		{	$$=new_node(NT_RAISEST);
			$$->x.raisest.QualId=$2;
		}
	;

RepeatSt
	: REPEAT S UNTIL Expr
		{	$$=new_node(NT_REPEATST);
			$$->x.expr_S.S=$2;
			$$->x.expr_S.Expr=$4;
		}
	;

ReturnSt
	: RETURN Expr
		{	$$=new_node(NT_RETURNST);
			$$->x.expr.Expr=$2;
		}
	| RETURN
		{	$$=new_node(NT_RETURNST); }
	;

TCaseSt
	: TYPECASE Expr OF TCases ELSE S END 
		{	$$=new_node(NT_TCASEST);
			$$->x.casest.Expr=$2;
			$$->x.casest.Cases=$4;
			$$->x.casest.S_else=$6;
		}
	| TYPECASE Expr OF TCases END	
		{	$$=new_node(NT_TCASEST);
			$$->x.casest.Expr=$2;
			$$->x.casest.Cases=$4;
		}
	;
TCases
	:	{	$$=0; }
	|	TCases "|" TCase
		{	$$=list_add($1,$3); }
	|	TCase
		{	$$=list_new($1); }
	;
TCase
	: Types "(" ID ")" "=>" S
		{	$$=new_node(NT_TCASE);
			$$->x.tcase.Types=$1;
			$$->x.tcase.Id=$3;
			$$->x.tcase.S=$6;
		}
	| Types "=>" S
		{	$$=new_node(NT_TCASE);
			$$->x.tcase.Types=$1;
			$$->x.tcase.S=$3;
		}
	;

TryXptSt
	: TRY S EXCEPT Handlers ELSE S END
		{	$$=new_node(NT_TRYXPTST);
			$$->x.tryxptst.S=$2;
			$$->x.tryxptst.Handlers=$4;
			$$->x.tryxptst.S_else=$6;
		}
	| TRY S EXCEPT Handlers END
		{	$$=new_node(NT_TRYXPTST);
			$$->x.tryxptst.S=$2;
			$$->x.tryxptst.Handlers=$4;
		}
	;		
Handlers
	:	{	$$=0; }
	|	Handlers "|" Handler
		{	$$=list_add($1,$3); }
	|	Handler
		{	$$=list_new($1); }
	;
Handler 
	: QualIds "(" ID ")" "=>" S
		{	$$=new_node(NT_HANDLER);
			$$->x.handler.QualIds=$1;
			$$->x.handler.Id=$3;
			$$->x.handler.S=$6;
		}
	| QualIds "=>" S
		{	$$=new_node(NT_HANDLER);
			$$->x.handler.QualIds=$1;
			$$->x.handler.S=$3;
		}
	;

TryFinSt
	: TRY S FINALLY S END
		{	$$=new_node(NT_TRYFINST);
			$$->x.tryfinst.S=$2;
			$$->x.tryfinst.S_final=$4;
		}
	;

WhileSt
	: WHILE Expr DO S END
		{	$$=new_node(NT_WHILEST);
			$$->x.expr_S.Expr=$2;
			$$->x.expr_S.S=$4;
		}
	;

WithSt
	: WITH Bindings DO S END
		{	$$=new_node(NT_WITHST);
			$$->x.withst.Bindings=$2;
			$$->x.withst.S=$4;
		}
	;
Bindings
	: Bindings "," Binding
		{	$$=list_add($1,$3); }
	| Binding
		{	$$=list_new($1); }
	;
Binding
	: ID "=" Expr 
		{	$$=new_node(NT_BINDING);
			$$->x.binding.Id=$1;
			$$->x.binding.Expr=$3;
		}
	;

					
/*****************************************************************************\
*                             Type Productions                                *
\*****************************************************************************/
Types 
	: Types "," Type 
		{	$$=list_add($1,$3); }
	| Type
		{	$$=list_new($1); }
	;
Type 
	: TypeName
	| ArrayType
	| PackedType
	| EnumType
	| ObjectType
	| ProcedureType
	| RecordType
	| RefType
	| SetType
	| SubrangeType
	| "(" Type ")"
		{	$$=$2; }
	;
ArrayType	
	: ARRAY Types OF Type
		{	$$=new_node(NT_ARRAYTYPE);
			$$->x.arraytype.Types=$2;
			$$->x.arraytype.Type=$4;
		}
	| ARRAY OF Type
		{	$$=new_node(NT_ARRAYTYPE);
			$$->x.arraytype.Type=$3;
		}
	;
PackedType
	: BITS ConstExpr FOR Type
		{	$$=new_node(NT_PACKEDTYPE);
			$$->x.packedtype.Type=$4;
			$$->x.packedtype.ConstExpr=$2;
		}
	;
EnumType
	: "{" "}"
		{	$$=new_node(NT_ENUMTYPE); }
	| "{" IdList "}"
		{	$$=new_node(NT_ENUMTYPE); 
			$$->x.List=$2;
		}
	;
ObjectType
	: ObjectSuper Brand OBJECT Fields Methods Overrides END
		{	$$=new_node(NT_OBJECTTYPE);
			$$->x.objecttype.super=$1;
			$$->x.objecttype.Brand=$2;
			$$->x.objecttype.Fields=$4;
			$$->x.objecttype.Methods=$5;
			$$->x.objecttype.Overrides=$6;
		}
	| Brand OBJECT Fields Methods Overrides END
		{	$$=new_node(NT_OBJECTTYPE);
			$$->x.objecttype.Brand=$1;
			$$->x.objecttype.Fields=$3;
			$$->x.objecttype.Methods=$4;
			$$->x.objecttype.Overrides=$5;
		}
	;
ObjectSuper
	: TypeName 
		{	ydebug("ObjectSuper[1]: TypeName"); }
	| ObjectType
		{	ydebug("ObjectSuper[2]: ObjectType"); }
//	|	{	ydebug("ObjectSuper[3]: _empty_"); }
	;
Brand
	: BRANDED ConstExpr 
		{	ydebug("Brand[1]: BRANDED ConstExpr");
			$$=$2; }
	| BRANDED 
		{	ydebug("Brand[2]: BRANDED");
			$$=(t_node*)BRAND_NAMELESS; }
	|	{	ydebug("Brand[3]: _empty_");
			$$=0; }
	;
Methods
	: METHODS MethodList
		{	$$=$2; }
	| 	{	$$=0; }
	;
MethodList
	: Method ";" MethodList
		{	$$=list_join(list_new($1),$3); }
	| Method
		{	$$=list_new($1); }
	|	{	$$=0; }
	;
Method
	: ID Signature ":=" ConstExpr
		{	$$=new_node(NT_METHOD);
			$$->x.method.Id=$1;
			$$->x.method.Signature=$2;
			$$->x.method.ConstExpr=$4;
		}
	| ID Signature 
		{	$$=new_node(NT_METHOD);
			$$->x.method.Id=$1;
			$$->x.method.Signature=$2;
		}
	;
Overrides
	: OVERRIDES OverrideList
		{	$$=$2; }
	| 	{	$$=0; }
	;
OverrideList
	: Override ";" OverrideList
		{	$$=list_join(list_new($1),$3); }
	| Override
		{	$$=list_new($1); }
	|	{	$$=0; }
	;
Override
	: ID ":=" ConstExpr
		{	$$=new_node(NT_OVERRIDE);
			$$->x.override.Id=$1;
			$$->x.override.ConstExpr=$3;
		}
	;
Fields
	: Field ";" Fields
		{	$$=list_join(list_new($1),$3); }
	| Field
		{	$$=list_new($1); }
	|	{	$$=0; }
	;
Field
	: IdList ":" Type ":=" ConstExpr 
		{	$$=new_node(NT_FIELD);
			$$->x.field.IdList=$1;
			$$->x.field.Type=$3;
			$$->x.field.ConstExpr=$5;
		}
	| IdList ":" Type
		{	$$=new_node(NT_FIELD);
			$$->x.field.IdList=$1;
			$$->x.field.Type=$3;
		}
	| IdList ":=" ConstExpr 
		{	$$=new_node(NT_FIELD);
			$$->x.field.IdList=$1;
			$$->x.field.ConstExpr=$3;
		}
	;	
ProcedureType
	: PROCEDURE Signature
		{	$$=new_node(NT_PROCEDURETYPE);
			$$->x.Signature=$2; 
		}
	;
RecordType
	: RECORD Fields END
		{	$$=new_node(NT_RECORDTYPE);
			$$->x.List=$2;
		}
	;
RefType
	: UNTRACED Brand REF Type
		{	$$=new_node(NT_REFTYPE);
			$$->x.reftype.untraced=1;
			$$->x.reftype.Brand=$2;
			$$->x.reftype.Type=$4;
		}
	| Brand REF Type
		{	$$=new_node(NT_REFTYPE);
			$$->x.reftype.Brand=$1;
			$$->x.reftype.Type=$3;
		}
	;
SetType
	: SET OF Type
		{	$$=new_node(NT_SETTYPE);
			$$->x.Type=$3;
		}
	;
SubrangeType
	: "[" ConstExpr ".." ConstExpr "]"
		{	$$=new_node(NT_SUBRANGETYPE);
			$$->x.subrangetype.Expr_from=$2;
			$$->x.subrangetype.Expr_to=$4;
		}
	;
/*****************************************************************************\
*                           Expression Productions                            *
\*****************************************************************************/
Exprs 
	: Exprs "," Expr
		{	$$=list_add($1,$3); }
	| Expr
		{	$$=list_new($1); }
	;
	
ConstExpr
	: Expr 
	;
Expr	
	: E0
	;
E0
	: E0 OR E1 
		{	$$=new_node(NT_EXPR2); $$->x.expr.operand=$2;
			$$->x.expr.Expr=$1; $$->x.expr.Expr2=$3;
		}
	| E1
	;
E1
	: E1 AND E2
		{	$$=new_node(NT_EXPR2); $$->x.expr.operand=$2;
			$$->x.expr.Expr=$1; $$->x.expr.Expr2=$3;
		}
	| E2
	;
E2	
	: NOT E2
		{	$$=new_node(NT_EXPR1); $$->x.expr.operand=$1;
			$$->x.expr.Expr=$2;
		}
	| E3
	;
E3	
	: E3 RelOp E4
		{	$$=new_node(NT_EXPR2); $$->x.expr.operand=$2;
			$$->x.expr.Expr=$1; $$->x.expr.Expr2=$3;
		}
	| E4
	;
E4	
	: E4 AddOp E5
		{	$$=new_node(NT_EXPR2); $$->x.expr.operand=$2;
			$$->x.expr.Expr=$1; $$->x.expr.Expr2=$3;
		}
	| E5
	;
E5	
	: E5 MulOp E6
		{	$$=new_node(NT_EXPR2); $$->x.expr.operand=$2;
			$$->x.expr.Expr=$1; $$->x.expr.Expr2=$3;
		}
	| E6
	;
E6	
	: "+" E6
		{	$$=new_node(NT_EXPR1); $$->x.expr.operand=$1;
			$$->x.expr.Expr=$2;
		}
	| "-" E6
		{	$$=new_node(NT_EXPR1); $$->x.expr.operand=$1;
			$$->x.expr.Expr=$2;
		}
	| E7
	;
E7	
	: E7 Selector
		{	$$=new_node(NT_EXPRSELECTED);		
			$$->x.expr.Expr=$1;
			$$->x.expr.Selector=$2;
		}
	| E8
	;
E8
	: ID
		{	$$=new_node(NT_ID);
			$$->x.Id=$1;
		}

	| INTEGER
		{	$$=new_node(NT_INTEGER);
			$$->x.intval=$1;
		}
	| FLOAT
		{	$$=new_node(NT_FLOAT);
			$$->x.floatval=$1;
		}
	| STRING
		{	$$=new_node(NT_STRING);
			$$->x.stringval=$1;
		}
	| Constructor 
	| "(" Expr ")"
		{	$$=$2; }
/* Special NEW handling */
	| NEW "(" Type "," Actuals ")"
		{	$$=new_node(NT_NEWEXPR);
			$$->x.newexpr.Type=$3;
			$$->x.newexpr.Actuals=$5
		}
	| NEW "(" Type ")"
		{	$$=new_node(NT_NEWEXPR);
			$$->x.newexpr.Type=$3;
		}
	
	;
Constructor 
	: ConsType "{" ConsList "}"
		{	$$=new_node(NT_CONSTRUCTOR); 
			$$->x.constructor.Type=$1;
			$$->x.constructor.ConsList=$3;
		}
	| ConsType "{" "}"
		{	$$=new_node(NT_CONSTRUCTOR); 
			$$->x.constructor.Type=$1;
		}
	;	
ConsList
	: ConsList "," Cons
		{	$$=list_add($1,$3); }
	| Cons
		{	$$=list_new($1); }
	;
Cons
	: Expr
		{	$$=new_node(NT_EXPR);
			$$->x.expr.Expr=$1; 
		}
	| Expr ".." Expr
		{	$$=new_node(NT_SETELT);
			$$->x.subrangetype.Expr_from=$1;
			$$->x.subrangetype.Expr_to=$3;
		}
	| ID ":=" Expr
		{	$$=new_node(NT_RECORDELT);
			$$->x.binding.Id=$1;
			$$->x.binding.Expr=$3;
		}
	| ".."
		{	$$=(t_node*)ARRAYELT_DOTDOT; }
	;
ConsType
	: ArrayType ;
	| RecordType ;
	| SetType ;
	| ID
		{	$$=new_node(NT_ID);
			$$->x.Id=$1;
		}
	;
Selector
	: "^"
		{	$$=new_node(NT_SELECTOR);
			$$->x.selector.operand=$1;
		}
	| "[" Exprs "]"
		{	$$=new_node(NT_SELECTOR);
			$$->x.selector.operand=$1;
			$$->x.selector.Exprs=$2;
		}
	| "(" Actuals ")"
		{	$$=new_node(NT_SELECTOR);
			$$->x.selector.operand=$1;
			$$->x.selector.Actuals=$2;
		}
	| "(" ")"
		{	$$=new_node(NT_SELECTOR);
			$$->x.selector.operand=$1;
		}
	| "." ID
		{	$$=new_node(NT_SELECTOR);
			$$->x.selector.operand=$1;
			$$->x.selector.Id=$2;
		}
	;
RelOp
	: "="	
	| "<>"	
	| "<"
	| ">"
	| "<="
	| ">="
	| IN
	;
AddOp	
	: "+"
	| "-"
	| "&"
	;
MulOp
	: "*"
	| "/"
	| DIV
	| MOD
	;
	

/*****************************************************************************\
*                          Miscelaneous Productions                           *
\*****************************************************************************/

IdList 
	: IdList "," ID
		{	$$=list_add($1,(t_node*)$3); }
	| ID
		{	$$=list_new((t_node*)$1); }
	;
QualIds
	: QualIds "," QualId 
		{	$$=list_add($1,$3); }
	| QualId
		{	$$=list_new($1); }
	;
QualId	
	: ID
		{	ydebug("Qualid[1]: ID");
			$$=new_node(NT_QUALID);
			$$->x.qualid.Id=$1;
		}
	| ID "." ID 
		{	ydebug("Qualid[2]: ID . ID");
			$$=new_node(NT_QUALID);
			$$->x.qualid.Id_qualifier=$1;
			$$->x.qualid.Id=$3;
		}
	;
TypeName
	: QualId
	| ROOT
		{	$$=new_node(NT_REFTYPE); }
	| UNTRACED ROOT
		{	$$=new_node(NT_REFTYPE); 
			$$->x.reftype.untraced=1; 
		}
	;
/*****************************************************************************\
*                            END OF PRODUCTIONS                               *
\*****************************************************************************/
%%

void ydebug(const char *rule) {
#ifdef M3_DEBUG_BISON
	fprintf(stderr,"[%d]   p>Rule: %s\n",yylineno,rule);
	fflush(stderr);
#endif
}
void yyerror( char *s )
{
	fatal_error( s ) ;
}
