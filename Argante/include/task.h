/*

   Argante virtual OS
   ------------------

   VCPU core code, tasking structures

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#include "memory.h"

#define LOAD_RESPAWN		0x31337

#define VCPU_FLAG_USED  	0x00001000
#define VCPU_FLAG_DEBUG		0x00000001
#define VCPU_FLAG_PROFILE	0x00000002

#define VCPU_STATE_STOPPED	0x00010000
#define VCPU_STATE_SLEEPFOR	0x00000001
#define VCPU_STATE_SLEEPUNTIL	0x00000002
#define VCPU_STATE_IOWAIT	0x00000004
#define VCPU_STATE_IPCWAIT	0x00000008	// by Bulba

struct vcpu_struct {
  char domain[MAX_EXEC_DOMAINS];  // execution domains (-1, >100 = end)
  unsigned char name[MAX_NAME];	  // process name
  unsigned int started_at;	  // Creation time
  unsigned int sleep_cycles;	  // Idle counter
  unsigned int work_cycles;	  // Work counter
  unsigned int iowait_cycles;	  // Work counter
  unsigned int ipc_reg;		  // IPC number
  unsigned int iowait_id;	  // I/O wait identity
  int (*iohandler)(int);	  // real I/O handler (called when IOWAIT)
  char* respawn;		  // Respawn mode?
  int handling_failure;	 	  // Handling exception?
  int first_except_ip;		  // First exception IP
  char current_domain;		  // currently choosen domain
  int domain_uid;		  // domain UID
  char timecrit;		  // IOWAIT is time-critical
  unsigned int flags;		  // flags (see VCPU_FLAG_xxx)
  unsigned int state;             // state (see VCPU_STATE_xxx)
  unsigned int cnt_down;	  // cycle countdown (when sleeping-for)
  unsigned int wake_on;		  // wakeup timer (when sleeping-until)
  unsigned int priority;	  // VCPU execution priority
  unsigned char* bytecode;	  // Bytecode playground
  unsigned int bytecode_size;	  // Bytecode size
  unsigned int IP;		  // Current instruction pointer
  unsigned int uregs[REGISTERS];  // Unsigned registers
  signed   int sregs[REGISTERS];  // Signed registers
  signed char in_handler;
  unsigned int u0_saved;
  float fregs[REGISTERS];	   // Floating point math registers
  unsigned int (*ex_st)[];          // Exception handler stack (dynamic, AOS 1.1)
  unsigned int saved_ex;	  // Saved exception handler
  unsigned int ex_at;		  // Saved exception handler stack ptr
  unsigned int (*stack)[];          // Program stack (dynamic since AOS 1.1)
  unsigned int dssiz;		  // Dynamic stack size
  unsigned int stack_ptr;	  // Stack pointer
  unsigned int user_stack;        // User stack virtual ptr
  unsigned int user_size;         // User stack dword size
  unsigned int user_ptr;          // User stack ptr
  unsigned int dmsiz;		  // Dynamic memory size.
  struct memarea (*mem)[MAX_MEMBLK];          // Memory blocks (dynamic since AOS 1.1)
  struct symtab (*sym)[MAX_MEMBLK]; // Symbol table
  unsigned int symsiz;		    // Symbol table size...
  unsigned int rsrvd;		 // Hahah ;)  
};

#ifndef I_AM_THE_TASK

extern struct vcpu_struct cpu[MAX_VCPUS];
extern int total_idle, total_work;
extern int used_memory,used_memblocks;
extern int exec_cnt,exit_proc,killed_proc,died_proc;
extern int used_bytecode,total_wait;
extern int fatal_errors,nonfatal_errors,handled_errors,syscalls,badsys;

#endif /* I_AM_THE_TASK */


void init_taskman(void);
void task_cycle(void);
void load_image(char*, int);
void destroy_task(int);
void destroy_task_respawn(int);
