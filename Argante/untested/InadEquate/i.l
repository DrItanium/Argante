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

<*>{eol}		{ADJ; EM_LineNo++; EM_TokPos=0; }

<INITIAL>"/*"		{ADJ; commentlevel=1; BEGIN COMMENT; }
<INITIAL>"*/"		{
			fprintf(stderr, "line %d: overterminated constant\n", EM_LineNo); exit(1);
			}

<COMMENT>"/*"		{ADJ; commentlevel++;}
<COMMENT>"*/"		{ADJ; commentlevel--; if (!commentlevel) BEGIN INITIAL; }
<COMMENT>.		{ADJ; }

<INITIAL>{white}	{ADJ; }

<INITIAL>"unsigned"	{ADJ; return UNSIGNED; }
<INITIAL>"signed"	{ADJ; return SIGNED; }
<INITIAL>"float"	{ADJ; return FLOAT; }
<INITIAL>"array"	{ADJ; return ARRAY; }
<INITIAL>"size"		{ADJ; return SIZE; }
<INITIAL>"string"	{ADJ; return STRING; }
<INITIAL>"pointer to array"	{ADJ; return POINTERARRAY; }
<INITIAL>"pointer to"	{ADJ; return POINTER; }
<INITIAL>"cast"		{ADJ; return CAST; }

<INITIAL>"type"		{ADJ; return TYPEDEF; }
<INITIAL>"func"		{ADJ; return FUNCDEF; }
<INITIAL>"var"		{ADJ; return VAR; }
<INITIAL>"errhandler"	{ADJ; return ERRHANDLER; }

<INITIAL>"do"		{ADJ; return DO; }
<INITIAL>"while"	{ADJ; return WHILE; }
<INITIAL>"until"	{ADJ; return UNTIL; }
<INITIAL>"if"		{ADJ; return IF; }
<INITIAL>"else"		{ADJ; return ELSE; }
<INITIAL>"asm"		{ADJ; return ASM; }

<INITIAL>"goto"		{ADJ; return GOTO; }
<INITIAL>"return"	{ADJ; return RETURN; }
<INITIAL>"raise"	{ADJ; return RAISE; }

<INITIAL>"sizeof"	{ADJ; return SIZEOF; }
<INITIAL>"new"		{ADJ; return NEW; }
<INITIAL>"destroy"	{ADJ; return DESTROY; }
<INITIAL>"resize"	{ADJ; return RESIZE; }

<INITIAL>{n_oct}	{ADJ; yylval.uinum=strtol(yytext, NULL, 8); return V_INT; } 
<INITIAL>{n_hex}	{ADJ; yylval.uinum=strtol(yytext, NULL, 16); return V_INT; } 
<INITIAL>{n_dec}	{ADJ; yylval.uinum=strtol(yytext, NULL, 10); return V_INT; }
<INITIAL>{n_cval}	{ADJ; yylval.uinum=strtol(yytext, NULL, 10); return V_INT; }

<INITIAL>{float}	{ADJ; yylval.ufnum=strtod(yytext, NULL); return V_FLOAT; } 
<INITIAL>{string}	{ADJ; yylval.ustring=String(yytext); return V_STRING; /* I don't like this string handler */}
<INITIAL>{id}		{ADJ; yylval.ustring=String(yytext); return ID; }

<INITIAL>"<="		{ADJ; return LEQ; }
<INITIAL>">="		{ADJ; return GEQ; }
<INITIAL>"=="		{ADJ; return EQ; }
<INITIAL>"!="		{ADJ; return NEQ; }
<INITIAL>"||"		{ADJ; return BOOL_AND; }
<INITIAL>"&&"		{ADJ; return BOOL_OR; }

<INITIAL>{keys}		{ADJ; return yytext[0]; }

.	{ EM_ErrorMessage("unrecognized char"); yynerrs++; return 0; }

