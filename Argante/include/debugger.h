/*

   Argante virtual OS
   ------------------

   Debugger flags and structures.

   Status: done

   Author:     Maurycy Prodeus <z33d@eth-security.net>
   Maintainer: Maurycy Prodeus <z33d@eth-security.net>

*/

#include "config.h"

struct debug_desc {
  int b_size;
  unsigned int breakpoints[MAXBREAKPOINTS]; 
  int traceflag;
};

#ifndef I_AM_DE_BURGER
extern struct debug_desc debug[MAX_VCPUS];
#endif

#define DEBUG_DEBUG	0x00000000
#define DEBUG_CONTINUE	0x00000001
#define DEBUG_STEP	0x00000002
#define DEBUG_SYSCALL	0x00000004
#define DEBUG_DEAD	0x00000008
#define DEBUG_RET	0x00000010

int main_debug(int);
void show_registers(int);
void step(int);
void continue_process(int);
void continue_process_syscall(int);
void show_memory(int, unsigned int, unsigned int, char);
void show_breakpoints(int);
void add_breakpoint(int, unsigned int);
void del_breakpoint(int, int);
void dump_instructions(int,int,int);
void show_stack(int);
void continue_ret(int);

