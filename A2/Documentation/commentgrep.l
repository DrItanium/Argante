%option noyywrap
%{
#include <stdio.h>

int commentlevel=0;
int broken;
/* Option yylineno feeds newlines through...? */
int LineNo=0;

/* much simplified inAdequate lexer... a dubious provenance... */
%}

%x COMMENT

%%

<INITIAL>\n		{ LineNo++; }
<COMMENT>\n([\t| ])*	{ LineNo++; broken=1; putc('\n', stdout); }
<INITIAL>"/*"		{ commentlevel=1; broken=0; BEGIN COMMENT; }
<INITIAL>"*/"		{ fprintf(stderr, "line %d: overterminated comment\n", LineNo); }

<COMMENT>"/*"		{ commentlevel++; fprintf(stderr, "line %d: nested comment\n", LineNo);}
<COMMENT>"*/"		{ commentlevel--;
			if (!commentlevel) {
				if (!broken) putc('\n', stdout); 
				BEGIN INITIAL;
				}
			}
<COMMENT>.		{ putc(*yytext, stdout); }
.			{ }
%%
int main(int argc, char **argv) {
	int i;
	argc--; argv++;
	if (argc) {
		for(i=0;i<argc;i++) {
		yyin=fopen(argv[i], "r");
		if (!yyin) fprintf(stderr, "couldn't open %s\n", argv[i]);
		else {
			printf("%s:\n", argv[i]);
			yylex();
			fclose(yyin);
		}
		}
	} else yylex();
}
