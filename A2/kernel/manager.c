/*
 * 'new' Argante2 manager core
 *
 * Spin the wheel round and round:
 * shadows crawl along the ground;
 * sun comes up and sun goes down;
 * spin the wheel round and round.
 */
#include "autocfg.h"
#include "compat/alloca.h"
#include "compat/bzero.h"
// #include "compat/strtok_r.h"
#include "compat/usleep.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
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
#include "file.h"

#include "imageman.h"
#include "manager.h"

#include <assert.h>

/*
 * Here's the plan:
 * you first 'lock' a cpu, meaning nobody else can mess with it.
 * then you mess with it - load bcode etc.
 * then you run it
 * then if it dies, you can debug it
 * finally you unlock it
 * then the system does away with the memory.
 */

#define ORPHANED_ID ((unsigned short) -1)

/* XXX: should go in (auto)config.h */
#define A2_MODULE_DIR "modules/"

struct pending_sock {
	char socktype[A2_CFDDESC_LEN];
	char *extradesc;
	flexlisock sock;
	struct pending_sock *next;
};

/* NOTE:
 * There's no multithread locking here. There's only one manager thread.
 * If you want to (ugh) use a thread per connection you will have to throw
 * all this out.
 */

struct metacpu {
	unsigned short ownerid; /* -1 = orphaned */
	unsigned short status;
	struct vcpu *cpu;
	struct pending_sock *li_first;
	struct pending_sock *li_last;
	pthread_t thread;
	/* This would be mutexed, if it wasn't an atomic type. */
	unsigned exit_type;
};

/* if cpu==NULL, ownerid=ORPHANED_ID, status=STATUS_STOPPED. */

static struct metacpu cpu_array[A2_MAX_VCPUS];

static int flexsock_available=0;

/* Yuck. Should go in agents (but then they'd be sys-dependant!?) */
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

/*********************
 * UTILITY FUNCTIONS *
 *********************/

static const char *human_exception_name(unsigned xid) {
#ifndef NO_XLIST
	size_t i;
	for(i=0;i<SCNUM;i++) {
		if (xlist[i].num == xid) return xlist[i].name;
	}
#endif
	return NULL;
}

static void drop_pending_socks(unsigned cpuid) {
	struct pending_sock *s, *tmp;
	s=cpu_array[cpuid].li_first;
	while(s) {
		FXS_CloseListener(s->sock);
		if (s->extradesc) free(s->extradesc);
		tmp=s->next;
		free(s);
		s=tmp;
	}
	cpu_array[cpuid].li_first=cpu_array[cpuid].li_last=NULL;
}

static void free_cpu(unsigned cpuid) {
	/* Dumb internal consistency things (should be redundant) */
	assert(cpu_array[cpuid].status==STATUS_STOPPED);
	assert(cpu_array[cpuid].ownerid==ORPHANED_ID);
	/* Someone trying to leak our memory? */
	drop_pending_socks(cpuid);
	/* Run destroy sc handlers */
	vcpu_modules_stop(cpu_array[cpuid].cpu);
	imageman_unload(cpu_array[cpuid].cpu);
	/* Free cpu struct */
	free(cpu_array[cpuid].cpu);
	cpu_array[cpuid].cpu=NULL;
}

static void modstart_all_vcpus(unsigned moduleid) {
	int i;
	for(i=0;i<A2_MAX_VCPUS;i++) {
		/* Fix all cpus, even stopped ones. */
		if (!cpu_array[i].cpu) continue;
		vcpu_module_start(cpu_array[i].cpu, moduleid);
	}
}

static void modstop_all_vcpus(unsigned moduleid) {
	int i;
	for(i=0;i<A2_MAX_VCPUS;i++) {
		if (!cpu_array[i].cpu) continue;
		vcpu_module_stop(cpu_array[i].cpu, moduleid);
	}
}

/*************************** grunt cpu stuff **/

