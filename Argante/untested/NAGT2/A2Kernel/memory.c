/*
 * Argante2 VM - memory management code.
 * (c) 2001 James Kehl <ecks@optusnet.com.au>
 * Use under LGPL.
 */
#include <stdlib.h>
#include <string.h> /* for bzero */
#include "config.h"
#include "taskman.h"
#include "exception.h"
#include "memory.h"

/* I hope GCC can work out a / (1 >> c) = a << c */
static inline struct memblk *mem_pageofaddr(struct vcpu *curr_cpu, unsigned addr)
{
	int a=addr / MAX_BLKSIZ;
	if (a >= curr_cpu->memblks) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
	return &curr_cpu->mem[a];
}
#define PAGEOFADDR(addr) mem_pageofaddr(curr_cpu, addr)
#define ADDRINADDR(addr) addr % MAX_BLKSIZ

anyval *mem_ro(struct vcpu *curr_cpu, unsigned addr) {
	struct memblk *x;
	int y;
	x=PAGEOFADDR(addr);
	y=ADDRINADDR(addr);
	if (y >= x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
#ifdef PARANOID
	if (!(x->mode & MEM_READ)) throw_except(curr_cpu, ERR_PROTFAULT);
#endif
	return &x->memory[y];
}

anyval *mem_rw(struct vcpu *curr_cpu, unsigned addr) {
	struct memblk *x;
	int y;
	x=PAGEOFADDR(addr);
	y=ADDRINADDR(addr);
	if (y >= x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
#ifdef PARANOID
	if (!(x->mode & MEM_READ)) throw_except(curr_cpu, ERR_PROTFAULT);
#endif
	if (!(x->mode & MEM_WRITE)) throw_except(curr_cpu, ERR_PROTFAULT);
	return &x->memory[y];
}

int mem_alloc(struct vcpu *curr_cpu, unsigned size, unsigned flags) {
	int i;
	struct memblk *d;
	/* 1. Check for spare pages */
	for (i=0;i<curr_cpu->memblks;i++)
	{
		if (!curr_cpu->mem[i].memory)
		{
			d=&curr_cpu->mem[i];
			/* Don't leave an inconsistent state if we run into
			 * an exception. */
			bzero(d, sizeof(struct memblk));
			if (!(d->memory=calloc(size, sizeof(anyval)))) throw_except(curr_cpu, ERR_OOM);
			d->mode=flags;
			d->size=size;
			d->destroy=NULL;
			return i * MAX_BLKSIZ;
		}
	}
	/* 2. Allocate a new page */
	i=curr_cpu->memblks;
	if (i >= MAX_MEMBLK) throw_except(curr_cpu, ERR_OOM);

	curr_cpu->memblks++;
	d=realloc(curr_cpu->mem, curr_cpu->memblks * sizeof(struct memblk));
	if (!d) throw_except(curr_cpu, ERR_OOM);
	curr_cpu->mem=d;
	/* Setup */
	d=&curr_cpu->mem[i];
	bzero(d, sizeof(struct memblk)); 
	if (!(d->memory=calloc(size, sizeof(anyval)))) throw_except(curr_cpu, ERR_OOM);
	d->mode=flags;
	d->size=size;
	d->destroy=NULL;
	return i * MAX_BLKSIZ;
}

void mem_realloc(struct vcpu *curr_cpu, unsigned addr, unsigned newsize) {
	struct memblk *x;
	anyval *m;
	x=PAGEOFADDR(addr);
	/* Perhaps allowing realloc on unallocated memory would be useful...
	 * except there's no guaranteed protflags then */
	if (!x->memory) throw_except(curr_cpu, ERR_OUTSIDE_MEM);

	/* Can't resize mmaps, and why would you want to resize constants? */
	if (x->mode & MEM_MAPPED) throw_except(curr_cpu, ERR_PROTFAULT);
	if (!(x->mode & MEM_WRITE)) throw_except(curr_cpu, ERR_PROTFAULT);

	m=realloc(x->memory, newsize * sizeof(anyval));
	if (!m) throw_except(curr_cpu, ERR_OOM);

	/* Paranoia: clear any newly-allocated space (XXX: check this pls) */
	if (newsize > x->size)
	{
		bzero(&x->memory[x->size], (newsize - x->size) * sizeof(anyval));
	}

	x->size=newsize;
	x->memory=m;
}

void mem_changeperm(struct vcpu *curr_cpu, unsigned addr, unsigned newflags) {
	struct memblk *x;
	x=PAGEOFADDR(addr);

	if (!x->memory) throw_except(curr_cpu, ERR_OUTSIDE_MEM);

	/* mmaps take more than a mere flag switch to change permissions on. */
	if (x->mode & MEM_MAPPED) throw_except(curr_cpu, ERR_PROTFAULT);

	if (newflags != (newflags & (MEM_READ | MEM_WRITE)))
		throw_except(curr_cpu, ERR_PROTFAULT); /* XXX: should it be CORRUPT_BCODE? */
	
	if (!(newflags & MEM_READ)) /* WRITE-ONLY MEMORY?! */
		throw_except(curr_cpu, ERR_PROTFAULT);

	x->mode=newflags;
}

void mem_dealloc(struct vcpu *curr_cpu, unsigned addr) {
	struct memblk *x;
	x=PAGEOFADDR(addr);

	if (!x->memory) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
	
	if (x->destroy)
		x->destroy(addr / MAX_BLKSIZ);
	else
		free(x->memory);
	/* Ideally we'd reduce the size of the mem chain too,
	 * but for efficiency reasons it doesn't happen here.
	 * (and laziness :)
	 */
	bzero(x, sizeof(struct memblk));
}
