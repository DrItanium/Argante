/*

   Argante virtual OS
   ------------------

   Access Control management.

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
  *(x++)=SYSCALL_ACCESS_SETDOMAIN;
  *(x++)=SYSCALL_ACCESS_SETUID;
  *(x)=SYSCALL_ENDLIST;
  printk(">> Access Control module loaded.\n");
}


inline static int is_on_list(char* dom,int what) {
  int i;
  if (what<1) return 1; // Dropping privledges is cool ;)
  for (i=0;i<MAX_EXEC_DOMAINS;i++) {
    if (dom[i]<1) return 0;
    if (what==dom[i]) return 1;
  }
  return 0;
}


void syscall_handler(int c,int num) {
  int setwhat;

  setwhat=cpu[c].uregs[0];

  switch (num) {

    case SYSCALL_ACCESS_SETDOMAIN:
      if ((!setwhat) && (!is_on_list(cpu[c].domain,setwhat))) 
        non_fatal(ERROR_NOPERM,"Attempt to set domain with no entry on domain list",c);
        else { cpu[c].domain_uid=0; cpu[c].current_domain=setwhat; }
      break;

    case SYSCALL_ACCESS_SETUID:
      cpu[c].domain_uid=setwhat;
      break;

  } 

}