static void do_cycle(struct vcpu *curr_cpu, unsigned prio) {
	anyval *a1, *a2;
	struct bcode_op *bop;
	unsigned i, z=0, z2=0;

hi:
	/* don't test cancel so often... for speed */
	if (z++ > prio) {
		/* with single-stepping, prio=0 */
		if (z2++ > 100 || !prio) return;
		CANCEL;
		z=0;
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
	
	/* Now, run JIT */
	i=curr_cpu->prev_ip=curr_cpu->ip;
	curr_cpu->next_ip=i + 1;
	(jit_calls[curr_cpu->cp_curr.jitoffs[CP_DATA_OF(i)]])(curr_cpu, a1, a2);
	/* When exceptions are thrown and cancellation
	 * occurs we want IP to be correct, right? */
	curr_cpu->ip=curr_cpu->next_ip;
	goto hi;
}

static unsigned vcpu_do_run(struct vcpu *curr_cpu) {
	/* xp changed after call to setjmp. */
	volatile unsigned xp;
	jmp_buf ex_curr;

	/* Don't use EXCEPT_CATCH, it could allocate too often. */
	curr_cpu->onexcept=&ex_curr;
	
	while(1) {
	/*
	 * How To Handle Exceptions(tm):
	 * 1. throw_except returns here.
	 * 2. No xip for this level? Pop the stack until one appears.
	 * 3. If we have an xip, set new IP there, and zero old xip.
	 * 4. Out of stack? It didn't handle it, then.
	 */
		if ((xp=setjmp(ex_curr)) == 0) {
			do_cycle(curr_cpu, curr_cpu->priority);
		} else {
			if (xp == X_CPUSHUTDOWN) return EXITTYPE_HALT;
			/* Set r31 to error no. */
			curr_cpu->reg[A2_REGISTERS - 1].val.u=xp;
			if (setjmp(ex_curr) == 0) {
				while (!curr_cpu->xip) {
					pop_ip_from_stack(curr_cpu, 1);
				}
				curr_cpu->ip=curr_cpu->xip;
				curr_cpu->xip=0;
			} else return EXITTYPE_FAIL;
		}
		/* Either the requisite number of cycles completed successfully
		 * or an exception was thrown. In any case, realize the 'priority'. */
		IDLE;
	}
}

static unsigned vcpu_do_step(struct vcpu *curr_cpu) {
	/* xp changed after call to setjmp. */
	volatile unsigned xp;
	jmp_buf ex_curr;

	/* Don't use EXCEPT_CATCH, it could allocate too often. */
	curr_cpu->onexcept=&ex_curr;
	
	if ((xp=setjmp(ex_curr)) == 0) {
		do_cycle(curr_cpu, 0);
	} else {
		if (xp == X_CPUSHUTDOWN) return EXITTYPE_HALT;
		/* Set r31 to error no. */
		curr_cpu->reg[A2_REGISTERS - 1].val.u=xp;
		if (setjmp(ex_curr) == 0) {
			while (!curr_cpu->xip) {
				pop_ip_from_stack(curr_cpu, 1);
			}
			curr_cpu->ip=curr_cpu->xip;
			curr_cpu->xip=0;
			return EXITTYPE_EXCEPT;
		} else return EXITTYPE_FAIL;
	}

	/* I don't think we need bother calling IDLE. I/O and thread
	 * latencies should provide more than enough slowdown... */
	return EXITTYPE_STEP;
}


static void *vcpu_run(void *v) {
	struct metacpu *mcpu=v;
	unsigned u;
	u=vcpu_do_run(mcpu->cpu);
	mcpu->exit_type=u;
	pthread_detach(pthread_self());
	return NULL;
}

static void *vcpu_step(void *v) {
	struct metacpu *mcpu=v;
	unsigned u;
	u=vcpu_do_step(mcpu->cpu);
	mcpu->exit_type=u;
	pthread_detach(pthread_self());
	return NULL;
}

/*************************** global lock **/

/* This is a gigantic hack. See modules.txt for details... */

static void cancel_all_vcpus(void) {
	unsigned i;
	int err;
	const char *emsg;
	for(i=0;i<A2_MAX_VCPUS;i++) {
		if (cpu_array[i].status == STATUS_STOPPED) continue;
		
		err=pthread_cancel(cpu_array[i].thread);
		if (!err) {
			err=pthread_join(cpu_array[i].thread, NULL);
			if (!err) continue;
			else emsg="join";
		} else emsg="cancel";

		/* Likely errors mean it already finished. Don't restart it. */
		/* XXX: tell owner state has changed */
		printk2(PRINTK_ERR, "thread %s failed for global lock: %s\n", emsg,
			(err==ESRCH) ? "no such thread" : (err==EDEADLK) ?
			"deadlock" : (err==EINVAL) ? "invalid" : "other");
		cpu_array[i].status=STATUS_STOPPED;
	}
}

static void continue_all_vcpus(void) {
	unsigned i;
	for(i=0;i<A2_MAX_VCPUS;i++) {
		if (cpu_array[i].status == STATUS_STOPPED)
			continue;
		else if (cpu_array[i].status == STATUS_RUNNING) {
			if (pthread_create(&cpu_array[i].thread, NULL, &vcpu_run,
						&cpu_array[i])) {
				printk2(PRINTK_ERR, "pthread_create failed\n");
				cpu_array[i].status = STATUS_STOPPED;
				/* XXX: Tell owner-agent status has changed */
			}
		} else if (cpu_array[i].status == STATUS_STEPPING) {
			if (pthread_create(&cpu_array[i].thread, NULL, &vcpu_step,
						&cpu_array[i])) {
				printk2(PRINTK_ERR, "pthread_create failed\n");
				cpu_array[i].status = STATUS_STOPPED;
				/* XXX: Tell owner-agent status has changed */
			}
		} else { /* ??? */
			cpu_array[i].status = STATUS_STOPPED;
			/* XXX: Tell owner-agent status has changed */
		}
	}
}

/*********************
 * MANAGER FUNCTIONS *
 *********************/

void man_initialize(void) {
	unsigned i;
	for(i=0;i<A2_MAX_VCPUS;i++) {
		cpu_array[i].ownerid=ORPHANED_ID;
		cpu_array[i].status=STATUS_STOPPED;
		cpu_array[i].cpu=NULL;
	}
	module_static_init();
	if (!FXS_Init()) {
		/* Don't let people upload sniffers */
		FXS_SetBindsAdvisory(1);
		/* Be more conservative than vconsole. Can't allow stdio */
		FXS_SetProtocols(
				FXS_PFLAG(FXST_UNIX_B) | FXS_PFLAG(FXST_UNIX_C)
				| FXS_PFLAG(FXST_TCP_B)  | FXS_PFLAG(FXST_TCP_C));
		flexsock_available=1;
	} else {
		printk2(PRINTK_ERR, "FlexSock library failed to initialize.\n");
	}
}

void man_shutdown(void) {
	unsigned i;

	printk2(PRINTK_INFO, "shutting down...\n");

	for(i=0;i<A2_MAX_VCPUS;i++) {
		if (cpu_array[i].cpu == NULL) continue;
		if (man_cpu_stop(cpu_array[i].ownerid, i) != MANERR_OK ||
			man_cpu_unlock(cpu_array[i].ownerid, i) != MANERR_OK) {
			printk2(PRINTK_ERR, "vcpu %d was not cleanly shut down.\n", i);
		}
	}
	
	printk2(PRINTK_INFO, "shutdown complete.\n");

	if (flexsock_available) FXS_Shutdown();
}

/*************************** cpus **/

int man_cpu_lock_new(unsigned ownerid, unsigned *cpuid) {
	unsigned i;
	for(i=0;i<A2_MAX_VCPUS;i++) {
		if (cpu_array[i].cpu != NULL) continue;
		
		cpu_array[i].cpu=malloc(sizeof(struct vcpu));
		if (!cpu_array[i].cpu) return MANERR_OOM;
		bzero(cpu_array[i].cpu, sizeof(struct vcpu));
		
		cpu_array[i].ownerid=ownerid;
		*cpuid=i;
		/* Initialize CPU */
		cpu_array[i].li_first=NULL;
		cpu_array[i].li_last=NULL;
		hac_init(cpu_array[i].cpu);
		vcpu_modules_start(cpu_array[i].cpu);
		return MANERR_OK;
	}
	return MANERR_NOCPU;
}

int man_cpu_lock_existing(unsigned ownerid, unsigned cpuid) {
	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].cpu == NULL) return MANERR_NOCPU;
	/* It's neccesary to allow one agent to steal another's VCPU.
	 * Otherwise the only way to kill someone else's cpu's is with ifconfig. */
	if (cpu_array[cpuid].ownerid != ORPHANED_ID) {
		if (cpu_array[cpuid].ownerid == ownerid) return MANERR_OK;
		/* XXX: Politeness dictates a message to the previous agent. */
		printk2(PRINTK_WARN, "VCPU %d stolen by client %d\n", cpuid, ownerid);
	}
	cpu_array[cpuid].ownerid = ownerid;
	return MANERR_OK;
}

