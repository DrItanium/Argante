
/* strcmpi is the old name for it, and as it's <8char DOS likes the old name. */
#ifndef HAVE_STRNCASECMP
#define strncasecmp strncmpi
#ifdef HAVE_STRNCMPI
#include <string.h>
#else
extern int strncmpi(const char *a1, const char *a2, unsigned size);
#endif
#endif
