#define REF_PTR 1
#define REF_SIZE_CHAR 2
#define REF_SIZE_DWORD 3

typedef struct _depend depend;
struct _depend {
	char reftype;
	char stortype;
	off_t location;
	depend *next;
};

typedef struct _symbol symbol;
struct _symbol {
	char name[62];
	char stortype;
	char defined;
	int location;
	int size;
	depend *dep;
	symbol *next;
};

#define STATE_OPTION 0
#define STATE_DATA 1
#define STATE_CODE 2

/* Stortype flags.
 * SYM_NO_CLASH means the linker should not check for duplicate definitions
 * of this symbol.
 */
#define SYM_NO_CLASH 0x10

typedef struct _context context;
struct _context {
	symbol *syms;
	context *prev;
};


