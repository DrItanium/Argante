/* A Bison parser, made from parser.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

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
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		151
#define	YYFLAG		-32768
#define	YYNTBASE	37

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 279 ? yytranslate[x] : 51)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      28,    30,     2,     2,    29,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    34,    26,
       2,    33,     2,     2,    27,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    35,     2,    36,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,    32,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     2,     5,     7,    10,    13,    16,    20,    23,
      26,    29,    33,    36,    42,    48,    54,    60,    66,    73,
      75,    78,    80,    82,    84,    86,    91,    96,    98,   101,
     103,   107,   110,   115,   120,   123,   127,   131,   139,   143,
     147,   150,   154,   158,   163,   166,   170,   178,   186,   192,
     198,   204,   210,   217,   220,   224,   227,   231
};
static const short yyrhs[] =
{
      46,     0,     1,    26,     0,     4,     0,    27,    39,     0,
      39,     8,     0,    28,    39,     0,    41,    29,    39,     0,
      41,    30,     0,    28,    30,     0,    28,    40,     0,    43,
      29,    40,     0,    43,    30,     0,     9,    42,     8,    42,
      26,     0,     9,    42,     8,    44,    26,     0,    10,    42,
       8,    42,    26,     0,    10,    42,     8,    44,    26,     0,
       9,    42,     8,    44,    38,     0,     9,    42,     8,    28,
      30,    38,     0,    38,     0,    46,    38,     0,    19,     0,
      11,     0,     8,     0,     3,     0,    12,    28,     5,    30,
       0,    13,    28,     5,    30,     0,    47,     0,    27,    47,
       0,    45,     0,    31,    46,    32,     0,    40,    26,     0,
      48,    33,    48,    26,     0,    48,     6,    48,    26,     0,
       8,    34,     0,    14,     8,    26,     0,    15,     8,    26,
       0,    16,    48,     7,    48,    14,     8,    26,     0,    18,
      48,    26,     0,    20,    48,    26,     0,    17,    26,     0,
      17,    50,    26,     0,     8,    50,    26,     0,     8,    35,
      36,    26,     0,    50,    26,     0,    50,    33,    26,     0,
      21,    28,    48,    29,    48,    30,    26,     0,    22,    28,
      48,    29,    48,    30,    26,     0,    23,    28,    48,    30,
      26,     0,    24,    28,    48,    30,    26,     0,    25,    28,
      48,    30,    26,     0,    50,    33,     8,    50,    26,     0,
      50,    33,     8,    35,    36,    26,     0,    35,    48,     0,
      49,    29,    48,     0,    35,     5,     0,    49,    29,     5,
       0,    49,    36,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    56,    60,    63,    64,    67,    70,    71,    73,    74,
      77,    78,    80,    83,    84,    85,    86,    87,    88,    91,
      92,    95,    96,    97,    98,    99,   100,   103,   104,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   126,   127,   128,   129,
     130,   133,   135,   138,   139,   140,   141,   145
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "VALUE", "TYPE", "STRING", "AOPER", "CMP", 
  "ID", "FUNC", "SYSCALL", "IGNORE", "STRADDR", "STRLEN", "GOTO", 
  "HANDLER", "IF", "RETURN", "WAIT", "ERRNO", "RAISE", "ALLOC", "REALLOC", 
  "FINALIZE", "UNFINALIZE", "DEALLOC", "';'", "'@'", "'('", "','", "')'", 
  "'{'", "'}'", "'='", "':'", "'['", "']'", "prog", "stm", "type1", 
  "itype", "typelistA", "typelist", "itypelistA", "itypelist", 
  "prototype", "stmlist", "var", "var1", "reglistA", "reglist", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    37,    38,    39,    39,    40,    41,    41,    42,    42,
      43,    43,    44,    45,    45,    45,    45,    45,    45,    46,
      46,    47,    47,    47,    47,    47,    47,    48,    48,    38,
      38,    38,    38,    38,    38,    38,    38,    38,    38,    38,
      38,    38,    38,    38,    38,    38,    38,    38,    38,    38,
      38,    38,    38,    49,    49,    49,    49,    50
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     1,     2,     1,     2,     2,     2,     3,     2,     2,
       2,     3,     2,     5,     5,     5,     5,     5,     6,     1,
       2,     1,     1,     1,     1,     4,     4,     1,     2,     1,
       3,     2,     4,     4,     2,     3,     3,     7,     3,     3,
       2,     3,     3,     4,     2,     3,     7,     7,     5,     5,
       5,     5,     6,     2,     3,     2,     3,     2
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       0,     0,    24,     3,    23,     0,     0,    22,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    19,     0,     0,    29,     0,
      27,     0,     0,     0,     2,    34,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    23,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
      28,     0,    55,    53,     5,    31,    20,     0,     0,     0,
      57,    44,     0,     0,    42,     9,     6,     0,     8,     0,
       0,     0,     0,    35,    36,     0,    41,    38,    39,     0,
       0,     0,     0,     0,    30,     0,     0,    56,    54,     0,
      45,    43,     7,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    33,    32,
       0,     0,     0,     6,    10,    13,     0,    12,    14,    17,
      15,    16,     0,     0,     0,    48,    49,    50,     0,    51,
      18,    11,     0,     0,     0,    52,    37,    46,    47,     0,
       0,     0
};

static const short yydefgoto[] =
{
     149,    25,    26,    27,    39,    40,   105,   106,    28,    29,
      30,    31,    32,    33
};

static const short yypact[] =
{
     223,    -8,-32768,-32768,    -4,    -2,    -2,-32768,    12,    24,
      14,    20,    78,   -10,    78,-32768,    78,    25,    26,    32,
      34,    35,   260,   223,   277,-32768,    57,    40,-32768,   103,
  -32768,     1,   -15,    -6,-32768,-32768,    56,    44,     5,    18,
      63,    64,    69,    71,    51,    52,-32768,   302,    80,-32768,
      59,    67,    68,    78,    78,    78,    78,    78,    11,-32768,
  -32768,   132,-32768,-32768,-32768,-32768,-32768,    78,    78,   289,
  -32768,-32768,     3,    73,-32768,-32768,-32768,    11,-32768,    72,
      81,    65,    99,-32768,-32768,    78,-32768,-32768,-32768,   102,
     108,   109,   128,   130,-32768,    84,   106,-32768,-32768,   126,
  -32768,-32768,-32768,    19,   136,    21,   165,     5,   139,   144,
  -32768,-32768,   157,    78,    78,   146,   167,   168,-32768,-32768,
     248,   173,   194,    57,-32768,-32768,    11,-32768,-32768,-32768,
  -32768,-32768,   193,   192,   198,-32768,-32768,-32768,   197,-32768,
  -32768,-32768,   204,   226,   229,-32768,-32768,-32768,-32768,   249,
     257,-32768
};

static const short yypgoto[] =
{
  -32768,   -24,   -19,   -90,-32768,     0,-32768,   182,-32768,   242,
     -14,   -12,-32768,    -3
};


#define	YYLAST		321


static const short yytable[] =
{
      48,    37,    51,    59,    52,    66,    41,    67,    60,     3,
      50,    99,    63,   124,    69,     3,    49,   124,    34,    76,
      71,    70,    44,     3,    63,    24,    38,    72,    45,   100,
      35,    36,    58,    60,    68,    75,   141,    66,    58,    59,
      42,    89,    90,    91,    92,    93,    58,    77,    78,   122,
     126,   127,    43,    53,    54,    95,    96,    98,   102,     2,
      55,    62,    56,    57,    46,    64,    65,     7,     8,     9,
      74,    79,    80,   112,    81,    15,    82,    83,    84,   104,
     108,     2,   129,    47,   123,    86,    46,    85,   123,     7,
       8,     9,    73,    87,    88,   110,   121,    15,   140,   101,
     103,   133,   134,    -1,     1,    47,     2,     3,    63,   107,
     118,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,   111,
      22,   113,   119,     1,    23,     2,     3,   114,    24,   115,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,   116,    22,
     117,   120,   125,    23,    94,   130,     1,    24,     2,     3,
     131,   132,   135,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,   128,    22,   136,   137,     1,    23,     2,     3,   139,
      24,   142,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -9,    22,   143,   145,     1,    23,     2,     3,   144,    24,
     146,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,   150,
      22,     2,   147,    62,    23,   148,    46,   151,    24,     7,
       8,     9,   109,     2,     3,    61,     0,    15,    46,     0,
       0,     7,     8,     9,     0,    47,     0,     0,     0,    15,
       2,     0,    62,     0,   138,    46,     0,    58,     7,     8,
       9,     0,     2,     0,    97,     0,    15,    46,     0,     0,
       7,     8,     9,     0,    47,     2,     0,     0,    15,     0,
      46,     0,     0,     7,     8,     9,    47,     0,     0,     0,
       0,    15
};

static const short yycheck[] =
{
      12,     4,    14,    22,    16,    29,     6,     6,    22,     4,
      13,     8,    24,   103,    29,     4,    26,   107,    26,    38,
      26,    36,     8,     4,    36,    35,    28,    33,     8,    26,
      34,    35,    27,    47,    33,    30,   126,    61,    27,    58,
      28,    53,    54,    55,    56,    57,    27,    29,    30,    30,
      29,    30,    28,    28,    28,    67,    68,    69,    77,     3,
      28,     5,    28,    28,     8,     8,    26,    11,    12,    13,
      26,     8,     8,    85,     5,    19,     5,    26,    26,    79,
      80,     3,   106,    27,   103,    26,     8,     7,   107,    11,
      12,    13,    36,    26,    26,    30,    99,    19,   122,    26,
      28,   113,   114,     0,     1,    27,     3,     4,   120,    28,
      26,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    30,
      27,    29,    26,     1,    31,     3,     4,    29,    35,    30,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    30,    27,
      30,    35,    26,    31,    32,    26,     1,    35,     3,     4,
      26,    14,    26,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    26,    26,     1,    31,     3,     4,    26,
      35,     8,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    30,    26,     1,    31,     3,     4,    30,    35,
      26,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      27,     3,    26,     5,    31,    26,     8,     0,    35,    11,
      12,    13,    80,     3,     4,    23,    -1,    19,     8,    -1,
      -1,    11,    12,    13,    -1,    27,    -1,    -1,    -1,    19,
       3,    -1,     5,    -1,    36,     8,    -1,    27,    11,    12,
      13,    -1,     3,    -1,     5,    -1,    19,     8,    -1,    -1,
      11,    12,    13,    -1,    27,     3,    -1,    -1,    19,    -1,
       8,    -1,    -1,    11,    12,    13,    27,    -1,    -1,    -1,
      -1,    19
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
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
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
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
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 56 "parser.y"
{ yyval.stm=prog=yyvsp[0].stm; Function(NULL, NULL, NULL, yyvsp[0].stm); ;
    break;}
case 2:
#line 60 "parser.y"
{ yyval.stm=NULL; yyerrok; ;
    break;}
case 3:
#line 63 "parser.y"
{ yyval.type=yyvsp[0].type; ;
    break;}
case 4:
#line 64 "parser.y"
{ yyvsp[0].type->pointerct++; yyval.type=yyvsp[0].type; ;
    break;}
case 5:
#line 67 "parser.y"
{ yyval.itype=IType(yyvsp[0].id, yyvsp[-1].type); ;
    break;}
case 6:
#line 70 "parser.y"
{ yyval.typelist=TypeList_AddType(NULL, yyvsp[0].type); ;
    break;}
case 7:
#line 71 "parser.y"
{ yyval.typelist=TypeList_AddType(yyvsp[-2].typelist, yyvsp[0].type); ;
    break;}
case 8:
#line 73 "parser.y"
{ yyval.typelist=yyvsp[-1].typelist; ;
    break;}
case 9:
#line 74 "parser.y"
{ yyval.typelist=NULL; ;
    break;}
case 10:
#line 77 "parser.y"
{ yyval.itypelist=ITypeList_AddType(NULL, yyvsp[0].itype); ;
    break;}
case 11:
#line 78 "parser.y"
{ yyval.itypelist=ITypeList_AddType(yyvsp[-2].itypelist, yyvsp[0].itype); ;
    break;}
case 12:
#line 80 "parser.y"
{ yyval.itypelist=yyvsp[-1].itypelist; ;
    break;}
case 13:
#line 83 "parser.y"
{ PrototypeT(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].typelist); yyval.stm=NULL; ;
    break;}
case 14:
#line 84 "parser.y"
{ PrototypeI(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].itypelist); yyval.stm=NULL; ;
    break;}
case 15:
#line 85 "parser.y"
{ PrototypeSCT(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].typelist); yyval.stm=NULL; ;
    break;}
case 16:
#line 86 "parser.y"
{ PrototypeSCI(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].itypelist); yyval.stm=NULL; ;
    break;}
case 17:
#line 87 "parser.y"
{ Function(yyvsp[-2].id, yyvsp[-3].typelist, yyvsp[-1].itypelist, yyvsp[0].stm); yyval.stm=NULL; ;
    break;}
case 18:
#line 88 "parser.y"
{ Function(yyvsp[-3].id, yyvsp[-4].typelist, NULL, yyvsp[0].stm); yyval.stm=NULL; ;
    break;}
case 19:
#line 91 "parser.y"
{ yyval.stm=yyvsp[0].stm; ;
    break;}
case 20:
#line 92 "parser.y"
{ yyval.stm=StmList_Join(yyvsp[-1].stm, yyvsp[0].stm); ;
    break;}
case 21:
#line 95 "parser.y"
{ yyval.var=Var_Errno(); ;
    break;}
case 22:
#line 96 "parser.y"
{ yyval.var=Var_Ignore(); ;
    break;}
case 23:
#line 97 "parser.y"
{ yyval.var=Var_Register(yyvsp[0].id); ;
    break;}
case 24:
#line 98 "parser.y"
{ yyval.var=yyvsp[0].var; ;
    break;}
case 25:
#line 99 "parser.y"
{ yyval.var=VarStrAddr(yyvsp[-1].string); ;
    break;}
case 26:
#line 100 "parser.y"
{ yyval.var=VarStrLen(yyvsp[-1].string); ;
    break;}
case 27:
#line 103 "parser.y"
{ yyval.var=yyvsp[0].var; ;
    break;}
case 28:
#line 104 "parser.y"
{ yyvsp[0].var->ptrderef=1; yyval.var=yyvsp[0].var; ;
    break;}
case 29:
#line 107 "parser.y"
{ yyval.stm=NULL; ;
    break;}
case 30:
#line 108 "parser.y"
{ yyval.stm=yyvsp[-1].stm; ;
    break;}
case 31:
#line 109 "parser.y"
{ yyval.stm=Stm_VarDef(yyvsp[-1].itype);  /* free($1); ? */  ;
    break;}