int man_cpu_unlock(unsigned ownerid, unsigned cpuid) {
	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	
	cpu_array[cpuid].ownerid = ORPHANED_ID;

	if (cpu_array[cpuid].status==STATUS_STOPPED) free_cpu(cpuid);
	
	return MANERR_OK;
}

int man_cpu_run_full(unsigned ownerid, unsigned cpuid) {
	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status != STATUS_STOPPED) return MANERR_RUNNING;
	if (cpu_array[cpuid].li_first != NULL) return MANERR_VFDPEND;
	
	cpu_array[cpuid].exit_type=EXITTYPE_NONE;

	if (pthread_create(&cpu_array[cpuid].thread, NULL, &vcpu_run, 
				&cpu_array[cpuid])) {
		printk2(PRINTK_ERR, "pthread_create failed\n");
		return MANERR_GENERIC;
	}
	cpu_array[cpuid].status = STATUS_RUNNING;
	return MANERR_OK;
}

int man_cpu_run_step(unsigned ownerid, unsigned cpuid) {
	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status != STATUS_STOPPED) return MANERR_RUNNING;
	if (cpu_array[cpuid].li_first != NULL) return MANERR_VFDPEND;

	cpu_array[cpuid].exit_type=EXITTYPE_NONE;

	if (pthread_create(&cpu_array[cpuid].thread, NULL, &vcpu_step,
				&cpu_array[cpuid])) {
		printk2(PRINTK_ERR, "pthread_create failed\n");
		return MANERR_GENERIC;
	}
	cpu_array[cpuid].status = STATUS_STEPPING;
	return MANERR_OK;
}

