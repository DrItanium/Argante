/*

   Argante virtual OS release 2
   ----------------------------

   Task manager structures.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

NOTE: This has altered heaps since Michal's original,
and I haven't even tried to tag the changes.
*/

#ifndef _HAVE_TASKMAN
#define _HAVE_TASKMAN 1

#include "config.h"
#include "bformat.h"
#include "anyval.h"
#include "bcode_op.h"
#include <setjmp.h>

/* Ugly but workable.
 * However, you _must_ not leave before except_end... */
#define EXCEPT_CATCH(curr_cpu, exceptid) { \
	jmp_buf _ex_curr, *_ex_last; \
	_ex_last=curr_cpu->onexcept; \
	curr_cpu->onexcept=&_ex_curr; \
	if ((exceptid=setjmp(*curr_cpu->onexcept)) == 0)
#define EXCEPT_END(curr_cpu) curr_cpu->onexcept=_ex_last; \
	}

struct codepage {
	struct bcode_op *bcode;
	unsigned short *jitoffs; /* Dirty speed hack. */
	unsigned int size; /* size of this page only */
	unsigned alib_id; /* basically, dl file # */
};

/* Could someone in-the-know look at reordering this for 100% efficiency?
 * I know the Linux Kernel people have to try to fit related bits of
 * structs within the same page or some cache...? */
struct vcpu {
  unsigned priority;		/* You guessed it */

  unsigned int ip;		/* Instruction pointer */
  unsigned int next_ip;
  unsigned int prev_ip;		/* For debugging */
  
  struct codepage cp_curr;	/* Current code page data */
  unsigned int cp_curr_id;	/* Which page is 'current' */ 
  
  anyval reg[A2_REGISTERS];	/* Registers */
  
  struct memblk (*mem);		/* Memory management struct */
  unsigned int memblks;		/* Number of memory blocks right now. */

  unsigned int *stack;		/* Dynamic stack */
  unsigned int *xstack;		/* Exception handler stack */
  unsigned int sptr;		/* Stack pointer */
  unsigned int stack_size;	/* Size of stack */
  unsigned int xip;		/* IP of current xhandler.
				   (Before any calls happen, there's no xstack) */

  unsigned int mstack;		/* Metastack start */
  unsigned int mstack_ptr;	/* Metastack pointer */
  unsigned int mstack_size;	/* Metastack size */

  jmp_buf *onexcept;		/* Place to go when an exception happens */

  /* IOhandler not needed for MT. And are domain/uid/etc needed when we have per-CPU HAC? */
  unsigned int domain;		/* Execution domain */
  unsigned int uid;		/* Domain UID */
  unsigned int *dlist;		/* List of available domains */
  
  void** reserved;		/* Reserved structures (for module structs)
				   Each module has one and only one entry here, reserved[lid]. */
  unsigned int reserved_ct;	/* It's easier to resize reserved in get_reserved */
  struct vfd *fds;		/* Common Virtual File Descriptors. */
  unsigned int fd_ct;		/* File Descriptor Count */

  struct symhash *st;		/* Symbol table */
  unsigned int syms;		/* Symbol table size */

  struct hactable *hac;		/* hac table */

  struct codepage *cp_all;	/* All code pages */
  unsigned int cp_count;	/* Number of code pages */
  unsigned int alib_max;	/* Next alibid - for dyn link handles */

  /* The boring trivial stuff */
  char pname[A2_MAX_PNAME];	/* Process name */
  unsigned int started_at;	/* time_t */
};

#endif /* not _HAVE_TASKMAN */