case 32:
#line 110 "parser.y"
{ yyval.stm=Stm_Assign(yyvsp[-3].var, AEq, yyvsp[-1].var); ;
    break;}
case 33:
#line 111 "parser.y"
{ yyval.stm=Stm_Assign(yyvsp[-3].var, yyvsp[-2].optype, yyvsp[-1].var); ;
    break;}
case 34:
#line 112 "parser.y"
{ yyval.stm=Stm_Label(yyvsp[-1].id); ;
    break;}
case 35:
#line 113 "parser.y"
{ yyval.stm=Stm_Goto(yyvsp[-1].id); ;
    break;}
case 36:
#line 114 "parser.y"
{ yyval.stm=Stm_Handler(yyvsp[-1].id); ;
    break;}
case 37:
#line 115 "parser.y"
{ yyval.stm=Stm_If(yyvsp[-5].var, yyvsp[-4].cmptype, yyvsp[-3].var, yyvsp[-1].id); ;
    break;}
case 38:
#line 116 "parser.y"
{ yyval.stm=Stm_Wait(yyvsp[-1].var); ;
    break;}
case 39:
#line 117 "parser.y"
{ yyval.stm=Stm_Raise(yyvsp[-1].var); ;
    break;}
case 40:
#line 118 "parser.y"
{ yyval.stm=Stm_Return(); ;
    break;}
