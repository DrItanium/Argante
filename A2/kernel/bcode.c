/*
 * A2 Virtual Machine - bytecode assistance routines
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
#include "compat/usleep.h"

#include <stdlib.h>
#include "config.h"
#include "taskman.h"
#include "exception.h"
#include "amemory.h"

/* It is totally vital each thread never messes with
 * a curr_cpu it hasn't been given. If it throws an
 * exception and returns to another thread's control loop
 * Bad Stuff Happens (tm.)
 */
void throw_except(struct vcpu *curr_cpu, int except)
{
/* printk("<-> Exception %d thrown...\n", except); */
	longjmp(*curr_cpu->onexcept, except);
}

void push_ip_on_stack(struct vcpu *curr_cpu)
{
	int i;
	unsigned int *z;
	if (curr_cpu->sptr >= curr_cpu->stack_size)
	{
		if (curr_cpu->stack_size == A2_MAX_STACK)
			throw_except(curr_cpu, ERR_CSTACK_OVER);
		else { /* So it gets at least 1 bigger, right? */
			i=curr_cpu->stack_size + CALLSTACK_UPSIZE;
			if (i > A2_MAX_STACK) i=A2_MAX_STACK;
			/* Callstack */
			z=realloc(curr_cpu->stack, i * sizeof(unsigned));
			if (!z) throw_except(curr_cpu, ERR_CSTACK_OVER);
			else curr_cpu->stack=z;
			/* Xstack */
			z=realloc(curr_cpu->xstack, i * sizeof(unsigned));
			/* If this fails, we haven't yet changed stack_size, and
			 * nothing fatal will happen. We just have a bigger cstack
			 * than we know about - and malloc knows it's real size... */
			if (!z) throw_except(curr_cpu, ERR_CSTACK_OVER);
			else curr_cpu->xstack=z;
			curr_cpu->stack_size=i;
		}
	}
	/* Ok, the stack's big enough - push */
	curr_cpu->stack[curr_cpu->sptr]=curr_cpu->next_ip;
	curr_cpu->xstack[curr_cpu->sptr]=curr_cpu->xip;
	curr_cpu->sptr++;
}

void pop_ip_from_stack(struct vcpu *curr_cpu, unsigned count)
{
	/* Pop count IP's off the stack */
	if (count > curr_cpu->sptr) throw_except(curr_cpu, ERR_CSTACK_UNDER);
	curr_cpu->sptr-=count;
	curr_cpu->next_ip=curr_cpu->stack[curr_cpu->sptr];
	curr_cpu->xip=curr_cpu->xstack[curr_cpu->sptr];
	/* Do we downsize now? XXX: the stack will be 100% full after this */
#ifdef CALLSTACK_DOWNSIZE
	if (curr_cpu->stack_size > curr_cpu->sptr + CALLSTACK_DOWNSIZE)
	{
		unsigned int *z;
		/* Again: we keep consistency */
		curr_cpu->stack_size=curr_cpu->sptr;
		/* cstack */
		z=realloc(curr_cpu->stack, curr_cpu->stack_size * sizeof(unsigned));
		if (!z) return; /* Hope for best */
		else curr_cpu->stack=z;
		/* Xstack */
		z=realloc(curr_cpu->xstack, curr_cpu->stack_size * sizeof(unsigned));
		if (!z) return;
		else curr_cpu->xstack=z;
	}
#endif
}

