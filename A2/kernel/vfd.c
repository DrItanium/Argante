/*
 * A2 Virtual Machine - Virtual File Descriptor wrappers
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

#include <stdio.h> /* perror */
#include <stdlib.h>
#include "printk.h"
#include "taskman.h"
#include "exception.h"
#include "vfd.h"
#include "cfdop.h"

struct vfd {
	unsigned lid; /* Library that owns this */
	void *data; /* Library-spec. data */
};

#define UNEG1 ((unsigned) -1)

/* No need to lock it, this is not touched outside CPU-thread */
void *vfd_get_data(struct vcpu *curr_cpu, unsigned lid, unsigned handle) {
	if (handle >= curr_cpu->fd_ct) throw_except(curr_cpu, ERR_BAD_FD);
	if (curr_cpu->fds[handle].lid != lid && lid!=A2_LID_ANY) throw_except(curr_cpu, ERR_BAD_FD);
	return curr_cpu->fds[handle].data;
}

/* No LID_ANY provisions here! Don't mess with others' data! */
void vfd_set_data(struct vcpu *curr_cpu, unsigned lid, unsigned handle, void *newd) {
	if (handle >= curr_cpu->fd_ct) throw_except(curr_cpu, ERR_BAD_FD);
	if (curr_cpu->fds[handle].lid != lid) throw_except(curr_cpu, ERR_BAD_FD);
	curr_cpu->fds[handle].data = newd;
}

int vfd_alloc_new(struct vcpu *curr_cpu, unsigned lid) {
	unsigned i, j;
	struct vfd *q;
	for(i=0;i < curr_cpu->fd_ct;i++) if (curr_cpu->fds[i].lid == UNEG1) {
		curr_cpu->fds[i].lid=lid;
		return i;
	}

	if (curr_cpu->fd_ct >= A2_MAX_VFDS) throw_except(curr_cpu, ERR_TOOMANY_FDS);
	i=curr_cpu->fd_ct + 8;
	if (i > A2_MAX_VFDS) i=A2_MAX_VFDS;
	
	if (curr_cpu->fds)
		q=realloc(curr_cpu->fds, i * sizeof(struct vfd));
	else
		q=malloc(i * sizeof(struct vfd));
	if (!q) {
		perror("malloc");
		throw_except(curr_cpu, ERR_OOM);
	}

	curr_cpu->fds=q;
	for(j=curr_cpu->fd_ct;j<i;j++) {
		curr_cpu->fds[j].lid=UNEG1;
		curr_cpu->fds[j].data=NULL;
	}
	j=curr_cpu->fd_ct;
	curr_cpu->fd_ct=i;

	curr_cpu->fds[j].lid=lid;
	return j;
}

/* Laziness etc. - should downsize fd struct if possible;
 * then again is it needed? how many apps will open 30 fds simultaneously,
 * then close them all, and keep running? Even NAGTv1 only used 8 or so :) */
void vfd_dealloc(struct vcpu *curr_cpu, unsigned lid, unsigned handle) {
	/* Take care... if this check serves any useful purpose,
	 * your module has serious problems! */
	if (handle >= curr_cpu->fd_ct) throw_except(curr_cpu, ERR_BAD_FD);
	if (curr_cpu->fds[handle].lid != lid && lid!=A2_LID_ANY) throw_except(curr_cpu, ERR_BAD_FD);

	curr_cpu->fds[handle].lid=UNEG1;
	curr_cpu->fds[handle].data=NULL;
}

/* For vcpu_stop stuff */
int vfd_find_mine(struct vcpu *curr_cpu, unsigned lid) {
	unsigned j;
	for(j=0;j<curr_cpu->fd_ct;j++) if (curr_cpu->fds[j].lid==lid) return j;
	return UNEG1;
}

/* Destroy ALL VFD's. */
void vfd_obliviate(struct vcpu *from) {
	unsigned i;
	for(i=0;i<from->fd_ct;i++)
		if (from->fds[i].lid != UNEG1) printk2(PRINTK_CRIT, "Library %d left a VFD behind!\n", from->fds[i].lid);
	if (from->fds)
		free(from->fds);
	from->fd_ct=0;
}

/* CFD Ops */
const struct cfdop_1 *cfdop1_vfd_get(struct vcpu *curr_cpu, unsigned handle) {
	if (handle >= curr_cpu->fd_ct) throw_except(curr_cpu, ERR_BAD_FD);
	return cfdop1_lid_get(curr_cpu->fds[handle].lid);
}

