/*

   Argante virtual OS
   ------------------

   Advanced Keyboard Led Controler

   Status: forever unstable ;>
   Reason: proof of lcamtuf's concept :))))

   Author:     Morisey Prodeus <z33d@dolnyslask.com>
   Maintainer: Morisey Prodeus <z33d@dolnyslask.com>
   
   WARNING: this module is an experiment and it's only for linux... btw
   kd.h is broken ;>
   
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "module.h"
#include "memory.h"
#include "console.h"
#include "syscall.h"
#include "acman.h"

void syscall_load(int* x) {
  *(x++)=SYSCALL_LED_SET;
  *(x++)=SYSCALL_LED_GET;
  *(x)=SYSCALL_ENDLIST;
  printk(">> Blinking led module loaded.\n");
}

void syscall_handler(int c, int num){
  int mode=cpu[c].uregs[0];
  switch(num){
    case SYSCALL_LED_SET:
      VALIDATE(c,"none", "led/set");
      check_status();
      if(ioctl(0, KDSKBLED, mode)==-1){
        non_fatal(ERROR_BAD_PARAM, "Can't set led flag",c);
	return;
      }
    break;
    case SYSCALL_LED_GET:
      VALIDATE(c,"none", "led/get");
      check_status();
      if(ioctl(0, KDGKBLED, &cpu[c].uregs[0])==-1){
        non_fatal(ERROR_PROTFAULT, "Can't get led flag",c);
        return;
      }
    break;
  }
}
