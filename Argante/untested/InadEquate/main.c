#include <stdio.h>
#include "string.h"
#include "tree.h"

extern int EM_TokPos;
extern int EM_LineNo;

void yyerror(char *s) {
	EM_ErrorMessage(s);
}

void EM_ErrorMessage(char *s)
{
	fprintf(stderr, "<+> Line %d: %s around char %d\n", EM_LineNo, s, EM_TokPos);
}

extern int yyparse();

extern int yynerrs;
extern int yydebug;

FILE *yyin;
int main()
{
	int e;
	yyin=stdin;
//	yydebug=1;

	/* Lex and parse */
	if ((e=yyparse()) || yynerrs)
	{
		fprintf(stderr, "<!> %d errors encountered.\n", yynerrs);
		return 3;
	}
	/* Ok, we have our tree. */
	/* Now finalize those type definitions */
	
	/* declare those variables and functions */
	
	/* Now loop through the tree, do typechecking,
	 * constant optimization, and break into temporaries */
	
	/* Register Allocation. */
	
	/* We're done. Dump code (core? :P) */
	return 0;
}
