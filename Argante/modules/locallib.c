/*

   Argante virtual OS
   ------------------

   Simple, useful functions: get machine name, get time,
   get OS name, get VS statistics, get real system stats etc.

   Status: done, needs porting

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifdef __linux__

#include <linux/kernel.h>
#include <linux/sys.h>
#include <sys/sysinfo.h>

#endif

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "module.h"
#include "memory.h"
#include "console.h"
#include "syscall.h"
#include "acman.h"

void syscall_load(int* x) {
  *(x++)=SYSCALL_LOCAL_GETTIME;
  *(x++)=SYSCALL_LOCAL_TIMETOSTR;
  *(x++)=SYSCALL_LOCAL_GETHOSTNAME;
  *(x++)=SYSCALL_LOCAL_VS_STAT;
  *(x++)=SYSCALL_LOCAL_RS_STAT;
  *(x++)=SYSCALL_LOCAL_GETRANDOM;
  *(x)=SYSCALL_ENDLIST;
  printk(">> LocalLibrary module loaded.\n");
}

void syscall_handler(int c,int nr) {
  struct timezone tz;
  struct timeval tv;
  char* sth;

  switch (nr) {
    case SYSCALL_LOCAL_GETTIME:

      // Ret: u0 - current time
      // Ret: u1 - current time, microseconds

      VALIDATE(c,"none","local/sys/real/time/get");
      gettimeofday(&tv,&tz);

      cpu[c].uregs[0]=tv.tv_sec;
      cpu[c].uregs[1]=tv.tv_usec;

      break;

    case SYSCALL_LOCAL_TIMETOSTR:

      // Input:  u0 - GETTIME result, u1 - buf, u2 - buf size
      // Return: s0 - bytes stored

      if (!(sth=verify_access(c,cpu[c].uregs[1],(cpu[c].uregs[2]+3)/4,MEM_FLAG_WRITE))) {
        non_fatal(ERROR_PROTFAULT,"ctime: Attempt to access protected memory",c);
        failure=1;
        return;
      }

      { char* nn;
        nn=ctime((void*)&cpu[c].uregs[0]);
        if (strlen(nn)>cpu[c].uregs[2]) {
          non_fatal(ERROR_RESULT_TOOLONG,"ctime: result buffer too small",c);
          failure=1;
          return;
        }
        if (nn[strlen(nn)-1]=='\n') nn[strlen(nn)-1]=0;
        memcpy(sth,nn,strlen(nn));
        cpu[c].sregs[0]=strlen(nn);
      }

      break;

    case SYSCALL_LOCAL_GETHOSTNAME:

      // u0 - bufor, u1 - adres
      // s0 - ilo¶æ znaków

      VALIDATE(c,"none","local/sys/real/hostname/get");

      if (!(sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_WRITE))) {
        non_fatal(ERROR_PROTFAULT,"gethostname: Attempt to access protected memory",c);
        failure=1;
        return;
      }

      if (gethostname(sth,cpu[c].uregs[1])) {
        non_fatal(ERROR_RESULT_TOOLONG,"gethostname: result buffer too small",c);
        failure=1;
        return;
      }    

      break;

    case SYSCALL_LOCAL_GETRANDOM:

      { int x,y; unsigned int l;

      // Return: u0 - random number

      VALIDATE(c,"none","local/sys/random/get");

      x=open("/dev/urandom",O_RDONLY|O_NONBLOCK);
      y=read(x,&l,4);
      close(x);
      if ((x<0) || (y<0)) { 
        non_fatal(ERROR_DEADLOCK,"Cannot access entropy pool",c);
        failure=1;
        return;
      }

      cpu[c].uregs[0]=l;

      }

      break;

    case SYSCALL_LOCAL_VS_STAT:

//            Efekt: u0 - ilo¶æ aktywnych VCPUs; u1 - ilo¶æ cykli idle od startu,
//                   u2 - ilo¶æ cykli pracy od startu, u3 - ilo¶æ syscalli,
//                   u4 - ilo¶æ z³ych syscalli, u5 - fatal errors...

      VALIDATE(c,"none","local/sys/virtual/stat");

      { int i,cnt=0; 
        for (i=0;i<MAX_VCPUS;i++) if (cpu[i].flags & VCPU_FLAG_USED) cnt++;
        cpu[c].uregs[0]=cnt;
      }

      cpu[c].uregs[1]=total_idle;
      cpu[c].uregs[2]=total_work;
      cpu[c].uregs[3]=syscalls;
      cpu[c].uregs[4]=badsys;
      cpu[c].uregs[5]=fatal_errors;


      break;

    case SYSCALL_LOCAL_RS_STAT:

      // u0 - load average

      VALIDATE(c,"none","local/sys/real/stat");

#ifndef __linux__

      printk("RS_STAT for current OS is not implemented.\n");

#else

     {
        struct sysinfo i;
        sysinfo(&i);

        cpu[c].uregs[0]=i.uptime;
        cpu[c].uregs[1]=i.loads[0];
        cpu[c].uregs[2]=i.totalram/1024;
        cpu[c].uregs[3]=i.freeram/1024;
        cpu[c].uregs[4]=i.totalswap/1024;
        cpu[c].uregs[5]=i.freeswap/1024;
        cpu[c].uregs[5]=i.procs;

     }

#endif /* __linux__ */

      break;

  }
}
