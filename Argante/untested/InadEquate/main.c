#include <stdio.h>

void yyerror(char *s) {
	fprintf(stderr, "%s", s);
}
extern int yyparse();

FILE *yyin;
int main()
{
	yyin=stdin;
	return yyparse();
}
