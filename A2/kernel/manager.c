/*
 * A2 Virtual Machine - manager thread & console
 * Copyright (c) 2001	James Kehl <ecks@optusnet.com.au>
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; version 2 of the License, with the
 * added restriction that it may only be converted to the version 2 of the
 * GNU General Public License.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "autocfg.h"
#include "compat/alloca.h"
#include "compat/bzero.h"
#include "compat/strtok_r.h"
#include "compat/usleep.h"
#include <stdio.h>
#include <stdlib.h>

/* for chdir */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#ifdef HAVE_DIR_H
#include <dir.h>
#endif
#endif

#include <string.h>
#include <signal.h>
#include <errno.h>

#ifndef NO_THREADS
#include <pthread.h>
#endif
/* must go after pthread.h */
#include "compat/time.h"

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

/* mallinfo() */
#ifdef HAVE_MALLINFO
#include <malloc.h>
#endif


#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "bcode.h"
#include "data_blk.h"
#include "exception.h"
#include "amemory.h"
#include "modload.h"
#include "hhac.h"
#include "cmd.h"
#include "cfdop.h"

#include "imageman.h"

static char *default_hac=NULL;

/*
key: - - RSN, x - never, * - done, ~ - a bit different from A1
	  ? - under consideration
?               * help
!               ? system statistics
$fn             * load binary image from file fn and run it on the first
                  VCPU available
%fn		? as above, loads a task in RESPAWN mode (it will be run
		  again if the process will be terminated by any command 
	          different than HALT)
>fn             * load library from file fn to a free slot
<id             * remove library in slot 'id'
#               ? list libraries with statistics
                  (supported syscalls, number of calls)
@fn             * run a console script
-nn             * kill a process on VCPU number nn
=nn             ? display statistics for a process on VCPU number nn
.               * system halt 
*nn             x execute nn system ticks without checking input
                  on management console; useful in scripts
:xx             ? subshell exit and execution of "xx"
|xx             * "nothing" - comment in scripts
^               ~ reread HAC table
~		* echo (for scripts)
w nn tmout      * wait for process nn termination fot tmout seconds
*/

void do_cycle(struct vcpu *curr_cpu) {
	anyval *a1, *a2;
	struct bcode_op *bop;
	unsigned z, z2, i;
	z2=z=0;
hi:
	/* don't test cancel so often... for speed */
	if (++z > curr_cpu->priority) {
		CANCEL;
		z=0;
		if (++z2 > 100) {
			IDLE;
			z2=0;
		}
	}

	/* Change pages? This is the place it counts most (imageman.c is the only other)*/
#ifndef NO_CODE_PAGES
	if (CP_PAGE_OF(curr_cpu->ip) != curr_cpu->cp_curr_id) {
		if (CP_PAGE_OF(curr_cpu->ip) >= curr_cpu->cp_count)
			throw_except(curr_cpu, ERR_OUTSIDE_CODE);
		/* otherwise we're more or less fine...
		 * hope cp_all's cleared properly if unused tho */
		memcpy(&curr_cpu->cp_curr, &curr_cpu->cp_all[CP_PAGE_OF(curr_cpu->ip)], sizeof(struct codepage));
		curr_cpu->cp_curr_id=CP_PAGE_OF(curr_cpu->ip);
	}
#endif

	if (CP_DATA_OF(curr_cpu->ip) >= curr_cpu->cp_curr.size)
		throw_except(curr_cpu, ERR_OUTSIDE_CODE);

	bop=&curr_cpu->cp_curr.bcode[CP_DATA_OF(curr_cpu->ip)];

	/* Evaluate A1 */
	if (bop->type & TYPE_A1(TYPE_REGISTER))
		a1=&curr_cpu->reg[bop->a1.val.u];
	else
		a1=&bop->a1;

	if (bop->type & TYPE_A1(TYPE_POINTER)) {
		if (jit_protflags[(unsigned) bop->bcode] & A1_READ_WRITE)
			a1=mem_rw(curr_cpu, a1->val.u);
		else
			a1=(anyval *) /* just trust JIT does const */ mem_ro(curr_cpu, a1->val.u); 
	}

	/* Evaluate A2 */
	if (bop->type & TYPE_A2(TYPE_REGISTER))
		a2=&curr_cpu->reg[bop->a2.val.u];
	else
		a2=&bop->a2;

	/* And yes, there are ops which modify A2... */
	if (bop->type & TYPE_A2(TYPE_POINTER)) {
		if (jit_protflags[(unsigned) bop->bcode] & A2_READ_WRITE)
			a2=mem_rw(curr_cpu, a2->val.u);
		else
			a2=(anyval *) mem_ro(curr_cpu, a2->val.u);
	}
	
/*	i=(bop->type & TYPE_A1(TYPE_VALMASK)) + ((bop->type & TYPE_A2(TYPE_VALMASK)) >> 4) * 3;
	i=i + (unsigned) bop->bcode * 9; */

	/* Now, run JIT */
	i=curr_cpu->ip;
	curr_cpu->next_ip=i + 1;
	(jit_calls[curr_cpu->cp_curr.jitoffs[CP_DATA_OF(i)]])(curr_cpu, a1, a2);
	/* When exceptions are thrown and cancellation occurs
	 * we want IP to be correct, right? */
	curr_cpu->ip=curr_cpu->next_ip;
	goto hi;
}

