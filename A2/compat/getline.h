
/* no #ifdef because we need _GNU_SOURCE for getline. Yuk. */
#include <stdio.h>
#include <sys/types.h>
extern ssize_t getline(char **LINEPTR, size_t *N, FILE *STREAM);

