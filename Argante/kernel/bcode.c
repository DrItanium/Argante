/*

   Argante virtual OS
   ------------------

   Binary code interpreter, memory management, exception handling.

   Status: done, JIT's mainly DONE!

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

   Several bugfixes suggested by Bulba were done.

   Heavy optimalization/JITS: Mariusz Woloszyn (Kil3r) <kil3r@dione.ids.pl>

*/

#define __HAVE_MY_OWN_MEMORY_MANAGEMENT

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "console.h"
#include "memory.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "bcode.h"
#include "module.h"
#include "debugger.h"
#include "cmd.h"

/* bswap macros */
#ifndef NO_SWAP_ENDIAN
#include <byteswap.h>
#endif

/* Is defined task.c */
extern int curr_cpu;
extern struct vcpu_struct *curr_cpu_p;
/* Is defined here */
char *bytecode_p;

void * JIT [(1+CMD_INVALID)*36];

#define JITS(cmd,t1,t2)	JIT[cmd*36+(t1+1)+6*(t2)]

extern void	(*task_ripc_handler)(void);

int but_in_fact_it_was_fatal=0;
int got_nonfatal_round=0;

// For memory and stack management, do not abuse!
#define CHECK_FAILURE     if (but_in_fact_it_was_fatal) break;
#define CHECK_FAILURE_FN  if (but_in_fact_it_was_fatal) return;
#define CHECK_FAILURE_FN2 if (but_in_fact_it_was_fatal) return 0;

// For command-switch
#define CHECK_EXCEPT_INSWITCH  if (got_nonfatal_round) break;
#define CHECK_EXCEPT_FN        if (got_nonfatal_round) return;
#define CHECK_EXCEPT_FN2       if (got_nonfatal_round) return 0;

void non_fatal(int,char*,int);
extern int failure;

/* Pushes the current VCPU's IP onto the top of it's call stack. 
   Note: This function works on the currently active VCPU (ie. the 
   one stored in curr_cpu_p and curr_cpu)! This is for speed reasons. */

inline void push_ip_on_stack() {
    CHECK_FAILURE_FN;

    if( curr_cpu_p->stack_ptr >= MAX_STACK ) {
        non_fatal( ERROR_STACK_OVER, "Cannot push - stack overflow", curr_cpu );
        return;
    }
  
    if( curr_cpu_p->stack_ptr >= curr_cpu_p->dssiz ) {
        void* q;
        curr_cpu_p->dssiz += STACK_GROW;
        q = realloc( curr_cpu_p->stack, 4 * curr_cpu_p->dssiz );
        
        if( !q ) {
            non_fatal( ERROR_STACK_OVER, "Cannot resize stack", curr_cpu );
            return;
        }
        
        curr_cpu_p->stack = q;
        q = realloc( curr_cpu_p->ex_st, 4 * (curr_cpu_p->dssiz + 1) );
        
        if( !q ) {
            non_fatal( ERROR_STACK_OVER, "Cannot resize exception stack", curr_cpu );
            return;
        }
        curr_cpu_p->ex_st = q;
    }
  
  (*curr_cpu_p->stack)[curr_cpu_p->stack_ptr++]=curr_cpu_p->IP;
  (*curr_cpu_p->ex_st)[curr_cpu_p->stack_ptr]=0;
}

/* Pops the current VCPU's IP from the top of it's call stack. 
   Note: This function works on the currently active VCPU (ie. the 
   one stored in curr_cpu_p and curr_cpu)! This is for speed reasons. */

inline void pop_ip_from_stack() {
    CHECK_FAILURE_FN;

    if( curr_cpu_p->stack_ptr > 0 ) {
        curr_cpu_p->IP = (*curr_cpu_p->stack)[--curr_cpu_p->stack_ptr]; 
    } else {
      non_fatal( ERROR_STACK_UNDER, "Attempt to pop from empty stack", curr_cpu );
      return;
    }

    if( curr_cpu_p->saved_ex ) {
        if( curr_cpu_p->stack_ptr == curr_cpu_p->ex_at ) {
          // printk("RET - Restoring saved_ex %d\n",curr_cpu_p->saved_ex);
          (*curr_cpu_p->ex_st)[curr_cpu_p->stack_ptr]=curr_cpu_p->saved_ex;
          curr_cpu_p->saved_ex = 0;
        }
    }
}

inline void set_mem_value(int c,unsigned int addr,int value);
inline int get_mem_value(int c,unsigned int addr);

inline int get_mem_value(int c,unsigned int addr) {
  int x;
  CHECK_EXCEPT_FN2;
  x=addr/(2+MAX_ALLOC_MEMBLK);

  if (x>=cpu[c].dmsiz) {
    non_fatal(ERROR_OUTSIDE_MEM,"Attempt to access excessive memblk ID",c);
    return 0;
  }

    if ((*cpu[c].mem)[x].real_memory)
      if (((*cpu[c].mem)[x].map_addr<=addr) && 
          ((*cpu[c].mem)[x].map_addr+(*cpu[c].mem)[x].size>addr)) {
            if (!( (*cpu[c].mem)[x].flags & MEM_FLAG_READ )) {
              non_fatal(ERROR_PROTFAULT,"Attempt to read protected memory",c);
              return 0;
            }
	    /* JK - support swap-endian .data seg */
#ifndef NO_SWAP_ENDIAN
	    {
		    int val;
		    val=(*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr];
		    if ((*cpu[c].mem)[x].flags & MEM_FLAG_SWAPENDIAN)
			    return bswap_32(val);
		    else
			    return val;
	    }
#else
            return (*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr];
#endif
          }

  non_fatal(ERROR_OUTSIDE_MEM,"Attempt to access non-allocated memory",c);
  return 0;
}


