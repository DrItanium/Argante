/*

   Argante virtual OS
   ------------------

   Multitasking support, task loading, etc.

   Status: done, optimizations welcome

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

   Several optimizations suggested by maxiu were done.

*/

#define I_AM_THE_TASK 1

#include <string.h>
#include <sys/time.h>
#include "config.h"
#include "console.h"
#include "task.h"
#include "memory.h"
#include "bcode.h"
#include "module.h"
#include "debugger.h"
#include <stdlib.h>
#include <unistd.h>

/* Globals */
int curr_cpu;
struct vcpu_struct *curr_cpu_p;

extern int be_quick_dude;
void	(*task_ripc_handler)(void) = NULL;
void	(*task_ipc_timeouter)(void) = NULL;

struct vcpu_struct cpu[MAX_VCPUS];
int total_idle=0, total_work=0;
int used_memory,used_memblocks;
int exec_cnt,exit_proc,killed_proc,died_proc;
int used_bytecode,syscalls,badsys,total_wait;
int fatal_errors,nonfatal_errors,handled_errors;
int top_cpu=0;
int handler_dirtywork=1;

int safe_usleep;
int wait_babe;
int sleep_to;

void init_taskman(void) {
  printk("Initializing virtual CPUs...\n");
  printk("  VCPU configuration (VCPU: %d, overall: %d bytes):\n",
         sizeof(struct vcpu_struct),sizeof(cpu));
  printk("  Max exec domains: %d\n",MAX_EXEC_DOMAINS);
  printk("  Process namelen:  %d\n",MAX_NAME);
  printk("  Registers:        %d\n",REGISTERS);
  printk("  Memory blocks:    %d\n",MAX_MEMBLK);
  printk("  Highest VCPU no:  %d\n",MAX_VCPUS);
}


#include <setjmp.h>
#include <signal.h>

jmp_buf dead_fish;
int kill_the_process=0;
int current_cpu;
int caught_sig;

void bad_signal(int x) {
  caught_sig=x;
  handler_dirtywork=1;
  longjmp(dead_fish,1);
  exit(0);
}


void destroy_task(int x) {
  int xx,doit=0,lala,wkcyc;
  char* RES;
  if (x<0 || x>MAX_VCPUS) {
    printk("Bad parameter.\n");
    return;
  }
  if (!(cpu[x].flags & VCPU_FLAG_USED)) {
    printk("Idle VCPU (%d) cleanup.\n",x);
  } else {
    doit=1;
  }
  for (lala=0;lala<MAX_MODULES;lala++) 
    if (mod[lala].handler && mod[lala].taskreap) {
//      printk("=> Calling task_cleanup() for module %s [%d]\n",mod[lala].name,lala);
      mod[lala].taskreap(x);
    }
  if (cpu[x].bytecode) { 
    free(cpu[x].bytecode); 
    if (doit) used_bytecode-=cpu[x].bytecode_size; 
  }
  for (xx=0;xx<cpu[x].dmsiz;xx++) 
    if ((*cpu[x].mem)[xx].real_memory) {
      if (doit) used_memblocks--; 
      if (doit) used_memory-=(*cpu[x].mem)[xx].size;
      free((*cpu[x].mem)[xx].real_memory);
    }
  if (doit) {
    printk("+> VCPU #%d (%s) terminated.\n",x,cpu[x].name); 
    killed_proc++;
  }
  RES=cpu[x].respawn;
  wkcyc=cpu[x].work_cycles;
  if (cpu[x].ex_st) free(cpu[x].ex_st);
  if (cpu[x].stack) free(cpu[x].stack);
  if (cpu[x].mem) free(cpu[x].mem);
  bzero(&cpu[x],sizeof(struct vcpu_struct));
  if (RES) {
    if (wkcyc<MIN_CYCLES_TO_RESPAWN) {
      printk("-> Task loaded as RESPAWN, but didn't run minimal number\n");
      printk("-> of work cycles to be respawned (%d instead of %d).\n",wkcyc,MIN_CYCLES_TO_RESPAWN);
      printk("+> This case is senseless, killing it forever.\n");
    } else {
      printk("=> Killed task is launched in respawn mode and worked enough\n");
      printk("=> amount of work cycles, reloading it (%s)...\n",RES);
      load_image(RES,LOAD_RESPAWN);
    }
  }
}



