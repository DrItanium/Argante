/*

   Argante virtual OS
   ------------------

   Loadable modules support.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#define __I_AM_THE_MODULE

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "console.h"
#include "memory.h"
#include "module.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#ifndef RTLD_GLOBAL // BSDish
#define RTLD_GLOBAL 0
#endif

struct sysentry mod[MAX_MODULES];

int (*loader)(int* list);
int failure;

void do_syscall(int c,int number) {
  int x,y,got=0;
  failure=0;
  if (number<=0) {
    non_fatal(ERROR_BAD_PARAM,"Syscall number should be positive",c);
    return;
  }
  for (x=0;x<MAX_MODULES;x++)
    if (mod[x].handler)
      for (y=0;(y<MAX_SERVE) && (mod[x].serve[y]>0);y++)
        if (mod[x].serve[y]==number) {
          got=1;          
          mod[x].call_count++;
          mod[x].handler(c,number);
          return;
        }
  if (!got) {
    printk("-> No syscall handler for scid %d (0x%x)...\n",number,number);
    non_fatal(ERROR_NOMODULE,"No syscall handler found",c);
    badsys++;
  }
  syscalls++;
}


void load_module(char* path) {
  int got,n;
  char* plum;
  void* x;
  int y;
  
  got=0;
  for (n=0;n<MAX_MODULES;n++) if (!mod[n].handler) { got=1; break; }

  if (!got) {
    printk("No free slots, sorry.\n");
    return;
  }

  bzero(&mod[n],sizeof(struct sysentry));
  x=dlopen(path,RTLD_LAZY|RTLD_GLOBAL);

  if (!x) {
    printk("Cannot open library %s.\n",path);
    printk("Error: %s\n",dlerror());
    return;
  } else
    printk("=> Loading module %s...\n",path);

  mod[n].handler=dlsym(x,"syscall_handler");
  if (!mod[n].handler) mod[n].handler=dlsym(x,"_syscall_handler");

  if (!mod[n].handler) {
    printk("-> ERROR: no syscall_handler() routine, aborting load.\n");
    dlclose(x);
    return;
  }

  mod[n].destructor=dlsym(x,"syscall_unload");
  if (!mod[n].destructor) mod[n].destructor=dlsym(x,"_syscall_unload");

  mod[n].taskreap=dlsym(x,"syscall_task_cleanup");
  if (!mod[n].taskreap) mod[n].taskreap=dlsym(x,"_syscall_task_cleanup");

  loader=dlsym(x,"syscall_load");
  if (!loader) loader=dlsym(x,"_syscall_load");
  
  if (loader) {
//    printk("+> Calling syscall_load() constructor...\n");
    loader(mod[n].serve);
  } else {
    printk("-> ERROR: no syscall_load() routine, aborting load.\n");
    dlclose(x);
    mod[n].handler=0;
    return;
  }


  plum=strrchr(path,'/');
  if (!plum) plum=path;

  strncpy(mod[n].name,plum+1,MAX_NAME-1);
  mod[n].call_count=0;
  mod[n].__hent=x;

  got=0;
  printk("+> This module will be serving: ");
  for (y=0;(y<MAX_SERVE) && (mod[n].serve[y]>0);y++) {
    got=1;
    printk("%d ",mod[n].serve[y]);
  }
  if (!got) printk("<empty list>");
  printk("\n");
//  printk("+> Module loaded successfully.\n");
}


void unload_module(int handle) {
  if (handle<0 || handle>MAX_MODULES) {
    printk("Bad parameter.\n");
    return;
  }
  if (mod[handle].handler)  {
    printk("=> Unloading module #%d (%s)...\n",handle,mod[handle].name);
    if (mod[handle].destructor) {
      printk("+> Calling destructor routine...\n");
      mod[handle].destructor();
    } else printk("=> Module has no destructor routine.\n");
    dlclose(mod[handle].__hent);
    mod[handle].handler=0;
    printk("+> Module slot freed.\n");
  } else {
    printk("Attempt to unload module from free slot.\n");
  }
}

void list_modules(void) {
  int got=0,x,y;
  for (x=0;x<MAX_MODULES;x++)  
   if (mod[x].handler) {
     got=0;
     printk("=> Slot #%d: module '%s' called %d times.\n",x,mod[x].name,mod[x].call_count);
     printk("=> Serving: ");
     for (y=0;(y<MAX_SERVE) && (mod[x].serve[y]>0);y++) {
       got=1;
       printk("%d ",mod[x].serve[y]);
     }
     if (!got) printk("<empty list>");
     printk("\n");
     got=1;
   }
  if (!got) printk("No modules loaded.\n");
}

