#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "m3common.h"

void print_info( char *info, char *pattern, va_list vl )
{
	fprintf(stderr, "%s: ", info);
	vfprintf(stderr, pattern, vl);
	fprintf(stderr,"\n");
} /* print_info */

void fatal_error(char *s, ...)
{
	va_list vl;
	
	va_start(vl,s);
	print_info("Fatal error",s,vl);
	va_end(vl);
	exit(1);
} /* fatal_error */

void info(char *s, ...)
{
	va_list vl;
	
	va_start(vl,s);
	print_info("INFO",s,vl);
	va_end(vl);
} /* info */

void warning( char *s, ...)
{
	va_list vl;
	
	va_start(vl,s);
	print_info("Warning",s,vl);
	va_end(vl);
} /* warning */

void *my_alloc(size_t size)
{
	void *p;
	p=malloc(size);
	if (!p)
		fatal_error("Can't allocate memory");
	memset(p,0,size);
	return p;
} /* my_alloc */
