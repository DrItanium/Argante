/*

   Argante virtual OS
   ------------------

   Main entry point.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#include <unistd.h>

#include "config.h"
#include "console.h"
#include "task.h"
#include "bcode.h"
#include "manager.h"
#include <string.h>
#include "acman.h"

#define BRI  "\x1b[1m"
#define DARK "\x1b[2m"
#define NORM "\x1b[0m"

#define FAST 1

void boot_system(void) {
  printk(DARK "               \"[We] use bad software and bad machines for "
              "the wrong things.\"\n");

  printk(BRI "\n"
         "    ___   ___   ___   ___   ___  |_   ___\n"
         "   '___| |   ` |   | '___| |   | |   |___|\n"
         "   |___| |     |___| |___| |   | |__ |___.\n"
         "               .___|\n\n");

#ifndef FAST
  usleep(1000000);
#endif

  printk(NORM "Booting " BRI "%s" NORM " [version %d.%03d build %04d]...\n",SYSNAME,SYS_MAJOR,SYS_MINOR,BUILD);
  printk(DARK "(C) 2000, 2001 Michal Zalewski <lcamtuf@ids.pl>\n");
  printk(     "(C) 2000, 2001 Argante Development Team <argante@cgs.pl>\n\n");
  printk(NORM "Compiled by %s.\n\n", IDSTR);

#ifndef FAST
  usleep(1000000);
#endif

  init_taskman();
  jit_init();
  load_rules(-2);
  start_manager();
}

int main(int argc,char* argv[]) {
  if (argc>1) strncpy(script_name,argv[1],sizeof(script_name)-1);
  if (argc>2)
    if (chdir(argv[2])) {
      perror("set_root_directory");
      exit(1);
    }
  console_setup();
  boot_system();
  return 1;
}

