/*
 * A2 syscall module: strfd
 *
 * Strings using VFD.
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
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>

#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "exception.h"
#include "amemory.h"
#include "module.h"
#include "hhac.h"
#include "vfd.h"
#include "cfdop.h"

static int stored_lid;

#define STRFD_DYNAMIC 2
#define STRFD_STATIC 1

struct strfd_vfd {
	int type;
	unsigned start_addr;
	unsigned size;
	unsigned offset; /* in characters */
};

static size_t strfd_cio_start(struct vcpu *curr_cpu, struct strfd_vfd *vfd);
static size_t strfd_cio_read(struct vcpu *curr_cpu, struct strfd_vfd *vfd, char *buf, size_t size);
static size_t strfd_cio_write(struct vcpu *curr_cpu, struct strfd_vfd *vfd, const char *buf, size_t size);
static void strfd_cio_close (struct vcpu *curr_cpu, struct strfd_vfd *vfd);

/* CFD ops table */
static const struct cfdop_1 strfd_ops = {
	(cfdop_start *) strfd_cio_start, /* Same start-read and start-write rtns */
	(cfdop_start *) strfd_cio_start,
	(cfdop_read_block *) strfd_cio_read,
	(cfdop_write_block *) strfd_cio_write,
	(cfdop_close_fd *) strfd_cio_close,
	/* We don't accept Agent-VFD's - could be fun though - Commandlines anyone? */
	A2_CFDDESC_NONE, NULL
};

static inline int module_internal_init(int lid)
{
	stored_lid=lid;
	cfdop1_lid_set(lid, &strfd_ops);

	printk2(PRINTK_INFO, "A2 StringStream module loaded.\n");
	return 0;
}

static inline void module_internal_vcpu_start(struct vcpu *cpu) {
}

static inline void module_internal_vcpu_stop(struct vcpu *cpu) {
	int fdh, c=0;
	struct strfd_vfd *fd;

	while((fdh=vfd_find_mine(cpu, stored_lid)) >= 0) {
		fd=vfd_get_data(cpu, stored_lid, fdh);
		if (fd) free(fd);
		vfd_dealloc(cpu, stored_lid, fdh);
		c++;
	}
	if (c) printk2(PRINTK_WARN, "StringStream: freed %d unclosed VFD's\n", c);
}

static inline void module_internal_shutdown() {
}

/*! assigned 401 - 500 */ 

/* Open an existing buffer as a strstream.
 * r0 - buffer addr, r1 - buffer size */
/*! STRFD_OPEN 401 = strfd_open */
static void strfd_open(SYSCALL_ARGS) {
	int j;
	struct strfd_vfd *new;
	new=malloc(sizeof(struct strfd_vfd));
	if (!new) throw_except(curr_cpu, ERR_OOM);

	new->type=STRFD_STATIC;
	new->start_addr=curr_cpu->reg[0].val.u;
	new->size=curr_cpu->reg[1].val.u;
	new->offset=0;

	j=vfd_alloc_new(curr_cpu, stored_lid);
	vfd_set_data(curr_cpu, stored_lid, j, new);
	curr_cpu->reg[0].val.u=j;
}

/* Create a new strfd.
 * arg - initial size */
/*! STRFD_CREATE 402 SYS2 = strfd_create */
static void strfd_create(SYSCALL_ARGS) {
	int j;
	struct strfd_vfd *new;
	new=malloc(sizeof(struct strfd_vfd));
	if (!new) throw_except(curr_cpu, ERR_OOM);

	new->type=STRFD_DYNAMIC;
	new->start_addr=mem_alloc(curr_cpu, dwords_of_bytes(arg->val.u),
		A2_MEM_READ | A2_MEM_WRITE);
	new->size=arg->val.u;
	new->offset=0;

	j=vfd_alloc_new(curr_cpu, stored_lid);
	vfd_set_data(curr_cpu, stored_lid, j, new);
	curr_cpu->reg[0].val.u=j;
}

/*! STRFD_CLOSE 403 SYS2 = strfd_close */
static void strfd_cio_close (struct vcpu *curr_cpu, struct strfd_vfd *vfd) {
	if (vfd) free(vfd);
}

static void strfd_close(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	strfd_cio_close(curr_cpu, fd);
	vfd_dealloc(curr_cpu, stored_lid, arg->val.u);
}

/*! STRFD_GET_OFFSET 404 SYS2 = strfd_get_offset */
static void strfd_get_offset(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	curr_cpu->reg[0].val.u=fd->offset;
}

/*! STRFD_SET_OFFSET 405 SYS2 = strfd_set_offset */
static void strfd_set_offset(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	fd->offset=curr_cpu->reg[1].val.u;
}

/*! STRFD_GET_ADDR 406 SYS2 = strfd_get_addr */
static void strfd_get_addr(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	curr_cpu->reg[0].val.u=fd->start_addr;
}

/*! STRFD_GET_SIZE 407 SYS2 = strfd_get_size */
static void strfd_get_size(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	curr_cpu->reg[0].val.u=fd->size;
}