case 41:
#line 119 "parser.y"
{ yyval.stm=StmList_Join(Stm_ReglistSet(yyvsp[-1].reglist), Stm_Return()); ;
    break;}
case 42:
#line 120 "parser.y"
{ yyval.stm=StmList_Join(Stm_ReglistSet(yyvsp[-1].reglist), Stm_Call(yyvsp[-2].id)); ;
    break;}
case 43:
#line 121 "parser.y"
{ yyval.stm=Stm_Call(yyvsp[-3].id); ;
    break;}
case 44:
#line 122 "parser.y"
{ yyval.stm=Stm_ReglistSet(yyvsp[-1].reglist); ;
    break;}
case 45:
#line 123 "parser.y"
{ yyval.stm=Stm_ReglistGet(yyvsp[-2].reglist); ;
    break;}
case 46:
#line 126 "parser.y"
{ yyval.stm=Stm_Alloc(yyvsp[-4].var, yyvsp[-2].var); ;
    break;}
case 47:
#line 127 "parser.y"
{ yyval.stm=Stm_Realloc(yyvsp[-4].var, yyvsp[-2].var); ;
    break;}
case 48:
#line 128 "parser.y"
{ yyval.stm=Stm_Finalize(yyvsp[-2].var); ;
    break;}
case 49:
#line 129 "parser.y"
{ yyval.stm=Stm_Unfinalize(yyvsp[-2].var); ;
    break;}
