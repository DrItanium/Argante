#include "bcode_op.h"
#include "data_blk.h"

#define STATE_OPTION 0
#define STATE_DATA 1
#define STATE_CODE 2

typedef struct _symbol symbol;
typedef struct _areloc areloc;

struct _symbol {
	struct stable s;
	symbol *next;
	areloc *reloc;
};

struct _areloc {
	struct reloc r;
	areloc *next;
};

typedef struct _context context;
struct _context {
	symbol *syms;
	context *prev;
};

/* Note that reloc entries always come just after their respective symbol. */
#define RELOC_ADDR		1
#define RELOC_SIZE_DWORD	2
#define RELOC_SIZE_BYTE		3