/* It's really quite arbitrary - we can deal with as much memory as can be buffered */
static size_t strfd_cio_start(struct vcpu *curr_cpu, struct strfd_vfd *vfd) { return 256; }

/* REMEMBER CANCELLATION!!! - course it doesn't happen here... */
static size_t strfd_cio_write(struct vcpu *curr_cpu, struct strfd_vfd *fd, const char *buf, size_t size)
{
	size_t i;
	unsigned tof;
	anyval *p;

	/* Hopefully not an off-by-one: we should allow dynamic to grow
	 * when offset=size */
	if (fd->offset > fd->size) throw_except(curr_cpu, ERR_STRFD_BOUNDS);
	if (fd->offset + size > fd->size) {
		if (fd->type == STRFD_DYNAMIC) {
			fd->start_addr=mem_realloc(curr_cpu, fd->start_addr, dwords_of_bytes(fd->offset+size));
			fd->size=fd->offset+size;
		} else {
		/* maybe they've given us a size for a REASON? */
			if (fd->offset < fd->size)
			strfd_cio_write(curr_cpu, fd, buf, fd->size - fd->offset);
			throw_except(curr_cpu, ERR_STRFD_BOUNDS);
		}
	}

	tof=fd->offset; i=0;

	p=mem_rw(curr_cpu, fd->start_addr + fd->offset / sizeof(anyval));
	while(i < size) {
		if (!(tof % sizeof(anyval))) {
			p=mem_rw(curr_cpu, fd->start_addr + tof / sizeof(anyval));
		}
		((char *) p)[tof % sizeof(anyval)]=buf[i];
		i++; fd->offset++; tof++;
	}
	return i;
}

static size_t strfd_cio_read(struct vcpu *curr_cpu, struct strfd_vfd *fd, char *buf, size_t size)
{
	size_t i;
	unsigned tof;
	const anyval *p;

	tof=fd->offset; i=0;
	if (fd->offset >= fd->size) throw_except(curr_cpu, ERR_STRFD_BOUNDS);
	if (fd->offset + size > fd->size) {
		/* have to die */
		strfd_cio_read(curr_cpu, fd, buf, fd->size - fd->offset);
		throw_except(curr_cpu, ERR_STRFD_BOUNDS);
	}

	p=mem_ro(curr_cpu, fd->start_addr + fd->offset / sizeof(anyval));
	while(i < size) {
		if (!(tof % sizeof(anyval))) {
			p=mem_ro(curr_cpu, fd->start_addr + tof / sizeof(anyval));
		}
		buf[i]=((const char *) p)[tof % sizeof(anyval)];
		i++; fd->offset++; tof++;
	}
	return i;
}

/*! STRFD_READ 408 SYS2 = strfd_read */
/* r1 - addr r2 - buf size; return r1 - end of string r2 - 0 */
static void strfd_read(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	char *buf;
	size_t i, bs, siz;

	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);

	siz=curr_cpu->reg[2].val.u;
	bs=strfd_cio_start(curr_cpu, fd);

	for(i=0;i<siz;i+=bs) {
		if (bs > (siz - i)) bs=siz-i;
		buf=(char *)mem_rw_block(curr_cpu, curr_cpu->reg[1].val.u, dwords_of_bytes(bs));
		strfd_cio_read(curr_cpu, fd, buf, bs);
		/* Watch that cancel. Got to keep consistent... sure we're a special case... */
		curr_cpu->reg[1].val.u+=bs / sizeof(anyval);
		curr_cpu->reg[2].val.u-=bs;
	}
}

/*! STRFD_WRITE 409 SYS2 = strfd_write */
/* r1 - addr r2 - buf size; return r1 - end of string r2 - 0 */
static void strfd_write(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	const char *buf;
	size_t i, bs, siz;

	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);

	siz=curr_cpu->reg[2].val.u;
	bs=strfd_cio_start(curr_cpu, fd);

	for(i=0;i<siz;i+=bs) {
		if (bs > (siz - i)) bs=siz-i;
		buf=(char *)mem_ro_block(curr_cpu, curr_cpu->reg[1].val.u, dwords_of_bytes(bs));
		strfd_cio_write(curr_cpu, fd, buf, bs);
		curr_cpu->reg[1].val.u+=bs / sizeof(anyval);
		curr_cpu->reg[2].val.u-=bs;
	}
}

/* Gets character AND INCREMENTS OFFSET */
/*! STRFD_GETCHAR 410 SYS2 = strfd_getchar */
static void strfd_getchar(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	const anyval *p;
	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	if (fd->offset >= fd->size) throw_except(curr_cpu, ERR_STRFD_BOUNDS);
	p=mem_ro(curr_cpu, fd->start_addr + fd->offset / sizeof(anyval));
	curr_cpu->reg[0].val.u=((const char *) p)[fd->offset % sizeof(anyval)];
	fd->offset++;
}

