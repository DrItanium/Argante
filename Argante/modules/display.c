/*

   Argante virtual OS
   ------------------

   Process console output support. It's really basical functionality,
   console shouldn't be used for any serious purposes, and processes
   shouldn't use it at all - just for debugging and testing :)

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "module.h"
#include "memory.h"
#include "console.h"
#include "syscall.h"
#include "acman.h"

void syscall_load(int* x) {
  *(x++)=SYSCALL_IO_PUTSTRING;
  *(x++)=SYSCALL_IO_PUTINT;
  *(x++)=SYSCALL_IO_PUTCHAR;
  *(x++)=SYSCALL_IO_PUTFLOAT;
  *(x++)=SYSCALL_IO_PUTHEX;
  *(x)=SYSCALL_ENDLIST;
  printk(">> Process console module loaded.\n");
}

void syscall_handler(int c,int num) {

  int cnt;
  int from;
  char* start;

  switch (num) {

    case SYSCALL_IO_PUTSTRING:

      VALIDATE(c,"none","display/output/text");
      check_status();

      from=cpu[c].uregs[0];
      cnt=cpu[c].uregs[1];

      start=verify_access(c,from,(cnt+3)/4,MEM_FLAG_READ);
      if (!start) {
        non_fatal(ERROR_PROTFAULT,"Can't print non-accessible memory",c);
        return;
      }
      write(2,start,cnt);
      return;
      break;

    case SYSCALL_IO_PUTINT:

      VALIDATE(c,"none","display/output/integer");
      check_status();

      printk("%d",cpu[c].uregs[0]);
      return;
      break;

    case SYSCALL_IO_PUTHEX:

      VALIDATE(c,"none","display/output/hex");
      check_status();

      printk("%x",cpu[c].uregs[0]);
      return;
      break;

    case SYSCALL_IO_PUTFLOAT:

      VALIDATE(c,"none","display/output/float");
      check_status();

      printk("%f",cpu[c].fregs[0]);
      return;
      break;


    case SYSCALL_IO_PUTCHAR:

      VALIDATE(c,"none","display/output/character");
      check_status();

      printk("%c",(char)cpu[c].uregs[0]);
      return;
      break;

  } 

}