#ifndef NO_XLIST

struct sdes {
  char* name;
  unsigned num;
  int type;
};

struct sdes xlist[]={
#include "xlist.h"
};
#define SCNUM (sizeof(xlist)/sizeof(struct sdes))

#endif

static int vcpu_do_code(struct vcpu *curr_cpu) {
	unsigned xp;
	int origip;
	xp=(int) &xp; /* Volatile blocker */
	origip=(int) &origip;
	
	while(1) {
	/* How To Handle Exceptions(tm):
	 * 1. throw_except returns here.
	 * 2. No xip for this level? Pop the stack until one appears.
	 * 3. If we have an xip, set new IP there, and zero old xip.
	 * 4. Out of stack? It didn't handle it, then.
	 */
		if((xp=setjmp(curr_cpu->onexcept))) {
			if (xp==X_CPUSHUTDOWN) {
				printk2(PRINTK_WARN, "Task committed suicide (HALT or RAISE -1).\n");
				return 0;
			}
			origip=curr_cpu->ip;
			while (!curr_cpu->xip) {
				if(setjmp(curr_cpu->onexcept))
				{
#ifndef NO_XLIST
					size_t i;
					for(i=0;i<SCNUM;i++) {
						if (xlist[i].num == xp) {
							printk2(PRINTK_ERR, "Unhandled exception %d (%s), origin 0x%x. Murdered.\n", xp, xlist[i].name, origip);
							return 1;
						}
					}
#endif
					printk2(PRINTK_ERR, "Unhandled exception %d, origin 0x%x. Murdered.\n", xp, origip);
					return 1;
				}
				pop_ip_from_stack(curr_cpu, 1);
			}
			curr_cpu->ip=curr_cpu->xip;
			/* Set r31 to error no. */
			curr_cpu->reg[A2_REGISTERS - 1].val.u=xp;
			curr_cpu->xip=0;
		}
		do_cycle(curr_cpu);
	}
}

#ifndef NO_THREADS

/* Waitfor - a bit of fun 'mahgick' */
static pthread_mutex_t waitmutex=PTHREAD_MUTEX_INITIALIZER; 
static pthread_cond_t waitcond=PTHREAD_COND_INITIALIZER;

static void *vcpu_waitfor(void *v) {
	pthread_t *t=v;

	/* If there's a timeout, we can't hold the lock!
	 * Ugly! */
	pthread_mutex_lock(&waitmutex);
	pthread_mutex_unlock(&waitmutex);
	/* Admittedly, it detaches on termination.
	 * But that's good enough... EINVAL will do! */
	pthread_join(*t, NULL);
	pthread_cond_broadcast(&waitcond);
	return NULL;
}

