#ifndef BISON_PARSER_H
# define BISON_PARSER_H

#ifndef YYSTYPE
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
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	VALUE	257
# define	TYPE	258
# define	STRING	259
# define	AOPER	260
# define	CMP	261
# define	ID	262
# define	FUNC	263
# define	SYSCALL	264
# define	IGNORE	265
# define	STRADDR	266
# define	STRLEN	267
# define	GOTO	268
# define	HANDLER	269
# define	IF	270
# define	RETURN	271
# define	WAIT	272
# define	ERRNO	273
# define	RAISE	274
# define	ALLOC	275
# define	REALLOC	276
# define	FINALIZE	277
# define	UNFINALIZE	278
# define	DEALLOC	279


extern YYSTYPE yylval;

#endif /* not BISON_PARSER_H */
