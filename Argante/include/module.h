/*

   Argante virtual OS
   ------------------

   Loadable modules support structures and macros

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

struct sysentry {
  char name[MAX_NAME];
  int call_count;
  void (*handler)(int,int);
  void (*taskreap)(int);
  void (*destructor)(void);
  int serve[MAX_SERVE];
  void* __hent;
};

void do_syscall(int c,int number);
void load_module(char* path);
void unload_module(int handle);
void list_modules(void);

#define clean_status()		failure=0
#define check_status()		if (failure) return

#ifndef __I_AM_THE_MODULE

extern struct sysentry mod[MAX_MODULES];

#endif

#define ENTER_IOWAIT(c,res,hndlr) 	\
	    cpu[c].iohandler=hndlr; 	\
	    cpu[c].iowait_id=res;	\
	    cpu[c].state|=VCPU_STATE_IOWAIT;

#define ENTER_CRITWAIT(c,res,hndlr) 	\
	    cpu[c].iohandler=hndlr; 	\
	    cpu[c].iowait_id=res;	\
	    cpu[c].timecrit=1;		\
	    cpu[c].state|=VCPU_STATE_IOWAIT;
