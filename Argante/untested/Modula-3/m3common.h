#ifndef _M3COMMON_H_
#define _M3COMMON_H_

#include <stdlib.h>
#include <stdio.h>

#define M3_DEBUG_LEX
#undef M3_DEBUG_BISON
#undef M3_DEBUG_NODE
#undef M3_DEBUG_TOKEN
#undef M3_PRINT_TREE
#undef M3_PRINT_LITERALS

#define SAYONARA { \
		fprintf(stderr,"%s:[%d]: Fatal Error - SAYONARA\n", \
			__FILE__, __LINE__); \
		exit(1); \
	}

#define t_id int
#define t_type int
#define t_env struct s_env

void fatal_error(char *s, ...);
void warning(char *s, ...);
void info(char *s, ...);
void *my_alloc(size_t size);
#endif