void destroy_task_respawn(int x) {
  int xx,doit=0,lala;
  if (x<0 || x>MAX_VCPUS) {
    printk("Bad parameter.\n");
    return;
  }
  if (!(cpu[x].flags & VCPU_FLAG_USED)) {
    printk("Idle VCPU (%d) cleanup.\n",x);
  } else {
    doit=1;
  }
  for (lala=0;lala<MAX_MODULES;lala++) 
    if (mod[lala].handler && mod[lala].taskreap) {
//      printk("=> Calling task_cleanup() for module %s [%d]\n",mod[lala].name,lala);
      mod[lala].taskreap(x);
    }
  if (cpu[x].bytecode) { 
    free(cpu[x].bytecode); 
    if (doit) used_bytecode-=cpu[x].bytecode_size; 
  }
  if (cpu[x].respawn) free(cpu[x].respawn);
  for (xx=0;xx<cpu[x].dmsiz;xx++) 
    if ((*cpu[x].mem)[xx].real_memory) {
      if (doit) used_memblocks--; 
      if (doit) used_memory-=(*cpu[x].mem)[xx].size;
      free((*cpu[x].mem)[xx].real_memory);
    }
  if (doit) {
    printk("+> VCPU #%d (%s) killed.\n",x,cpu[x].name); 
    killed_proc++;
  }
  if (cpu[x].ex_st) free((void*)cpu[x].ex_st);
  if (cpu[x].stack) free(cpu[x].stack);
  if (cpu[x].mem) free(cpu[x].mem);
  bzero(&cpu[x],sizeof(struct vcpu_struct));
}



int someone_is_sleeping;
int datime;

inline void task_cycle(void) {
  int x;
  struct timeval tv;
  struct timezone tz;

  safe_usleep=1;
  wait_babe=0;

  if (someone_is_sleeping) {
    gettimeofday(&tv,&tz);
    datime=tv.tv_sec*1000000+tv.tv_usec;
    someone_is_sleeping=0;
    sleep_to=datime+DEFAULT_SAFE_USLEEP;
    wait_babe=1;
  }

  if (task_ripc_handler) {
      task_ripc_handler();
  }

  if (task_ipc_timeouter) {
      task_ipc_timeouter();
  }

  kill_the_process=0;

  if (handler_dirtywork) {

    signal(SIGBUS,bad_signal);
    signal(SIGSEGV,bad_signal);
    signal(SIGILL,bad_signal);
    signal(SIGFPE,bad_signal);
    signal(SIGPIPE,SIG_IGN); // Aarm, ignore it
    signal(SIGIO,bad_signal);
    handler_dirtywork=0;
  }

  if (setjmp(dead_fish)) {

    if (current_cpu<0) {
      printk("task_cycle: Caught fatal OS signal %d, dying.\n",caught_sig);
      exit(1);
    }

    x=current_cpu;

    printk("-> Error: VCPU %d caught OS exception %d, killing it.\n",current_cpu,
           caught_sig);
    printk("-> VCPU process name: %s\n",curr_cpu_p->name);
    printk("-> Instruction pointer: 0x%x\n",curr_cpu_p->IP);
    printk("-> Task flags=0x%x, state=0x%x\n",curr_cpu_p->flags,curr_cpu_p->state);
    destroy_task(x);
    died_proc++;
    killed_proc--;
    fatal_errors++;
    return;
  }

  for (x=0;x<top_cpu+1;x++) {
	  /* curr_cpu globalisation by shykta */
    curr_cpu=current_cpu=x;
    curr_cpu_p=&cpu[x];
    if (curr_cpu_p->flags & VCPU_FLAG_USED) {
      if (curr_cpu_p->state & VCPU_STATE_IOWAIT) {
        curr_cpu_p->iowait_cycles++;
        total_wait++;
        if (curr_cpu_p->timecrit) wait_babe=safe_usleep=0;
        if (curr_cpu_p->iohandler) {
          if (curr_cpu_p->iohandler(x))
            if (curr_cpu_p->state & VCPU_STATE_IOWAIT) {
              curr_cpu_p->state-=VCPU_STATE_IOWAIT;
              curr_cpu_p->iohandler=0;
              curr_cpu_p->iowait_id=0;
              curr_cpu_p->timecrit=0;
            }
          } else {
          curr_cpu_p->state-=VCPU_STATE_IOWAIT;
          curr_cpu_p->iohandler=0;
          curr_cpu_p->iowait_id=0;
          wait_babe=0;
          non_fatal(ERROR_DEADLOCK,"STATE_IOWAIT but no iohandler()",x);
        }
      } else
      if (curr_cpu_p->state & VCPU_STATE_IPCWAIT) {	// by Bulba
          curr_cpu_p->iowait_cycles++;
	  total_wait++;
      } else 
      if (!(curr_cpu_p->state & (VCPU_STATE_SLEEPFOR|VCPU_STATE_SLEEPUNTIL))) {
        do_cycles(curr_cpu_p->priority);
        safe_usleep=wait_babe=0;
        curr_cpu_p->work_cycles++;
        total_work++;
      } else
      if (curr_cpu_p->state & VCPU_STATE_SLEEPFOR) {
        if (curr_cpu_p->cnt_down>0) curr_cpu_p->cnt_down--;
        if (!curr_cpu_p->cnt_down)
          curr_cpu_p->state-=VCPU_STATE_SLEEPFOR;
        safe_usleep=wait_babe=0;
        curr_cpu_p->sleep_cycles++; total_idle++;
      } else if (curr_cpu_p->state & VCPU_STATE_SLEEPUNTIL) {
        someone_is_sleeping=1;
        if (datime>curr_cpu_p->wake_on)
          curr_cpu_p->state-=VCPU_STATE_SLEEPUNTIL;
        safe_usleep=0;
        if (curr_cpu_p->wake_on<sleep_to) sleep_to=curr_cpu_p->wake_on;
        curr_cpu_p->sleep_cycles++; total_idle++;       
      }
    } else total_idle++;
  }

  if (safe_usleep) if (!be_quick_dude) usleep(DEFAULT_SAFE_USLEEP);
  if (wait_babe) if (!be_quick_dude) if (sleep_to && (sleep_to-datime>0))
    usleep((sleep_to-datime)/2);

  current_cpu=-1;

}

