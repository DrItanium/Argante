/*

   Argante virtual OS
   ------------------

   System console I/O macros and decls

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#ifndef __HAVE_CONSOLE_H
#define __HAVE_CONSOLE_H

#include <stdio.h>
#include <stdlib.h>

#define printk(x...)	 fprintf(stderr,x)

void clear_console(void);
void console_setup(void);
char* get_command(void);
#ifdef __USE_TERMCAP
  extern char *tgetstr();
  extern int tgetent(), tputs();
  void outc(int);
#endif

#endif /* __HAVE_CONSOLE_H */
