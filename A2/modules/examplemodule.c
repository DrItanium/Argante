/*
 * An Example A2 Module.
 *
 * ++++++++++++ PLEASE NOTE ++++++++++++++
 * I wrote this as my first module; it is left to illustrate
 * how to create yours. However, this does not mean this file
 * should be taken at face value. It is unmaintained and therefore
 * contains bugs, bad practice, no HAC, etc. etc.
 *
 * Please look at the live code for the 'right' way to do things.
 * 						JSK
 */
#include "autocfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "taskman.h"
#include "printk.h"
#include "module.h"
#include "exception.h"
#include "amemory.h"

/*
 * Modules have to be reentrant. Please remember that.
 * However, non-reentrant functions are OK if you can
 * be absolutely positively sure they only get used in
 * the manager thread.
 */
static int stored_lid; /* Library ID - per-vcpu storage */

/*
 * OK. Hand-coding the module load sequence is dumb. Not only
 * is it harder to maintain, but if someone clever like me came
 * along and said "I can make syscalls 100% faster using a hash
 * table" you'd have to hand-modify every single module.
 *
 * Additionally - we want A2 to run on windows (moral objections
 * aside) and it may well be neccesary to abandon dynamic linking
 * for that. Hence - the autogenerator.
 *
 * Format is as JIT, only: */
/*. SYSCALL_NAME [ID] ["SYS2"] = func_name */

/* If you omit the ID, the generator will create one for you.
 * Of course it depends on the phase of the moon, so you
 * should make it permanent ASAP. */
/* Omit SYS2 if your syscall cannot work with SYSCALL2. */

/* Module-specific init goes here; should always be static.
 * Return 1 for failure, 0 for success. */
static inline int module_internal_init(int lid)
{
	stored_lid=lid;
	printk2(PRINTK_INFO, "Hi, I'm the example module\n");
	return 0;
}

static inline void module_internal_vcpu_start(struct vcpu *cpu) {}
static inline void module_internal_vcpu_stop(struct vcpu *cpu) {}
static inline void module_internal_shutdown() {}

/*! assigned 1 - 100 */ 

/*! IO_PUTSTRING 0x1 = io_putstring */
static void io_putstring(SYSCALL_ARGS) {
	int i;
	char *x=NULL;
	anyval *p;
	i=0;
	while(i < curr_cpu->reg[1].val.u)
	{
		if (!(i % 4)) {
			p=mem_ro(curr_cpu, curr_cpu->reg[0].val.u + i / 4);
			x=(char *) &p->val.u;
		} else {
			x++;
		}
		i++;
		fputc(*x, stdout);
	}
}

/*! IO_PUTINT 0x2 SYS2 = io_putint */
static void io_putint(SYSCALL_ARGS) {
	printf("%ld", arg->val.u);
}

/*! IO_PUTFLOAT 0x4 SYS2 = io_putfloat */
static void io_putfloat(SYSCALL_ARGS) {
	printf("%g", arg->val.f);
}

/*! IO_PUTHEX 0x5 SYS2 = io_puthex */
static void io_puthex(SYSCALL_ARGS) {
	printf("%lx", arg->val.u);
}
/*! IO_PUTCHAR 0x3 SYS2 = io_putchar */
static void io_putchar(SYSCALL_ARGS) {
	printf("%c", (char) arg->val.u);
}

/*! LOCAL_GETTIME 301 = local_gettime */
static void local_gettime(SYSCALL_ARGS) {
	/* LOCAL_GETTIME */
	struct timezone tz;
	struct timeval tv;
	gettimeofday(&tv,&tz);
	
	curr_cpu->reg[0].val.u=tv.tv_sec;
	curr_cpu->reg[1].val.u=tv.tv_usec;
}

/* Incorporate generated tables */
#include "examplemodule.h"
