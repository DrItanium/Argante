/*
 * A2 syscall module: display
 *
 * Author:           hard to say, really: completely rewritten!
 * AOSr1 author:     Michal Zalewski <lcamtuf@ids.pl>
 * A2 author:        James Kehl <ecks@optusnet.com.au>
 *
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "module.h"
#include "amemory.h"
#include "exception.h"
#include "hhac.h"

/*! assigned 1 - 100 */ 

static inline int module_internal_init(int lid)
{
	printk2(PRINTK_INFO, "Display module loaded. \n");
	return 0;
}

static inline void module_internal_vcpu_start(struct vcpu *cpu) {}
static inline void module_internal_vcpu_stop(struct vcpu *cpu) {}
static inline void module_internal_shutdown(void) {}

/*! IO_PUTSTRING 0x1 = io_putstring */
static void io_putstring(SYSCALL_ARGS) {
	unsigned i;
	char *x=NULL;
	const anyval *p;
	
	VALIDATE("/display","/write/text");
	
	i=0;
	while(i < curr_cpu->reg[1].val.u)
	{
		if (!(i % sizeof(anyval))) { /* mem_ro throws the exceptions, not us */
			p=mem_ro(curr_cpu, curr_cpu->reg[0].val.u + i / sizeof(anyval));
			x=(char *) &p->val.u;
		} else {
			x++;
		}
		i++;
		/* Don't burp nuls, it's not polite. */
		if (*x) putc(*x, stdout);
	}
}

/*! IO_PUTINT 0x2 SYS2 = io_putint */
static void io_putint(SYSCALL_ARGS) {
	VALIDATE("/display","/write/integer");
	printf("%ld", arg->val.u);
}

/*! IO_PUTCHAR 0x3 SYS2 = io_putchar */
static void io_putchar(SYSCALL_ARGS) {
	VALIDATE("/display","/write/character");
	printf("%c", (char) arg->val.u);
}

/*! IO_PUTFLOAT 0x4 SYS2 = io_putfloat */
static void io_putfloat(SYSCALL_ARGS) {
	VALIDATE("/display","/write/float");
	printf("%g", arg->val.f);
}

/*! IO_PUTHEX 0x5 SYS2 = io_puthex */
static void io_puthex(SYSCALL_ARGS) {
	VALIDATE("/display","/write/hex");
	printf("%lx", arg->val.u);
}

/* Incorporate generated tables */
#include "display.h"