static void waitforthread(pthread_t *t, int tmout) {
	pthread_t newthread;
	struct timespec tspec;
	int e;

	pthread_mutex_lock(&waitmutex);
	if (pthread_create(&newthread, NULL, &vcpu_waitfor, t)) {
		pthread_mutex_unlock(&waitmutex);
		printk2(PRINTK_ERR, "pthread_create failed\n");
		return;
	}

	tspec.tv_sec=time(NULL) + tmout;
	tspec.tv_nsec=0;
	if ((e=pthread_cond_timedwait(&waitcond, &waitmutex, &tspec))) {
		if (e==ETIMEDOUT) printk2(PRINTK_WARN, "timed out.\n");
		else printk2(PRINTK_WARN, "signal interrupt?\n");
		pthread_cancel(newthread);
	}
	pthread_join(newthread, NULL);
	pthread_mutex_unlock(&waitmutex);
}

static struct vcpu cpus[A2_MAX_VCPUS];
/* Racy pacy. But it's atomic at least...? */
static pthread_t cpu_thread[A2_MAX_VCPUS];

static void *vcpu_run(void *v) {
	struct vcpu *curr_cpu=v;
	vcpu_do_code(curr_cpu);
	/* Everything after this has to be prepared to
	 * be cancelled halfway and restarted... */
	vcpu_modules_stop(curr_cpu);
	imageman_unload(curr_cpu);
	/* Don't waste stack */
	curr_cpu->status=STATUS_UNUSED;
	pthread_detach(pthread_self());
	return NULL;
}

/* The Global Lock :) */
static void cancel_all_vcpus() {
	int i, err;
/* printk("<.> acquiring 'global lock'...\n"); */
	for(i=0;i<A2_MAX_VCPUS;i++) {
		if (cpus[i].status==STATUS_UNUSED) continue;
		err=pthread_cancel(cpu_thread[i]);
		if (err)
			printk2(PRINTK_ERR, "thread cancel failed\n");
		else {
			err=pthread_join(cpu_thread[i], NULL);
			if (err)
			printk2(PRINTK_ERR, "thread join failed for global lock: %s\n",
				(err==ESRCH) ? "no such thread" : (err==EDEADLK) ?
				"deadlock" : (err==EINVAL) ? "invalid" : "other");
		}
	}
}

static void continue_all_vcpus() {
	int i;
/* printk("<.> releasing 'global lock'...\n"); */
	for(i=0;i<A2_MAX_VCPUS;i++) {
		if (cpus[i].status==STATUS_UNUSED) continue;
		if (pthread_create(&cpu_thread[i], NULL, &vcpu_run, &cpus[i])) {
			printk2(PRINTK_ERR, "pthread_create failed\n");
			imageman_unload(&cpus[i]);
			cpus[i].status=STATUS_UNUSED;
		}
	}
}

static void func_all_vcpus(void *func(void *)) {
	int i, err;
	for(i=0;i<A2_MAX_VCPUS;i++) {
		if (cpus[i].status==STATUS_UNUSED) continue;
		if (pthread_create(&cpu_thread[i], NULL, func, &cpus[i]))
			printk2(PRINTK_ERR, "pthread_create failed\n");
		else {
			err=pthread_join(cpu_thread[i], NULL);
			if (err) printk2(PRINTK_ERR, "thread join failed\n");
		}
	}
}
#else
static inline void cancel_all_vcpus(void) {}
static inline void continue_all_vcpus(void) {}
static inline void func_all_vcpus(void *func(void *)) {}
#endif


/* This is for debugging the interpreter with no pthreads getting in the way.
 * It's also invaluble for tracing leaks.
 */
