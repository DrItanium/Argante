
/* strcmpi is the old name for it, and as it's <8char DOS likes the old name. */
#ifndef HAVE_STRCASECMP
#define strcasecmp strcmpi
#define strncasecmp strncmpi
#ifdef HAVE_STRCMPI
#include <string.h>
#else
extern int strcmpi(const char *a1, const char *a2);
extern int strncmpi(const char *a1, const char *a2, unsigned size);
#endif
#endif
