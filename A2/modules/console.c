/*
 * A2 syscall module: console
 * designed for use in combination with Generic IO...
 * may eventually evolve into a Ncurses-style interface
 *
 * author:		James Kehl <ecks@optusnet.com.au>
 *
 * Copyright (c) 2002	James Kehl <ecks@optusnet.com.au>
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
#include "compat/io.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "module.h"
#include "vfd.h"
#include "cfdop.h"
#include "amemory.h"
#include "exception.h"
#include "hhac.h"

#define CON_FDDESC 0x
/*! assigned 601 - 600 */ 

static int stored_lid;

struct con_vfd {
	int in;
	int out;
};

static size_t con_sread(struct vcpu *curr_cpu, struct con_vfd *vfd);
static size_t con_swrite(struct vcpu *curr_cpu, struct con_vfd *vfd);
static size_t con_write(struct vcpu *curr_cpu, struct con_vfd *vfd, const char *buf, size_t size);
static size_t con_read(struct vcpu *curr_cpu, struct con_vfd *vfd, char *buf, size_t size);
static void con_close (struct vcpu *curr_cpu, struct con_vfd *vfd);
static int con_create (struct vcpu *curr_cpu, const char *desc, int in, int out);

/* CFD ops table */
static struct cfdop_1 con_ops = {
	(cfdop_start *) con_sread,
	(cfdop_start *) con_swrite,
	(cfdop_read_block *) con_read,
	(cfdop_write_block *) con_write,
	(cfdop_close_fd *) con_close,
	0, /* Changed later */
	(cfdop_create_fd *) con_create
};

static inline int module_internal_init(int lid)
{
	stored_lid=lid;
	/* TCON -> 0x54434f4e on BE systems */
	con_ops.fd_desc=*((int *) "TCON"); /* ehhe - 64bit warn! */
	cfdop1_lid_set(lid, &con_ops);

	printk2(PRINTK_INFO, "Console module loaded.\n");
	return 0;
}

static inline void module_internal_shutdown(void) {};

static inline void module_internal_vcpu_start(struct vcpu *cpu) {}

static inline void module_internal_vcpu_stop(struct vcpu *cpu)
{
	int fdh;
	struct con_vfd *fd;

	/* Don't blab about unclosed FD's - for console who cares? */
	while((fdh=vfd_find_mine(cpu, stored_lid)) >= 0) {
		fd=vfd_get_data(cpu, stored_lid, fdh);
		if (fd) free(fd);
		vfd_dealloc(cpu, stored_lid, fdh);
	}
}

static int con_create (struct vcpu *curr_cpu, const char *desc, int in, int out) {
	int j;
	struct con_vfd *new;
	new=malloc(sizeof(struct con_vfd));
	if (!new) return 0;

	new->in=in;
	new->out=out;

	j=vfd_alloc_new(curr_cpu, stored_lid);
	vfd_set_data(curr_cpu, stored_lid, j, new);
	return j;
}

static void con_close (struct vcpu *curr_cpu, struct con_vfd *vfd) {
	if (vfd) free(vfd);
}

static size_t con_sread(struct vcpu *curr_cpu, struct con_vfd *vfd) {
	VALIDATE("/console","/read");
	return 256;
}

static size_t con_swrite(struct vcpu *curr_cpu, struct con_vfd *vfd) {
	VALIDATE("/console","/write");
	return 256;
}

static size_t con_write(struct vcpu *curr_cpu, struct con_vfd *vfd, const char *buf, size_t size) {
	int x;
	x=write(vfd->out, buf, size);
	if (x < 0) throw_except(curr_cpu, ERR_GENERIC);
	return x;
}

static size_t con_read(struct vcpu *curr_cpu, struct con_vfd *vfd, char *buf, size_t size) {
	int x;
	x=read(vfd->in, buf, size);
	if (x < 0) throw_except(curr_cpu, ERR_GENERIC);
	return x;
}

/* Incorporate generated tables */
#include "console.h"