int man_cpu_stop(unsigned ownerid, unsigned cpuid) {
	int err;
	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status == STATUS_STOPPED) return MANERR_OK;

	if (pthread_cancel(cpu_array[cpuid].thread))
		printk2(PRINTK_ERR, "thread cancel failed\n");
	else {
		err=pthread_join(cpu_array[cpuid].thread, NULL);
		if (!err) {
			cpu_array[cpuid].status = STATUS_STOPPED;
			return MANERR_OK;
		}
		printk2(PRINTK_ERR, "thread join failed: %s\n",
			(err==ESRCH) ? "no such thread" : (err==EDEADLK) ?
			"deadlock" : (err==EINVAL) ? "invalid" : "other");
	}
	return MANERR_GENERIC;
}

/*************************** syscall modules **/

int man_module_add(unsigned ownerid, const char *filename, unsigned *lid_to) {
	char *tmpbuf;
	unsigned u_slen, f_slen;
	unsigned lid;
	ALLOCA_STACK;
	
	/* It would not be a good idea to allow people to load ../../../evilmodule.so.
	 * And if fold()'s buggy, we're screwed anyway :) */
	f_slen=strlen(A2_MODULE_DIR);
	u_slen=strlen(filename) + 1;
	tmpbuf=alloca(f_slen + u_slen);
	memcpy(tmpbuf, A2_MODULE_DIR, f_slen);
	memcpy(tmpbuf + f_slen, filename, u_slen);
	fold(tmpbuf + f_slen);
	
	/* Now load it */
	cancel_all_vcpus();
	
	lid=module_dyn_load(tmpbuf);
	if (lid != ((unsigned) -1)) {
		/* Success! Run VCPU starts */
		modstart_all_vcpus(lid);
	}
	
	continue_all_vcpus();
	
	if (lid == ((unsigned) -1)) {
		printk2(PRINTK_ERR, "Module load failed.\n");
		return MANERR_GENERIC;
	}
	*lid_to=lid;
	return MANERR_OK;
}