inline void get_mem_block(int c,char* dest,unsigned int addr, unsigned int cnt) {
  int x;
  // a flagi?

  CHECK_FAILURE_FN;

  x=addr/(2+MAX_ALLOC_MEMBLK);

  if (x>=cpu[c].dmsiz) {
    non_fatal(ERROR_OUTSIDE_MEM,"Attempt to access excessive memblk ID",c);
    return;
  }

    if ((*cpu[c].mem)[x].real_memory)
      if (((*cpu[c].mem)[x].map_addr<=addr) && 
          ((*cpu[c].mem)[x].map_addr+(*cpu[c].mem)[x].size>addr+cnt-1)) {
            if (!( (*cpu[c].mem)[x].flags & MEM_FLAG_READ )) {
              non_fatal(ERROR_PROTFAULT,"Attempt to read protected memory",c);
              return;
            }
#ifdef NO_SWAP_ENDIAN
            memcpy(dest,&(*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr],cnt*4);
#else
	    if ((*cpu[c].mem)[x].flags & MEM_FLAG_SWAPENDIAN)
	    {
		    int *d, *s;
		    d=(int *) dest;
		    s=(int *) &(*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr];
		    while(cnt > 0)
		    {
			    *d=bswap_32(*s);
			    cnt--; d++; s++;
		    }
	    } else
	            memcpy(dest,&(*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr],cnt*4);
#endif
            return;
          }
  non_fatal(ERROR_OUTSIDE_MEM,"Attempt to access non-allocated memory",c);
  return;
}


inline void set_mem_block(int c,char* src,unsigned int addr, unsigned int cnt) {
  int x;
  // a flagi?

  CHECK_EXCEPT_FN;

  x=addr/(2+MAX_ALLOC_MEMBLK);

  if (x>=cpu[c].dmsiz) {
    non_fatal(ERROR_OUTSIDE_MEM,"Attempt to access excessive memblk ID",c);
    return;
  }

    if ((*cpu[c].mem)[x].real_memory)
      if (((*cpu[c].mem)[x].map_addr<=addr) && 
          ((*cpu[c].mem)[x].map_addr+(*cpu[c].mem)[x].size>addr+cnt-1)) {
            if (!( (*cpu[c].mem)[x].flags & MEM_FLAG_WRITE )) {
              non_fatal(ERROR_PROTFAULT,"Attempt to read protected memory",c);
              return;
            }
#ifdef NO_SWAP_ENDIAN
            memcpy(&(*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr],src,cnt*4);
#else
	    if ((*cpu[c].mem)[x].flags & MEM_FLAG_SWAPENDIAN)
	    {
		    int *d, *s;
		    s=(int *) src;
		    d=(int *) &(*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr];
		    while(cnt > 0)
		    {
			    *d=bswap_32(*s);
			    cnt--; d++; s++;
		    }
	    } else
		    memcpy(&(*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr],src,cnt*4);
#endif

            return;
          }
  non_fatal(ERROR_OUTSIDE_MEM,"Attempt to access non-allocated memory",c);
  return;
}


inline void* verify_access(int c,unsigned int addr,unsigned int cnt,unsigned int fl) {
  int x;
  // a flagi?

  CHECK_FAILURE_FN2;

  x=addr/(2+MAX_ALLOC_MEMBLK);
  if (x>=cpu[c].dmsiz) return 0;

    if ((*cpu[c].mem)[x].real_memory)
      if (((*cpu[c].mem)[x].map_addr<=addr) &&
          ((*cpu[c].mem)[x].map_addr+(*cpu[c].mem)[x].size>addr+cnt-1)) {
            if (( (*cpu[c].mem)[x].flags & fl )!=fl) {
              return 0;
            }
            return &(*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr];
          }
  return 0;
}





inline int mem_alloc(int c,unsigned int size,unsigned int flags) {
  int x;

  CHECK_EXCEPT_FN2;

  if (size>MAX_ALLOC_MEMBLK || size<=0) {
    non_fatal(ERROR_TOOBIG,"Excessive alloc attempt",c);
    return -1;
  }
  
  if (flags & ~(MEM_FLAG_READ|MEM_FLAG_WRITE|MEM_FLAG_STRICT)) {
    non_fatal(ERROR_BAD_PARAM,"Unacceptable alloc flags",c);
    return -1;
  }

  for (x=0;x<MAX_MEMBLK;x++) {

    if (x>=cpu[c].dmsiz) {
      void *q;
      int old=cpu[c].dmsiz;
      cpu[c].dmsiz+=MEM_GROW;
      q=realloc(cpu[c].mem,sizeof(struct memarea)*cpu[c].dmsiz);
      if (!q) {
        non_fatal(ERROR_NOMEM,"No space for memarea grow",c);
	return -1;
      }
      cpu[c].mem=q;
      // Be sure to fill new chunks with zeros...
      memset(&cpu[c].mem[old],0,sizeof(struct memarea)*MEM_GROW);
    }
      
    if (!(*cpu[c].mem)[x].real_memory) {
      (*cpu[c].mem)[x].real_memory=malloc(size*4);

      if (!(*cpu[c].mem)[x].real_memory) {
        non_fatal(ERROR_NOMEM,"No space for block alloc",c);
        return -1;
      }
      bzero((*cpu[c].mem)[x].real_memory,size*4);
      (*cpu[c].mem)[x].flags=flags;
      (*cpu[c].mem)[x].size=size;
      (*cpu[c].mem)[x].map_addr=x*(2+MAX_ALLOC_MEMBLK);
      used_memory+=(*cpu[c].mem)[x].size;
      used_memblocks++;
      return x;
    }
  }

  non_fatal(ERROR_NOMEM,"No space for block alloc",c);

  return -1;
}


inline void mem_dealloc(int c,int h) {

  CHECK_EXCEPT_FN;

  if (h<0 || h>=cpu[c].dmsiz) {
    non_fatal(ERROR_BAD_PARAM,"Excessive memblock identifier",c);
    return;
  }
  if ((*cpu[c].mem)[h].real_memory) {
    if (!((*cpu[c].mem)[h].flags & MEM_FLAG_MAPPED)) 
      free((*cpu[c].mem)[h].real_memory);
    (*cpu[c].mem)[h].real_memory=0;
    used_memory-=(*cpu[c].mem)[h].size;
    used_memblocks--;
    return;
  }
  non_fatal(ERROR_PROTFAULT,"Dealloc on free block",c);
}


