/*

   Argante virtual OS
   ------------------

   Dynamic linker code

   Status: undone

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

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

#define SEPARATOR	CMD_HALT


struct link_entry (*msg)[MAX_MEMBLK];
unsigned int msgsiz;
    

int parse_dynamic(int fd) {
  // Parse dynamic section of main executable.
}


int load_library() {
  // Add library (relocate, load messages)
};


int do_resolving() {
  // Parse all dependencies, bail out on failure.
}


void build_table() {
  // Build permanent symbol table
}


void free_all() {
  // "panic" handler.
}
