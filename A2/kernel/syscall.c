/*
 * A2 Virtual Machine - 'module-library' support
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
#include <sys/types.h>

#include <stdio.h> /* For perror */
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "exception.h"
#include "modload.h"
#include "cfdop.h"

/* Plenty of room, rehashing is expensive */
#define SYSCALL_HASHSIZE 50

typedef struct _syscallent syscallent;

struct _syscallent {
	unsigned id;
	syscallfunc *f;
	syscallent *next;
	syscallent *prev; /* We want to be able to remove ents. But we will hardly ever...
			     is double-linked excessive? */
};

struct syscallhash {
	int entries;
	int size;
	syscallent **tab;
};

static struct syscallhash schash = {0, 0, NULL};

void agt_syscall(struct vcpu *curr_cpu, unsigned callno, const anyval *arg)
{
	int i;
	syscallfunc *f;
	syscallent *e;

	if (!arg) arg=&curr_cpu->reg[0];
	
	if (!schash.size) goto fail; /* Don't entirely trust CSE :) */
	i=callno % schash.size;
	e=schash.tab[i];
	while (e && e->id!=callno)
		e=e->next;
	
	if (!e) goto fail;
	f=e->f;
	if (!f) goto fail; 
	
	f(curr_cpu, arg);
	return;
fail: /* Must be careful not to lose semaphore! */
	throw_except(curr_cpu, ERR_NOSYSCALL);
}

static int register_syscall_i(unsigned id, syscallfunc *f) {
	syscallent *d;
	int p;

	/* Should be impossible but WTF */
	if (schash.entries >= schash.size) {
		printk2(PRINTK_CRIT, "Hash table full?!\n");
		return 1;
	}

	schash.entries++;
	p=id % schash.size;
	d=malloc(sizeof(syscallent));
	if (!d) {
		perror("malloc");
		return 1;
	}
	d->id=id;
	d->f=f;
	d->next=schash.tab[p];
	d->prev=NULL;
	if (schash.tab[p]) schash.tab[p]->prev=d;
	schash.tab[p]=d;
	return 0;
}

int register_syscall(unsigned id, syscallfunc *f)
{


	if (schash.entries >= 0.8 * schash.size) {
		int s, i, x;
		syscallent **new, **old, *q, *d;
		/* EEEEEK! REHASH TIME! */
		s=schash.size + SYSCALL_HASHSIZE;

		new=calloc(sizeof(syscallent *), s);
		if (!new) {
			perror("malloc");
			return 1;
		}

		/* Swap in new hash */
		x=schash.size;
		/* printk2(PRINTK_INFO, "rehashing from %d to %d\n", x, s); */
		schash.size=s;
		schash.entries=0;
		old=schash.tab;
		schash.tab=new;

		/* Rehash */
		for(i=0;i<x;i++)
		{
			d=old[i];
			while (d) {
				q=d->next;
				if (register_syscall_i(d->id, d->f)) return 1;
				free(d);
				d=q;
			}
		}
		/* And away with the old... */
		free(old);
	}

	return register_syscall_i(id, f);
}

int unregister_syscall(unsigned callno)
{
	int i;
	syscallent *e;
	
	/* Can't throw exceptions - this is manager thread... */
	if (!schash.size) return 1;
	i=callno % schash.size;
	e=schash.tab[i];
	while (e && e->id!=callno)
		e=e->next;
	
	if (!e) return 1;
	
	if (e->prev) e->prev->next=e->next; else schash.tab[i]=e->next;
	if (e->next) e->next->prev=e->prev;

	free(e);
	schash.entries--;
	return 0;
}

struct moduleent {
	unsigned lid; /* A kinda pointless redundancy check, I guess. */
	char *name;
	modulevcpufunc *start;	
	modulevcpufunc *stop;
	const struct cfdop_1 *cfd1;
};

static struct moduleent *ments=NULL;
static unsigned mentcount=0;

unsigned lid_create() {
	unsigned i, l;
	struct moduleent *new;
	
	for(i=0;i<mentcount;i++)
		if (ments[i].lid != i) {
			ments[i].lid = i;
			return i;
		}
	
	i=mentcount+1;
	/* Limit LID-count, bug protection... */
	if (i>=A2_MAX_RSRVD) {
		printk2(PRINTK_CRIT, "LID-limit reached.\n"
				 " If you need more than %d modules loaded, edit config.h.\n"
				 " Otherwise something is broken...\n", A2_MAX_RSRVD);
		return -1;
	}

	if (ments)
		new=realloc(ments, i * sizeof(struct moduleent));
	else
		new=malloc(i * sizeof(struct moduleent));
	if (!new) {
		perror("malloc");
		return -1;
	}
	l=mentcount;
	ments=new;
	ments[l].lid=l;
	ments[l].name=NULL;
	ments[l].start=NULL;
	ments[l].stop=NULL;
	ments[l].cfd1=NULL;
	mentcount=i;
	return l;
}

