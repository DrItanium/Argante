/*

   Argante virtual OS release 2
   ----------------------------

   Memory management definitions

   Status: under development

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#ifndef _HAVE_MEMORY
#define _HAVE_MEMORY 1

#include "config.h"
#include "bcode_op.h"

#define MEM_READ                1
#define MEM_WRITE               2
#define MEM_MAPPED              4

struct memblk {
  unsigned int mode;		// Access permissions
  anyval* memory;	// Real pointer
  unsigned int size;		// Size
  void (*destroy)(unsigned int blknum);	// MAPPED destroy handler
};

extern anyval *mem_ro(struct vcpu *curr_cpu, unsigned addr);
extern anyval *mem_rw(struct vcpu *curr_cpu, unsigned addr);
extern int mem_alloc(struct vcpu *curr_cpu, unsigned size, unsigned flags);
extern void mem_realloc(struct vcpu *curr_cpu, unsigned addr, unsigned newsize);
extern void mem_changeperm(struct vcpu *curr_cpu, unsigned addr, unsigned newflags);
extern void mem_dealloc(struct vcpu *curr_cpu, unsigned addr);

#endif /* not _HAVE_MEMORY */
