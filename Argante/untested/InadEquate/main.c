#include <stdio.h>

FILE *yyin;
int main()
{
	yyin=stdin;
	return yyparse();
}
