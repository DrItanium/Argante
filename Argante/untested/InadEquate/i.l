%option noyywrap
%{
#include "string.h"
#include "unistd.h"
#include "stdlib.h"
#include "tree.h"
#include "i.tab.h"

void yyerror(char *);
/*
typedef union {
	int uinum;
	string ustring;
	float ufnum;
	AExpr uexpr;
	AStm ustmt;
	AType utype;
	AExprList uexprlist;
	AParmList uparmlist;
} YYSTYPE;
*/

YYSTYPE yylval;

extern int yynerrs;
int charPos=1;
int EM_TokPos=0;
int EM_LineNo=1;
int commentlevel=0;
#define ADJ EM_TokPos+=charPos; charPos=yyleng
#define YY_USER_ACTION ADJ;

/* Heaps cribbed from Eru's Modula3. */
%}

%x COMMENT

n_oct		0[0-7]+
n_hex		0x[0-9a-fA-F]+
n_dec		[[:digit:]]+
char		[[:print:]]|\\[[:print:]]
n_cval		\'{char}\'

string		\"([^\"]*|(\\\")*)*\"|\'([^\']*|(\\\')*)*\'

eol		\n
white 		[ \t]+
id		[[:alpha:]][[:alpha:][:digit:]_]*
float		[[:digit:]]+\.[[:digit:]]+("E"(\+|\-)?[[:digit:]]+)?
keys		[\{\}\(\,\);=<>\+\-\*/\|&!%]
%%

<*>{eol}		{EM_LineNo++; EM_TokPos=0; }

<INITIAL>"/*"		{ commentlevel=1; BEGIN COMMENT; }
<INITIAL>"*/"		{
				EM_ErrorMessage("overterminated constant");
				yynerrs++;
			}

<COMMENT>"/*"		{ commentlevel++;}
<COMMENT>"*/"		{ commentlevel--; if (!commentlevel) BEGIN INITIAL; }
<COMMENT>.		{ }

<INITIAL>{white}	{ }

<INITIAL>"unsigned"	{ return UNSIGNED; }
<INITIAL>"signed"	{ return SIGNED; }
<INITIAL>"float"	{ return FLOAT; }
<INITIAL>"array"	{ return ARRAY; }
<INITIAL>"size"		{ return SIZE; }
<INITIAL>"string"	{ return STRING; }
<INITIAL>"pointer to array"	{ return POINTERARRAY; }
<INITIAL>"pointer to"	{ return POINTER; }
<INITIAL>"cast"		{ return CAST; }

<INITIAL>"type"		{ return TYPEDEF; }
<INITIAL>"func"		{ return FUNCDEF; }
<INITIAL>"var"		{ return VAR; }
<INITIAL>"errhandler"	{ return ERRHANDLER; }

<INITIAL>"do"		{ return DO; }
<INITIAL>"while"	{ return WHILE; }
<INITIAL>"until"	{ return UNTIL; }
<INITIAL>"if"		{ return IF; }
<INITIAL>"else"		{ return ELSE; }
<INITIAL>"asm"		{ return ASM; }

<INITIAL>"goto"		{ return GOTO; }
<INITIAL>"return"	{ return RETURN; }
<INITIAL>"raise"	{ return RAISE; }

<INITIAL>"sizeof"	{ return SIZEOF; }
<INITIAL>"new"		{ return NEW; }
<INITIAL>"destroy"	{ return DESTROY; }
<INITIAL>"resize"	{ return RESIZE; }

<INITIAL>{n_oct}	{ yylval.uinum=strtol(yytext, NULL, 8); return V_INT; } 
<INITIAL>{n_hex}	{ yylval.uinum=strtol(yytext, NULL, 16); return V_INT; } 
<INITIAL>{n_dec}	{ yylval.uinum=strtol(yytext, NULL, 10); return V_INT; }
<INITIAL>{n_cval}	{ yylval.uinum=strtol(yytext, NULL, 10); return V_INT; }

<INITIAL>{float}	{ yylval.ufnum=strtod(yytext, NULL); return V_FLOAT; } 
<INITIAL>{string}	{ yylval.ustring=String(yytext); return V_STRING; /* I don't like this string handler */}
<INITIAL>{id}		{ yylval.ustring=String(yytext); return ID; }

<INITIAL>"<="		{ return LEQ; }
<INITIAL>">="		{ return GEQ; }
<INITIAL>"=="		{ return EQ; }
<INITIAL>"!="		{ return NEQ; }
<INITIAL>"||"		{ return BOOL_AND; }
<INITIAL>"&&"		{ return BOOL_OR; }

<INITIAL>{keys}		{ return yytext[0]; }

.	{ EM_ErrorMessage("unrecognized char"); yynerrs++; }

