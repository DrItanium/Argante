/*

   Argante virtual OS release 2
   ----------------------------

   Binary image format, CHANGED BY JAMES KEHL

   Status: AWAITING COMMENT

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#ifndef _HAVE_BFORMAT
#define _HAVE_BFORMAT 1

#include "config.h"

#define BMAGIC		0xdeadbea7 /* Something a little different from v1, thanks ****CHANGED****/

// Binary image format:
// message_desc (must start with BFMT_PROGSPEC)
// message_data
// message_desc
// message_data


#define BFMT_PROGSPEC		0	// struct progspec
#define BFMT_CODE		1	// code segment (once!) **** see bcode_op.h for details
#define BFMT_DATA		2	// data segment (numerous) **** see data_blk.h for details
#define BFMT_RODATA		3	// read-only data
#define BFMT_SYM		4	// struct symbol
#define BFMT_RELOC		5	/****NEW****/
#define BFMT_EOF		100	// end of file (terminator)


struct message_desc {
  unsigned int type;		// BFMT_...
  unsigned int size;		// Eventual payload size
};

struct progspec {
  unsigned int magic;		// BMAGIC
  char name[MAX_PNAME];		// Program name
  unsigned int domains[MAX_DOMAINS];	// 0-terminated list
  unsigned int init_domain;		// 0 = none
  unsigned int priority;		// 1 - MAXPRIORITY
};

#define SYM_CODE                1 /**** CHANGED ****/
#define SYM_DATA                2
/****NEW****/
#define SYM_PLACEMASK		(SYM_CODE | SYM_DATA)
/* These are bitflags for the place field, |'d with SYM_CODE or _DATA */
#define SYM_UNDEFINED		0100	// Something to have its refs fixed by the linker
#define SYM_UNNAMED		0200	// Something just for the debugger, the linker

struct stable {
  unsigned int addr;		// Address
  unsigned char place;		// code or data?
  unsigned int size;		/**** NEW ****/
  char fn[MAX_SNAME];		// Name
} __attribute__ ((packed));

/* Note that reloc entries always come just after their respective symbol. */
#define RELOC_ADDR		1
#define RELOC_SIZE_DWORD	2
#define RELOC_SIZE_BYTE		3

struct reloc {
	unsigned int addr;
	unsigned char place; /* Is place in code or data? */
	unsigned char type; /* What sort of reference - RELOC_ADDR etc. */
} __attribute__ ((packed));
	
#endif /* not _HAVE_BFORMAT */
