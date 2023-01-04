/*

   Argante virtual OS
   ------------------

   VCPU core code, tasking structures

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/
#ifndef ARGANTE_TASK_H__
#define ARGANTE_TASK_H__
#include <stdint.h>
#include "memory.h"
#include "linker.h"

#define LOAD_RESPAWN		0x31337

#define VCPU_FLAG_USED  	0x00001000
#define VCPU_FLAG_DEBUG		0x00000001
#define VCPU_FLAG_PROFILE	0x00000002

#define VCPU_STATE_STOPPED	0x00010000
#define VCPU_STATE_SLEEPFOR	0x00000001
#define VCPU_STATE_SLEEPUNTIL	0x00000002
#define VCPU_STATE_IOWAIT	0x00000004
#define VCPU_STATE_IPCWAIT	0x00000008	// by Bulba
union FloatRegister {
    float f;
    int32_t i;
    uint32_t u;
} ;
struct vcpu_struct {
  char domain[MAX_EXEC_DOMAINS];  // execution domains (-1, >100 = end)
  unsigned char name[MAX_NAME];	  // process name
  uint32_t started_at;	  // Creation time
  uint32_t  sleep_cycles;	  // Idle counter
  uint32_t  work_cycles;	  // Work counter
  uint32_t  iowait_cycles;	  // Work counter
  uint32_t  ipc_reg;		  // IPC number
  uint32_t  iowait_id;	  // I/O wait identity
  int32_t (*iohandler)(int32_t);	  // real I/O handler (called when IOWAIT)
  char* respawn;		  // Respawn mode?
  int32_t handling_failure;	 	  // Handling exception?
  int32_t first_except_ip;		  // First exception IP
  char current_domain;		  // currently choosen domain
  int32_t domain_uid;		  // domain UID
  char timecrit;		  // IOWAIT is time-critical
  uint32_t flags;		  // flags (see VCPU_FLAG_xxx)
  uint32_t state;             // state (see VCPU_STATE_xxx)
  uint32_t cnt_down;	  // cycle countdown (when sleeping-for)
  uint32_t wake_on;		  // wakeup timer (when sleeping-until)
  uint32_t priority;	  // VCPU execution priority
  unsigned char* bytecode;	  // Bytecode playground
  uint32_t bytecode_size;	  // Bytecode size
  uint32_t IP;		  // Current instruction pointer
  uint32_t uregs[REGISTERS];  // Unsigned registers
  int32_t  sregs[REGISTERS];  // Signed registers
  signed char in_handler;
  uint32_t u0_saved;
  union FloatRegister fregs[REGISTERS];	   // Floating point math registers
  uint32_t (*ex_st)[];          // Exception handler stack (dynamic, AOS 1.1)
  uint32_t saved_ex;	  // Saved exception handler
  uint32_t ex_at;		  // Saved exception handler stack ptr
  uint32_t (*stack)[];          // Program stack (dynamic since AOS 1.1)
  uint32_t dssiz;		  // Dynamic stack size
  uint32_t stack_ptr;	  // Stack pointer
  uint32_t user_stack;        // User stack virtual ptr
  uint32_t user_size;         // User stack dword size
  uint32_t user_ptr;          // User stack ptr
  uint32_t dmsiz;		  // Dynamic memory size.
  struct memarea (*mem)[MAX_MEMBLK];          // Memory blocks (dynamic since AOS 1.1)
  struct symtab (*sym)[MAX_MEMBLK]; // Symbol table
  uint32_t symsiz;		    // Symbol table size...
  uint32_t rsrvd;		 // Hahah ;)  
};

#ifndef I_AM_THE_TASK

extern struct vcpu_struct cpu[MAX_VCPUS];
extern int32_t total_idle, total_work;
extern int32_t used_memory,used_memblocks;
extern int32_t exec_cnt,exit_proc,killed_proc,died_proc;
extern int32_t used_bytecode,total_wait;
extern int32_t fatal_errors,nonfatal_errors,handled_errors,syscalls,badsys;

#endif /* I_AM_THE_TASK */


void init_taskman(void);
void task_cycle(void);
void load_image(char*, int);
void destroy_task(int);
void destroy_task_respawn(int);

#endif // end !defined(ARGANTE_TASK_H__)