inline void mem_realloc(int c,int h,int newsize) {

  int flags=cpu[c].uregs[0];
  void* x;
  CHECK_EXCEPT_FN;
  
  if (h<0 || h>=cpu[c].dmsiz) {
    non_fatal(ERROR_BAD_PARAM,"Excessive memblock identifier",c);
    return;
  }

  if (newsize>MAX_ALLOC_MEMBLK || newsize<0) {
    non_fatal(ERROR_TOOBIG,"Excessive realloc attempt",c);
    return;
  }

  if ((*cpu[c].mem)[h].real_memory) {

    if (newsize) {
    
      if ((*cpu[c].mem)[h].flags & MEM_FLAG_MAPPED) {
        non_fatal(ERROR_BAD_PARAM,"Cannot resize directly mapped memory",c);
        return;
      }
    
    
      if (!(x=realloc((*cpu[c].mem)[h].real_memory,newsize))) {
        non_fatal(ERROR_NOMEM,"No space for block realloc",c);
        return;
      }
      (*cpu[c].mem)[h].real_memory=x;
      used_memory-=(*cpu[c].mem)[h].size;
      used_memory+=newsize;
      (*cpu[c].mem)[h].size=newsize;
      
    } else {
    
      if (flags & ~(MEM_FLAG_READ|MEM_FLAG_WRITE|MEM_FLAG_STRICT)) {
        non_fatal(ERROR_BAD_PARAM,"Unacceptable realloc flags",c);
        return;
      }	      
      
      if ((*cpu[c].mem)[h].flags & MEM_FLAG_STRICT) {
        non_fatal(ERROR_PROTFAULT,"Cannot modify permissions (MEM_FLAG_STRICT).",c);
        return;
      }	      
      
      (*cpu[c].mem)[h].flags=flags;

    }

    return;
  }
  non_fatal(ERROR_PROTFAULT,"Realloc on free block",c);
}




inline void set_mem_value(int c,unsigned int addr,int value) {
  int x;
  // a flagi?

  CHECK_EXCEPT_FN;

  x=addr/(2+MAX_ALLOC_MEMBLK);
  if (x>=cpu[c].dmsiz) {
    non_fatal(ERROR_OUTSIDE_MEM,"Attempt to access excessive memblk ID",c);
    return;
  }

    if ((*cpu[c].mem)[x].real_memory)
      if (((*cpu[c].mem)[x].map_addr<=addr) && 
          ((*cpu[c].mem)[x].map_addr+(*cpu[c].mem)[x].size>addr)) {
             if (!( (*cpu[c].mem)[x].flags & MEM_FLAG_WRITE )) {
               non_fatal(ERROR_PROTFAULT,"Attempt to write protected memory",c);
               return;
             }
#ifndef NO_SWAP_ENDIAN
	    {
		    int *dest;
		    dest=&(*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr];
		    if ((*cpu[c].mem)[x].flags & MEM_FLAG_SWAPENDIAN)
			    *dest=bswap_32(value);
		    else
			    *dest=value;
	    }
#else
            (*cpu[c].mem)[x].real_memory[addr-(*cpu[c].mem)[x].map_addr]=value;
#endif

             return;
          }
  non_fatal(ERROR_OUTSIDE_MEM,"Attempt to access non-allocated memory",c);
  return;
}

int change;


// Main exception handler - check if exception can be handled,
// call handler or terminate VCPU.

int nofatalinside;

void non_fatal(int code,char* desc,int c) {
  int i,x;
  int orig_ip;

  CHECK_FAILURE_FN;

  failure=got_nonfatal_round=1;

  orig_ip=cpu[c].IP;
  x = cpu[c].stack_ptr;

  for (;x>=0;x--) {
    if( (*cpu[c].ex_st)[x] ) break;
    if( x > 0 ) {
        // WARNING: possible problem if c != curr_cpu !!  
        pop_ip_from_stack();
    } // Kosmos ;>
  }

  // zed's changes
  if (cpu[c].flags & VCPU_FLAG_DEBUG){
    if (!(cpu[c].flags & VCPU_FLAG_USED)) return; // Dud.
    printk("Debugging trap.\n");
    printk("Non-fatal exception %d - %s.\n",code,desc);
    printk("%d vCPU is stopped\n",c);
    cpu[c].state|=VCPU_STATE_STOPPED;
    if ((x<0) || !(*cpu[c].ex_st)[x]){
      printk("Process died.\n");
      nonfatal_errors++;
      died_proc++;
      killed_proc--;
      debug[c].traceflag|=DEBUG_DEAD; // this process is dead
      // but it's still in memory
      return;
    }
  }
  // end of zed's changes
  
  if ((!nofatalinside) && x>=0 && (*cpu[c].ex_st)[x]) {

    if (!cpu[c].in_handler) cpu[c].u0_saved=cpu[c].uregs[0];
    cpu[c].in_handler++;
    nofatalinside=1;
    printk("=> VCPU #%d handling X%d h%d A%d->%d (%s).\n",c,code,(*cpu[c].ex_st)[x],orig_ip,cpu[c].IP,desc);
    if (!cpu[c].handling_failure) {
      cpu[c].handling_failure=code;
      cpu[c].first_except_ip=orig_ip;
    }
    cpu[c].uregs[0]=code;
    cpu[c].saved_ex=(*cpu[c].ex_st)[x];
    cpu[c].ex_at=cpu[c].stack_ptr;
    (*cpu[c].ex_st)[x]=0;
    push_ip_on_stack();
    nofatalinside=0;
    cpu[c].IP=cpu[c].saved_ex; 
    change=0;
    nonfatal_errors++;
    handled_errors++;
  } else {
    printk("-> VCPU #%d (%s): Unhandled non-fatal exception %d.\n"
           "-> Exception description: %s.\n",c,cpu[c].name,
           code,desc);
    printk("-> Instruction pointer: 0x%x flags=0x%x, state=0x%x\n",
           orig_ip,cpu[c].flags,cpu[c].state);

    printk("-> Stack trace: ");
    for (i=0;i<cpu[c].stack_ptr;i++) 
      if (i<cpu[c].dssiz) printk("0x%x %s",(*cpu[c].stack)[i],
        ((*cpu[c].stack)[i]==cpu[c].first_except_ip)?"(first exception) ":"");
    printk("<<IP:0x%x>>\n",cpu[c].IP); 

    but_in_fact_it_was_fatal=1;

    if (cpu[c].handling_failure) {
      printk("-> NOTE: exception raised while handling exception #%d\n",cpu[c].handling_failure);
      printk("-> First exception at address: 0x%x\n",cpu[c].first_except_ip);
      printk("\n"); 
    }
    destroy_task(c);
    died_proc++;
    killed_proc--;
    nonfatal_errors++;
  }

}


