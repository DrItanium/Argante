/*

   Argante virtual OS
   ------------------

   Dynamic linker message structures.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#define MAX_LIBNAM 32
#define MAX_FNAM   32

#define SYM_CODE   0
#define SYM_DATA   1

#define MTYPE_1	   1
#define MTYPE_2	   2
#define MTYPE_3	   3

#define LINK_MAGIC	0xdadc0ffe

struct message_1 {
  unsigned int offset;
  char place;
};


struct message_2 {
  unsigned int offset;
  char place;
  char lib[MAX_LIBNAM];
  char fn[MAX_FNAM];
};


struct message_3 {
  unsigned int offset;
  char place;
  char fn[MAX_FNAM];
};


struct symtab {
  unsigned int addr;
  char place;
  char lib[MAX_LIBNAM];
  char fn[MAX_FNAM];
};


struct link_entry {
  unsigned int magic;
  int mtype;
  union {
    struct message_1 m1;
    struct message_2 m2;
    struct message_3 m3;
  };
};