int man_module_rm(unsigned ownerid, unsigned lid) {
	int err;
	
	if (lid >= A2_MAX_RSRVD) {
		printk2(PRINTK_ERR, "Invalid LID\n");
		return MANERR_GENERIC;
	}
	printk2(PRINTK_INFO, "Unloading %d\n", lid);
	
	cancel_all_vcpus();
	/* Run VCPU stops */
	modstop_all_vcpus(lid);
	/* Try shutdown */
	err=module_dyn_unload(lid);
	if (err) {
		printk2(PRINTK_ERR, 
			"Module unload failed. Attempting reload (state data has been lost)\n");
		modstart_all_vcpus(lid);
	}
	continue_all_vcpus();
	
	return (err) ? MANERR_GENERIC : MANERR_OK;
}

int man_module_reload(unsigned ownerid, unsigned lid) {
	int err;
	
	if (lid >= A2_MAX_RSRVD) {
		printk2(PRINTK_ERR, "Invalid LID\n");
		return MANERR_GENERIC;
	}
	printk2(PRINTK_INFO, "Reloading %d\n", lid);
	
	cancel_all_vcpus();
	/* Try shutdown */
	err=module_dyn_reload(lid);
	continue_all_vcpus();

	if (err) return MANERR_GENERIC;
	
	return MANERR_OK;
}

int man_module_stat(unsigned ownerid, unsigned *lid,
		const char **filename, const char **otherdesc) {
	unsigned ltmp;
	ltmp=lid_getdata(*lid, filename, otherdesc);
	if (ltmp == -1) return MANERR_NOLID;
	
	*lid=ltmp;
	return MANERR_OK;
}

/*************************** load-from-file routines **/

/* XXX:
 * These will be removed in future.
 * The plan is that dealing with the icky details of the image file
 * format will be reserved for the agents, who'll decode it and send
 * it to us in network byte order and make our life really easy :)
 * 
 * For efficiency, the parts of the image would be uploaded to a cache
 * before the agent would be able to load them into a CPU. Otherwise
 * RELOAD would get exceptionally sloooow.
 * 
 * (You'd think static code -> shared code!!!)
 *
 * Seriously, we should not be getting messy with the file format;
 * (but... how do we do DL if we don't know how to load images?!)
 */

int man_cpu_image_addfile(unsigned ownerid, unsigned cpuid,
		const char *filename, unsigned *alib_id) {
	char *buf;
	unsigned fn_len, alib_tmp;
	ALLOCA_STACK;

	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status != STATUS_STOPPED) return MANERR_RUNNING;
	
	/* XXX: This will confuse a lot of people. The alternative
	 * is to let people add /etc/shadow and debug it.
	 * (Ok, chances are slim that /etc/shadow will be valid bytecode,
	 *  but you should know what I mean.)
	 */
	fn_len=strlen(filename) + 1;
	buf=alloca(fn_len);
	memcpy(buf, filename, fn_len);
	fold(buf);
	
	printk2(PRINTK_INFO, "loading %s...\n", buf);
	alib_tmp=imageman_loadimage(buf, cpu_array[cpuid].cpu);
	if (!alib_tmp) return MANERR_GENERIC;
	*alib_id=alib_tmp;
	return MANERR_OK;
}

int man_cpu_alib_rm(unsigned ownerid, unsigned cpuid, unsigned alib_id) {
	int err;

	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status != STATUS_STOPPED) return MANERR_RUNNING;
	
	if (alib_id >= A2_MAX_ALID) return MANERR_GENERIC;
	
	printk2(PRINTK_INFO, "unloading %d...\n", alib_id);
	err=imageman_unload_al(cpu_array[cpuid].cpu, alib_id);

	if (err) return MANERR_GENERIC;
	return MANERR_OK;
}

/* XXX: We might like some method of enumerating libraries.
 * Pity they have no names. */

