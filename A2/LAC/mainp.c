/*
 * ia^H^H main module.
 * (c) 2001 James Kehl.
 * Licensed under GPL version 2.
 * 
 *         lambent neon lights
 *     in the storm of snowflakes
 *         enveloping silence
 */
#include <stdio.h>
#include "lstring.h"
#include "main.h"
#include "codegen.h"

extern int EM_TokPos;

void yyerror(char *s) {
	EM_Error("%s at char %d", s, EM_TokPos);
}

extern int yyparse(void);

extern int yydebug;

void NErrs(int d)
{
		fprintf(stderr, "<!> %d errors encountered.\n", yynerrs);
		exit(d);
}

extern FILE *yyin;
FILE *codeout;

string EM_FileName;

int main(int argc, char **argv)
{
	int e;
	if (argc < 3) {
		fprintf(stderr, "Usage: %s file.in file.out\n", argv[0]);
		exit(1);
	}
	
	yyin=fopen(argv[1], "r");
	EM_FileName=String(argv[1]);
	if (!yyin) perror("fopen in");
	codeout=fopen(argv[2], "w");
	if (!codeout) perror("fopen out");

	if ((e=yyparse()) || yynerrs) NErrs(2);
	
	fprintf(codeout, ".RODATA\n");
	DumpStringTable();
	fprintf(codeout, ".DATA\n");
	GenStackMan_Main();
	GenGlobals();
	fprintf(codeout, ".CODE\n");
	GenAllCode();
	
	if (yynerrs) NErrs(1);

	return 0;
}