/* Sets character AND INCREMENTS OFFSET */
/*! STRFD_SETCHAR 411 SYS2 = strfd_setchar */
static void strfd_setchar(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	char c;
	c=(char) curr_cpu->reg[1].val.u;
	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	/* Do it via _write so we grow dynamics */
	strfd_cio_write(curr_cpu, fd, &c, 1);
}

/* Shift offset by r1 until STRFD_GETCHAR would return r2 */
/*! STRFD_STRCHR 412 SYS2 = strfd_strchr */
static void strfd_strchr(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	int shift;
	unsigned addr;
	char needle;
	const anyval *p;

	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	if (fd->offset >= fd->size) throw_except(curr_cpu, ERR_STRFD_BOUNDS);

	shift=curr_cpu->reg[1].val.s; /* ! */
	if (!shift) shift=1;
	needle=(char) curr_cpu->reg[2].val.u;

	addr=fd->offset / sizeof(anyval);
	p=mem_ro(curr_cpu, fd->start_addr + addr);
	while(((const char *) p)[fd->offset % sizeof(anyval)] != needle) {
		fd->offset+=shift; /* hm. unsigned + signed fingers x'd */
		if (fd->offset >= fd->size) throw_except(curr_cpu, ERR_STRFD_SEARCHFAIL);
		if (addr != fd->offset / sizeof(anyval)) {
			addr=fd->offset / sizeof(anyval);
			p=mem_ro(curr_cpu, fd->start_addr + addr);
		}
	}
}

/* Shift offset by r1 until STRFD_READ would return r2:r3 */
/*! STRFD_STRSTR 413 SYS2 = strfd_strstr */
static void strfd_strstr(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	int shift;
	unsigned addr;
	unsigned needleaddr, needlelen, i;
	const anyval *p, *q;

	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	shift=curr_cpu->reg[1].val.s; /* ! */
	needleaddr=curr_cpu->reg[2].val.u;
	needlelen=curr_cpu->reg[3].val.u;
	if (!shift) shift=1;

	if (fd->offset >= fd->size) throw_except(curr_cpu, ERR_STRFD_BOUNDS);

	addr=fd->offset / sizeof(anyval);
	
	q=mem_ro(curr_cpu, needleaddr);
	p=mem_ro(curr_cpu, fd->start_addr + addr);

	while(1) {
		if (((char *) p)[fd->offset % sizeof(anyval)] == ((char *) q)[0]) {
			const anyval *r, *s;
			r=p;
			s=q;
			/* Oooh! A possible match! */
			for(i=1;i<needlelen;i++) {
				if ((i+fd->offset) % sizeof(anyval) == 0)
					p=mem_ro(curr_cpu, fd->start_addr + (i+fd->offset) / sizeof(anyval));
				if (i % sizeof(anyval) == 0)
					q=mem_ro(curr_cpu, needleaddr + i / sizeof(anyval));
				if (((char *) p)[(i+fd->offset) % sizeof(anyval)] != ((char *) q)[i % sizeof(anyval)])
					break;
			}
			if (i == needlelen) return;
			p=r;
			q=s;
		}
		fd->offset+=shift; /* hm. unsigned + signed fingers x'd */
		if (fd->offset >= fd->size) throw_except(curr_cpu, ERR_STRFD_SEARCHFAIL);
		if (addr != fd->offset / sizeof(anyval)) {
			addr=fd->offset / sizeof(anyval);
			p=mem_ro(curr_cpu, fd->start_addr + addr);
		}
	}
}

/* set r0 to first difference between string and r1:r2
 * should really be called strNcmp */
/*! STRFD_STRCMP 414 SYS2 = strfd_strcmp */
static void strfd_strcmp(SYSCALL_ARGS) {
	struct strfd_vfd *fd;
	const anyval *p, *q;
	const char *x, *y;
	unsigned len, needleaddr, i;

	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	if (fd->offset >= fd->size) throw_except(curr_cpu, ERR_STRFD_BOUNDS);
	
	len=curr_cpu->reg[2].val.u;
	needleaddr=curr_cpu->reg[1].val.u;

	p=mem_ro(curr_cpu, needleaddr);
	x=(char *) p;
	q=mem_ro(curr_cpu, fd->start_addr + fd->offset / sizeof(anyval));
	y=(char *) q + (fd->offset % sizeof(anyval));

	for(i=1;*x == *y && i < len;i++) {
		if (!(i % sizeof(anyval))) {
			p=mem_ro(curr_cpu, needleaddr + i / sizeof(anyval));
			x=(char *) p;
		} else x++;
		if (!((fd->offset + i) % sizeof(anyval))) {
			q=mem_ro(curr_cpu, fd->start_addr + (fd->offset + i) / sizeof(anyval));
			y=(char *) q;
		} else y++;
	}
	curr_cpu->reg[0].val.s=*x - *y;
}

/* TODO: STRFD_SPN, STRFD_CSPN, and maybe even STRFD_REGEXP, STRFD_SUBST */

/* Incorporate generated tables */
#include "strfd.h"