int man_cpu_hac_addfile(unsigned ownerid, unsigned cpuid, const char *filename) {
	char *buf;
	unsigned fn_len;
	int err;
	FILE *f;
	ALLOCA_STACK;

	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status != STATUS_STOPPED) return MANERR_RUNNING;
	
	/* fold() as per 'standard' */
	fn_len=strlen(filename) + 1;
	buf=alloca(fn_len);
	memcpy(buf, filename, fn_len);
	fold(buf);
	
	printk2(PRINTK_INFO, "loading hac %s...\n", buf);

	f=fopen(buf, "r");
	if (!f) {
		perror("fopen");
		return MANERR_GENERIC;
	}
	/* Note hac_loadfile will unload a previous HAC before adding a new one! */
	err=hac_loadfile(cpu_array[cpuid].cpu, f);
	fclose(f);
	if (err) return MANERR_GENERIC;
	return MANERR_OK;
}

/*************************** CFDop routines **/

int man_cpu_cfdop_add(unsigned ownerid, unsigned cpuid, char desc[A2_CFDDESC_LEN],
		const char *extradesc,
		const struct flexsock_desc *to, struct flexsock_desc *revers) {
	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status != STATUS_STOPPED) return MANERR_RUNNING;

	if (!flexsock_available) return MANERR_GENERIC;
	if (FXS_IsConnectType(to)) {
		flexsock f;
		unsigned vfd;
		volatile unsigned xid;
		const struct cfdop_1 *cfdtbl;
		
		/* XXX: May take a long time to return? */
		f=FXS_ConnectTo(to);
		if (!f) return MANERR_GENERIC;
	
		cfdtbl=cfdop1_fddesc_get(desc);
		if (!cfdtbl) return MANERR_GENERIC;
		
		EXCEPT_CATCH(cpu_array[cpuid].cpu, xid) {
			vfd=cfdtbl->fd_create(cpu_array[cpuid].cpu, extradesc, f);
			cpu_array[cpuid].cpu->reg[A2_REGISTERS - 1].val.u=vfd+1;
		}
		EXCEPT_END(cpu_array[cpuid].cpu);

		if (xid==ERR_OOM) return MANERR_OOM;
		else if (xid != 0) return MANERR_GENERIC;
	} else {
		flexlisock fl;
		struct pending_sock *s;
		
		fl=FXS_BindTo(to, revers);
		if (!fl) return MANERR_GENERIC;

		/* Accept might well take forever. Set it up for later. */
		s=malloc(sizeof(struct pending_sock));
		if (!s) return MANERR_OOM;
		
		memcpy(s->socktype, desc, A2_CFDDESC_LEN);
		s->sock=fl;
		s->extradesc=(extradesc) ? strdup(extradesc) : NULL;
		s->next=NULL;
		if (cpu_array[cpuid].li_last)
			cpu_array[cpuid].li_last->next=s;
		else
			cpu_array[cpuid].li_first=s;
		cpu_array[cpuid].li_last=s;
	}
	return MANERR_OK;
}

int man_cpu_cfdop_accept(unsigned ownerid, unsigned cpuid, int *remaining) {
	struct pending_sock *s, *prev=NULL;
	unsigned createfail=0;
	flexsock remt;

	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status != STATUS_STOPPED) return MANERR_RUNNING;

	*remaining=0;
	s=cpu_array[cpuid].li_first;
	while(s) {
		remt=FXS_AcceptPoll(s->sock);
		if (remt == NULL) {
			(*remaining)++;
			if (prev)
				prev->next=s;
			else
				cpu_array[cpuid].li_first=s;
			prev=s;
			s=s->next;
		} else {
			unsigned vfd;
			volatile unsigned xid;
			const struct cfdop_1 *cfdtbl;
			struct pending_sock *tmp;
			
			FXS_CloseListener(s->sock);
			
			cfdtbl=cfdop1_fddesc_get(s->socktype);
			if (cfdtbl) {
				EXCEPT_CATCH(cpu_array[cpuid].cpu, xid) {
					vfd=cfdtbl->fd_create(cpu_array[cpuid].cpu,
							s->extradesc, remt);
					cpu_array[cpuid].cpu->reg[A2_REGISTERS - 1].val.u=vfd+1;
				}
				EXCEPT_END(cpu_array[cpuid].cpu);
				/* Save the first exception. It can't hurt in
				 * OOM to free the remaining sockets anyway... */
				if (xid && !createfail) createfail=xid;
			} else {
				/* Good enough, anyway. */
				createfail=ERR_BAD_FD;
			}
	
			tmp=s->next;
			if (s->extradesc) free(s->extradesc);
			free(s);
			s=tmp;
		}
	}
	if (!prev) cpu_array[cpuid].li_first=NULL;
	cpu_array[cpuid].li_last=prev;

	if (createfail == ERR_OOM) return MANERR_OOM;
	else if (createfail != 0) return MANERR_GENERIC;
	return MANERR_OK;
}

