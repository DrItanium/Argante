/*

   Argante virtual OS
   ------------------

   Language structures for AHLL.

   Status: partially done, under development

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#define STATE_NONE   0 // In root
#define STATE_CODE   1 // In normal code
#define STATE_FNDEF  2 // In function params definition
#define STATE_LDEF   3 // In local {}
#define STATE_LNEST 10 // In local {}, parsing struct / array init
#define STATE_EXCEPT 4 // In exception {}
#define STATE_STRUCT 5 // In structure type
#define STATE_INIT   6 // Before LDEF or CODE
#define STATE_FNPAR  7 // In function params description
#define STATE_ASSIGN 8 // In structural/array assignment
#define STATE_GDEF   9 // In structural/array initializer
#define STATE_SYSCALL 11 // Syscall parameter parsing

#define TYPE_UNSIG   1
#define TYPE_SIGNED  2
#define TYPE_FLOAT   3
#define TYPE_BCHUNK  4
#define TYPE_STRUCT  5
#define TYPE_ARRAY   6
#define TYPE_IPADDR  7

#define TYPE_UNSIGNED TYPE_UNSIG

#define V_NORMAL      0
#define V_ADDRESSABLE 1
#define V_POINTER     2

#define P_NORMAL      0
#define P_WRITABLE    1

#define P_LOOP        0
#define P_WHILE       5
#define P_IF          1
#define P_CASE        2
#define P_GUARD       3

#define CTYPE_SIMPLE  0
#define CTYPE_NOT     1
#define CTYPE_EQ      2
#define CTYPE_NEQ     3
#define CTYPE_BELOW   4
#define CTYPE_ABOVE   5

struct typedesc {
  char name[MAX_HLL_NAME];
  char type;
  int  a_start;
  int  a_end;
  int  a_type;
  char a_simple;
  int  s_fields;
  char s_field[MAX_STRFIELDS][MAX_HLL_NAME];
  int  s_type[MAX_STRFIELDS];
  char s_flag[MAX_STRFIELDS];
  char issim[MAX_STRFIELDS];
  int  btype_comp;
  int  size;
};


struct symdesc {
  char name[MAX_HLL_NAME];
  char flag;
  char issim;
  char* in;
  int isize;
  int  type;
};


struct fndesc {
  char name[MAX_HLL_NAME];
  int  params;
  char s_name[REGISTERS][MAX_HLL_NAME];
  int  s_type[REGISTERS];
  char s_issim[REGISTERS];
  char writ[REGISTERS];
};


struct blockdesc {
  int nest;
  int no;
  int type;
  char* checkc;
};

