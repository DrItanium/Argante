typedef struct _mVarH mVarH;
struct _mVarH {
	mVar *v;
	mVarH *next;
};

typedef struct _function function;
struct _function {
	string id;
	
	Stm *code;
	
	int declregs; /* Registers used by the function's call */
	int callregs; /* Registers used by calls throughout the function */
	int usedregs; /* Registers used generally */

	mVarH **varhash;
	int varhashsz;
	int varhashused;
	
	function *next;
};

extern function *global;
extern function *chain;

extern int Call_IsSyscall(string id);
extern mVar *FindVarInFunc(string v, function *f);

