
/*  A Bison parser, made from parser.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

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

#line 1 "parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lstring.h"
#include "main.h"

#define YYERROR_VERBOSE
#define YYDEBUG 1

void yyerror(char *);
int yylex(void);

Stm *prog;


#line 17 "parser.y"
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
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		151
#define	YYFLAG		-32768
#define	YYNTBASE	37

#define YYTRANSLATE(x) ((unsigned)(x) <= 280 ? yytranslate[x] : 51)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    28,
    30,     2,     2,    29,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    34,    26,     2,
    33,     2,     2,    27,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    35,     2,    36,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    31,     2,    32,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     7,    10,    13,    16,    20,    23,    26,
    29,    33,    36,    42,    48,    54,    60,    66,    73,    75,
    77,    80,    84,    87,    89,    91,    93,    95,   100,   105,
   107,   110,   115,   120,   123,   127,   131,   139,   143,   147,
   150,   154,   158,   163,   166,   170,   178,   186,   192,   198,
   204,   210,   217,   220,   224,   227,   231
};

static const short yyrhs[] = {    46,
     0,     1,    26,     0,     4,     0,    27,    39,     0,    39,
     8,     0,    28,    39,     0,    41,    29,    39,     0,    41,
    30,     0,    28,    30,     0,    28,    40,     0,    43,    29,
    40,     0,    43,    30,     0,     9,    42,     8,    42,    26,
     0,     9,    42,     8,    44,    26,     0,    10,    42,     8,
    42,    26,     0,    10,    42,     8,    44,    26,     0,     9,
    42,     8,    44,    38,     0,     9,    42,     8,    28,    30,
    38,     0,    45,     0,    38,     0,    46,    38,     0,    31,
    46,    32,     0,    40,    26,     0,    19,     0,    11,     0,
     8,     0,     3,     0,    12,    28,     5,    30,     0,    13,
    28,     5,    30,     0,    47,     0,    27,    47,     0,    48,
    33,    48,    26,     0,    48,     6,    48,    26,     0,     8,
    34,     0,    14,     8,    26,     0,    15,     8,    26,     0,
    16,    48,     7,    48,    14,     8,    26,     0,    18,    48,
    26,     0,    20,    48,    26,     0,    17,    26,     0,    17,
    50,    26,     0,     8,    50,    26,     0,     8,    35,    36,
    26,     0,    50,    26,     0,    50,    33,    26,     0,    21,
    28,    48,    29,    48,    30,    26,     0,    22,    28,    48,
    29,    48,    30,    26,     0,    23,    28,    48,    30,    26,
     0,    24,    28,    48,    30,    26,     0,    25,    28,    48,
    30,    26,     0,    50,    33,     8,    50,    26,     0,    50,
    33,     8,    35,    36,    26,     0,    35,    48,     0,    49,
    29,    48,     0,    35,     5,     0,    49,    29,     5,     0,
    49,    36,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    56,    59,    61,    62,    64,    66,    67,    68,    69,    71,
    72,    73,    75,    76,    77,    78,    79,    80,    82,    84,
    85,    86,    87,    89,    90,    91,    92,    93,    94,    96,
    97,    99,   100,   101,   102,   103,   104,   105,   106,   108,
   109,   110,   111,   112,   113,   115,   116,   117,   118,   119,
   121,   122,   124,   125,   127,   128,   130
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","VALUE",
"TYPE","STRING","AOPER","CMP","ID","FUNC","SYSCALL","IGNORE","STRADDR","STRLEN",
"GOTO","HANDLER","IF","RETURN","WAIT","ERRNO","RAISE","ALLOC","REALLOC","FINALIZE",
"UNFINALIZE","DEALLOC","';'","'@'","'('","','","')'","'{'","'}'","'='","':'",
"'['","']'","prog","stm","type1","itype","typelistA","typelist","itypelistA",
"itypelist","prototype","stmlist","var","var1","reglistA","reglist", NULL
};
#endif

static const short yyr1[] = {     0,
    37,    38,    39,    39,    40,    41,    41,    42,    42,    43,
    43,    44,    45,    45,    45,    45,    45,    45,    38,    46,
    46,    38,    38,    47,    47,    47,    47,    47,    47,    48,
    48,    38,    38,    38,    38,    38,    38,    38,    38,    38,
    38,    38,    38,    38,    38,    38,    38,    38,    38,    38,
    38,    38,    49,    49,    49,    49,    50
};

static const short yyr2[] = {     0,
     1,     2,     1,     2,     2,     2,     3,     2,     2,     2,
     3,     2,     5,     5,     5,     5,     5,     6,     1,     1,
     2,     3,     2,     1,     1,     1,     1,     4,     4,     1,
     2,     4,     4,     2,     3,     3,     7,     3,     3,     2,
     3,     3,     4,     2,     3,     7,     7,     5,     5,     5,
     5,     6,     2,     3,     2,     3,     2
};

static const short yydefact[] = {     0,
     0,    27,     3,    26,     0,     0,    25,     0,     0,     0,
     0,     0,     0,     0,    24,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    20,     0,     0,    19,     0,    30,
     0,     0,     0,     2,    34,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    26,     0,     0,    40,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     4,    31,
     0,    55,    53,     5,    23,    21,     0,     0,     0,    57,
    44,     0,     0,    42,     9,     6,     0,     8,     0,     0,
     0,     0,    35,    36,     0,    41,    38,    39,     0,     0,
     0,     0,     0,    22,     0,     0,    56,    54,     0,    45,
    43,     7,     0,     0,     0,     0,     0,     0,     0,    28,
    29,     0,     0,     0,     0,     0,     0,    33,    32,     0,
     0,     0,     6,    10,    13,     0,    12,    14,    17,    15,
    16,     0,     0,     0,    48,    49,    50,     0,    51,    18,
    11,     0,     0,     0,    52,    37,    46,    47,     0,     0,
     0
};

static const short yydefgoto[] = {   149,
    25,    26,    27,    39,    40,   105,   106,    28,    29,    30,
    31,    32,    33
};

static const short yypact[] = {   223,
    -8,-32768,-32768,    -4,    -2,    -2,-32768,    12,    24,    14,
    20,    78,   -10,    78,-32768,    78,    25,    26,    32,    34,
    35,   260,   223,   277,-32768,    57,    40,-32768,   103,-32768,
     1,   -15,    -6,-32768,-32768,    56,    44,     5,    18,    63,
    64,    69,    71,    51,    52,-32768,   302,    80,-32768,    59,
    67,    68,    78,    78,    78,    78,    78,    11,-32768,-32768,
   132,-32768,-32768,-32768,-32768,-32768,    78,    78,   289,-32768,
-32768,     3,    73,-32768,-32768,-32768,    11,-32768,    72,    81,
    65,    99,-32768,-32768,    78,-32768,-32768,-32768,   102,   108,
   109,   128,   130,-32768,    84,   106,-32768,-32768,   126,-32768,
-32768,-32768,    19,   136,    21,   165,     5,   139,   144,-32768,
-32768,   157,    78,    78,   146,   167,   168,-32768,-32768,   248,
   173,   194,    57,-32768,-32768,    11,-32768,-32768,-32768,-32768,
-32768,   193,   192,   198,-32768,-32768,-32768,   197,-32768,-32768,
-32768,   204,   226,   229,-32768,-32768,-32768,-32768,   249,   257,
-32768
};

static const short yypgoto[] = {-32768,
   -24,   -19,   -90,-32768,     0,-32768,   182,-32768,   242,   -14,
   -12,-32768,    -3
};


#define	YYLAST		321


static const short yytable[] = {    48,
    37,    51,    59,    52,    66,    41,    67,    60,     3,    50,
    99,    63,   124,    69,     3,    49,   124,    34,    76,    71,
    70,    44,     3,    63,    24,    38,    72,    45,   100,    35,
    36,    58,    60,    68,    75,   141,    66,    58,    59,    42,
    89,    90,    91,    92,    93,    58,    77,    78,   122,   126,
   127,    43,    53,    54,    95,    96,    98,   102,     2,    55,
    62,    56,    57,    46,    64,    65,     7,     8,     9,    74,
    79,    80,   112,    81,    15,    82,    83,    84,   104,   108,
     2,   129,    47,   123,    86,    46,    85,   123,     7,     8,
     9,    73,    87,    88,   110,   121,    15,   140,   101,   103,
   133,   134,    -1,     1,    47,     2,     3,    63,   107,   118,
     4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,   111,    22,
   113,   119,     1,    23,     2,     3,   114,    24,   115,     4,
     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    18,    19,    20,    21,   116,    22,   117,
   120,   125,    23,    94,   130,     1,    24,     2,     3,   131,
   132,   135,     4,     5,     6,     7,     8,     9,    10,    11,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
   128,    22,   136,   137,     1,    23,     2,     3,   139,    24,
   142,     4,     5,     6,     7,     8,     9,    10,    11,    12,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    -9,
    22,   143,   145,     1,    23,     2,     3,   144,    24,   146,
     4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,   150,    22,
     2,   147,    62,    23,   148,    46,   151,    24,     7,     8,
     9,   109,     2,     3,    61,     0,    15,    46,     0,     0,
     7,     8,     9,     0,    47,     0,     0,     0,    15,     2,
     0,    62,     0,   138,    46,     0,    58,     7,     8,     9,
     0,     2,     0,    97,     0,    15,    46,     0,     0,     7,
     8,     9,     0,    47,     2,     0,     0,    15,     0,    46,
     0,     0,     7,     8,     9,    47,     0,     0,     0,     0,
    15
};

static const short yycheck[] = {    12,
     4,    14,    22,    16,    29,     6,     6,    22,     4,    13,
     8,    24,   103,    29,     4,    26,   107,    26,    38,    26,
    36,     8,     4,    36,    35,    28,    33,     8,    26,    34,
    35,    27,    47,    33,    30,   126,    61,    27,    58,    28,
    53,    54,    55,    56,    57,    27,    29,    30,    30,    29,
    30,    28,    28,    28,    67,    68,    69,    77,     3,    28,
     5,    28,    28,     8,     8,    26,    11,    12,    13,    26,
     8,     8,    85,     5,    19,     5,    26,    26,    79,    80,
     3,   106,    27,   103,    26,     8,     7,   107,    11,    12,
    13,    36,    26,    26,    30,    99,    19,   122,    26,    28,
   113,   114,     0,     1,    27,     3,     4,   120,    28,    26,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    30,    27,
    29,    26,     1,    31,     3,     4,    29,    35,    30,     8,
     9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    30,    27,    30,
    35,    26,    31,    32,    26,     1,    35,     3,     4,    26,
    14,    26,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    26,    26,     1,    31,     3,     4,    26,    35,
     8,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    30,    26,     1,    31,     3,     4,    30,    35,    26,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,     0,    27,
     3,    26,     5,    31,    26,     8,     0,    35,    11,    12,
    13,    80,     3,     4,    23,    -1,    19,     8,    -1,    -1,
    11,    12,    13,    -1,    27,    -1,    -1,    -1,    19,     3,
    -1,     5,    -1,    36,     8,    -1,    27,    11,    12,    13,
    -1,     3,    -1,     5,    -1,    19,     8,    -1,    -1,    11,
    12,    13,    -1,    27,     3,    -1,    -1,    19,    -1,     8,
    -1,    -1,    11,    12,    13,    27,    -1,    -1,    -1,    -1,
    19
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 56 "parser.y"
{ yyval.stm=prog=yyvsp[0].stm; Function(NULL, NULL, NULL, yyvsp[0].stm); ;
    break;}
case 2:
#line 59 "parser.y"
{ yyval.stm=NULL; yyerrok; ;
    break;}
case 3:
#line 61 "parser.y"
{ yyval.type=yyvsp[0].type; ;
    break;}
case 4:
#line 62 "parser.y"
{ yyvsp[0].type->pointerct++; yyval.type=yyvsp[0].type; ;
    break;}
case 5:
#line 64 "parser.y"
{ yyval.itype=IType(yyvsp[0].id, yyvsp[-1].type); ;
    break;}
case 6:
#line 66 "parser.y"
{ yyval.typelist=TypeList_AddType(NULL, yyvsp[0].type); ;
    break;}
case 7:
#line 67 "parser.y"
{ yyval.typelist=TypeList_AddType(yyvsp[-2].typelist, yyvsp[0].type); ;
    break;}
case 8:
#line 68 "parser.y"
{ yyval.typelist=yyvsp[-1].typelist; ;
    break;}
case 9:
#line 69 "parser.y"
{ yyval.typelist=NULL; ;
    break;}
case 10:
#line 71 "parser.y"
{ yyval.itypelist=ITypeList_AddType(NULL, yyvsp[0].itype); ;
    break;}
case 11:
#line 72 "parser.y"
{ yyval.itypelist=ITypeList_AddType(yyvsp[-2].itypelist, yyvsp[0].itype); ;
    break;}
case 12:
#line 73 "parser.y"
{ yyval.itypelist=yyvsp[-1].itypelist; ;
    break;}
case 13:
#line 75 "parser.y"
{ PrototypeT(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].typelist); yyval.stm=NULL; ;
    break;}
case 14:
#line 76 "parser.y"
{ PrototypeI(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].itypelist); yyval.stm=NULL; ;
    break;}
case 15:
#line 77 "parser.y"
{ PrototypeSCT(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].typelist); yyval.stm=NULL; ;
    break;}
case 16:
#line 78 "parser.y"
{ PrototypeSCI(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].itypelist); yyval.stm=NULL; ;
    break;}
case 17:
#line 79 "parser.y"
{ Function(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].itypelist, yyvsp[0].stm); yyval.stm=NULL; ;
    break;}
case 18:
#line 80 "parser.y"
{ Function(yyvsp[-3].id, yyvsp[-4].typelist, NULL, yyvsp[0].stm); yyval.stm=NULL; ;
    break;}
case 19:
#line 82 "parser.y"
{ yyval.stm=NULL; ;
    break;}
case 20:
#line 84 "parser.y"
{ yyval.stm=yyvsp[0].stm; ;
    break;}
case 21:
#line 85 "parser.y"
{ yyval.stm=StmList_Join(yyvsp[-1].stm, yyvsp[0].stm); ;
    break;}
case 22:
#line 86 "parser.y"
{ yyval.stm=yyvsp[-1].stm; ;
    break;}
case 23:
#line 87 "parser.y"
{ yyval.stm=Stm_VarDef(yyvsp[-1].itype);  /* free($1); ? */  ;
    break;}