/* Destroy all pending-accept VFDs. (opposite of accept :) */
int man_cpu_cfdop_reject(unsigned ownerid, unsigned cpuid) {
	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status != STATUS_STOPPED) return MANERR_RUNNING;

	drop_pending_socks(cpuid);
	return MANERR_OK;
}

/*************************** debugging functions **/

int man_cpu_exitstatus_get(unsigned ownerid, unsigned cpuid, unsigned *exittype) {
	if (cpuid >= A2_MAX_VCPUS) return MANERR_NOCPU;
	if (cpu_array[cpuid].ownerid != ownerid) return MANERR_NOLOCK;
	if (cpu_array[cpuid].status != STATUS_STOPPED) return MANERR_RUNNING;

	*exittype=cpu_array[cpuid].exit_type;

	return MANERR_OK;
}

/* This is the heart of the update cycle.
 * It always takes POLLTIME to complete  */
int man_pollall(void) {
	unsigned i, xid;
	const char *c;
	struct vcpu *curr_cpu;
	for(i=0;i<A2_MAX_VCPUS;i++) {
		if (cpu_array[i].status == STATUS_STOPPED) continue;
		if (cpu_array[i].exit_type == EXITTYPE_NONE) continue;

		/* Ooh, cpu has stopped. (well, it set exit code) */
		cpu_array[i].status=STATUS_STOPPED;
		curr_cpu=cpu_array[i].cpu;

		switch(cpu_array[i].exit_type) {
			case EXITTYPE_HALT:
				/* Normal halt */
				printk2(PRINTK_WARN, "VCPU %d stopped: HALT or RAISE -1.\n", i);
				break;
			case EXITTYPE_FAIL:
				xid=curr_cpu->reg[A2_REGISTERS - 1].val.u;
				c=human_exception_name(xid);
				
				if (c)
					printk2(PRINTK_ERR, "VCPU %d stopped: unhandled exception %d (%s), origin 0x%x.\n", i, xid, c, curr_cpu->prev_ip);
				else
					printk2(PRINTK_ERR, "VCPU %d stopped: unhandled exception %d, origin 0x%x.\n", i, xid, curr_cpu->prev_ip);
				break;
			case EXITTYPE_EXCEPT:
				xid=curr_cpu->reg[A2_REGISTERS - 1].val.u;
				c=human_exception_name(xid);
				
				if (c)
					printk2(PRINTK_ERR, "VCPU %d stopped: HANDLED exception %d (%s), origin 0x%x.\n", i, xid, c, curr_cpu->prev_ip);
				else
					printk2(PRINTK_ERR, "VCPU %d stopped: HANDLED exception %d, origin 0x%x.\n", i, xid, curr_cpu->prev_ip);
				break;
			case EXITTYPE_STEP:
				printk2(PRINTK_ERR, "VCPU %d stopped: step completed: did 0x%x, next 0x%x\n", i, curr_cpu->prev_ip, curr_cpu->ip);
				break;
		}

		if (cpu_array[i].ownerid==ORPHANED_ID) {
			free_cpu(i);
		} else {
			/* Cpu's locked, so they'll debug it or unlock later.
			 * XXX: tell them it stopped! */
		}
	}
	usleep(A2_POLLTIME * 1000);
	return MANERR_OK;
}