#ifdef NO_THREADS
static void debug_execution(char *ln) {
	struct vcpu cpu;
	char *reent, *tok;
	const struct cfdop_1 *cfdtbl;
	bzero(&cpu, sizeof(struct vcpu));
	cpu.status=STATUS_RUN;
	/* Now we have DL - you can load multiple images with :'s between */
	tok=strtok_r(ln, ":", &reent);
	if (!tok || !*tok) {
		printk2(PRINTK_INFO, "loading %s...\n", ln);
		if (!imageman_loadimage(ln, &cpu)) {
			cpu.status=STATUS_UNUSED;
			imageman_unload(&cpu);
			printk2(PRINTK_ERR, "image load failed\n");
			return;
		}
	} else while(tok) {
		printk2(PRINTK_INFO, "loading %s...\n", tok);
		if (!imageman_loadimage(tok, &cpu)) {
			cpu.status=STATUS_UNUSED;
			imageman_unload(&cpu);
			printk2(PRINTK_ERR, "image load failed\n");
			return;
		}
		tok=strtok_r(NULL, ":", &reent);
	}
	/* Load HAC */
	hac_init(&cpu);
	if (default_hac) {
		FILE *f;
		f=fopen(default_hac, "r");
		if (!f) perror("fopen");
		else {
			hac_loadfile(&cpu, f);
			fclose(f);
			printk2(PRINTK_INFO, "HAC loaded from %s\n", default_hac);
		}
	}
	/* Start modules (waiting for finish) */
	vcpu_modules_start(&cpu);
	/* Volunteer a VFD...? */
	cfdtbl=cfdop1_fddesc_get(*((int *) "TCON"));
	if (cfdtbl) {
		cpu.reg[0].val.s=cfdtbl->fd_create(&cpu, NULL, fileno(stdin), fileno(stdout));
	} else {
		printk2(PRINTK_INFO, "TCON unavailable\n");
	}
	/* Start CPU */
	vcpu_do_code(&cpu);
	/* Done */
	vcpu_modules_stop(&cpu);
	imageman_unload(&cpu);
	cpu.status=STATUS_UNUSED;
	return;
}
#endif

static int do_script(char *script);

