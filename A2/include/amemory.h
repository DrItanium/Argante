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

#define A2_MEM_READ                1
#define A2_MEM_WRITE               2
#define A2_MEM_MAPPED              4

struct memblk {
  unsigned int mode;		/* Access permissions */
  anyval* memory;		/* Real pointer */
  unsigned int size;		/* Size */
  int destroy_scnum;		/* Syscallnum to do a basic FREE */
  unsigned alib_id;		/* so we can get rid of lib's mem */
};

extern const anyval *mem_ro(struct vcpu *curr_cpu, unsigned addr);
extern anyval *mem_rw(struct vcpu *curr_cpu, unsigned addr);
extern unsigned mem_alloc(struct vcpu *curr_cpu, unsigned size, unsigned flags);
extern unsigned mem_realloc(struct vcpu *curr_cpu, unsigned addr, unsigned newsize);
extern void mem_changeperm(struct vcpu *curr_cpu, unsigned addr, unsigned newflags);
extern void mem_dealloc(struct vcpu *curr_cpu, unsigned addr);
extern int kerntoa_strcpy(struct vcpu *curr_cpu, unsigned addrto, int size, const char *from);

/* No fear, it gets optimized into << and & */
#define dwords_of_bytes(a) ((a / sizeof(anyval)) + ((a % sizeof(anyval)) ? 1 : 0)) 

extern const anyval *mem_ro_block(struct vcpu *curr_cpu, unsigned addr, unsigned dwords);
extern anyval *mem_rw_block(struct vcpu *curr_cpu, unsigned addr, unsigned dwords);

extern int kerntoa_memcpy(struct vcpu *curr_cpu, unsigned addrto, const char *from, int size);
extern int atokern_memcpy(struct vcpu *curr_cpu, char *to, unsigned addrfrom, int size);
#endif /* not _HAVE_MEMORY */
