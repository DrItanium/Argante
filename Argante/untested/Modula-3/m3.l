%{
#include "m3common.h"
#include "m3tree.h"
#include "m3parser.h"

#define retkey(k) \
	ldebug("__key__"); \
	yylval.key=k; \
	return k; 

long yylineno = 1;
void ldebug (char *rule);
%}

digit8		[0-7]
digit16		[0-9a-fA-F]
eol 		[\n]
white 		[ \t]+
comment		\(\*([^\*]|(\*[^\)]))*\*?\*\)
pragma		\<\*([^\*]|(\*[^\>]))*\*?\*\>
id			[[:alpha:]][[:alpha:][:digit:]_]*
integer		[[:digit:]]+
float		[[:digit:]]+\.[[:digit:]]+("E"(\+|\-)?[[:digit:]]+)?
string		\"([^\"]*|(\\\")*)*\"|\'([^\']*|(\\\')*)*\'

%%
{eol}			{	//ldebug("eol");
					yylineno++;
				}
{white}			{	//ldebug("white"); 
				}
{comment}		{	char *p=yytext;
					//ldebug("comment");
					while(*p) {
						if (*p == '\n') 
							yylineno++;
						p++;
					}
				}
{pragma}		{	char *p=yytext;
					//ldebug("pragma");
					while(*p) {
						if (*p == '\n')
							yylineno++;
						p++;
					}
				}

xxx_WRITEINT	{	ldebug("xxx_WRITEINT");	return xxx_WRITEINT; }
AND				{	retkey(AND); }
ANY				{	ldebug("ANY");			return ANY; }
ARRAY			{	ldebug("ARRAY");		return ARRAY; }
AS				{	ldebug("AS");			return AS; }
BEGIN			{	ldebug("BEGINB");		return BEGINB; }
BITS			{	ldebug("BITS");			return BITS; }
BRANDED			{	ldebug("BRANDED");		return BRANDED; }
BY				{	ldebug("BY");			return BY; }
CASE			{	ldebug("CASE");			return CASE; }
CONST			{	ldebug("CONST");		return CONST; }
DIV				{	retkey(DIV); }
DO				{	ldebug("DO");			return DO; }
ELSE			{	ldebug("ELSE");			return ELSE; }
ELSIF			{	ldebug("ELSIF");		return ELSIF; }
END				{	ldebug("END");			return END; }
EVAL			{	ldebug("EVAL");			return EVAL; }
EXCEPT			{	ldebug("EXCEPT");		return EXCEPT; }
EXCEPTION		{	ldebug("EXCEPTION");	return EXCEPTION; }
EXIT			{	ldebug("EXIT");			return EXIT; }
EXPORTS			{	ldebug("EXPORTS");		return EXPORTS; }
FINALLY			{	ldebug("FINALLY");		return FINALLY; }
FOR				{	ldebug("FOR");			return FOR; }
FROM			{	ldebug("FROM");			return FROM; }
GENERIC			{	ldebug("GENERIC");		return GENERIC; }
IF				{	ldebug("IF");			return IF; }
IMPORT			{	ldebug("IMPORT");		return IMPORT; }
IN				{	retkey(IN); }
INTERFACE		{	ldebug("INTERFACE");	return INTERFACE; }
LOCK			{	ldebug("LOCK");			return LOCK; }
LOOP			{	ldebug("LOOP");			return LOOP; }
METHODS			{	ldebug("METHODS");		return METHODS; }
MOD				{	retkey(MOD); }
MODULE			{	ldebug("MODULE");		return MODULE; }
NOT				{	retkey(NOT); }
OBJECT			{	ldebug("OBJECT");		return OBJECT; }
OF				{	ldebug("OF");			return OF; }
OR				{	retkey(OR); }
OVERRIDES		{	ldebug("OVERRIDES");	return OVERRIDES; }
PROCEDURE		{	ldebug("PROCEDURE");	return PROCEDURE; }
RAISE			{	ldebug("RAISE");		return RAISE; }
RAISES			{	ldebug("RAISES");		return RAISES; }
READONLY		{	ldebug("READONLY");		return READONLY; }
RECORD			{	ldebug("RECORD");		return RECORD; }
REF				{	ldebug("REF");			return REF; }
REPEAT			{	ldebug("REPEAT");		return REPEAT; }
RETURN			{	ldebug("RETURN");		return RETURN; }
REVEAL			{	ldebug("REVEAL");		return REVEAL; }
ROOT			{	ldebug("ROOT");			return ROOT; }
SET				{	ldebug("SET");			return SET; }
THEN			{	ldebug("THEN");			return THEN; }
TO				{	ldebug("TO");			return TO; }
TRY				{	ldebug("TRY");			return TRY; }
TYPE			{	ldebug("TYPE");			return TYPE; }
TYPECASE		{	ldebug("TYPECASE");		return TYPECASE; }
UNSAFE			{	ldebug("UNSAFE");		return UNSAFE; }
UNTIL			{	ldebug("UNTIL");		return UNTIL; }
UNTRACED		{	ldebug("UNTRACED");		return UNTRACED; }
VALUE			{	ldebug("VALUE");		return VALUE; }
VAR				{	ldebug("VAR");			return VAR; }
WHILE			{	ldebug("WHILE");		return WHILE; }
WITH			{	ldebug("WITH");			return WITH; }	
NEW				{	ldebug("NEW");			return NEW; }

{id}			{	
					ldebug("id");
					yylval.number = save_literal(yytext);
					return ID;
				}
{integer}		{	
					ldebug("integer");
					yylval.number = atol( yytext );
					return INTEGER;
				}
{float}			{	
					ldebug("real");
					yylval.fnumber = atof( yytext );
					return FLOAT;
				}
{string}		{	
					ldebug("string");
					yylval.number = save_literal( yytext );
					return STRING;
				}
"+"				{	retkey(KEY_PLUS); }
"-"				{	retkey(KEY_MINUS); }
"*"				{	retkey(KEY_MUL); }
"/"				{	retkey(KEY_DIV); }
"<"				{	retkey(KEY_LT); }
">"				{	retkey(KEY_GT); }
"<="			{	retkey(KEY_LEQ); }
">="			{	retkey(KEY_GEQ); }
"#"				{	retkey(KEY_NEQ); /* KEY_HASH redundant */ }
"{"				{	retkey(KEY_LBRA); }
"("				{	retkey(KEY_LPAR); }
"["				{	retkey(KEY_LSPAR); }
"="				{	retkey(KEY_EQ); }
"}"				{	retkey(KEY_RBRA); }
")"				{	retkey(KEY_RPAR); }
"]"				{	retkey(KEY_RSPAR); }
";"				{	retkey(KEY_SEMI); }
"|"				{	retkey(KEY_PIPE); }
"^"				{	retkey(KEY_UP); }
"."				{	retkey(KEY_DOT); }
".."			{	retkey(KEY_DOTDOT); }
":="			{	retkey(KEY_ASSIGN); }
","				{	retkey(KEY_COMMA); }
"&"				{	retkey(KEY_AND); }
":"				{	retkey(KEY_COL); }
"<:"			{	retkey(KEY_SUB); }
"=>"			{	retkey(KEY_IMP); }
"<>"			{	retkey(KEY_NEQ); }

.				{	ldebug("garbage");		fatal_error("Bad lexem: %s",yytext); }
%%

void ldebug(char *rule) {
#ifdef M3_DEBUG_LEX
		fprintf(stderr,"[%ld] s> |%-30s|%-30s| (%d)\n", yylineno, rule, yytext, yyleng);
		fflush(stderr);
#endif
}