case 24:
#line 89 "parser.y"
{ yyval.var=Var_Errno(); ;
    break;}
case 25:
#line 90 "parser.y"
{ yyval.var=Var_Ignore(); ;
    break;}
case 26:
#line 91 "parser.y"
{ yyval.var=Var_Register(yyvsp[0].id); ;
    break;}
case 27:
#line 92 "parser.y"
{ yyval.var=yyvsp[0].var; ;
    break;}
case 28:
#line 93 "parser.y"
{ yyval.var=VarStrAddr(yyvsp[-1].string); ;
    break;}
case 29:
#line 94 "parser.y"
{ yyval.var=VarStrLen(yyvsp[-1].string); ;
    break;}
case 30:
#line 96 "parser.y"
{ yyval.var=yyvsp[0].var; ;
    break;}
case 31:
#line 97 "parser.y"
{ yyvsp[0].var->ptrderef=1; yyval.var=yyvsp[0].var; ;
    break;}
case 32:
#line 99 "parser.y"
{ yyval.stm=Stm_Assign(yyvsp[-3].var, AEq, yyvsp[-1].var); ;
    break;}
case 33:
#line 100 "parser.y"
{ yyval.stm=Stm_Assign(yyvsp[-3].var, yyvsp[-2].optype, yyvsp[-1].var); ;
    break;}
