#ifndef HAVE_STRTOK_R
extern char *strtok_r(char *buf, const char *sep, char **reent);
#else
#include <string.h>
#endif
