typedef union {
	mVar *var;
	mType *type;
	mIType *itype;
	string string;
	string id;
	AOper optype;
	CmpOper cmptype;
	Stm *stm;
	RegList *reglist;
	TypeList *typelist;
	ITypeList *itypelist;
} YYSTYPE;
#define	VALUE	258
#define	TYPE	259
#define	STRING	260
#define	AOPER	261
#define	CMP	262
#define	ID	263
#define	FUNC	264
#define	SYSCALL	265
#define	IGNORE	266
#define	STRADDR	267
#define	STRLEN	268
#define	GOTO	269
#define	HANDLER	270
#define	IF	271
#define	RETURN	272
#define	WAIT	273
#define	ERRNO	274
#define	RAISE	275
#define	ALLOC	276
#define	REALLOC	277
#define	FINALIZE	278
#define	UNFINALIZE	279
#define	DEALLOC	280


extern YYSTYPE yylval;