// Bytecode interpreter. It's not so ugly any more.
extern int curr_cpu;
extern struct vcpu_struct *curr_cpu_p;

void do_cycles(int cnt) {
  int L;
  if (cnt==0) cnt=1;
  for (L=0;L<cnt;L++) {

    if (task_ripc_handler) task_ripc_handler(); // Ugly.

    if (!(curr_cpu_p->flags & VCPU_FLAG_USED)) return;

    if ((curr_cpu_p->state & (VCPU_STATE_SLEEPFOR | VCPU_STATE_SLEEPUNTIL
        | VCPU_STATE_IOWAIT | VCPU_STATE_IPCWAIT))) return;

    // z33d's changes
    if (curr_cpu_p->state & VCPU_STATE_STOPPED) continue;
    if (curr_cpu_p->flags & VCPU_FLAG_DEBUG)
      if(main_debug(curr_cpu)) // go to debugger
        continue;
    // end of z33d's changes

    /* This must come after debugger or hell results in stepping out of a program */
    if (curr_cpu_p->IP>=curr_cpu_p->bytecode_size) {
      non_fatal(ERROR_OUTSIDE_CODE,"IP beyond code segment",curr_cpu);
      continue;
    }

    change=1;
    bytecode_p=&(curr_cpu_p->bytecode[curr_cpu_p->IP*12]);

    got_nonfatal_round=0;
    but_in_fact_it_was_fatal=0;

    ((void (*)()) JIT [bytecode_p[0]*36+(bytecode_p[1]+1)+6*bytecode_p[2]]) ();

    curr_cpu_p->IP=curr_cpu_p->IP+change;
  }

}

// JITs initialisation

