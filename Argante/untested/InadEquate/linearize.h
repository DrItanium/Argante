#define REG_U 0x10000
#define REG_S 0x20000
#define REG_F 0x40000

struct _AVar {
	string id;
	AType type;
	int escaperefs; /* Whether we must move this into memory every call */
	int regType; /* u0? f0? f15? */
	enum { VTemp, VParam, VDeclared } kind;
	AVar next;
};

#define VARTABSIZE 100

typedef struct _AFunc *AFunc;
struct _AFunc {
	string id;
	AType retType;
	union { string s; AFunc f; } errhandler;
	AStm code;
	AVar vars[VARTABSIZE];
	AFunc parent;
	AFunc next;
};

extern void Phase1();
extern void TypeDoTID();
extern int CanFunctionSee(AFunc me, AFunc see);
extern AFunc FindFunction(AFunc perspective, string id);
extern AType FindType(string id);

extern AVar makeVar(string id, AFunc in);
extern AVar findVar(AFunc in, void *id);

extern AFunc NullFunc;

