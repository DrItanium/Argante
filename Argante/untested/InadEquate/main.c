/*
 * inadEquate main module.
 * (c) 2001 James Kehl.
 * Licensed under GPL
 * 
 *         lambent neon lights
 *     in the storm of snowflakes
 *         enveloping silence
 */
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

void NErrs(int d)
{
		fprintf(stderr, "<!> %d errors encountered.\n", yynerrs);
		exit(d);
}

FILE *yyin;
int main()
{
	int e;
	yyin=stdin;
//	yydebug=1;

	/* PHASE0: Lex and parse */
	if ((e=yyparse()) || yynerrs) NErrs(3);
	/* PHASE1: Split out functions, Resolve TIDs */
	Phase1();
	if (yynerrs) NErrs(2);
	/* PHASE2: finalize compound types, constant optimization */
	
	/* PHASE(3/4?): Register Allocation. */
	
	/* We're done. Dump code (core? :P) */
	return 0;
}
