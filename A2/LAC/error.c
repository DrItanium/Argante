#include <stdio.h>
#include <stdarg.h>
#include "lstring.h"
#include "main.h"

int EM_Error(const char *format, ...)
{
	va_list ap;
	int bytes;

	bytes=fprintf(stderr, "<+> %s, line %d: ", StringToChar(EM_FileName), EM_LineNo);
	va_start(ap, format);
	bytes+=vfprintf(stderr, format, ap);
	va_end(ap);
	bytes+=fprintf(stderr, "\n");
   yynerrs++;
	return bytes;
}

int EM_Warn(const char *format, ...)
{
	va_list ap;
	int bytes;

	bytes=fprintf(stderr, "<-> %s, line %d: ", StringToChar(EM_FileName), EM_LineNo);
	va_start(ap, format);
	bytes+=vfprintf(stderr, format, ap);
	va_end(ap);
	bytes+=fprintf(stderr, "\n");
	return bytes;
}
