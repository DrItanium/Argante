/*
 * A2 syscall module: dummy
 *
 * VFD tester.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>

#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "exception.h"
#include "amemory.h"
#include "module.h"
#include "hhac.h"
#include "vfd.h"

struct fs_vcpu {
	char *wd;
};

/* Reent-OK because only used at init time... I hope */
static int stored_lid; 

static inline int module_internal_init(int lid)
{
	stored_lid=lid;
	printk2(PRINTK_INFO, "A2 Dummy FD module loaded.\n");
	return 0;
}

static inline void module_internal_vcpu_start(struct vcpu *cpu) {
}

static inline void module_internal_vcpu_stop(struct vcpu *cpu) {
	int fdh, c=0;

	/* The difference between >0 and >=0 is amazing. */
	while((fdh=vfd_find_mine(cpu, stored_lid)) >= 0) {
		vfd_dealloc(cpu, stored_lid, fdh);
		c++;
	}
	if (c) printk2(PRINTK_WARN, "DummyFD: freed %d unclosed VFD's\n", c);
}

static inline void module_internal_shutdown() {
}

/*! assigned 901 - 905 */ 

/*! DUMMY_OPEN 901 = fs_open */
static void fs_open(SYSCALL_ARGS) {
	int j;
	j=vfd_alloc_new(curr_cpu, stored_lid);
	vfd_set_data(curr_cpu, stored_lid, j, NULL);
	curr_cpu->reg[0].val.u=j;
}

/*! DUMMY_CLOSE 902 SYS2 = fs_close */
static void fs_close(SYSCALL_ARGS) {
	vfd_dealloc(curr_cpu, stored_lid, arg->val.u);
}

/* Incorporate generated tables */
#include "dummy.h"
