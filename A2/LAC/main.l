%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "lstring.h"
#include "main.h"
#include "parser.h"

void yyerror(char *);

/* double definition of this is bad */
extern YYSTYPE yylval;

extern int yynerrs;
int EM_TokPos=0;
int EM_LineNo=1;

static int commentlevel=0;
static int charPos=1;

struct FileStack {
	int lineno;
	string file;
	YY_BUFFER_STATE stream;
};

#define MAX_FILESTACK 16
static struct FileStack fstack[MAX_FILESTACK];
static int fstack_ptr=0;

static void pop_fstack(void) {
	fstack_ptr--;
	EM_LineNo=fstack[fstack_ptr].lineno;
	EM_FileName=fstack[fstack_ptr].file;
	
	yy_delete_buffer(YY_CURRENT_BUFFER);
	yy_switch_to_buffer(fstack[fstack_ptr].stream);
}


static void push_fstack(void) {
	if (fstack_ptr>= MAX_FILESTACK) {
		yyerror("excessive #include nest depth");
		yynerrs++;
		return;
	}
	fstack[fstack_ptr].lineno=EM_LineNo;
	fstack[fstack_ptr].file=EM_FileName;
	fstack[fstack_ptr].stream=YY_CURRENT_BUFFER;
	fstack_ptr++;
}

#define ADJ EM_TokPos+=charPos; charPos=yyleng
#define YY_USER_ACTION ADJ;

/* Heaps cribbed from Eru's Modula3. */
%}

%x COMMENT INCLUDE

n_oct		0[0-7]+
n_hex		0x[0-9a-fA-F]+
n_dec		[[:digit:]]+

string		\"([^\"]*|(\\\")*)*\"
charval		\'([^\']*|(\\\')*)*\'

include		^\#include

eol		\n
white 		[ \t]+
id		[[:alpha:]][[:alpha:][:digit:]_]*
float		[[:digit:]]+\.[[:digit:]]+("E"(\+|\-)?[[:digit:]]+)?

keys		[\[\]\(\)\{\}\,@;=:]
%%

<*>{eol}		{EM_LineNo++; EM_TokPos=0; }

<*><<EOF>>		{
				if (commentlevel) {
					yyerror("comment unterminated at EOF");
					yynerrs++;
					commentlevel=0;
				}
				if (fstack_ptr < 1)
					yyterminate();
				else 
					pop_fstack();
				BEGIN INITIAL;
			}

<INITIAL>{include}	{ BEGIN INCLUDE; }

<INCLUDE>{string}$	{
				FILE *f;
				char *q;
				YY_BUFFER_STATE new;
				q=&yytext[yyleng - 1]; *q=0;
				q=yytext+1;
				f=fopen(q, "r");

				if (f) {
					push_fstack();
					new=yy_create_buffer(f, YY_BUF_SIZE);
					yy_switch_to_buffer(new);
					EM_FileName=String(q);
					EM_LineNo=0;
				} else EM_Error("couldn't open %s: %s", q, strerror(errno));
				BEGIN INITIAL;
			}
<INCLUDE>{white}	{ }
<INCLUDE>.		{ yyerror("#include not understood"); yynerrs++; BEGIN INITIAL; }

<INITIAL>"/*"		{ commentlevel=1; BEGIN COMMENT; }
<INITIAL>"*/"		{
				yyerror("overterminated comment");
				yynerrs++;
			}

<COMMENT>"/*"		{ commentlevel++;}
<COMMENT>"*/"		{ commentlevel--; if (!commentlevel) BEGIN INITIAL; }
<COMMENT>.		{ }

<INITIAL>{white}	{ }

<INITIAL>"unsigned"	{ yylval.type=Type(TypeUnsigned); return TYPE; }
<INITIAL>"signed"	{ yylval.type=Type(TypeSigned); return TYPE; }
<INITIAL>"float"	{ yylval.type=Type(TypeFloat); return TYPE; }

<INITIAL>{n_oct}	{ yylval.var=Var_ConstU(strtoul(yytext, NULL, 8)); return VALUE; } 
<INITIAL>{n_hex}	{ yylval.var=Var_ConstU(strtoul(yytext, NULL, 16)); return VALUE; } 
<INITIAL>{n_dec}	{ yylval.var=Var_ConstU(strtoul(yytext, NULL, 10)); return VALUE; }
<INITIAL>{float}	{ yylval.var=Var_ConstF(strtod(yytext, NULL)); return VALUE; } 

<INITIAL>"func"		{ return FUNC; } 
<INITIAL>"syscall"	{ return SYSCALL; } 

<INITIAL>"ignore"	{ return IGNORE; } 

<INITIAL>"wait"		{ return WAIT; } 

<INITIAL>"straddr"	{ return STRADDR; } 
<INITIAL>"strlen"	{ return STRLEN; } 

<INITIAL>"alloc"	{ return ALLOC; } 
<INITIAL>"realloc"	{ return REALLOC; } 
<INITIAL>"finalize"	{ return FINALIZE; } 
<INITIAL>"unfinalize"	{ return UNFINALIZE; } 
<INITIAL>"dealloc"	{ return DEALLOC; } 

<INITIAL>"goto"		{ return GOTO; } 
<INITIAL>"handler"	{ return HANDLER; } 
<INITIAL>"if"		{ return IF; } 
<INITIAL>"return"	{ return RETURN; } 
<INITIAL>"errno"	{ return ERRNO; } 
<INITIAL>"raise"	{ return RAISE; } 

<INITIAL>"+="		{ yylval.optype=APlus; return AOPER; } 
<INITIAL>"-="		{ yylval.optype=AMinus; return AOPER; } 
<INITIAL>"*="		{ yylval.optype=AMul; return AOPER; } 
<INITIAL>"/="		{ yylval.optype=ADiv; return AOPER; } 

<INITIAL>"<<="		{ yylval.optype=AShl; return AOPER; } 
<INITIAL>">>="		{ yylval.optype=AShr; return AOPER; } 
<INITIAL>"<<<="		{ yylval.optype=ARol; return AOPER; } 
<INITIAL>">>>="		{ yylval.optype=ARor; return AOPER; }

<INITIAL>"^="		{ yylval.optype=AXor; return AOPER; } 
<INITIAL>"|="		{ yylval.optype=AOr; return AOPER; } 
<INITIAL>"&="		{ yylval.optype=AAnd; return AOPER; } 
<INITIAL>"%="		{ yylval.optype=AMod; return AOPER; } 

<INITIAL>"=="		{ yylval.cmptype=IEq; return CMP; } 
<INITIAL>"!="		{ yylval.cmptype=INEq; return CMP; } 
<INITIAL>">"		{ yylval.cmptype=IGt; return CMP; } 
<INITIAL>"<"		{ yylval.cmptype=ILt; return CMP; } 
<INITIAL>{keys}		{ return yytext[0]; }

<INITIAL>{string}	{
				char *q;
				q=&yytext[yyleng - 1]; *q=0;
				yylval.string=StringEscaped(yytext+1);
				return STRING;
			}
<INITIAL>{charval}	{
				char *q;
				const char *s;
				q=&yytext[yyleng - 1]; *q=0;
				s=StringToChar(StringEscaped(yytext+1));
				yylval.var=Var_ConstU(s[0]); 
				if (s[1]) yyerror("excess characters in \'\' ignored\n");
				return VALUE;
			}

<INITIAL>{id}		{ yylval.id=String(yytext); return ID; }

.	{ yyerror("unrecognized char"); yynerrs++; }

