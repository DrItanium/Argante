/*
 * A2 Virtual Machine - JIT functions
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
#include <math.h>

#include "config.h"
#include "anyval.h"
#include "taskman.h"
#include "exception.h"
#include "amemory.h"
#include "cmd.h"
#include "bcode.h"

/*#ifndef NULL
#define NULL (void *) 0
#endif*/

#define NEW_IP(x) curr_cpu->next_ip=x

/* Note the comments prefixed with !'s are absolutely essential,
 * they are used by the autogenerator which creates the JIT table
 * and the language spec (used by NAGT).
 *
 * The format must be exactly, down to the spaces around the = and
 * starting and finishing on the same line:
 * (replace . with !, of course) */

/*. OPCODE ARGS [TYPE1 PROT1 [TYPE2 PROT2]] = FUNC_NAME */

/*! NOP 0 = cmd_nop */
static void cmd_nop(JIT_ARGS) {
}

/*! MOV 2 u RW u RO = cmd_mov_uu */
/*! MOV 2 u RW s RO = cmd_mov_us */
/*! MOV 2 u RW f RO = cmd_mov_uf */
/*! MOV 2 s RW u RO = cmd_mov_su */
/*! MOV 2 s RW s RO = cmd_mov_ss */
/*! MOV 2 s RW f RO = cmd_mov_sf */
/*! MOV 2 f RW u RO = cmd_mov_fu */
/*! MOV 2 f RW s RO = cmd_mov_fs */
/*! MOV 2 f RW f RO = cmd_mov_ff */
static void cmd_mov_uu(JIT_ARGS) { a1->val.u=a2->val.u; }
static void cmd_mov_us(JIT_ARGS) { a1->val.u=a2->val.s; }
static void cmd_mov_uf(JIT_ARGS) { a1->val.u=a2->val.f; }
static void cmd_mov_su(JIT_ARGS) { a1->val.s=a2->val.u; }
static void cmd_mov_ss(JIT_ARGS) { a1->val.s=a2->val.s; }
static void cmd_mov_sf(JIT_ARGS) { a1->val.s=a2->val.f; }
static void cmd_mov_fu(JIT_ARGS) { a1->val.f=a2->val.u; }
static void cmd_mov_fs(JIT_ARGS) { a1->val.f=a2->val.s; }
static void cmd_mov_ff(JIT_ARGS) { a1->val.f=a2->val.f; }

/*-------------------------------------------------------
  - Conditional jumps                                   -
  -------------------------------------------------------*/
