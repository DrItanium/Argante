/*

   Argante virtual OS
   ------------------

   System console stuff.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>
   Patched:    Artur Skura <arturs@people.pl> - readline support

*/

#define I_AM_THE_CONSOLE

#include <unistd.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/time.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#include <sched.h>
#endif

#include "config.h"
#include "console.h"
#include "manager.h"
#include "task.h"

void clear_console(void) {
/* YECK - Unix and GNU termcap differences are annoying */
#ifdef __USE_TERMCAP

  #ifdef __USE_GNU_TERMCAP	/* Define the GNU termcap lib */
    char *entry=NULL;		/* And work dynamically with tgetent() */
  #else
    char entry[2048];		/* Assume Unix termcap - isn't dynamic :( */
    char *caps;			/* Hold all capabilities here */
  #endif

  char *term, *cls;		/* Hold terminal variable and cls */
  int status;

  if(!(term = (char *)getenv("TERM"))) {
    fprintf(stderr, "No terminal set\n");
    exit(-1);
  }

  status = tgetent(entry, term);
  if(!status) {
    fprintf(stderr, "Unknown terminal: %s\n", term);
    exit(-1);
  }
  if(status<0) {
    fprintf(stderr, "Can't access termcap database\n");
    exit(-1);
  }

  #ifdef __USE_GNU_TERMCAP
    cls = tgetstr("cl", NULL);
  #else
    caps = (char *)malloc(strlen(entry)+1); /* Worst case scenario */
    cls = tgetstr("cl", &caps);
  #endif

  tputs(cls, 1, outc);

  #ifdef __USE_GNU_TERMCAP
    free(cls);			/* XXX: entry cannot be freed */
  #endif
#else
  printk("\033[1H\033[2J");
#endif
}

#ifdef __USE_TERMCAP
void outc(int c)
{
  printk("%c", c);
}
#endif

void console_setup(void) {
  clear_console();
#ifdef HAVE_READLINE
  using_history();
#endif
}


char cmd_buffer[4096];
int prompt_already;
int wait_for_task=-1;
int wait_time_started;
int wait_time_max;
int be_quick_dude;
int stupid_pid=-1;

int saved_be_quick;

#ifdef HAVE_READLINE
int running_already,use_readline;
char* read_stack;
char* read_tmp; 

int read_the_line(void* arg) {
  while (1) {
    while (read_tmp) usleep(1000);
    read_tmp=readline(PROMPT);
  }
  return 0;
}
#endif
  

char* get_command(void) {
  int cnt;
  char burp[10];

  if (wait_for_task>=0) {
    int ti;
    ti=time(0);
    if (wait_for_task>=MAX_VCPUS) {
      printk("Excessive VCPU number.\n");
      wait_for_task=-1;
      be_quick_dude=saved_be_quick;
      return 0;
    }
    if (wait_time_max < ti - wait_time_started) {
      printk("VCPU #%d wait timeout (%d seconds).\n",wait_for_task,wait_time_max);
      wait_for_task=-1;
      be_quick_dude=saved_be_quick;
      return 0;
    }
    if (cpu[wait_for_task].flags & VCPU_FLAG_USED) {
      fcntl(0,F_SETFL,O_NONBLOCK);
      if (!fgets(burp,10,stdin)) return 0;
      printk("VCPU #%d wait aborted by user.\n",wait_for_task);
      wait_for_task=-1;
      be_quick_dude=saved_be_quick;
    } else {
      wait_for_task=-1;
      be_quick_dude=saved_be_quick;
      return 0;
    }
  }

  if (script_name[0]) {
    scr=fopen(script_name,"r");
    if (!scr) {
      printk("-> cannot open script %s.\n",script_name);
      perror("fopen");
      script_name[0]=0;
      return 0;
    }
    printk("+> reading script %s...\n",script_name);
    script_name[0]=0; be_quick_dude=1;
  }

  if (scr) {
    if (!fgets(cmd_buffer,4095,scr)) {
      printk("+> script successfully executed.\n");
      fclose(scr); be_quick_dude=0;
      scr=0;
      return 0;
    }
    if (cmd_buffer[strlen(cmd_buffer)-1]=='\n') cmd_buffer[strlen(cmd_buffer)-1]=0;
    return cmd_buffer;
  }

#ifdef HAVE_READLINE

    if (!running_already) use_readline=isatty(0);

    if (use_readline) {

      if (!running_already) {
        running_already=1;
        read_stack=malloc(200000)+100000; // In the middle of the...
        stupid_pid=clone(read_the_line,read_stack,CLONE_VM,0);
      }

      if (read_tmp) {
        strncpy(cmd_buffer,read_tmp,sizeof(cmd_buffer)-1);
//        read_tmp=0;
        if (strlen(cmd_buffer))
          add_history(cmd_buffer);
      } else return 0;

      return cmd_buffer;  

  } else {
 
#endif

    if (!prompt_already) {
      printk("%s",PROMPT);
      fcntl(0,F_SETFL,O_NONBLOCK);
      prompt_already=1;
    }
    cnt=read(0,cmd_buffer,4095);
    if (cnt>0) {
      cmd_buffer[cnt]=0;
      if (cmd_buffer[cnt-1]=='\n') cmd_buffer[cnt-1]=0;
      prompt_already=0;
      return cmd_buffer;
    }
    return 0;
 
#ifdef HAVE_READLINE

  } /* if use_readline */

#endif  
  
}