void lid_destroy(unsigned lid) {
	struct moduleent *new;
	unsigned q;

	if (lid >= mentcount || ments[lid].lid!=lid) {
		printk2(PRINTK_CRIT, "Corrupt LID!\n");
		return;
	}

	ments[lid].lid=-1;
	ments[lid].cfd1=NULL;
	q=mentcount-1;
	if (lid != q) return;
	/* Downsize */
	while(ments[q].lid != q && mentcount > 0) { mentcount=q; q=mentcount-1; }
	if (mentcount) {
		/* Of course ments exists! */
		new=realloc(ments, mentcount * sizeof(struct moduleent));
		if (!new) {
			/* As with stack, mentcount's wrong, but who cares? */
			perror("malloc");
			return;
		}
		ments=new;
	} else {
		free(ments);
		ments=NULL;
	}
}

void lid_assign(unsigned lid, char *name,
		modulevcpufunc *start, modulevcpufunc *stop) {
	if (lid >= mentcount || ments[lid].lid!=lid) {
		printk2(PRINTK_CRIT, "Corrupt LID!\n");
		return;
	}

	ments[lid].name=name;
	ments[lid].start=start;
	ments[lid].stop=stop;
}

/* Common Input/Output layer setup thingies */
void cfdop1_lid_set(unsigned lid, const struct cfdop_1 *a) { 
	if (lid >= mentcount || ments[lid].lid!=lid) {
		printk2(PRINTK_CRIT, "Corrupt LID!\n");
		return;
	}
	ments[lid].cfd1=a;
}

const struct cfdop_1 *cfdop1_lid_get(unsigned lid) { 
	if (lid >= mentcount || ments[lid].lid!=lid) {
		printk2(PRINTK_CRIT, "Corrupt LID!\n");
		return NULL;
	}
	return ments[lid].cfd1;
}

const struct cfdop_1 *cfdop1_fddesc_get(int fddesc) { 
	unsigned i;
	for(i=0;i < mentcount;i++) {
		if (ments[i].cfd1 && ments[i].cfd1->fd_close &&
			ments[i].cfd1->fd_desc==fddesc) {
			return ments[i].cfd1;
		}
	}
	printk2(PRINTK_ERR, "Couldn't find module for desc %d\n", fddesc);
	return NULL;
}

/* (per-vcpu) possibly Pthread calls, which is why the void * */
void *vcpu_modules_start(void *v) {
	unsigned i;
	struct vcpu *curr_cpu=v;
	for(i=0;i < mentcount;i++) {
		if (ments[i].lid == i && ments[i].start)
			ments[i].start(curr_cpu);
	}
	return NULL;
}

void *vcpu_modules_stop(void *v) {
	unsigned i;
	struct vcpu *curr_cpu=v;
	for(i=0;i < mentcount;i++) {
		if (ments[i].lid == i && ments[i].start)
			ments[i].stop(curr_cpu);
	}
	return NULL;
}

#define UNEG1 (unsigned) -1

static unsigned moduleid=UNEG1;
void vcpu_set_moduleid(unsigned new) {
	if (new >= mentcount) {
		printk2(PRINTK_CRIT, "nasty moduleid!\n");
		new=UNEG1;
	}
	moduleid=new;
}

void *vcpu_module_start(void *v) {
	struct vcpu *curr_cpu=v;
	printk2(PRINTK_DEBUG, "UNEG1 = 0x%x\n", UNEG1);
	if (moduleid == UNEG1) return NULL;
	if (ments[moduleid].lid == moduleid && ments[moduleid].start)
		ments[moduleid].start(curr_cpu);
	return NULL;
}

void *vcpu_module_stop(void *v) {
	struct vcpu *curr_cpu=v;
	if (moduleid == UNEG1) return NULL;
	if (ments[moduleid].lid == moduleid && ments[moduleid].stop)
		ments[moduleid].stop(curr_cpu);
	return NULL;
}

/* Get the reserved structs, creating them if needed.
 * Called by CPU-threads, but can't throw exceptions:
 * these are used in vcpu_start/_stop */
void *module_get_reserved(struct vcpu *cpu, unsigned lid) {
	if (lid >= cpu->reserved_ct) {
		if (lid >= A2_MAX_RSRVD) printk2(PRINTK_CRIT, "Corrupt LID!\n");
		return NULL;
	}
	return cpu->reserved[lid];
}

int module_set_reserved(struct vcpu *cpu, unsigned lid, void *newdata) {
	if (lid >= cpu->reserved_ct) {
		void **new;
		unsigned i;
	
		/* By def., mentcount < MAX_RESERVED */
		if (lid >= mentcount) {
			printk2(PRINTK_CRIT, "Corrupt LID!\n");
			return 1;
		}

		/* mentcount is effectively locked (no CPU-threads
		 * when it's modified) so using it is safe as houses.
		 *                           (-- what houses, where?) */
		if (cpu->reserved)
			new=realloc(cpu->reserved, mentcount * sizeof(void *));
		else
			new=malloc(mentcount * sizeof(void *));
		if (!new) {
			perror("malloc");
			return 1;
		}
		cpu->reserved=new;
		/* another "check me please" */
		for(i=cpu->reserved_ct;i<mentcount;i++)
			cpu->reserved[i]=NULL;
		cpu->reserved_ct=mentcount;
	}
	cpu->reserved[lid]=newdata;
	return 0;
}