static void do_line(char *ln) {
	char t=*ln;
	int err;
	ln++;
	switch(t) {
		/* debug only thing */
/*		case 's': 
			cancel_all_vcpus();
			sleep(5);
			continue_all_vcpus();
			return; */
		case '?': {
			const char *s;
		s=
"?               - help\n"
"!               - system statistics\n"
".               - system halt\n"
"@fn             - run a console script\n"
"^fn nn          - set default HAC file, or change nn's HACtable to fn\n"
"$fn             - load binary image from file fn\n"
#ifndef NO_THREADS
"-nn             - kill a process on VCPU number nn\n"
#endif
">fn             - load library from file fn\n"
"<id             - remove library in slot 'id'\n"
/*"=id             - reload library in slot 'id'\n"*/
"|xx             - comment, for scripts\n"
"~xx             - echo, for scripts\n"
#ifndef NO_THREADS
"wnn dd		 - wait for cpu NN termination for dd seconds\n"
#endif
		;
			printk(s);
			return;
		  }
		case '@': {
			do_script(ln);
			return;
		}
		case '!': {
#ifdef HAVE_MALLINFO
			int i;
			i=mallinfo().uordblks;
			printk2(PRINTK_INFO, "%d bytes in mallocated memory\n", i);
#else
			printk2(PRINTK_INFO, "malloc statistics unavailable\n");
#endif
		}
		case '|': return; /* Comment */
		case '~': puts(ln); return;
		case '^': { /* Load HAC, or change default HAC */
			char file[128];
			int cpuid;
			FILE *f;
			err=sscanf(ln, "%127s %d", file, &cpuid);
			file[sizeof(file) - 1]=0;

			if (!err || !strlen(ln)) {
				printk2(PRINTK_ERR, "^ usage:\n"
					" <FILE> set default HAC filename\n"
					" <FILE> <VCPU> alter VCPU's HAC\n"
					);
				return;
			}
			if (err > 1) {
#ifndef NO_THREADS
				if (cpuid >= A2_MAX_VCPUS) {
					printk2(PRINTK_ERR, "Invalid VCPU\n");
					return;
				}

				if (cpus[cpuid].status==STATUS_UNUSED) {
					printk2(PRINTK_ERR, "That VCPU is already dead.\n");
					return;
				}
				f=fopen(file, "r");
				if (!f) {
					perror("fopen");
					return;
				}
				hac_loadfile(&cpus[cpuid], f);
				fclose(f);
				printk2(PRINTK_INFO, "%d's HAC loaded from %s\n", cpuid, file);
#else
				printk2(PRINTK_ERR, "Multithreading is disabled, so which VCPU could you possibly mean?\n");
#endif
				return;
			}
			/* Test file readability etc. */
			f=fopen(file, "r");
			if (!f) {
				perror("fopen");
				return;
			}
			fclose(f);
			printk2(PRINTK_INFO, "Default HAC set to %s\n", file);
			if (default_hac) free(default_hac);
			default_hac=strdup(file);
			return;
		}
#ifndef NO_THREADS
		case '-': {
			char *ret;
			unsigned cpuid;
			if (!*ln) {
				printk2(PRINTK_ERR, "- usage: -<VCPU>\n");
				return;
			}
			cpuid=strtoul(ln, &ret, 0);
			if (*ret) {
				printk2(PRINTK_ERR, "- usage: -<VCPU>\n");
				return;
			}
			if (cpuid >= A2_MAX_VCPUS) {
				printk2(PRINTK_ERR, "Invalid VCPU\n");
				return;
			}

			if (cpus[cpuid].status==STATUS_UNUSED) {
				printk2(PRINTK_ERR, "That VCPU is already dead.\n");
				return;
			}
			/* pthread_cancel won't break if a thread quits just
			 * before this */
			err=pthread_cancel(cpu_thread[cpuid]);
			if (err)
				printk2(PRINTK_ERR, "thread cancel failed\n");
			else {
				err=pthread_join(cpu_thread[cpuid], NULL);
				if (err)
					printk2(PRINTK_ERR, "thread join failed\n");
				/* Now vcpu_stop it */
				if (pthread_create(&cpu_thread[cpuid], NULL, &vcpu_modules_stop, &cpus[cpuid]))
					printk2(PRINTK_ERR, "pthread_create failed\n");
				else {
					err=pthread_join(cpu_thread[cpuid], NULL);
					if (err) printk2(PRINTK_ERR, "thread join failed\n");
				}
				/* Because it's been cancelled, it hasn't freed */
				imageman_unload(&cpus[cpuid]);
				cpus[cpuid].status=STATUS_UNUSED;
			}
			return;
		}
#endif
		case '$': {
#ifndef NO_THREADS
			int i;
			for(i=0;i<A2_MAX_VCPUS;i++) {
				/* It might become zero after this,
				 * but it won't become one. So it's safe!(?) */
				if (cpus[i].status==STATUS_UNUSED) {
					char *reent, *tok;
					bzero(&cpus[i], sizeof(struct vcpu));

					cpus[i].status=STATUS_RUN;
					/* Now we have DL - you can load multiple images with :'s between */
					tok=strtok_r(ln, ":", &reent);
					if (!tok || !*tok) {
						printk2(PRINTK_INFO, "loading %s...\n", ln);
						if (!imageman_loadimage(ln, &cpus[i])) {
							cpus[i].status=STATUS_UNUSED;
							imageman_unload(&cpus[i]);
							printk2(PRINTK_ERR, "image load failed\n");
							return;
						}
					} else while(tok) {
						printk2(PRINTK_INFO, "loading %s...\n", tok);
						if (!imageman_loadimage(tok, &cpus[i])) {
							cpus[i].status=STATUS_UNUSED;
							imageman_unload(&cpus[i]);
							printk2(PRINTK_ERR, "image load failed\n");
							return;
						}
						tok=strtok_r(NULL, ":", &reent);
					}
					/* Load HAC */
					hac_init(&cpus[i]);
					if (default_hac) {
						FILE *f;
						f=fopen(default_hac, "r");
						if (!f) perror("fopen");
						else {
							hac_loadfile(&cpus[i], f);
							fclose(f);
							printk2(PRINTK_INFO, "HAC loaded from %s\n", default_hac);
						}
					}
					/* Start modules (waiting for finish) */
					if (pthread_create(&cpu_thread[i], NULL, &vcpu_modules_start, &cpus[i]))
						printk2(PRINTK_ERR, "pthread_create failed\n");
					else {
						err=pthread_join(cpu_thread[i], NULL);
						if (err) printk2(PRINTK_ERR, "thread join failed\n");
					}
					/* Start CPU */
					if (pthread_create(&cpu_thread[i], NULL, &vcpu_run, &cpus[i])) {
						printk2(PRINTK_ERR, "<+> pthread_create failed\n");
						imageman_unload(&cpus[i]);
						cpus[i].status=STATUS_UNUSED;
					}
					return;
				}
			}
			printk2(PRINTK_ERR, "No free VCPUs\n");
#else
			debug_execution(ln);
#endif
			return;
		}
		case '>': { /* Load library */
			unsigned lid;
			cancel_all_vcpus();
			lid=module_dyn_load(ln);
			if (lid == ((unsigned) -1)) {
				printk2(PRINTK_ERR, "Module load failed.\n");
				continue_all_vcpus();
				return;
			}
			/* Success! Run VCPU starts */
			vcpu_set_moduleid(lid);
			func_all_vcpus(vcpu_module_start);
			continue_all_vcpus();
			return;
		}
		case '<': { /* Unload library */
			unsigned lid;
			char *ret;
			if (!*ln) {
				printk2(PRINTK_ERR, "< usage: <<LID>\n");
				return;
			}
			lid=strtoul(ln, &ret, 0);
			if (*ret) {
				printk2(PRINTK_ERR, "< usage: <<LID>\n");
				return;
			}
			if (lid >= A2_MAX_RSRVD) {
				printk2(PRINTK_ERR, "Invalid LID\n");
				return;
			}
			printk2(PRINTK_INFO, "Unloading %d\n", lid);
			cancel_all_vcpus();
			/* Run VCPU stops */
			vcpu_set_moduleid(lid);
			func_all_vcpus(vcpu_module_stop);
			/* Try shutdown */
			if (!module_dyn_unload(lid)) {
				/* Success! restart threads then */
				continue_all_vcpus();
				return;
			}
			printk2(PRINTK_ERR, "Module unload failed. Attempting reload (state data has been lost)\n");
			func_all_vcpus(vcpu_module_start);
			continue_all_vcpus();
			return;
		}
		case 'w': {
#ifndef NO_THREADS
			unsigned cpuid;
			unsigned tmout;
			err=sscanf(ln, "%d %d", &cpuid, &tmout);
			if (err < 2 || !strlen(ln)) {
				printk2(PRINTK_ERR, "invalid argument\n");
				return;
			}
			if (cpuid >= A2_MAX_VCPUS) {
				printk2(PRINTK_ERR, "Invalid VCPU\n");
				return;
			}
			if (cpus[cpuid].status==STATUS_UNUSED) {
				printk2(PRINTK_ERR, "That VCPU is already dead.\n");
				return;
			}
			if (tmout > 5 * 60 * 60) {
				printk2(PRINTK_WARN, "It probably isn't a good idea to sleep that long."
					" Doing 5min instead.\n");
				tmout=5*60*60;
			} else if (tmout < 1) {
				printk2(PRINTK_WARN, "Minimum timeout is 1sec, ok?\n");
				tmout=1;
			}
			printk2(PRINTK_INFO, "Waiting for VCPU %d for %d seconds\n", cpuid, tmout);
			waitforthread(&cpu_thread[cpuid], tmout);
#endif
			return;
		}
		case '.': { /* Shutdown */
#ifndef NO_THREADS
			int i;
#endif
			printk2(PRINTK_WARN, "Shutting down...\n\n");

			/* Stop everything */
			cancel_all_vcpus();

#ifndef NO_THREADS
			/* Run VCPU stops */
			for(i=0;i<A2_MAX_VCPUS;i++) {
				if (cpus[i].status==STATUS_UNUSED) continue;
				/* VCPU_stop */
				if (pthread_create(&cpu_thread[i], NULL, &vcpu_modules_stop, &cpus[i]))
					printk2(PRINTK_ERR, "pthread_create failed\n");
				else {
					err=pthread_join(cpu_thread[i], NULL);
					if (err) printk2(PRINTK_ERR, "thread join failed\n");
				}

				imageman_unload(&cpus[i]);
				cpus[i].status=STATUS_UNUSED;
			}
#endif
			/* Unloading dynamic modules not possible. */
			printk2(PRINTK_INFO, "Goodbyte... .  .  .\n"); /* Good nite? */
			free(default_hac);

#ifdef HAVE_MALLINFO
			printk2(PRINTK_INFO, "%d bytes in mallocated memory\n", mallinfo().uordblks);
#endif
			exit(0);
		}
		default:
			  printk2(PRINTK_ERR, "What?! Type ? for help.\n");
	}
}