case 34:
#line 101 "parser.y"
{ yyval.stm=Stm_Label(yyvsp[-1].id); ;
    break;}
case 35:
#line 102 "parser.y"
{ yyval.stm=Stm_Goto(yyvsp[-1].id); ;
    break;}
case 36:
#line 103 "parser.y"
{ yyval.stm=Stm_Handler(yyvsp[-1].id); ;
    break;}
case 37:
#line 104 "parser.y"
{ yyval.stm=Stm_If(yyvsp[-5].var, yyvsp[-4].cmptype, yyvsp[-3].var, yyvsp[-1].id); ;
    break;}
case 38:
#line 105 "parser.y"
{ yyval.stm=Stm_Wait(yyvsp[-1].var); ;
    break;}
case 39:
#line 106 "parser.y"
{ yyval.stm=Stm_Raise(yyvsp[-1].var); ;
    break;}
case 40:
#line 108 "parser.y"
{ yyval.stm=Stm_Return(); ;
    break;}
case 41:
#line 109 "parser.y"
{ yyval.stm=StmList_Join(Stm_ReglistSet(yyvsp[-1].reglist), Stm_Return()); ;
    break;}
case 42:
#line 110 "parser.y"
{ yyval.stm=StmList_Join(Stm_ReglistSet(yyvsp[-1].reglist), Stm_Call(yyvsp[-2].id)); ;
    break;}