/* BE AGGRESSIVE, B-E AGGRESSIVE, B-E A-G-G-R-E-S-S-I-V-E !! */


int 
task_check_bcode(void *code,int code_len) {
    unsigned int oper;
    unsigned int t1;
    unsigned int t2;
    unsigned int *r;
    unsigned char *tabliczka = code;
    int cnt;


    for (cnt=0;cnt<code_len;cnt++,tabliczka+=12) {
	oper = tabliczka[0];
	if (oper>=CMD_INVALID) {
          printk("-> ERROR: illegal opcode at IP %x\n", cnt);
          return 0;
        }
	t1 = tabliczka[1];
	t2 = tabliczka[2];
	if (t1 > TYPE_UPTR || t2 > TYPE_UPTR) {
          printk("-> ERROR: illegal paramtype at IP %x\n", cnt);
          return 0;
        };
	if (t1!= TYPE_IMMEDIATE && t1!= TYPE_IMMPTR ) {
	    r = (unsigned int *)(tabliczka+4);	    
	    if (*r >= REGISTERS) {
              printk("-> ERROR: invalid param 1 register at IP %x\n", cnt);
              return 0;
            }
	}	
	if (t2!= TYPE_IMMEDIATE && t2!= TYPE_IMMPTR) {
	    r = (unsigned int *)(tabliczka+8);	    
	    if (*r >= REGISTERS) {
              printk("-> ERROR: invalid param 2 register at IP %x\n", cnt);
              return 0;
            }	
	}	
	    
    }
    printk("+> On-load bytecode validation successful.\n");
    return 1;
}

#include "bformat.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

/* For endian-swap; if problematic define NO_ENDIAN_SWAP */
#ifndef NO_ENDIAN_SWAP
/* Concept and original implementation by Peter Turczak */
#include <byteswap.h>

/* Will be mostly optimized out. */
#define bswap_gen(x) \
	if (sizeof(x)==2) x=bswap_16(x); \
	if (sizeof(x)==4) x=bswap_32(x); \
	if (sizeof(x)==8) x=bswap_64(x);

/* Format conversions can be guaranteed to work;
 * all data is infinitely predictable.
 * Bytecode conversion is much harder. */
static void fmtswap (struct bformat *FMT)
{
	printk("-> Doing swap-endian...\n");
	
	bswap_gen(FMT->magic1);
	
	bswap_gen(FMT->flags);
	bswap_gen(FMT->priority);
	bswap_gen(FMT->ipc_reg);
	bswap_gen(FMT->init_IP);
	bswap_gen(FMT->current_domain);
	bswap_gen(FMT->domain_uid);
	bswap_gen(FMT->bytesize);
	bswap_gen(FMT->memflags);
	FMT->memflags|=MEM_FLAG_SWAPENDIAN;
	bswap_gen(FMT->datasize);
	
	bswap_gen(FMT->magic2);
}