/*! IFEQ 2 u RO u RO = cmd_ifeq_uu */
/*! IFEQ 2 u RO s RO = cmd_ifeq_us */
/*! IFEQ 2 u RO f RO = cmd_ifeq_uf */
/*! IFEQ 2 s RO u RO = cmd_ifeq_su */
/*! IFEQ 2 s RO s RO = cmd_ifeq_ss */
/*! IFEQ 2 s RO f RO = cmd_ifeq_sf */
/*! IFEQ 2 f RO u RO = cmd_ifeq_fu */
/*! IFEQ 2 f RO s RO = cmd_ifeq_fs */
/*! IFEQ 2 f RO f RO = cmd_ifeq_ff */
static void cmd_ifeq_uu(JIT_ARGS) { if (a1->val.u != a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifeq_us(JIT_ARGS) { if (a1->val.u != a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifeq_uf(JIT_ARGS) { if (a1->val.u != a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifeq_su(JIT_ARGS) { if (a1->val.s != a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifeq_ss(JIT_ARGS) { if (a1->val.s != a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifeq_sf(JIT_ARGS) { if (a1->val.s != a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifeq_fu(JIT_ARGS) { if (a1->val.f != a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifeq_fs(JIT_ARGS) { if (a1->val.f != a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifeq_ff(JIT_ARGS) { if (a1->val.f != a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }

/*! IFNEQ 2 u RO u RO = cmd_ifneq_uu */
/*! IFNEQ 2 u RO s RO = cmd_ifneq_us */
/*! IFNEQ 2 u RO f RO = cmd_ifneq_uf */
/*! IFNEQ 2 s RO u RO = cmd_ifneq_su */
/*! IFNEQ 2 s RO s RO = cmd_ifneq_ss */
/*! IFNEQ 2 s RO f RO = cmd_ifneq_sf */
/*! IFNEQ 2 f RO u RO = cmd_ifneq_fu */
/*! IFNEQ 2 f RO s RO = cmd_ifneq_fs */
/*! IFNEQ 2 f RO f RO = cmd_ifneq_ff */
static void cmd_ifneq_uu(JIT_ARGS) { if (a1->val.u == a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifneq_us(JIT_ARGS) { if (a1->val.u == a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifneq_uf(JIT_ARGS) { if (a1->val.u == a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifneq_su(JIT_ARGS) { if (a1->val.s == a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifneq_ss(JIT_ARGS) { if (a1->val.s == a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifneq_sf(JIT_ARGS) { if (a1->val.s == a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifneq_fu(JIT_ARGS) { if (a1->val.f == a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifneq_fs(JIT_ARGS) { if (a1->val.f == a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifneq_ff(JIT_ARGS) { if (a1->val.f == a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }

/*! IFABO 2 u RO u RO = cmd_ifabo_uu */
/*! IFABO 2 u RO s RO = cmd_ifabo_us */
/*! IFABO 2 u RO f RO = cmd_ifabo_uf */
/*! IFABO 2 s RO u RO = cmd_ifabo_su */
/*! IFABO 2 s RO s RO = cmd_ifabo_ss */
/*! IFABO 2 s RO f RO = cmd_ifabo_sf */
/*! IFABO 2 f RO u RO = cmd_ifabo_fu */
/*! IFABO 2 f RO s RO = cmd_ifabo_fs */
/*! IFABO 2 f RO f RO = cmd_ifabo_ff */
static void cmd_ifabo_uu(JIT_ARGS) { if (a1->val.u <= a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifabo_us(JIT_ARGS) { if (a1->val.u <= a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifabo_uf(JIT_ARGS) { if (a1->val.u <= a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifabo_su(JIT_ARGS) { if (a1->val.s <= a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifabo_ss(JIT_ARGS) { if (a1->val.s <= a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifabo_sf(JIT_ARGS) { if (a1->val.s <= a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifabo_fu(JIT_ARGS) { if (a1->val.f <= a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifabo_fs(JIT_ARGS) { if (a1->val.f <= a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifabo_ff(JIT_ARGS) { if (a1->val.f <= a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }

/*! IFBEL 2 u RO u RO = cmd_ifbel_uu */
/*! IFBEL 2 u RO s RO = cmd_ifbel_us */
/*! IFBEL 2 u RO f RO = cmd_ifbel_uf */
/*! IFBEL 2 s RO u RO = cmd_ifbel_su */
/*! IFBEL 2 s RO s RO = cmd_ifbel_ss */
/*! IFBEL 2 s RO f RO = cmd_ifbel_sf */
/*! IFBEL 2 f RO u RO = cmd_ifbel_fu */
/*! IFBEL 2 f RO s RO = cmd_ifbel_fs */
/*! IFBEL 2 f RO f RO = cmd_ifbel_ff */
static void cmd_ifbel_uu(JIT_ARGS) { if (a1->val.u >= a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifbel_us(JIT_ARGS) { if (a1->val.u >= a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifbel_uf(JIT_ARGS) { if (a1->val.u >= a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifbel_su(JIT_ARGS) { if (a1->val.s >= a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifbel_ss(JIT_ARGS) { if (a1->val.s >= a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifbel_sf(JIT_ARGS) { if (a1->val.s >= a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifbel_fu(JIT_ARGS) { if (a1->val.f >= a2->val.u) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifbel_fs(JIT_ARGS) { if (a1->val.f >= a2->val.s) { NEW_IP(curr_cpu->ip + 2); } }
static void cmd_ifbel_ff(JIT_ARGS) { if (a1->val.f >= a2->val.f) { NEW_IP(curr_cpu->ip + 2); } }

/*-------------------------------------------------------
  - Arithmetic operators                                -
  -------------------------------------------------------*/
/*! ADD 2 u RW u RO = cmd_add_uu */
/*! ADD 2 u RW s RO = cmd_add_us */
/*! ADD 2 u RW f RO = cmd_add_uf */
/*! ADD 2 s RW u RO = cmd_add_su */
/*! ADD 2 s RW s RO = cmd_add_ss */
/*! ADD 2 s RW f RO = cmd_add_sf */
/*! ADD 2 f RW u RO = cmd_add_fu */
/*! ADD 2 f RW s RO = cmd_add_fs */
/*! ADD 2 f RW f RO = cmd_add_ff */
static void cmd_add_uu(JIT_ARGS) { a1->val.u+=a2->val.u; }
static void cmd_add_us(JIT_ARGS) { a1->val.u+=a2->val.s; }
static void cmd_add_uf(JIT_ARGS) { a1->val.u+=a2->val.f; }
static void cmd_add_su(JIT_ARGS) { a1->val.s+=a2->val.u; }
static void cmd_add_ss(JIT_ARGS) { a1->val.s+=a2->val.s; }
static void cmd_add_sf(JIT_ARGS) { a1->val.s+=a2->val.f; }
static void cmd_add_fu(JIT_ARGS) { a1->val.f+=a2->val.u; }
static void cmd_add_fs(JIT_ARGS) { a1->val.f+=a2->val.s; }
static void cmd_add_ff(JIT_ARGS) { a1->val.f+=a2->val.f; }

/*! SUB 2 u RW u RO = cmd_sub_uu */
/*! SUB 2 u RW s RO = cmd_sub_us */
/*! SUB 2 u RW f RO = cmd_sub_uf */
/*! SUB 2 s RW u RO = cmd_sub_su */
/*! SUB 2 s RW s RO = cmd_sub_ss */
/*! SUB 2 s RW f RO = cmd_sub_sf */
/*! SUB 2 f RW u RO = cmd_sub_fu */
/*! SUB 2 f RW s RO = cmd_sub_fs */
/*! SUB 2 f RW f RO = cmd_sub_ff */
static void cmd_sub_uu(JIT_ARGS) { a1->val.u-=a2->val.u; }
static void cmd_sub_us(JIT_ARGS) { a1->val.u-=a2->val.s; }
static void cmd_sub_uf(JIT_ARGS) { a1->val.u-=a2->val.f; }
static void cmd_sub_su(JIT_ARGS) { a1->val.s-=a2->val.u; }
static void cmd_sub_ss(JIT_ARGS) { a1->val.s-=a2->val.s; }
static void cmd_sub_sf(JIT_ARGS) { a1->val.s-=a2->val.f; }
static void cmd_sub_fu(JIT_ARGS) { a1->val.f-=a2->val.u; }
static void cmd_sub_fs(JIT_ARGS) { a1->val.f-=a2->val.s; }
static void cmd_sub_ff(JIT_ARGS) { a1->val.f-=a2->val.f; }

/*! MUL 2 u RW u RO = cmd_mul_uu */
/*! MUL 2 u RW s RO = cmd_mul_us */
/*! MUL 2 u RW f RO = cmd_mul_uf */
/*! MUL 2 s RW u RO = cmd_mul_su */
/*! MUL 2 s RW s RO = cmd_mul_ss */
/*! MUL 2 s RW f RO = cmd_mul_sf */
/*! MUL 2 f RW u RO = cmd_mul_fu */
/*! MUL 2 f RW s RO = cmd_mul_fs */
/*! MUL 2 f RW f RO = cmd_mul_ff */
static void cmd_mul_uu(JIT_ARGS) { a1->val.u*=a2->val.u; }
static void cmd_mul_us(JIT_ARGS) { a1->val.u*=a2->val.s; }
static void cmd_mul_uf(JIT_ARGS) { a1->val.u*=a2->val.f; }
static void cmd_mul_su(JIT_ARGS) { a1->val.s*=a2->val.u; }
static void cmd_mul_ss(JIT_ARGS) { a1->val.s*=a2->val.s; }
static void cmd_mul_sf(JIT_ARGS) { a1->val.s*=a2->val.f; }
static void cmd_mul_fu(JIT_ARGS) { a1->val.f*=a2->val.u; }
static void cmd_mul_fs(JIT_ARGS) { a1->val.f*=a2->val.s; }
static void cmd_mul_ff(JIT_ARGS) { a1->val.f*=a2->val.f; }

/*! DIV 2 u RW u RO = cmd_div_uu */
/*! DIV 2 u RW s RO = cmd_div_us */
/*! DIV 2 u RW f RO = cmd_div_uf */
/*! DIV 2 s RW u RO = cmd_div_su */
/*! DIV 2 s RW s RO = cmd_div_ss */
/*! DIV 2 s RW f RO = cmd_div_sf */
/*! DIV 2 f RW u RO = cmd_div_fu */
/*! DIV 2 f RW s RO = cmd_div_fs */
/*! DIV 2 f RW f RO = cmd_div_ff */
static void cmd_div_uu(JIT_ARGS) { a1->val.u/=a2->val.u; }
static void cmd_div_us(JIT_ARGS) { a1->val.u/=a2->val.s; }
static void cmd_div_uf(JIT_ARGS) { a1->val.u/=a2->val.f; }
static void cmd_div_su(JIT_ARGS) { a1->val.s/=a2->val.u; }
static void cmd_div_ss(JIT_ARGS) { a1->val.s/=a2->val.s; }
static void cmd_div_sf(JIT_ARGS) { a1->val.s/=a2->val.f; }
static void cmd_div_fu(JIT_ARGS) { a1->val.f/=a2->val.u; }
static void cmd_div_fs(JIT_ARGS) { a1->val.f/=a2->val.s; }
static void cmd_div_ff(JIT_ARGS) { a1->val.f/=a2->val.f; }

/*! MOD 2 u RW u RO = cmd_mod */
static void cmd_mod(JIT_ARGS) {
	if (a2->val.u == 0) throw_except(curr_cpu, ERR_MATHERROR);
	a1->val.u%=a2->val.u;
}

/*-------------------------------------------------------
  - Binary operators (no, no floating ones :)           -
  -------------------------------------------------------*/

/*! XOR 2 u RW u RO = cmd_xor */
static void cmd_xor(JIT_ARGS) { a1->val.u^=a2->val.u; }

/*! OR 2 u RW u RO = cmd_or */
static void cmd_or(JIT_ARGS) { a1->val.u|=a2->val.u; }

/*! AND 2 u RW u RO = cmd_and */
static void cmd_and(JIT_ARGS) { a1->val.u&=a2->val.u; }

/*! NOT 1 u RW = cmd_not */
static void cmd_not(JIT_ARGS) { a1->val.u=~a1->val.u; }

/*! SHL 2 u RW u RO = cmd_shl */
static void cmd_shl(JIT_ARGS) { a1->val.u<<=a2->val.u; }

/*! SHR 2 u RW u RO = cmd_shr */
static void cmd_shr(JIT_ARGS) { a1->val.u>>=a2->val.u; }

/*! ROL 2 u RW u RO = cmd_rol */
static void cmd_rol(JIT_ARGS) {
/* GCC IS PATHETIC! MY CAT CAN WRITE BETTER ASM! */
	unsigned i=a2->val.u;
	while (i) {
		/* this is the only way to get a REAL roll op? */
		a1->val.u=(a1->val.u << 1) | (a1->val.u >> (sizeof(unsigned) * 8 - 1));
		i--;
	}
}

/*! ROR 2 u RW u RO = cmd_ror */
static void cmd_ror(JIT_ARGS)
{
	unsigned i=a2->val.u;
	while (i) {
		/* This isn't a rorl op, this is STILL a roll op! */
		a1->val.u=(a1->val.u << (sizeof(unsigned) * 8 - 1)) | (a1->val.u >> 1);
		i--;
	}
}

/*-------------------------------------------------------
  - Stack handlers                                      -
  -------------------------------------------------------*/
/*! STACK 2 u RO u RO = cmd_stack */
static void cmd_stack(JIT_ARGS) {
	curr_cpu->mstack=a1->val.u;
	/* XXX: What if metastack is DOWNSIZED and ptr is large? */
	curr_cpu->mstack_size=a2->val.u;
}

/*! PUSH 1 u RO = cmd_push_u */
/*! PUSH 1 s RO = cmd_push_s */
/*! PUSH 1 f RO = cmd_push_f */
#define PUSH_STACK(a) if(curr_cpu->mstack_ptr >= curr_cpu->mstack_size) \
	throw_except(curr_cpu, ERR_MSTACK_OVER); \
	a=mem_rw(curr_cpu, curr_cpu->mstack_ptr + curr_cpu->mstack); \
	curr_cpu->mstack_ptr++
	
static void cmd_push_u(JIT_ARGS) { anyval *z;
	PUSH_STACK(z); z->val.u=a1->val.u; }
static void cmd_push_s(JIT_ARGS) { anyval *z;
	PUSH_STACK(z); z->val.s=a1->val.s; }
static void cmd_push_f(JIT_ARGS) { anyval *z;
	PUSH_STACK(z); z->val.f=a1->val.f; }

	
/*! POP 1 u RW = cmd_pop_u */
/*! POP 1 s RW = cmd_pop_s */
/*! POP 1 f RW = cmd_pop_f */
#define POP_STACK(a) if(curr_cpu->mstack_ptr == 0) \
	throw_except(curr_cpu, ERR_MSTACK_UNDER); \
	curr_cpu->mstack_ptr--; \
	a=mem_ro(curr_cpu, curr_cpu->mstack_ptr + curr_cpu->mstack)
static void cmd_pop_u(JIT_ARGS) { const anyval *z;
	POP_STACK(z); a1->val.u=z->val.u; }
static void cmd_pop_s(JIT_ARGS) { const anyval *z;
	POP_STACK(z); a1->val.s=z->val.s; }
static void cmd_pop_f(JIT_ARGS) { const anyval *z;
	POP_STACK(z); a1->val.f=z->val.f; }

/*-------------------------------------------------------
  - Memory handlers                                     -
  -------------------------------------------------------*/
/* NOTE #2: I dislike the ol' "block id" thing. An address
 * is equally unique to a memblock. So...
 */
/* NOTE: Not even ALLOC deserves explicit return registers.
 * So for ALLOC: we return map address in *size
 * For REALLOC: we be really tricky and overload it,
 * 	so us version changes permissions,
 * 	and uu version changes sizes.
 * DEALLOC returns nothing
 */

/*! ALLOC 2 u RW u RO = cmd_alloc */
static void cmd_alloc(JIT_ARGS) {
	a1->val.u=mem_alloc(curr_cpu, a1->val.u, a2->val.u);
}

/*! REALLOC 2 u RW u RO = cmd_realloc_size */
/*! REALLOC 2 u RW s RO = cmd_realloc_perm */
static void cmd_realloc_size(JIT_ARGS) {
	a1->val.u=mem_realloc(curr_cpu, a1->val.u, a2->val.u);
}

static void cmd_realloc_perm(JIT_ARGS) {
	mem_changeperm(curr_cpu, a1->val.u, a2->val.u);
}

/*! FREE 1 u RO = cmd_free */
static void cmd_free(JIT_ARGS) {
	mem_dealloc(curr_cpu, a1->val.u);
}

/*-------------------------------------------------------
  - Other execution modifiers                           -
  -------------------------------------------------------*/

/* Note: LOOP is now a 2-code op. Let's face it, using s0
 * for everything was just plain bad.
 * It runs like:
 * 	LOOP s:count, u:addr
 * or	LOOP u:count, u:addr
 * though I think the unsigned version is not 100% useful.
 */

/*! LOOP 2 s RW u RO = cmd_loop_s */
/*! LOOP 2 u RW u RO = cmd_loop_u */
/*! LOOP 2 s RW s RO = cmd_loop_ss */
/*! LOOP 2 u RW s RO = cmd_loop_us */

static void cmd_loop_s(JIT_ARGS) {
	if (a1->val.s > 0)
	{
		a1->val.s--;
		NEW_IP(a2->val.u);
	}
}

static void cmd_loop_u(JIT_ARGS) {
	if (a1->val.u)
	{
		a1->val.u--;
		NEW_IP(a2->val.u);
	}
}

static void cmd_loop_ss(JIT_ARGS) {
	if (a1->val.s > 0)
	{
		a1->val.s--;
		NEW_IP(curr_cpu->ip + a2->val.s);
	}
}

static void cmd_loop_us(JIT_ARGS) {
	if (a1->val.u)
	{
		a1->val.u--;
		NEW_IP(curr_cpu->ip + a2->val.s);
	}
}

/*! JMP 1 u RO = cmd_jmp */
/*! JMP 1 s RO = cmd_jmp_s */
static void cmd_jmp(JIT_ARGS) {	NEW_IP(a1->val.u); }
static void cmd_jmp_s(JIT_ARGS) { NEW_IP(curr_cpu->ip + a1->val.s); }

/*! CALL 1 u RO = cmd_call */
/*! CALL 1 s RO = cmd_call_s */
static void cmd_call(JIT_ARGS) {
	push_ip_on_stack(curr_cpu);
	NEW_IP(a1->val.u);
	curr_cpu->xip=0; /* Zero doesn't seem the best 'null' address, but WTF */
}
static void cmd_call_s(JIT_ARGS) {
	push_ip_on_stack(curr_cpu);
	NEW_IP(curr_cpu->ip + a1->val.s);
	curr_cpu->xip=0;
}


/*! RET 1 u RO = cmd_ret */
static void cmd_ret(JIT_ARGS) {
	unsigned work;

	work=a1->val.u;
	if (!work) work=1;

	pop_ip_from_stack(curr_cpu, work);
}

/*! HALT 0 = cmd_halt */
static void cmd_halt(JIT_ARGS) {
	/* Cheaty cheaty. Still, the exception mechanism
	 * is there for jumping to a more powerful level,
	 * we don't want to try deallocating ourselves here */
	throw_except(curr_cpu, X_CPUSHUTDOWN);
}

/*! WAIT 1 u RO = cmd_wait */
static void cmd_wait(JIT_ARGS) {
	/* We're thread-per-vcpu, so this is OK. */
	usleep(a1->val.u);
}

/*! SYSCALL 1 u RO = cmd_syscall */
static void cmd_syscall(JIT_ARGS) {
	agt_syscall(curr_cpu, a1->val.u, NULL);
}

/*! SYSCALL2 2 u RO u RO = cmd_syscall2 */
/*! SYSCALL2 2 u RO s RO = cmd_syscall2 */
/*! SYSCALL2 2 u RO f RO = cmd_syscall2 */
static void cmd_syscall2(JIT_ARGS) {
	agt_syscall(curr_cpu, a1->val.u, a2);
}

/*! HANDLER 1 u RO = cmd_handler */
static void cmd_handler(JIT_ARGS) {
	curr_cpu->xip=a1->val.u;
}

/*! RAISE 1 u RO = cmd_raise */
static void cmd_raise(JIT_ARGS) {
	throw_except(curr_cpu, a1->val.u);
}

/*-------------------------------------------------------
  - The bear essentials...                              -
  -------------------------------------------------------*/
/* No comment-bang for this one; it shouldn't get used. */
static void cmd_corrupt_bcode(JIT_ARGS) {
	throw_except(curr_cpu, ERR_CORRUPT_CODE);
}

/* Include table here to save us from heinous prototypes */
#include "cmdtabs.h"
