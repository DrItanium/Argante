/*
 * Argante2 VM.
 * (c) 2001 James Kehl <ecks@optusnet.com.au>
 * Use under LGPL.
 */
#include <stdlib.h>
#include "config.h"
#include "taskman.h"
#include "exception.h"
#include "memory.h"
#include "cmd.h"

/* It is totally vital each thread never messes with
 * a curr_cpu it hasn't been given. If it throws an
 * exception and returns to another thread's control loop
 * Bad Stuff Happens (tm.)
 */
void throw_except(struct vcpu *curr_cpu, int except)
{
	printk("<-> Exception %d thrown...\n", except);
	longjmp(curr_cpu->onexcept, except);
}

/* As an efficiency thing, the call stack grows in large
 * jumps and doesn't often get downsized. If you undefine
 * CALLSTACK_DOWNSIZE it never does, which gives a minor
 * perf boost at the cost of chewing memory.
 */

#define CALLSTACK_UPSIZE 64 /* 256 bytes - pretty miniscule */
#define CALLSTACK_DOWNSIZE 256 /* 1k */

#ifndef NONREENTRANT
void push_ip_on_stack_mt(struct vcpu *curr_cpu)
#else
void push_ip_on_stack_st()
#endif
{
	int i;
	int *z;
	if (curr_cpu->sptr >= curr_cpu->stack_size)
	{
		if (curr_cpu->stack_size == MAX_STACK)
			throw_except(curr_cpu, ERR_CSTACK_OVER);
		else { /* So it gets at least 1 bigger, right? */
			i=curr_cpu->stack_size + CALLSTACK_UPSIZE;
			if (i > MAX_STACK) i=MAX_STACK;
	//		printf("Upsizing callstack (to %d) - BAD!\n", i);
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
	curr_cpu->stack[curr_cpu->sptr]=curr_cpu->ip;
	curr_cpu->xstack[curr_cpu->sptr]=curr_cpu->xip;
	curr_cpu->sptr++;
}

#ifndef NONREENTRANT
void pop_ip_from_stack_mt(struct vcpu *curr_cpu, unsigned count)
#else
void pop_ip_from_stack_st(unsigned count)
#endif
{
//	int i, x; for(i=0;i<1000;i++) x++;
	/* Pop count IP's off the stack */
	if (count > curr_cpu->sptr) throw_except(curr_cpu, ERR_CSTACK_UNDER);
	curr_cpu->sptr-=count;
	curr_cpu->ip=curr_cpu->stack[curr_cpu->sptr];
	curr_cpu->xip=curr_cpu->xstack[curr_cpu->sptr];
	/* Do we downsize now? XXX: the stack will be 100% full after this */
#ifdef CALLSTACK_DOWNSIZE
	if (curr_cpu->stack_size > curr_cpu->sptr + CALLSTACK_DOWNSIZE)
	{
		int *z;
		/* Again: we keep consistency */
		curr_cpu->stack_size=curr_cpu->sptr;
	//	printf("Downsizing callstack (to %d) - BAD!\n", curr_cpu->sptr);
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

#ifndef NONREENTRANT
void do_cycle(struct vcpu *curr_cpu) {
	anyval *a1, *a2;
	struct _bcode_op *bop;
	int i;
#else
anyval *a1, *a2;
struct vcpu *curr_cpu;

void do_cycle(struct vcpu *setcpu) {
	struct _bcode_op *bop;
	int i;
	curr_cpu=setcpu;
#endif
hi:
	if (curr_cpu->ip > curr_cpu->csize)
		throw_except(curr_cpu, ERR_OUTSIDE_CODE);

	bop=&curr_cpu->bcode[curr_cpu->ip];

	/* Evaluate A1 */
	if (bop->type & TYPE_A1(TYPE_REGISTER))
		a1=&curr_cpu->reg[bop->a1.val.u];
	else
		a1=&bop->a1;

	if (bop->type & TYPE_A1(TYPE_POINTER)) {
		if (jit_protflags[(unsigned) bop->bcode] & A1_READ_WRITE)
			a1=mem_rw(curr_cpu, a1->val.u);
		else
			a1=mem_ro(curr_cpu, a1->val.u);
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
			a2=mem_ro(curr_cpu, a2->val.u);
	}
	
/*	i=(bop->type & TYPE_A1(TYPE_VALMASK)) + ((bop->type & TYPE_A2(TYPE_VALMASK)) >> 4) * 3;
	i=i + (unsigned) bop->bcode * 9; */

	/* Now, run JIT */
	i=curr_cpu->ip;
	curr_cpu->ip++;
#ifndef NONREENTRANT
	(jit_calls[curr_cpu->bcode_jitoffs[i]])(curr_cpu, a1, a2);
#else
	(jit_calls[curr_cpu->bcode_jitoffs[i]])();
#endif
	goto hi;
}

int validate_bcode(struct vcpu *curr_cpu)
{
	int i;
	struct _bcode_op *bop;

	/* Now this is an ugly cheat, but I so want to beat AOS1's speeds */
	curr_cpu->bcode_jitoffs=malloc(curr_cpu->csize * sizeof(short));
	if (!curr_cpu->bcode_jitoffs) return 1; /* XXX: too cryptic */
	
	for(i=0;i<curr_cpu->csize;i++)
	{
		bop=&curr_cpu->bcode[i];

		if (jit_protflags[(unsigned) bop->bcode] & A1_READ_WRITE
			&& !(bop->type & TYPE_A1(TYPE_POINTER | TYPE_REGISTER)))
			return 1; /* Code tried to selfmodify */
		if (jit_protflags[(unsigned) bop->bcode] & A2_READ_WRITE
			&& !(bop->type & TYPE_A2(TYPE_POINTER | TYPE_REGISTER)))
			return 1;
		/* (0 or 1 or 2) + (0 or 1 or 2) * 3 = range 0-8. */
		curr_cpu->bcode_jitoffs[i]=(bop->type & TYPE_A1(TYPE_VALMASK))
			+ ((bop->type & TYPE_A2(TYPE_VALMASK)) >> 4) * 3
			+ (unsigned) bop->bcode * 9;
		if (curr_cpu->bcode_jitoffs[i] >= (CMD_INVALID - 1) * 9)
			return 1; /* throw_except(curr_cpu, ERR_CORRUPT_CODE); */
	}
	return 0;
}
