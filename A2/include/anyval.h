#ifndef _HAVE_ANYVAL
#define _HAVE_ANYVAL

typedef struct {
	union {
		unsigned long u;
		signed long s;
		a2float f;
	} val;
} anyval;

#endif
