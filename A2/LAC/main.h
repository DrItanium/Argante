
/* ERROR HANDLING */
extern string EM_FileName;
extern int EM_LineNo;
extern int yynerrs;

extern int EM_Error(const char *format, ...);
extern int EM_Warn(const char *format, ...);

extern FILE *yyin;
extern FILE *codeout;

/* Types and typelists */
typedef enum { TypeUnsigned, TypeSigned, TypeFloat, TypeIgnore /* 'void' */ } TypeBase;
typedef struct {
	TypeBase basetype;
	int pointerct;
} mType;

typedef struct _TypeList TypeList;
struct _TypeList {
	mType type;
	TypeList *next;
	TypeList *last;
};

/* Argument lists */
typedef struct {
	string id;
	mType type;
} mIType;

typedef struct _ITypeList ITypeList;
struct _ITypeList {
	mIType itype;
	ITypeList *next;
	ITypeList *last;
};

/* Variables and register lists */
#define STRREF_NOT 0
#define STRREF_ADDR 1
#define STRREF_LEN 2

typedef struct {
	mType type;
	enum { VarConst, VarRegister, VarGlobal, VarErrno } kind;
	int ptrderef;
	int strreftype;
	string id;
	union {
		unsigned constnt_u;
		signed constnt_s;
		float constnt_f;
		int registr;
	} u;
} mVar;

typedef struct _RegList RegList;
struct _RegList {
	mVar *var;
	RegList *next;
	RegList *last;
};

typedef enum { AEq, APlus, AMinus, AMul, ADiv, AShl, AShr, ARol, ARor, AXor, AOr, AAnd, AMod } AOper;
typedef enum { IEq, INEq, IGt, ILt } CmpOper;
typedef struct _Stm Stm;
struct _Stm {
	string file;
	int lineno;
	enum { StmVarDef, StmAssign, StmLabel, StmHandler, StmGoto, StmIf,
		StmCall, StmReglist, StmReturn, StmWait, StmRaise,
		StmAlloc, StmRealloc, StmFinalize, StmUnfinalize, StmDealloc
	} kind;
	union {
		/* VarDef */
		struct {
			mType type;
			string id;
		} vardef;
		/* Assign */
		struct {
			mVar *to;
			mVar *from;
			AOper kind;
		} assign;
		/* Label, Handler, Goto */
		string label;
		/* If */
		struct {
			mVar *a1;
			mVar *a2;
			CmpOper kind;
			string label;
		} sif;
		/* Call */
		string func;
		/* RegList */
		struct {
			RegList *r;
			enum { RSet, RDeref } kind;
		} reglist;
		/* Wait */
		struct {
			mVar *w;
		} wait;
		/* (re)alloc */
		struct {
			mVar *addrdest;
			mVar *size;
		} alloc;
		/* (un)finalize */
		struct {
			mVar *addr;
		} fin;
	} u;
	Stm *next;
	Stm *last;
};

extern mType *Type(TypeBase base);
extern mIType *IType(string id, mType *t);

extern TypeList *TypeList_AddType(TypeList *to, mType *next);
extern ITypeList *ITypeList_AddType(ITypeList *, mIType *);
extern TypeList *TypeList_fromI(ITypeList *from);
extern RegList *Reglist_AddVar(RegList *, mVar *);

extern void PrototypeT(string id, TypeList *ret, TypeList *args);
extern void PrototypeI(string id, TypeList *ret, ITypeList *args);
extern void PrototypeSCT(string id, TypeList *ret, TypeList *args);
extern void PrototypeSCI(string id, TypeList *ret, ITypeList *args);
extern void Function(string id, TypeList *ret, ITypeList *args, Stm *code);

extern mVar *Var_Ignore(void);
extern mVar *Var_Errno(void);
extern mVar *Var_Register(string id);
extern mVar *Var_Global(string id);
extern mVar *Var_ConstU(unsigned);
extern mVar *Var_ConstF(float);
extern mVar *VarStrAddr(string);
extern mVar *VarStrLen(string);

extern Stm *StmList_Join(Stm *, Stm *);
extern Stm *Stm_VarDef(mIType *);
extern Stm *Stm_Assign(mVar *, AOper, mVar *);
extern Stm *Stm_Label(string);
extern Stm *Stm_Goto(string);
extern Stm *Stm_Handler(string);
extern Stm *Stm_If(mVar *, CmpOper, mVar *, string);
extern Stm *Stm_Return(void);
extern Stm *Stm_Wait(mVar *);
extern Stm *Stm_Raise(mVar *);
extern Stm *Stm_Call(string);
extern Stm *Stm_ReglistSet(RegList *);
extern Stm *Stm_ReglistGet(RegList *);

extern Stm *Stm_Alloc(mVar *addrdest, mVar *size);
extern Stm *Stm_Realloc(mVar *addrdest, mVar *size);
extern Stm *Stm_Finalize(mVar *addr);
extern Stm *Stm_Unfinalize(mVar *addr);
extern Stm *Stm_Dealloc(mVar *addr);

#define ERRNO_REG 31