void jit_init() {
  int i;

	printf("Seeding JIT table: ");
	printf(".");
	fflush(stdout);

  	for (i=0;i<=36;i++) { JIT[CMD_NOP+i]=cmd_nop; }
	printf(".");
	fflush(stdout);


	for (i=0;i<6;i++) {
		JIT[CMD_JMP*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_jmp_immediate;
		JIT[CMD_JMP*36+(TYPE_UREG+1)+6*i]=cmd_jmp_ureg;
		JIT[CMD_JMP*36+(TYPE_IMMPTR+1)+6*i]=cmd_jmp_immptr;
		JIT[CMD_JMP*36+(TYPE_UPTR+1)+6*i]=cmd_jmp_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_IFEQ*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_ifeq_immediate;
		JIT[CMD_IFEQ*36+(TYPE_UREG+1)+6*i]=cmd_ifeq_ureg;
		JIT[CMD_IFEQ*36+(TYPE_SREG+1)+6*i]=cmd_ifeq_sreg;
		JIT[CMD_IFEQ*36+(TYPE_FREG+1)+6*i]=cmd_ifeq_freg;
		JIT[CMD_IFEQ*36+(TYPE_IMMPTR+1)+6*i]=cmd_ifeq_immptr;
		JIT[CMD_IFEQ*36+(TYPE_UPTR+1)+6*i]=cmd_ifeq_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_IFNEQ*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_ifneq_immediate;
		JIT[CMD_IFNEQ*36+(TYPE_UREG+1)+6*i]=cmd_ifneq_ureg;
		JIT[CMD_IFNEQ*36+(TYPE_SREG+1)+6*i]=cmd_ifneq_sreg;
		JIT[CMD_IFNEQ*36+(TYPE_FREG+1)+6*i]=cmd_ifneq_freg;
		JIT[CMD_IFNEQ*36+(TYPE_IMMPTR+1)+6*i]=cmd_ifneq_immptr;
		JIT[CMD_IFNEQ*36+(TYPE_UPTR+1)+6*i]=cmd_ifneq_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_IFABO*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_ifabo_immediate;
		JIT[CMD_IFABO*36+(TYPE_UREG+1)+6*i]=cmd_ifabo_ureg;
		JIT[CMD_IFABO*36+(TYPE_SREG+1)+6*i]=cmd_ifabo_sreg;
		JIT[CMD_IFABO*36+(TYPE_FREG+1)+6*i]=cmd_ifabo_freg;
		JIT[CMD_IFABO*36+(TYPE_IMMPTR+1)+6*i]=cmd_ifabo_immptr;
		JIT[CMD_IFABO*36+(TYPE_UPTR+1)+6*i]=cmd_ifabo_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_IFBEL*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_ifbel_immediate;
		JIT[CMD_IFBEL*36+(TYPE_UREG+1)+6*i]=cmd_ifbel_ureg;
		JIT[CMD_IFBEL*36+(TYPE_SREG+1)+6*i]=cmd_ifbel_sreg;
		JIT[CMD_IFBEL*36+(TYPE_FREG+1)+6*i]=cmd_ifbel_freg;
		JIT[CMD_IFBEL*36+(TYPE_IMMPTR+1)+6*i]=cmd_ifbel_immptr;
		JIT[CMD_IFBEL*36+(TYPE_UPTR+1)+6*i]=cmd_ifbel_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_CALL*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_call_immediate;
		JIT[CMD_CALL*36+(TYPE_UREG+1)+6*i]=cmd_call_ureg;
		JIT[CMD_CALL*36+(TYPE_IMMPTR+1)+6*i]=cmd_call_immptr;
		JIT[CMD_CALL*36+(TYPE_UPTR+1)+6*i]=cmd_call_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_RET*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_ret_immediate;
		JIT[CMD_RET*36+(TYPE_UREG+1)+6*i]=cmd_ret_ureg;
		JIT[CMD_RET*36+(TYPE_IMMPTR+1)+6*i]=cmd_ret_immptr;
		JIT[CMD_RET*36+(TYPE_UPTR+1)+6*i]=cmd_ret_uptr;
	}
	printf(".");
	fflush(stdout);

  	for (i=0;i<=36;i++) { JIT[CMD_HALT*36+i]=cmd_halt; }
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_SYSCALL*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_syscall_immediate;
		JIT[CMD_SYSCALL*36+(TYPE_UREG+1)+6*i]=cmd_syscall_ureg;
		JIT[CMD_SYSCALL*36+(TYPE_IMMPTR+1)+6*i]=cmd_syscall_immptr;
		JIT[CMD_SYSCALL*36+(TYPE_UPTR+1)+6*i]=cmd_syscall_uptr;
	}
	printf(".");
	fflush(stdout);

	JITS(CMD_ADD,TYPE_UREG,TYPE_IMMEDIATE)=cmd_add_ureg_immediate;
	JITS(CMD_ADD,TYPE_UREG,TYPE_UREG)=cmd_add_ureg_ureg;
	JITS(CMD_ADD,TYPE_UREG,TYPE_SREG)=cmd_add_ureg_sreg;
	JITS(CMD_ADD,TYPE_UREG,TYPE_FREG)=cmd_add_ureg_freg;
	JITS(CMD_ADD,TYPE_UREG,TYPE_IMMPTR)=cmd_add_ureg_immptr;
	JITS(CMD_ADD,TYPE_UREG,TYPE_UPTR)=cmd_add_ureg_uptr;

	JITS(CMD_ADD,TYPE_SREG,TYPE_IMMEDIATE)=cmd_add_sreg_immediate;
	JITS(CMD_ADD,TYPE_SREG,TYPE_UREG)=cmd_add_sreg_ureg;
	JITS(CMD_ADD,TYPE_SREG,TYPE_SREG)=cmd_add_sreg_sreg;
	JITS(CMD_ADD,TYPE_SREG,TYPE_FREG)=cmd_add_sreg_freg;
	JITS(CMD_ADD,TYPE_SREG,TYPE_IMMPTR)=cmd_add_sreg_immptr;
	JITS(CMD_ADD,TYPE_SREG,TYPE_UPTR)=cmd_add_sreg_uptr;

	JITS(CMD_ADD,TYPE_IMMPTR,TYPE_IMMEDIATE)=cmd_add_immptr_immediate;
	JITS(CMD_ADD,TYPE_IMMPTR,TYPE_UREG)=cmd_add_immptr_ureg;
	JITS(CMD_ADD,TYPE_IMMPTR,TYPE_SREG)=cmd_add_immptr_sreg;
	JITS(CMD_ADD,TYPE_IMMPTR,TYPE_FREG)=cmd_add_immptr_freg;
	JITS(CMD_ADD,TYPE_IMMPTR,TYPE_IMMPTR)=cmd_add_immptr_immptr;
	JITS(CMD_ADD,TYPE_IMMPTR,TYPE_UPTR)=cmd_add_immptr_uptr;

	JITS(CMD_ADD,TYPE_UPTR,TYPE_IMMEDIATE)=cmd_add_uptr_immediate;
	JITS(CMD_ADD,TYPE_UPTR,TYPE_UREG)=cmd_add_uptr_ureg;
	JITS(CMD_ADD,TYPE_UPTR,TYPE_SREG)=cmd_add_uptr_sreg;
	JITS(CMD_ADD,TYPE_UPTR,TYPE_FREG)=cmd_add_uptr_freg;
	JITS(CMD_ADD,TYPE_UPTR,TYPE_IMMPTR)=cmd_add_uptr_immptr;
	JITS(CMD_ADD,TYPE_UPTR,TYPE_UPTR)=cmd_add_uptr_uptr;

	JITS(CMD_ADD,TYPE_FREG,TYPE_IMMEDIATE)=cmd_add_freg_immediate;
	JITS(CMD_ADD,TYPE_FREG,TYPE_UREG)=cmd_add_freg_ureg;
	JITS(CMD_ADD,TYPE_FREG,TYPE_SREG)=cmd_add_freg_sreg;
	JITS(CMD_ADD,TYPE_FREG,TYPE_FREG)=cmd_add_freg_freg;
	JITS(CMD_ADD,TYPE_FREG,TYPE_IMMPTR)=cmd_add_freg_immptr;
	JITS(CMD_ADD,TYPE_FREG,TYPE_UPTR)=cmd_add_freg_uptr;

	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_SUB*36+(TYPE_FREG+1)+6*i]=cmd_sub_freg;
	}

	JITS(CMD_SUB,TYPE_UREG,TYPE_IMMEDIATE)=cmd_sub_ureg_immediate;
	JITS(CMD_SUB,TYPE_UREG,TYPE_UREG)=cmd_sub_ureg_ureg;
	JITS(CMD_SUB,TYPE_UREG,TYPE_SREG)=cmd_sub_ureg_sreg;
	JITS(CMD_SUB,TYPE_UREG,TYPE_FREG)=cmd_sub_ureg_freg;
	JITS(CMD_SUB,TYPE_UREG,TYPE_IMMPTR)=cmd_sub_ureg_immptr;
	JITS(CMD_SUB,TYPE_UREG,TYPE_UPTR)=cmd_sub_ureg_uptr;

	JITS(CMD_SUB,TYPE_SREG,TYPE_IMMEDIATE)=cmd_sub_sreg_immediate;
	JITS(CMD_SUB,TYPE_SREG,TYPE_UREG)=cmd_sub_sreg_ureg;
	JITS(CMD_SUB,TYPE_SREG,TYPE_SREG)=cmd_sub_sreg_sreg;
	JITS(CMD_SUB,TYPE_SREG,TYPE_FREG)=cmd_sub_sreg_freg;
	JITS(CMD_SUB,TYPE_SREG,TYPE_IMMPTR)=cmd_sub_sreg_immptr;
	JITS(CMD_SUB,TYPE_SREG,TYPE_UPTR)=cmd_sub_sreg_uptr;

	JITS(CMD_SUB,TYPE_IMMPTR,TYPE_IMMEDIATE)=cmd_sub_immptr_immediate;
	JITS(CMD_SUB,TYPE_IMMPTR,TYPE_UREG)=cmd_sub_immptr_ureg;
	JITS(CMD_SUB,TYPE_IMMPTR,TYPE_SREG)=cmd_sub_immptr_sreg;
	JITS(CMD_SUB,TYPE_IMMPTR,TYPE_FREG)=cmd_sub_immptr_freg;
	JITS(CMD_SUB,TYPE_IMMPTR,TYPE_IMMPTR)=cmd_sub_immptr_immptr;
	JITS(CMD_SUB,TYPE_IMMPTR,TYPE_UPTR)=cmd_sub_immptr_uptr;

	JITS(CMD_SUB,TYPE_UPTR,TYPE_IMMEDIATE)=cmd_sub_uptr_immediate;
	JITS(CMD_SUB,TYPE_UPTR,TYPE_UREG)=cmd_sub_uptr_ureg;
	JITS(CMD_SUB,TYPE_UPTR,TYPE_SREG)=cmd_sub_uptr_sreg;
	JITS(CMD_SUB,TYPE_UPTR,TYPE_FREG)=cmd_sub_uptr_freg;
	JITS(CMD_SUB,TYPE_UPTR,TYPE_IMMPTR)=cmd_sub_uptr_immptr;
	JITS(CMD_SUB,TYPE_UPTR,TYPE_UPTR)=cmd_sub_uptr_uptr;

	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_MUL*36+(TYPE_UREG+1)+6*i]=cmd_mul_ureg;
		JIT[CMD_MUL*36+(TYPE_SREG+1)+6*i]=cmd_mul_sreg;
		JIT[CMD_MUL*36+(TYPE_FREG+1)+6*i]=cmd_mul_freg;
		JIT[CMD_MUL*36+(TYPE_IMMPTR+1)+6*i]=cmd_mul_immptr;
		JIT[CMD_MUL*36+(TYPE_UPTR+1)+6*i]=cmd_mul_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_DIV*36+(TYPE_UREG+1)+6*i]=cmd_div_ureg;
		JIT[CMD_DIV*36+(TYPE_SREG+1)+6*i]=cmd_div_sreg;
		JIT[CMD_DIV*36+(TYPE_FREG+1)+6*i]=cmd_div_freg;
		JIT[CMD_DIV*36+(TYPE_IMMPTR+1)+6*i]=cmd_div_immptr;
		JIT[CMD_DIV*36+(TYPE_UPTR+1)+6*i]=cmd_div_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_MOD*36+(TYPE_UREG+1)+6*i]=cmd_mod_ureg;
		JIT[CMD_MOD*36+(TYPE_SREG+1)+6*i]=cmd_mod_sreg;
		JIT[CMD_MOD*36+(TYPE_IMMPTR+1)+6*i]=cmd_mod_immptr;
		JIT[CMD_MOD*36+(TYPE_UPTR+1)+6*i]=cmd_mod_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_XOR*36+(TYPE_UREG+1)+6*i]=cmd_xor_ureg;
		JIT[CMD_XOR*36+(TYPE_SREG+1)+6*i]=cmd_xor_sreg;
		JIT[CMD_XOR*36+(TYPE_IMMPTR+1)+6*i]=cmd_xor_immptr;
		JIT[CMD_XOR*36+(TYPE_UPTR+1)+6*i]=cmd_xor_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_NOT*36+(TYPE_UREG+1)+6*i]=cmd_not_ureg;
		JIT[CMD_NOT*36+(TYPE_SREG+1)+6*i]=cmd_not_sreg;
		JIT[CMD_NOT*36+(TYPE_IMMPTR+1)+6*i]=cmd_not_immptr;
		JIT[CMD_NOT*36+(TYPE_UPTR+1)+6*i]=cmd_not_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_AND*36+(TYPE_UREG+1)+6*i]=cmd_and_ureg;
		JIT[CMD_AND*36+(TYPE_SREG+1)+6*i]=cmd_and_sreg;
		JIT[CMD_AND*36+(TYPE_IMMPTR+1)+6*i]=cmd_and_immptr;
		JIT[CMD_AND*36+(TYPE_UPTR+1)+6*i]=cmd_and_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_OR*36+(TYPE_UREG+1)+6*i]=cmd_or_ureg;
		JIT[CMD_OR*36+(TYPE_SREG+1)+6*i]=cmd_or_sreg;
		JIT[CMD_OR*36+(TYPE_IMMPTR+1)+6*i]=cmd_or_immptr;
		JIT[CMD_OR*36+(TYPE_UPTR+1)+6*i]=cmd_or_uptr;
	}
	printf(".");
	fflush(stdout);

	JITS(CMD_MOV,TYPE_UREG,TYPE_IMMEDIATE)=cmd_mov_ureg_immediate;
	JITS(CMD_MOV,TYPE_UREG,TYPE_UREG)=cmd_mov_ureg_ureg;
	JITS(CMD_MOV,TYPE_UREG,TYPE_SREG)=cmd_mov_ureg_sreg;
	JITS(CMD_MOV,TYPE_UREG,TYPE_FREG)=cmd_mov_ureg_freg;
	JITS(CMD_MOV,TYPE_UREG,TYPE_IMMPTR)=cmd_mov_ureg_immptr;
	JITS(CMD_MOV,TYPE_UREG,TYPE_UPTR)=cmd_mov_ureg_uptr;

	JITS(CMD_MOV,TYPE_SREG,TYPE_IMMEDIATE)=cmd_mov_sreg_immediate;
	JITS(CMD_MOV,TYPE_SREG,TYPE_UREG)=cmd_mov_sreg_ureg;
	JITS(CMD_MOV,TYPE_SREG,TYPE_SREG)=cmd_mov_sreg_sreg;
	JITS(CMD_MOV,TYPE_SREG,TYPE_FREG)=cmd_mov_sreg_freg;
	JITS(CMD_MOV,TYPE_SREG,TYPE_IMMPTR)=cmd_mov_sreg_immptr;
	JITS(CMD_MOV,TYPE_SREG,TYPE_UPTR)=cmd_mov_sreg_uptr;

	JITS(CMD_MOV,TYPE_FREG,TYPE_IMMEDIATE)=cmd_mov_freg_immediate;
	JITS(CMD_MOV,TYPE_FREG,TYPE_UREG)=cmd_mov_freg_ureg;
	JITS(CMD_MOV,TYPE_FREG,TYPE_SREG)=cmd_mov_freg_sreg;
	JITS(CMD_MOV,TYPE_FREG,TYPE_FREG)=cmd_mov_freg_freg;
	JITS(CMD_MOV,TYPE_FREG,TYPE_IMMPTR)=cmd_mov_freg_immptr;
	JITS(CMD_MOV,TYPE_FREG,TYPE_UPTR)=cmd_mov_freg_uptr;

	JITS(CMD_MOV,TYPE_IMMPTR,TYPE_IMMEDIATE)=cmd_mov_immptr_immediate;
	JITS(CMD_MOV,TYPE_IMMPTR,TYPE_UREG)=cmd_mov_immptr_ureg;
	JITS(CMD_MOV,TYPE_IMMPTR,TYPE_SREG)=cmd_mov_immptr_sreg;
	JITS(CMD_MOV,TYPE_IMMPTR,TYPE_FREG)=cmd_mov_immptr_freg;
	JITS(CMD_MOV,TYPE_IMMPTR,TYPE_IMMPTR)=cmd_mov_immptr_immptr;
	JITS(CMD_MOV,TYPE_IMMPTR,TYPE_UPTR)=cmd_mov_immptr_uptr;

	JITS(CMD_MOV,TYPE_UPTR,TYPE_IMMEDIATE)=cmd_mov_uptr_immediate;
	JITS(CMD_MOV,TYPE_UPTR,TYPE_UREG)=cmd_mov_uptr_ureg;
	JITS(CMD_MOV,TYPE_UPTR,TYPE_SREG)=cmd_mov_uptr_sreg;
	JITS(CMD_MOV,TYPE_UPTR,TYPE_FREG)=cmd_mov_uptr_freg;
	JITS(CMD_MOV,TYPE_UPTR,TYPE_IMMPTR)=cmd_mov_uptr_immptr;
	JITS(CMD_MOV,TYPE_UPTR,TYPE_UPTR)=cmd_mov_uptr_uptr;

	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_SLEEPFOR*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_sleepfor_immediate;
		JIT[CMD_SLEEPFOR*36+(TYPE_UREG+1)+6*i]=cmd_sleepfor_ureg;
		JIT[CMD_SLEEPFOR*36+(TYPE_SREG+1)+6*i]=cmd_sleepfor_sreg;
		JIT[CMD_SLEEPFOR*36+(TYPE_IMMPTR+1)+6*i]=cmd_sleepfor_immptr;
		JIT[CMD_SLEEPFOR*36+(TYPE_UPTR+1)+6*i]=cmd_sleepfor_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_WAITTILL*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_waittill_immediate;
		JIT[CMD_WAITTILL*36+(TYPE_UREG+1)+6*i]=cmd_waittill_ureg;
		JIT[CMD_WAITTILL*36+(TYPE_SREG+1)+6*i]=cmd_waittill_sreg;
		JIT[CMD_WAITTILL*36+(TYPE_IMMPTR+1)+6*i]=cmd_waittill_immptr;
		JIT[CMD_WAITTILL*36+(TYPE_UPTR+1)+6*i]=cmd_waittill_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_ALLOC*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_alloc_immediate;
		JIT[CMD_ALLOC*36+(TYPE_UREG+1)+6*i]=cmd_alloc_ureg;
		JIT[CMD_ALLOC*36+(TYPE_SREG+1)+6*i]=cmd_alloc_sreg;
		JIT[CMD_ALLOC*36+(TYPE_IMMPTR+1)+6*i]=cmd_alloc_immptr;
		JIT[CMD_ALLOC*36+(TYPE_UPTR+1)+6*i]=cmd_alloc_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_REALLOC*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_realloc_immediate;
		JIT[CMD_REALLOC*36+(TYPE_UREG+1)+6*i]=cmd_realloc_ureg;
		JIT[CMD_REALLOC*36+(TYPE_SREG+1)+6*i]=cmd_realloc_sreg;
		JIT[CMD_REALLOC*36+(TYPE_IMMPTR+1)+6*i]=cmd_realloc_immptr;
		JIT[CMD_REALLOC*36+(TYPE_UPTR+1)+6*i]=cmd_realloc_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_DEALLOC*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_dealloc_immediate;
		JIT[CMD_DEALLOC*36+(TYPE_UREG+1)+6*i]=cmd_dealloc_ureg;
		JIT[CMD_DEALLOC*36+(TYPE_SREG+1)+6*i]=cmd_dealloc_sreg;
		JIT[CMD_DEALLOC*36+(TYPE_IMMPTR+1)+6*i]=cmd_dealloc_immptr;
		JIT[CMD_DEALLOC*36+(TYPE_UPTR+1)+6*i]=cmd_dealloc_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_CMPCNT*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_cmpcnt_immediate;
		JIT[CMD_CMPCNT*36+(TYPE_UREG+1)+6*i]=cmd_cmpcnt_ureg;
		JIT[CMD_CMPCNT*36+(TYPE_SREG+1)+6*i]=cmd_cmpcnt_sreg;
		JIT[CMD_CMPCNT*36+(TYPE_IMMPTR+1)+6*i]=cmd_cmpcnt_immptr;
		JIT[CMD_CMPCNT*36+(TYPE_UPTR+1)+6*i]=cmd_cmpcnt_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_CPCNT*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_cpcnt_immediate;
		JIT[CMD_CPCNT*36+(TYPE_UREG+1)+6*i]=cmd_cpcnt_ureg;
		JIT[CMD_CPCNT*36+(TYPE_SREG+1)+6*i]=cmd_cpcnt_sreg;
		JIT[CMD_CPCNT*36+(TYPE_IMMPTR+1)+6*i]=cmd_cpcnt_immptr;
		JIT[CMD_CPCNT*36+(TYPE_UPTR+1)+6*i]=cmd_cpcnt_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_ONFAIL*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_onfail_immediate;
		JIT[CMD_ONFAIL*36+(TYPE_UREG+1)+6*i]=cmd_onfail_ureg;
		JIT[CMD_ONFAIL*36+(TYPE_IMMPTR+1)+6*i]=cmd_onfail_immptr;
		JIT[CMD_ONFAIL*36+(TYPE_UPTR+1)+6*i]=cmd_onfail_uptr;
	}
	printf(".");
	fflush(stdout);

  	for (i=0;i<=36;i++) { JIT[CMD_NOFAIL*36+i]=cmd_nofail; }
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_LOOP*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_loop_immediate;
		JIT[CMD_LOOP*36+(TYPE_UREG+1)+6*i]=cmd_loop_ureg;
		JIT[CMD_LOOP*36+(TYPE_IMMPTR+1)+6*i]=cmd_loop_immptr;
		JIT[CMD_LOOP*36+(TYPE_UPTR+1)+6*i]=cmd_loop_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_RAISE*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_raise_immediate;
		JIT[CMD_RAISE*36+(TYPE_UREG+1)+6*i]=cmd_raise_ureg;
		JIT[CMD_RAISE*36+(TYPE_IMMPTR+1)+6*i]=cmd_raise_immptr;
		JIT[CMD_RAISE*36+(TYPE_UPTR+1)+6*i]=cmd_raise_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_LDB*36+(TYPE_UREG+1)+6*i]=cmd_ldb_ureg;
		JIT[CMD_LDB*36+(TYPE_SREG+1)+6*i]=cmd_ldb_sreg;
		JIT[CMD_LDB*36+(TYPE_FREG+1)+6*i]=cmd_ldb_freg;
		JIT[CMD_LDB*36+(TYPE_IMMPTR+1)+6*i]=cmd_ldb_immptr;
		JIT[CMD_LDB*36+(TYPE_UPTR+1)+6*i]=cmd_ldb_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_SETSTACK*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_setstack_immediate;
		JIT[CMD_SETSTACK*36+(TYPE_UREG+1)+6*i]=cmd_setstack_ureg;
		JIT[CMD_SETSTACK*36+(TYPE_IMMPTR+1)+6*i]=cmd_setstack_immptr;
		JIT[CMD_SETSTACK*36+(TYPE_UPTR+1)+6*i]=cmd_setstack_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_PUSHS*36+(TYPE_IMMEDIATE+1)+6*i]=cmd_push_immediate;
		JIT[CMD_PUSHS*36+(TYPE_UREG+1)+6*i]=cmd_push_ureg;
		JIT[CMD_PUSHS*36+(TYPE_SREG+1)+6*i]=cmd_push_sreg;
		JIT[CMD_PUSHS*36+(TYPE_FREG+1)+6*i]=cmd_push_freg;
		JIT[CMD_PUSHS*36+(TYPE_IMMPTR+1)+6*i]=cmd_push_immptr;
		JIT[CMD_PUSHS*36+(TYPE_UPTR+1)+6*i]=cmd_push_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_POPS*36+(TYPE_UREG+1)+6*i]=cmd_pop_ureg;
		JIT[CMD_POPS*36+(TYPE_SREG+1)+6*i]=cmd_pop_sreg;
		JIT[CMD_POPS*36+(TYPE_FREG+1)+6*i]=cmd_pop_freg;
		JIT[CMD_POPS*36+(TYPE_IMMPTR+1)+6*i]=cmd_pop_immptr;
		JIT[CMD_POPS*36+(TYPE_UPTR+1)+6*i]=cmd_pop_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<6;i++) {
		JIT[CMD_STOB*36+(TYPE_UREG+1)+6*i]=cmd_stob_ureg;
		JIT[CMD_STOB*36+(TYPE_SREG+1)+6*i]=cmd_stob_sreg;
		JIT[CMD_STOB*36+(TYPE_IMMPTR+1)+6*i]=cmd_stob_immptr;
		JIT[CMD_STOB*36+(TYPE_UPTR+1)+6*i]=cmd_stob_uptr;
	}
	printf(".");
	fflush(stdout);

	for (i=0;i<CMD_INVALID*36;i++) {
		if (JIT[i]==NULL) JIT[i]=cmd_invalid;
	}

	printf(" OK\n");


}