/* Large changes by shykta. This does not (yet)
 * fix swap-endian atad; this should be dealt with
 * in get/set_mem_value. */

/* This is according to README */
struct _bcode_op {
	char bcode;
	char t1;
	char t2;
	char rsrvd;
	long a1;
	long a2;
};

static void swapcode(void *code, int datasize)
{
	int x; 
	struct _bcode_op *op;
	
	op=code;
	
	for (x=0;x<datasize;x+=sizeof(struct _bcode_op)) {
		/* Chars need no swap */
		bswap_gen(op->a1);
		bswap_gen(op->a2);
		op++;
	}
	printk("OK: code conversion done...");
}
#else
void fmtswap (struct bformat *FMT)
{
	printk("+> Compiled without swap-endian support; nothing done.\n");
}

void swapcode(void *code, int datasize) {}
#endif

void load_image(char* fn, int debugging) {
  int x;
  int f,nn;
  int got=0;
  struct bformat FMT;
  char* respawn_it=0;
  int swapendian=0;

  if (debugging==LOAD_RESPAWN) {
    debugging=0;
    respawn_it=strdup(fn);
  }

  if (!fn[0]) {
    printk("ERROR: Filename required.\n");
    return;
  }
  for (x=0;x<MAX_VCPUS;x++) 
    if (!(cpu[x].flags & VCPU_FLAG_USED)) { got=1;break; }

  if (!got) {
    printk("ERROR: No free VCPUs, sorry.\n");
    return;
  }

  if (top_cpu<x) top_cpu=x;

  printk("+> Free VCPU #%d, binary image from: %s...\n",x,fn);
  f=open(fn,O_RDONLY);
  if (f<0) {
    perror(fn);
    return;
  }

  bzero(&FMT,sizeof(FMT));
  read(f,&FMT,sizeof(FMT));

  /* Try bswapping it...? */
  if (FMT.magic1!=BFMT_MAGIC1) {
	  fmtswap(&FMT);
	  swapendian=1;
  }
  if (FMT.magic1!=BFMT_MAGIC1) {
    printk("-> ERROR: Bad bformat.magic1 value.\n");
    return;
  }

  if (FMT.magic2!=BFMT_MAGIC2) {
    printk("-> ERROR: Bad bformat.magic2 value (file too new, too old or corrupted).\n");
    return;
  }

  exec_cnt++;

  bzero(&cpu[x],sizeof(struct vcpu_struct));
  memcpy(cpu[x].domain,FMT.domains,sizeof(FMT.domains));

  printk("=> Execution domains: ");

  for (nn=0;nn<MAX_EXEC_DOMAINS;nn++) {
    if (FMT.domains[nn]<1) {
      if (!nn) printk("(empty list)");
      break;
    }
    printk("%d ",FMT.domains[nn]);
  }

  cpu[x].domain_uid=0;
  cpu[x].current_domain=0;

  if (FMT.current_domain>0) cpu[x].current_domain=FMT.current_domain;
  if (FMT.domain_uid>0) cpu[x].domain_uid=FMT.domain_uid;

  if (cpu[x].current_domain>0) {
    if (cpu[x].domain_uid>0)
      printk("\n=> Active subdomain: %d:%d\n",cpu[x].current_domain,cpu[x].domain_uid);
      else printk("\n=> Active domain: %d (no subdomain)\n",cpu[x].current_domain);
  } else printk("\n=> No active domain.\n");

  if (FMT.priority<1) FMT.priority=1;
  if (FMT.priority>MAX_PRIORITY) FMT.priority=MAX_PRIORITY;

  cpu[x].flags=FMT.flags | VCPU_FLAG_USED;
  // z33d's changes
  if (debugging){
    printk("=> Debug mode: ON\n");
    cpu[x].flags|=VCPU_FLAG_DEBUG;
    debug[x].traceflag=DEBUG_DEBUG;
    debug[x].b_size=0;
  }
  // end of z33d's changes

  printk("=> Execution flags: 0x%x, priority %d\n",FMT.flags,FMT.priority);
  cpu[x].priority=FMT.priority;

  printk("=> Bytecode: %d packets, initial ip %d (0x%x)\n",FMT.bytesize,
          FMT.init_IP,FMT.init_IP);

  if (FMT.bytesize>MAX_LOAD_BYTESIZE) {
    printk("-> ERROR: binary image too large, max. %d packets. Aborting.\n",MAX_LOAD_BYTESIZE);
    destroy_task(x);
    return;
  }

  cpu[x].IP=FMT.init_IP;
  cpu[x].bytecode_size=FMT.bytesize;

  printk("+> Signature: %s\n",FMT.signature);

  if (strchr(fn,'/')) fn=strrchr(fn,'/')+1;
  strncpy(cpu[x].name,fn,MAX_NAME-1);
  cpu[x].started_at=time(0);

  cpu[x].bytecode=malloc(FMT.bytesize*12);
  
  if (!cpu[x].bytecode) {
    printk("-> ERROR: No memory for bytecode. Emergency destroy_task().\n");
    destroy_task(x);
    return;
  }
  
     cpu[x].stack=malloc(4*STACK_GROW); 
     cpu[x].dssiz=STACK_GROW;
     cpu[x].ex_st=malloc(4*(STACK_GROW+1));
     cpu[x].mem=malloc(sizeof(struct memarea)*MEM_GROW);
     cpu[x].dmsiz=MEM_GROW;
  
  if (!cpu[x].stack) {
    printk("-> ERROR: No memory for initial stack. Emergency destroy_task().\n");
    cpu[x].dssiz=0;
    destroy_task(x);
    return;
  }

  if (!cpu[x].ex_st) {
    printk("-> ERROR: No memory for initial ex stack. Emergency destroy_task().\n");
    cpu[x].dssiz=0;
    destroy_task(x);
    return;
  }

  if (!cpu[x].mem) {
    printk("-> ERROR: No memory for initial address space. Emergency destroy_task().\n");
    cpu[x].dmsiz=0;
    destroy_task(x);
    return;
  }
  
  memset(cpu[x].stack,0,cpu[x].dssiz*4);
  memset(cpu[x].ex_st,0,cpu[x].dssiz*4);
  memset(cpu[x].mem,0,cpu[x].dmsiz*sizeof(struct memarea));

  if (FMT.datasize) {

    (*cpu[x].mem)[0].real_memory=malloc(FMT.datasize*4);
    if (!(*cpu[x].mem)[0].real_memory) {
      printk("-> ERROR: No memory for data. Emergency destroy_task().\n");
      destroy_task(x);
      return;
    }

    printk("+> Data: %d dwords allocated.\n",FMT.datasize);
    used_memblocks++; 

  }

  (*cpu[x].mem)[0].flags=FMT.memflags;
  (*cpu[x].mem)[0].size=FMT.datasize;
  
  nn=read(f,cpu[x].bytecode,FMT.bytesize*12);

  if (nn!=FMT.bytesize*12) {
    printk("-> Warning: binary image smaller than declared (n=%d)!\n",nn);
    cpu[x].bytecode_size=nn/12;
  }

  used_bytecode+=cpu[x].bytecode_size;

  if (swapendian) swapcode(cpu[x].bytecode, nn);

  if (!task_check_bcode(cpu[x].bytecode,nn/12)) {
    destroy_task(x);
    return;
  }

  if (FMT.datasize) {
    nn=read(f,(*cpu[x].mem)[0].real_memory,FMT.datasize*4);

    if (nn!=FMT.datasize*4) {
      printk("-> Warning: data image smaller than declared (%d)!\n",nn);
      (*cpu[x].mem)[0].size=nn/4;
    }
  }

  nn=read(f,&got,1);
  used_memory+=(*cpu[x].mem)[0].size;

  if (nn>0)
    printk("-> Warning: junk at the end of binary file!\n");

  if (nn>FMT.bytesize) nn=FMT.bytesize;

  if (FMT.init_IP>nn) {
    printk("-> Warning: initial IP beyond available address space.\n");
  }

  cpu[x].ipc_reg=FMT.ipc_reg;

  if (cpu[x].ipc_reg)
    printk("=> Process IPC identity number: 0x%x\n",FMT.ipc_reg);
    else printk("=> No initial IPC identity.\n");

  cpu[x].respawn=respawn_it;

  if (!respawn_it)
    printk("+> Process launched successfully.\n");
  else printk("+> Process launched successfully in RESPAWN mode.\n");

  close(f);

}
