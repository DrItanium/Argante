/*
 * A2 Virtual Machine - memory management code
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
#include "compat/bzero.h"

#include <stdio.h> /* perror */
#include <stdlib.h>
#include <string.h> /* strlen */
#include "config.h"
#include "anyval.h"
#include "taskman.h"
#include "exception.h"
#include "amemory.h"
#include "bcode.h"

/* I hope GCC can work out a / (1 >> c) = a << c */
static inline struct memblk *mem_pageofaddr(struct vcpu *curr_cpu, unsigned addr)
{
	unsigned a=addr / A2_MAX_BLKSIZ;
	if (a >= curr_cpu->memblks) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
	return &curr_cpu->mem[a];
}
#define PAGEOFADDR(addr) mem_pageofaddr(curr_cpu, addr)
#define ADDRINADDR(addr) addr % A2_MAX_BLKSIZ

const anyval *mem_ro(struct vcpu *curr_cpu, unsigned addr) {
	struct memblk *x;
	unsigned y;
	x=PAGEOFADDR(addr);
	y=ADDRINADDR(addr);
	if (y >= x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
#ifdef PARANOID
	if (!(x->mode & A2_MEM_READ)) throw_except(curr_cpu, ERR_PROTFAULT);
#endif
	return &x->memory[y];
}

anyval *mem_rw(struct vcpu *curr_cpu, unsigned addr) {
	struct memblk *x;
	unsigned y;
	x=PAGEOFADDR(addr);
	y=ADDRINADDR(addr);
	if (y >= x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
#ifdef PARANOID
	if (!(x->mode & A2_MEM_READ)) throw_except(curr_cpu, ERR_PROTFAULT);
#endif
	if (!(x->mode & A2_MEM_WRITE)) throw_except(curr_cpu, ERR_PROTFAULT);
	return &x->memory[y];
}

/* A2 0.008: Check a large amount of memory at once, and
   guarantee it's contiguous (i.e. you can ++ it)  */
const anyval *mem_ro_block(struct vcpu *curr_cpu, unsigned addr, unsigned dwords) {
	struct memblk *x;
	unsigned y;
	x=PAGEOFADDR(addr);
	y=ADDRINADDR(addr);
	if (y >= x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
	/* Don't be fooled by -1 memory! It is > - we'll have at least 1 dword. */
	if (dwords > x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM); 
	if (y + dwords > x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM); 
#ifdef PARANOID
	if (!(x->mode & A2_MEM_READ)) throw_except(curr_cpu, ERR_PROTFAULT);
#endif
	return &x->memory[y];
}

anyval *mem_rw_block(struct vcpu *curr_cpu, unsigned addr, unsigned dwords) {
	struct memblk *x;
	unsigned y;
	x=PAGEOFADDR(addr);
	y=ADDRINADDR(addr);
	if (y >= x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
	/* Don't be fooled by -1 memory! It is > - we'll have at least 1 dword. */
	if (dwords > x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM); 
	if (y + dwords > x->size) throw_except(curr_cpu, ERR_OUTSIDE_MEM); 
#ifdef PARANOID
	if (!(x->mode & A2_MEM_READ)) throw_except(curr_cpu, ERR_PROTFAULT);
#endif
	if (!(x->mode & A2_MEM_WRITE)) throw_except(curr_cpu, ERR_PROTFAULT);
	return &x->memory[y];
}

unsigned mem_alloc(struct vcpu *curr_cpu, unsigned size, unsigned flags) {
	unsigned i;
	struct memblk *d;
	/* Fixme. Add multiblocking. */
	if (size > A2_MAX_BLKSIZ) throw_except(curr_cpu, ERR_OOM);

	/* 1. Check for spare pages */
	for (i=0;i<curr_cpu->memblks;i++)
	{
		if (!curr_cpu->mem[i].memory)
		{
			d=&curr_cpu->mem[i];
			/* Don't leave an inconsistent state if we run into
			 * an exception. */
			bzero(d, sizeof(struct memblk));
			d->memory=calloc(size, sizeof(anyval));
			if (!d->memory) throw_except(curr_cpu, ERR_OOM);
			d->mode=flags;
			d->size=size;
			d->destroy_scnum=0;
			return i * A2_MAX_BLKSIZ;
		}
	}
	/* 2. Allocate a new page */
	i=curr_cpu->memblks;
	if (i >= A2_MAX_MEMBLK) throw_except(curr_cpu, ERR_OOM);

	curr_cpu->memblks++;
	d=realloc(curr_cpu->mem, curr_cpu->memblks * sizeof(struct memblk));
	if (!d) throw_except(curr_cpu, ERR_OOM);
	curr_cpu->mem=d;
	/* Setup */
	d=&curr_cpu->mem[i];
	bzero(d, sizeof(struct memblk));
	d->memory=calloc(size, sizeof(anyval));
	if (!d->memory) throw_except(curr_cpu, ERR_OOM);
	d->mode=flags;
	d->size=size;
	d->destroy_scnum=0;
	return i * A2_MAX_BLKSIZ;
}

unsigned mem_realloc(struct vcpu *curr_cpu, unsigned addr, unsigned newsize) {
	struct memblk *x;
	anyval *m;
	/* Fixme. Add multiblocking. */
	if (newsize > A2_MAX_BLKSIZ) throw_except(curr_cpu, ERR_OOM);

	x=PAGEOFADDR(addr);
	/* Perhaps allowing realloc on unallocated memory would be useful...
	 * except there's no guaranteed protflags then */
	if (!x->memory) throw_except(curr_cpu, ERR_OUTSIDE_MEM);

	/* Can't resize mmaps, and why would you want to resize constants? */
	if (x->mode & A2_MEM_MAPPED) throw_except(curr_cpu, ERR_PROTFAULT);
	if (!(x->mode & A2_MEM_WRITE)) throw_except(curr_cpu, ERR_PROTFAULT);

	m=realloc(x->memory, newsize * sizeof(anyval));
	if (!m) {
		perror("realloc");
		throw_except(curr_cpu, ERR_OOM);
	}

	/* Paranoia: clear any newly-allocated space.
	 * I said "check this please", and whaddya know, it was buggy! */
	if (newsize > x->size)
	{
		bzero(&m[x->size], (newsize - x->size) * sizeof(anyval));
	}

	x->size=newsize;
	x->memory=m;
	return addr;
}

void mem_changeperm(struct vcpu *curr_cpu, unsigned addr, unsigned newflags) {
	struct memblk *x;
	x=PAGEOFADDR(addr);

	if (!x->memory) throw_except(curr_cpu, ERR_OUTSIDE_MEM);

	/* mmaps take more than a mere flag switch to change permissions on. */
	if (x->mode & A2_MEM_MAPPED) throw_except(curr_cpu, ERR_PROTFAULT);

	if (newflags != (newflags & (A2_MEM_READ | A2_MEM_WRITE)))
		throw_except(curr_cpu, ERR_PROTFAULT); /* XXX: should it be CORRUPT_BCODE? */
	
	if (!(newflags & A2_MEM_READ)) /* WRITE-ONLY MEMORY?! */
		throw_except(curr_cpu, ERR_PROTFAULT);

	x->mode=newflags;
}

void mem_dealloc(struct vcpu *curr_cpu, unsigned addr) {
	struct memblk *x;
	x=PAGEOFADDR(addr);

	if (!x->memory) throw_except(curr_cpu, ERR_OUTSIDE_MEM);
	
	if (x->destroy_scnum) {
		anyval adr;
		adr.val.u=addr;
		agt_syscall(curr_cpu, x->destroy_scnum, &adr);
	}
	else
		free(x->memory);
	/* Ideally we'd reduce the size of the mem chain too,
	 * but for efficiency reasons it doesn't happen here.
	 * (and laziness :)
	 */
	bzero(x, sizeof(struct memblk));
}

int kerntoa_memcpy(struct vcpu *curr_cpu, unsigned addrto, const char *from, int size) {
	char *x;

	x=(char *) mem_rw_block(curr_cpu, addrto, dwords_of_bytes(size));
	memcpy(x, from, size);
	return size;
}

int atokern_memcpy(struct vcpu *curr_cpu, char *to, unsigned addrfrom, int size) {
	const char *x=NULL;

	x=(const char *) mem_ro_block(curr_cpu, addrfrom, dwords_of_bytes(size));
	memcpy(to, x, size);
	return size;
}

int kerntoa_strcpy(struct vcpu *curr_cpu, unsigned addrto, int size, const char *from) {
	int z;
	z=strlen(from);
	if (z > size) throw_except(curr_cpu, ERR_RESULT_TOOLONG);
	return kerntoa_memcpy(curr_cpu, addrto, from, z);
}