static int do_script(char *script) {
	/* Run boot script */
	FILE *f;
	char *search;
	char buf[1000];
	f=fopen(script, "r");
	if (!f) {
		perror("fopen");
		return 1;
	}
	while(fgets(buf, sizeof(buf), f)) {
		search=strchr(buf, '\n');
		if (search) *search=0;
		do_line(buf);
	}
	fclose(f);
	return 0;
}

static int boot(void)
{
#define BRI  "\x1b[1m"
#define DARK "\x1b[2m"
#define NORM "\x1b[0m"
#define RED "\x1b[31m"
#define BLU "\x1b[34m"
#define NAMECOL BRI RED

  printf("Booting A2 version %s:\n\t\""
/* Insert aphorism here. I think this one's better for the prealpha. */
   "I accept chaos. I am not sure whether it accepts me."
/* "[We] use bad software and bad machines for the wrong things." */
		  "\"\n\n", A2_VERSION);

  printf("(C) 2001 James Kehl <ecks@optusnet.com.au>\n"
	"(C) 2000, 2001 Michal Zalewski <lcamtuf@bos.bindview.com>\n"
	"(C) 2000, 2001 Argante Development Team <argante@linuxpl.org>\n\n");

#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif
	/* Boot up static modules */
	module_static_init();
#ifndef NO_THREADS
	{
		int i;
		for(i=0;i<A2_MAX_VCPUS;i++) cpus[i].status=STATUS_UNUSED;
	}
#endif
	return 0;
}

