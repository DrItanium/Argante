/*

   Argante virtual OS
   ------------------

   Console management - not much to do :)

   Status: done, can be extended

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#define __I_AM_THE_MANAGER

#include "config.h"
#include "manager.h"
#include "console.h"
#include "task.h"
#include "bcode.h"
#include "module.h"
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/fcntl.h>
#include <ctype.h>
#include <signal.h>

#include "acman.h"
#include "debugger.h"

char script_name[110];
FILE* scr;

extern int stupid_pid;

#ifdef HAVE_READLINE
extern
#endif
       char* read_tmp;

void vcpu_status(void) {
  int x;
  int used=0,work=0,sleep=0,guarded=0,wait=0;
  int tot_idle=0,tot_work=0,tot_wait=0;
  int debugged=0, stopped=0, dead=0;
  int tot_res=0;
  for (x=0;x<MAX_VCPUS;x++) {
    if (cpu[x].flags & VCPU_FLAG_USED) {
      used++;
      if (cpu[x].flags & VCPU_FLAG_DEBUG) debugged++;
      if (cpu[x].state & VCPU_STATE_STOPPED) stopped++;
      if (debug[x].traceflag & DEBUG_DEAD) dead++;
       if ((*cpu[x].ex_st)[0]) guarded++;
      tot_idle+=cpu[x].sleep_cycles;
      tot_work+=cpu[x].work_cycles;
      tot_wait+=cpu[x].iowait_cycles;
      if (cpu[x].respawn) tot_res++;
      if (cpu[x].state & VCPU_STATE_IOWAIT) wait++; else
        if (cpu[x].state & VCPU_STATE_SLEEPFOR) sleep++; else
          if (cpu[x].state & VCPU_STATE_SLEEPUNTIL) sleep++; else
            work++;
    }
  }
  printk("=> VCPUs stats:   %d, %d used (%d wrk, %d sleeping).\n",
         MAX_VCPUS,used,work,sleep);
  printk("=> Total cycles:  %d idle, %d work, %d i/o wait.\n"
         "=> Current tasks: %d idle cycles, %d work, %d i/o wait.\n",
         total_idle,total_work,total_wait,tot_idle,tot_work,tot_wait);

  printk("=> Memory usage:  %d dwords allocated (in %d blocks).\n"
         "=> Bytecode size: %d packets.\n",
         used_memory,used_memblocks,used_bytecode); 
  printk("=> Processes:     %d executions (%d exited, %d killed, %d died, %d alive).\n",
          exec_cnt,exit_proc,killed_proc,died_proc,used);
  printk("=> Execution:     %d active VCPUs guarded, %d unguarded\n",guarded,used-guarded);
  printk("=> Exceptions:    %d total, %d fatal, %d non-fatal (%d handled).\n",fatal_errors+nonfatal_errors,fatal_errors,nonfatal_errors,handled_errors);
  printk("=> Syscalls:      %d overall, %d handled, %d incorrect.\n",syscalls,syscalls-badsys,badsys);
  printk("=> Debug:         %d debugged, %d stopped, %d dead.\n",debugged,stopped,dead);
  printk("=> Respawn:       %d tasks running in respawn mode.\n",tot_res);
  for (x=0;x<MAX_VCPUS;x++) {
    if (cpu[x].flags & VCPU_FLAG_USED) {
      printk("=> + VCPU #%d running process %s (%d packets).\n",x,cpu[x].name,cpu[x].bytecode_size);
    }
  }


}


void vcpu_stat(int x) {
  int nn;
  char buf[100];
  if (x<0 || x>MAX_VCPUS) {
    printk("Bad parameter.\n");
    return;
  }
  if (!(cpu[x].flags & VCPU_FLAG_USED)) {
    printk("This VCPU is idle.\n");
    return;
  }
  printk("=> VCPU #%d statistics:\n",x); 
  printk("=> Process name: %s\n",cpu[x].name);

  printk("=> Execution domains: ");

  for (nn=0;nn<MAX_EXEC_DOMAINS;nn++) {
    if (cpu[x].domain[nn]<1) {
      if (!nn) printk("(empty list)");
      break;
    }
    printk("%d ",cpu[x].domain[nn]);
  }
  printk("\n");

  if (cpu[x].domain>0) {
    if (cpu[x].domain_uid>0) 
      printk("=> Active subdomain: %d:%d\n",cpu[x].current_domain,cpu[x].domain_uid);
      else printk("=> Active domain: %d (no subdomain)\n",cpu[x].current_domain);
  } else printk("=> No active domain.\n");
  if (cpu[x].ipc_reg>0) printk("=> IPC registered as: %d\n", cpu[x].ipc_reg);
      else printk("=> Not registered for IPC.\n");

  strcpy(buf,ctime((time_t*)&cpu[x].started_at));

  if (buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;

  printk("=> Started at: %s\n",buf);
  printk("=> System usage: %d idle cycles, %d work cycles, %d i/o wait cycles.\n",cpu[x].sleep_cycles,
         cpu[x].work_cycles,cpu[x].iowait_cycles);
  printk("=> Flags 0x%x, state 0x%x, priority %d\n",cpu[x].flags,cpu[x].state,
         cpu[x].priority);
  printk("=> Bytecode: %d packets, IP at %d.\n",cpu[x].bytecode_size,
         cpu[x].IP);
  if (((*cpu[x].ex_st)[0]))
    printk("=> Top exception handler installed at 0x%x\n",(*cpu[x].ex_st)[0]);
    else printk("=> No exception handler installed for now.\n");
  printk("=> Stack pointer: %d dwords on stack.\n",cpu[x].stack_ptr);

  for (nn=0;nn<cpu[x].dmsiz;nn++) {
    if (!(*cpu[x].mem)[nn].real_memory) continue;
    printk("=> + VCPU Memory block #%d (fl=0x%x) is %d dwords mapped at %d (0x%x - 0x%x)\n",
           nn,(*cpu[x].mem)[nn].flags,(*cpu[x].mem)[nn].size,(*cpu[x].mem)[nn].map_addr,
           (*cpu[x].mem)[nn].map_addr,(*cpu[x].mem)[nn].map_addr+(*cpu[x].mem)[nn].size);
  }
  if (cpu[x].flags & VCPU_FLAG_DEBUG){
    printk("=> vCPU is debugged.\n");
    show_breakpoints(x);
    if (debug[x].traceflag & DEBUG_DEAD)
      printk("=> vCPU is dead (unhandled exception during debugging or kill).\n");
  }
  if (cpu[x].state & VCPU_STATE_STOPPED)
    printk("=> vCPU is stopped.\n");
  if (cpu[x].respawn)
    printk("=> Process '%s' running in respawn mode.\n",cpu[x].respawn);
  if (cpu[x].user_size) 
    printk("=> User mode metastack of size %d starting at 0x%x (%d used)\n",cpu[x].user_size,cpu[x].user_stack,cpu[x].user_ptr);
}


void tunkutunku(void) {
  printk("%s %d.%03d management console operations:\n",SYSNAME,SYS_MAJOR,SYS_MINOR);
  printk("  !           - dump system statistics\n");
  printk("  ?           - this help page\n");
  printk("  $fn         - load and run binary image from specified file\n");
  printk("  %%fn        - load and run image in respawn mode\n");
  printk("  >fn         - load and run system module\n");
  printk("  @fn         - load and run management script\n");
  printk("  #           - display list of loaded modules\n");
  printk("  -nn         - kill vCPU nn\n");
  printk("  <nn         - unload system module nn\n");
  printk("  =nn         - dump vCPU nn statistics\n");
  printk("  :xx         - execute subshell command 'xx'\n");
  printk("  ^           - reload access rules\n");
  printk("  *nn         - avoid input parsing for nn cycles [scripts]\n");
  printk("  ~xx         - display string 'xx' [scripts]\n");
  printk("  |xx         - do nothing - comment [scripts]\n");
  printk("  w a tmout   - wait for VCPU #a to halt (up to tmout seconds)\n");
  printk("  .           - halt the system\n");
  printk("Debug operations:\n");
  printk("  dfn         - load and run binary in debug mode\n");
  printk("  rnn         - show nn vCPU registers\n");
  printk("  xnn adr c t - show c bytes of memory on nn vCPU [t -> s,x,b,f]\n");
  printk("  nnn         - step exactly one instruction of nn vCPU\n");
  printk("  cnn         - continue process on nn vCPU\n");
  printk("  snn         - continue process on nn vCPU to next syscall\n");
  printk("  fnn         - continue process on nn vCPU to next ret\n");
  printk("  lnn         - list breakpoints on nn vCPU\n");
  printk("  bnn zz      - add breakpoint on nn vCPU at zz IP\n");
  printk("  unn zz      - delete zz breakpoint on nn vCPU\n");
  printk("  inn IP c    - disassemble c instructions at IP on VCPU nn\n");
  printk("  tnn         - show stack trace on nn vCPU\n");
}

void shutdown_system(void) {
  printk("+> Halting the system.\n");
  if (stupid_pid>0) kill(stupid_pid,2);
  sleep(1);
  printk("\nGoodbyte... :)\n\n");
  exit(0);
}

void do_nothing() {};

extern int wait_for_task;
extern int wait_time_started;
extern int wait_time_max;
extern int be_quick_dude,saved_be_quick;

void start_manager(void) {
  char* cmd;
  int x,t,t1;
  char type;

  int now_doing=1;
  char *p;

  if (!script_name[0]) {
    printk("\nInternal startup complete, entering management level.\n");
    printk("Executing boot-up script (%s)...\n",BOOTSCRIPT);
    strcpy(script_name,BOOTSCRIPT);
  }
  while (1) {
    for (x=0;x<now_doing;x++) { task_cycle(); }
    now_doing=1;
    x=1;
    cmd=get_command();
    if (!cmd || !cmd[0]) {
      read_tmp=0;
      continue;
    }
    x=-1000;
    if (cmd[1] && (isdigit(cmd[1]))) x=atoi(&cmd[1]);
    if (cmd[strlen(cmd)-1]==' ') cmd[strlen(cmd)-1]=0;
    if (cmd[0]=='!') vcpu_status(); else
    if (cmd[0]=='?') tunkutunku(); else
    if (cmd[0]=='$') load_image(&cmd[1],0); else
    if (cmd[0]=='%') load_image(&cmd[1],LOAD_RESPAWN); else
    // debugger function
    if (cmd[0]=='d') load_image(&cmd[1],1); else
    if (cmd[0]=='r') show_registers(x); else
    if (cmd[0]=='c') continue_process(x); else
    if (cmd[0]=='n') step(atoi(&cmd[1])); else
    if (cmd[0]=='s') continue_process_syscall(x); else
    if (cmd[0]=='l') show_breakpoints(x); else
    if (cmd[0]=='b'){
        int C,P;
        if (sscanf(&cmd[1],"%i %i",&C,&P)!=2) {
          printk("Bad parameters.\n");
          read_tmp=0;
          continue;
        }
        add_breakpoint(C, P);
    } else
    if (cmd[0]=='u'){
       if (!strchr(&cmd[1],' ')){
          printk("Bad parameter.\n");
          read_tmp=0;
          continue;
        }
        p=strchr(&cmd[1],' ')+1;
        sscanf(p,"%i",&t);
        del_breakpoint(atoi(&cmd[1]), t);
     } else
    if (cmd[0]=='x'){
      int C;
      if (sscanf(&cmd[1],"%i %i %i %c",&C,&t,&t1,&type)<3) {
          printk("Bad parameter.\n");
          read_tmp=0;
          continue;
      }
      if (type!='s' && type!='b' && type!='x' && type!='f')
          type='s';
      show_memory(C, t1, t, type);
    } else

    if (cmd[0]=='i'){
      int C;
      if (sscanf(&cmd[1],"%i %i %i",&C,&t,&t1)!=3) {
          printk("Bad parameter.\n");
          read_tmp=0;
          continue;
      }
      dump_instructions(C,t,t1);
    } else
    if (cmd[0]=='f') continue_ret(x); else
    if (cmd[0]=='t') show_stack(x); else
    if (cmd[0]=='>') load_module(&cmd[1]); else
    if (cmd[0]=='-') {
	if (x<0 || x>=MAX_VCPUS){
	    printk("Bad parameter.\n");
	    read_tmp=0;
	    continue;
	}
	if (!(cpu[x].flags & VCPU_FLAG_USED)){
	    printk("Idle VCPU (%d) cleanup.\n",x);
	    read_tmp=0;
	    continue;
	} 
	if (cpu[x].flags & VCPU_FLAG_DEBUG){
	    printk("Debugging trap.\n");
	    printk("%d VCPU killed.\n",x);
	    printk("Process died.\n");
	    killed_proc++;
	    debug[x].traceflag|=DEBUG_DEAD;
	    cpu[x].state|=VCPU_STATE_STOPPED;
	} else destroy_task_respawn(x); } else
    if (cmd[0]=='#') list_modules(); else
    if (cmd[0]=='=') vcpu_stat(x); else
    if (cmd[0]=='<') unload_module(x); else
    if (cmd[0]=='~') printk("%s\n",&cmd[1]); else
    if (cmd[0]=='|') do_nothing(); else
    if (cmd[0]==':') {
      fcntl(0,F_SETFL,O_SYNC);
      system(&cmd[1]);
    } else
    if (cmd[0]=='^') load_rules(-1); else
    if (cmd[0]=='*') { if (x>0) now_doing=x; } else
    if (cmd[0]=='@') strncpy(script_name,&cmd[1],sizeof(script_name)-1); else
    if (cmd[0]=='w') {
      wait_time_started=time(0);
      if (sscanf(&cmd[1],"%d %d",&wait_for_task,&wait_time_max)!=2) {
        wait_for_task=-1;
        printk("wait_for_task: Incorrect parameters.\n");
      } else {
        saved_be_quick=be_quick_dude;
        be_quick_dude=0;
      }
    } else 
    if (cmd[0]=='.') shutdown_system(); else
      printk("Pardon?\n");
    read_tmp=0;
  }
}
