/*

   Argante virtual OS release 2
   ----------------------------

   Task manager structures.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#ifndef _HAVE_TASKMAN
#define _HAVE_TASKMAN 1

#include "config.h"
#include "bformat.h"
#include "bcode_op.h"
#include <setjmp.h>

#define STATUS_STOPPED		1
#define STATUS_RUN		2
#define STATUS_STEP		3
#define STATUS_TWAIT		4
#define STATUS_CWAIT		5
#define STATUS_IOWAIT		6

#define FLAGS_RESPAWN		1

/* Could someone in-the-know look at reordering this for 100% efficiency? */
struct vcpu {
  unsigned int status;		// Task status
  unsigned int flags;		// Task flags
  unsigned int wait;		// Wait parameter
  jmp_buf onexcept;		// Place to go when an exception happens **** NEW ****

  unsigned int ip;		// Instruction pointer
  bcode_op *bcode;		// Pointer to bytecode **** CHANGED ****
  short *bcode_jitoffs;		// Dirty cheat that duadextuples speed. **** NEW ****
  unsigned int csize;		// Bytecode size
  
  anyval reg[REGISTERS];	// Registers **** CHANGED ****
  
  struct memblk (*mem);		// Memory management struct
  unsigned int memblks;		// Number of memory blocks right now.

  unsigned int *stack;		// Dynamic stack
  unsigned int *xstack;		// Exception handler stack
  unsigned int sptr;		// Stack pointer
  unsigned int stack_size;	// Size of stack **** NEW ****
  unsigned int xip;		// IP of current xhandler. (Before any calls happen, there's no xstack)

  unsigned int mstack;		// Metastack start
  unsigned int mstack_ptr;	// Metastack pointer
  unsigned int mstack_size;	// Metastack size

  char pname[MAX_PNAME];	// Process name
  unsigned int started_at;	// time_t
  
  void (*iohandler)(void);	// I/O handler if in IOWAIT
  unsigned int domain;		// Execution domain
  unsigned int uid;		// Domain UID
  unsigned int *dlist;		// List of available domains
  
  void* reserved[MAX_RSRVD];	// Reserved structures (for module structs)
  unsigned int lid[MAX_RSRVD];	// Library ID for every struct above.

  struct stable (*st)[];	// Symbol table
  unsigned int syms;		// Symbol table size
};

#include <stdio.h>
#define printk(x...)	 fprintf(stderr,x)
void destroy_task_respawn(struct vcpu *);

//destroy_task_respawn

#endif /* not _HAVE_TASKMAN */