int main(int argc, char **argv)
{
	char *search;
	static char buf[1000];
#ifdef USE_READLINE
	char *m;
#endif
	ALLOCA_STACK;

	/* CD to the argante exe directory */
	search=strrchr(argv[0], '/');
	if (search) {
		char *dir;
		int i;
		i=search - argv[0];
		dir=alloca(i + 1);
		memcpy(dir, argv[0], i);
		dir[i]=0;
		chdir(dir);
	}
	boot();

	if (argc > 2 && argv[2]) default_hac=strdup(argv[2]);
	do_script("conf/scripts/argboot.scr");

	/* Single-thread (well, 3 at most :), batch mode */
	if (argc > 1 && argv[1]) {
		sprintf(buf, "!");
		do_line(buf);
		sprintf(buf, "$%s", argv[1]);
		do_line(buf);
		sprintf(buf, "!");
		do_line(buf);
		sprintf(buf, "w0 180");
		do_line(buf);
		sprintf(buf, ".");
		do_line(buf);
		return 0;
	}

#ifdef USE_READLINE
	using_history();
	while((m=readline("[Agt+] "))) {
		search=strchr(m, '\n');
		if (search) *search=0;
		/* A2 0.008. Strip trailing spaces when using readline. */
		search=strchr(m, 0);
		while(search > m && *(search-1)==' ') search--;
		*search=0;
		do_line(m);
	        if (strlen(m)) add_history(m);
		free(m);
	}
#else
	/* Ensures fputs happens before fgets, but the only answer that counts is gets */
	while((fputs("[Agt+] ", stdout) & 0) || fgets(buf, sizeof(buf), stdin)) {
		search=strchr(buf, '\n');
		if (search) *search=0;
		do_line(buf);
	}
#endif
	return 0;
}