case 50:
#line 130 "parser.y"
{ yyval.stm=Stm_Dealloc(yyvsp[-2].var); ;
    break;}
case 51:
#line 133 "parser.y"
{ yyval.stm=StmList_Join(
		StmList_Join(Stm_ReglistSet(yyvsp[-1].reglist), Stm_Call(yyvsp[-2].id)), Stm_ReglistGet(yyvsp[-4].reglist) ); ;
    break;}
case 52:
#line 135 "parser.y"
{ yyval.stm=StmList_Join(Stm_Call(yyvsp[-3].id), Stm_ReglistGet(yyvsp[-5].reglist)); ;
    break;}
case 53:
#line 138 "parser.y"
{ yyval.reglist=Reglist_AddVar(NULL, yyvsp[0].var); ;
    break;}
case 54:
#line 139 "parser.y"
{ yyval.reglist=Reglist_AddVar(yyvsp[-2].reglist, yyvsp[0].var); ;
    break;}
case 55:
#line 140 "parser.y"
{ yyval.reglist=Reglist_AddVar(Reglist_AddVar(NULL, VarStrAddr(yyvsp[0].string)), VarStrLen(yyvsp[0].string)); ;
    break;}
case 56:
#line 141 "parser.y"
{ yyval.reglist=Reglist_AddVar(
		Reglist_AddVar(yyvsp[-2].reglist, VarStrAddr(yyvsp[0].string)), VarStrLen(yyvsp[0].string) ); ;
    break;}
case 57:
#line 145 "parser.y"
{ yyval.reglist=yyvsp[-1].reglist; ;
    break;}
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
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

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 148 "parser.y"
