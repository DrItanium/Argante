/*
 * A2 syscall module: alib
 *
 * Run-time linker controls.
 * 
 * A2 maintainer:         James Kehl <ecks@optusnet.com.au>
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
#include "compat/alloca.h"
#include "compat/limits.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "bformat.h"
#include "exception.h"
#include "amemory.h"
#include "module.h"
#include "hhac.h"
#include "file.h"
#include "imageman.h"
#include "symman.h"

static inline int module_internal_init(int lid)
{
	printk2(PRINTK_INFO, "ALib syscalls loaded.\n");
	return 0;
}

static inline void module_internal_vcpu_start(struct vcpu *cpu) {}

static inline void module_internal_vcpu_stop(struct vcpu *cpu) {}

static inline void module_internal_shutdown(void) {
}

/*! assigned 1901 - 1905 */ 

/*! ALIB_OPEN 1901 = alib_open */
/* r0 - path r1 - path len
 * return: r0 - alib handle
 * throws ERR_ALIB_FAIL */

/* Lifted from FS module! bEwArE!!!! */
static void alib_open(SYSCALL_ARGS) {
	char *HACname;
	char *filename, *username; 
	const char *t;
	unsigned bufsize, bufaddr;
	ALLOCA_STACK;

	bufaddr=curr_cpu->reg[0].val.u;
	bufsize=curr_cpu->reg[1].val.u;

	/* Ok. Step 1, retrieve the file path. */
	/* Alloca segfaults if it runs out of memory, but real filenames
	 * can't exceed PATH_MAX or so anyway. Test it. */
	if (bufsize >= PATH_MAX) throw_except(curr_cpu, ERR_ARG_TOOLONG);

	/* ... check that mem is good for what it claims */
	t=(const char *) mem_ro_block(curr_cpu, bufaddr, dwords_of_bytes(bufsize));

	if (*t == '/') /* Absolute path */ {
		filename=alloca(bufsize + 6); /* Remember Astrings aren't 0-term */
		strcpy(filename, "./lib");
		username=filename + 5;
	} else {
		filename=alloca(bufsize + 7);
		strcpy(filename, "./lib/");
		username=filename + 6;
	}
	HACname=filename + 1;
	/* Now an atosys strcpy */
	memcpy(username, t, bufsize);
	username[bufsize]=0; /* NT-it */

	/* Now fold it */
	fold(username);
	/* Do it! */
	VALIDATE(HACname, "/load");
	printk2(PRINTK_DEBUG, "Alib: loading %s\n", filename);
	curr_cpu->reg[0].val.u=imageman_loadimage(filename, curr_cpu);
	if (!curr_cpu->reg[0].val.u) throw_except(curr_cpu, ERR_ALIB_FAIL);
}

/*! ALIB_LOOKUP 1902 SYS2 = alib_lookup */
/* arg = alib handle or 0 for any
 * r1 = symbol r2 - sym len
 * return: r0 - sym address
 * throws ERR_ALIB_NOSYM and maybe ERR_ALIB_FAIL on bad alib_id */

static void alib_lookup(SYSCALL_ARGS) {
	struct symbol *x;
	char buf[A2_MAX_SNAME + 1];
	if (arg->val.u > A2_MAX_ALID) throw_except(curr_cpu, ERR_ALIB_FAIL); 
	if (curr_cpu->reg[2].val.u > A2_MAX_SNAME) throw_except(curr_cpu, ERR_ARG_TOOLONG);

	atokern_memcpy(curr_cpu, buf, curr_cpu->reg[1].val.u, curr_cpu->reg[2].val.u);
	buf[A2_MAX_SNAME]=0; /* XXX: Are the other functions safe from non-0term strings? */
	
	printk2(PRINTK_DEBUG, "Alib: looking up %32.32s\n", buf);

	if ((x=symman_find_symbol(curr_cpu, buf, arg->val.u))) {
		if (!(x->s.place & SYM_UNDEFINED)) {
			curr_cpu->reg[0].val.u=x->s.addr;
			return;
		}
	}
	throw_except(curr_cpu, ERR_ALIB_NOSYM);
}

/*! ALIB_CLOSE 1903 SYS2 = alib_close */
/* arg = alib handle
 * throws ERR_ALIB_FAIL if you pass it a bad id */
static void alib_close(SYSCALL_ARGS) {
	if (arg->val.u > A2_MAX_ALID || !arg->val.u) throw_except(curr_cpu, ERR_ALIB_FAIL); 
	if (imageman_unload_al(curr_cpu, arg->val.u)) {
		throw_except(curr_cpu, ERR_ALIB_FAIL);
	}
}

/* Incorporate generated tables */
#include "alib.h"