case 43:
#line 111 "parser.y"
{ yyval.stm=Stm_Call(yyvsp[-3].id); ;
    break;}
case 44:
#line 112 "parser.y"
{ yyval.stm=Stm_ReglistSet(yyvsp[-1].reglist); ;
    break;}
case 45:
#line 113 "parser.y"
{ yyval.stm=Stm_ReglistGet(yyvsp[-2].reglist); ;
    break;}
case 46:
#line 115 "parser.y"
{ yyval.stm=Stm_Alloc(yyvsp[-4].var, yyvsp[-2].var); ;
    break;}
case 47:
#line 116 "parser.y"
{ yyval.stm=Stm_Realloc(yyvsp[-4].var, yyvsp[-2].var); ;
    break;}
case 48:
#line 117 "parser.y"
{ yyval.stm=Stm_Finalize(yyvsp[-2].var); ;
    break;}
case 49:
#line 118 "parser.y"
{ yyval.stm=Stm_Unfinalize(yyvsp[-2].var); ;
    break;}
case 50:
#line 119 "parser.y"
{ yyval.stm=Stm_Dealloc(yyvsp[-2].var); ;
    break;}
case 51:
#line 121 "parser.y"
{ yyval.stm=StmList_Join(StmList_Join(Stm_ReglistSet(yyvsp[-1].reglist), Stm_Call(yyvsp[-2].id)), Stm_ReglistGet(yyvsp[-4].reglist)); ;
    break;}
case 52:
#line 122 "parser.y"
{ yyval.stm=StmList_Join(Stm_Call(yyvsp[-3].id), Stm_ReglistGet(yyvsp[-5].reglist)); ;
    break;}
case 53:
#line 124 "parser.y"
{ yyval.reglist=Reglist_AddVar(NULL, yyvsp[0].var); ;
    break;}
case 54:
#line 125 "parser.y"
{ yyval.reglist=Reglist_AddVar(yyvsp[-2].reglist, yyvsp[0].var); ;
    break;}
case 55:
#line 127 "parser.y"
{ yyval.reglist=Reglist_AddVar(Reglist_AddVar(NULL, VarStrAddr(yyvsp[0].string)), VarStrLen(yyvsp[0].string)); ;
    break;}
case 56:
#line 128 "parser.y"
{ yyval.reglist=Reglist_AddVar(Reglist_AddVar(yyvsp[-2].reglist, VarStrAddr(yyvsp[0].string)), VarStrLen(yyvsp[0].string)); ;
    break;}
case 57:
#line 130 "parser.y"
{ yyval.reglist=yyvsp[-1].reglist; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 132 "parser.y"
