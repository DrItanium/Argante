/*
 * A2 syscall module: cfdop
 *
 * Common filedescriptor operations.
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

#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "exception.h"
#include "amemory.h"
#include "module.h"
#include "hhac.h"
#include "vfd.h"
#include "cfdop.h"

static inline int module_internal_init(int lid)
{
	printk2(PRINTK_INFO, "Common FD operations available.\n");
	return 0;
}

static inline void module_internal_vcpu_start(struct vcpu *cpu) {}
static inline void module_internal_vcpu_stop(struct vcpu *cpu) {}
static inline void module_internal_shutdown() {}

/*! assigned 501 - 600 */ 

/*! CFD_CLOSE 501 SYS2 = cfd_close */
static void cfd_close(SYSCALL_ARGS) {
	void *fd;
	const struct cfdop_1 *tbl;
	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->fd_close) throw_except(curr_cpu, ERR_BAD_FD);

	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);
	tbl->fd_close(curr_cpu, fd);
	vfd_dealloc(curr_cpu, A2_LID_ANY, arg->val.u);
}

/*! CFD_READ 510 SYS2 = cfd_read */
/* r1 - addr r2 - buf size; return r1 - end of string r2 - 0 */
static void cfd_read(SYSCALL_ARGS) {
	void *fd;
	char *buf;
	size_t i, bs, siz, bs2;
	const struct cfdop_1 *tbl;

	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->read_start || !tbl->read_block) throw_except(curr_cpu, ERR_BAD_FD);
	
	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);

	siz=curr_cpu->reg[2].val.u;
	bs=tbl->read_start(curr_cpu, fd);
	/* We cannot use a blocksize that is not going to change the offset! */ 
	if (bs % sizeof(anyval)) {
		printk2(PRINTK_CRIT, "read_start returned an illegal value");
		throw_except(curr_cpu, ERR_GENERIC);
	}
	
	for(i=0;i<siz;i+=bs) {
		if (bs > (siz - i)) bs=siz-i;
		buf=(char *)mem_rw_block(curr_cpu, curr_cpu->reg[1].val.u, dwords_of_bytes(bs));
		bs2=tbl->read_block(curr_cpu, fd, buf, bs);
		/* Got to keep consistent if we cancel... */
		curr_cpu->reg[1].val.u+=bs2 / sizeof(anyval);
		curr_cpu->reg[2].val.u-=bs2;
		if (bs2 != bs) return; /* Out of data! */
	}
}

/*! CFD_WRITE 520 SYS2 = cfd_write */
/* r1 - addr r2 - buf size; return r1 - end of string r2 - 0 */
static void cfd_write(SYSCALL_ARGS) {
	void *fd;
	const char *buf;
	size_t i, bs, siz, bs2;
	const struct cfdop_1 *tbl;

	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->write_start || !tbl->write_block) throw_except(curr_cpu, ERR_BAD_FD);

	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);

	siz=curr_cpu->reg[2].val.u;
	bs=tbl->write_start(curr_cpu, fd);
	/* We cannot use a blocksize that is not going to change the offset! */ 
	if (bs % sizeof(anyval)) {
		printk2(PRINTK_CRIT, "write_start returned an illegal value");
		throw_except(curr_cpu, ERR_GENERIC);
	}

	for(i=0;i<siz;i+=bs) {
		if (bs > (siz - i)) bs=siz-i;
		buf=(char *)mem_ro_block(curr_cpu, curr_cpu->reg[1].val.u, dwords_of_bytes(bs));
		bs2=tbl->write_block(curr_cpu, fd, buf, bs);
		curr_cpu->reg[1].val.u+=bs2 / sizeof(anyval);
		curr_cpu->reg[2].val.u-=bs2;
		if (bs2 != bs) return;
	}
}

/*! CFD_WRITE_NT 521 SYS2 = cfd_write_nt */
/* r1 - addr r2 - buf size; return r1 - addr of first \000 r2 - length left */
static void cfd_write_nt(SYSCALL_ARGS) {
	void *fd;
	const char *buf, *bp;
	size_t i, bs, siz, bs2;
	const struct cfdop_1 *tbl;

	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->write_start || !tbl->write_block) throw_except(curr_cpu, ERR_BAD_FD);

	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);
	siz=curr_cpu->reg[2].val.u;
	bs=tbl->write_start(curr_cpu, fd);
	/* We cannot use a blocksize that is not going to change the offset! */ 
	if (bs % sizeof(anyval)) {
		printk2(PRINTK_CRIT, "write_start returned an illegal value");
		throw_except(curr_cpu, ERR_GENERIC);
	}

	for(i=0;i<siz;i+=bs) {
		if (bs > (siz - i)) bs=siz-i;
		buf=(char *)mem_ro_block(curr_cpu, curr_cpu->reg[1].val.u, dwords_of_bytes(bs));
		bp=memchr(buf, 0, bs);
		if (bp) bs=bp-buf; /* Might be 0... */
		bs2=tbl->write_block(curr_cpu, fd, buf, bs);
		curr_cpu->reg[1].val.u+=bs2 / sizeof(anyval);
		curr_cpu->reg[2].val.u-=bs2;
		if (bp || bs2 != bs) return;
	}
}

/*! CFD_WRITE_CHAR 522 SYS2 = cfd_write_char */
/* r1 - char */
static void cfd_write_char(SYSCALL_ARGS) {
	void *fd;
	const struct cfdop_1 *tbl;
	char c;

	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->write_start || !tbl->write_block) throw_except(curr_cpu, ERR_BAD_FD);

	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);
	tbl->write_start(curr_cpu, fd);
	c=curr_cpu->reg[1].val.u;
	tbl->write_block(curr_cpu, fd, (char *) &c, sizeof(char));
}

/*! CFD_WRITE_INT 523 SYS2 = cfd_write_int */
/* r1 - int to print */
static void cfd_write_int(SYSCALL_ARGS) {
	void *fd;
	const struct cfdop_1 *tbl;
	char c[16];
	unsigned c_len;

	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->write_start || !tbl->write_block) throw_except(curr_cpu, ERR_BAD_FD);

	c_len=snprintf(c, sizeof(c), "%ld", curr_cpu->reg[1].val.s);
	if (c_len >= sizeof(c)) throw_except(curr_cpu, ERR_ARG_TOOLONG);

	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);
	tbl->write_start(curr_cpu, fd);
	tbl->write_block(curr_cpu, fd, c, c_len);
}

/*! CFD_WRITE_UINT 524 SYS2 = cfd_write_uint */
/* r1 - int to print */
static void cfd_write_uint(SYSCALL_ARGS) {
	void *fd;
	const struct cfdop_1 *tbl;
	char c[16];
	unsigned c_len;

	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->write_start || !tbl->write_block) throw_except(curr_cpu, ERR_BAD_FD);

	c_len=snprintf(c, sizeof(c), "%lu", curr_cpu->reg[1].val.u);
	if (c_len >= sizeof(c)) throw_except(curr_cpu, ERR_ARG_TOOLONG);

	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);
	tbl->write_start(curr_cpu, fd);
	tbl->write_block(curr_cpu, fd, c, c_len);
}

/*! CFD_WRITE_HEX 525 SYS2 = cfd_write_hex */
/* r1 - int to print */
static void cfd_write_hex(SYSCALL_ARGS) {
	void *fd;
	const struct cfdop_1 *tbl;
	char c[16];
	unsigned c_len;

	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->write_start || !tbl->write_block) throw_except(curr_cpu, ERR_BAD_FD);

	c_len=snprintf(c, sizeof(c), "%lx", curr_cpu->reg[1].val.u);
	if (c_len >= sizeof(c)) throw_except(curr_cpu, ERR_ARG_TOOLONG);

	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);
	tbl->write_start(curr_cpu, fd);
	tbl->write_block(curr_cpu, fd, c, c_len);
}

/*! CFD_WRITE_OCT 526 SYS2 = cfd_write_oct */
/* r1 - int to print */
static void cfd_write_oct(SYSCALL_ARGS) {
	void *fd;
	const struct cfdop_1 *tbl;
	char c[16];
	unsigned c_len;

	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->write_start || !tbl->write_block) throw_except(curr_cpu, ERR_BAD_FD);

	c_len=snprintf(c, sizeof(c), "%lo", curr_cpu->reg[1].val.u);
	if (c_len >= sizeof(c)) throw_except(curr_cpu, ERR_ARG_TOOLONG);

	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);
	tbl->write_start(curr_cpu, fd);
	tbl->write_block(curr_cpu, fd, c, c_len);
}

/*! CFD_WRITE_FLOAT 527 SYS2 = cfd_write_float */
/* r1 - int to print r2 - min digits r3 - max digits */
static void cfd_write_float(SYSCALL_ARGS) {
	void *fd;
	const struct cfdop_1 *tbl;
	char c[32];
	unsigned c_len;
	char format_str[32];

	/* No, I haven't lost my mind entirely. But do you trust me on that? */
	if (curr_cpu->reg[2].val.u > 8) throw_except(curr_cpu, ERR_ARG_TOOLONG);
	if (curr_cpu->reg[3].val.u > 8) throw_except(curr_cpu, ERR_ARG_TOOLONG);
	if (curr_cpu->reg[2].val.u < 2 || curr_cpu->reg[3].val.u < 2) {
		/* Don't force decimal point. */
		c_len=snprintf(format_str, sizeof(format_str), "%%%u.%ug",
			(short) curr_cpu->reg[2].val.u, (short) curr_cpu->reg[3].val.u);
	} else {
		c_len=snprintf(format_str, sizeof(format_str), "%%#%u.%ug",
			(short) curr_cpu->reg[2].val.u, (short) curr_cpu->reg[3].val.u);
	}
	if (c_len >= sizeof(format_str)) throw_except(curr_cpu, ERR_ARG_TOOLONG);

	tbl=cfdop1_vfd_get(curr_cpu, arg->val.u);
	if (!tbl) throw_except(curr_cpu, ERR_BAD_FD);
	if (!tbl->write_start || !tbl->write_block) throw_except(curr_cpu, ERR_BAD_FD);

	c_len=snprintf(c, sizeof(c), format_str, curr_cpu->reg[1].val.f);
	if (c_len >= sizeof(c)) throw_except(curr_cpu, ERR_ARG_TOOLONG);

	fd=vfd_get_data(curr_cpu, A2_LID_ANY, arg->val.u);
	tbl->write_start(curr_cpu, fd);
	tbl->write_block(curr_cpu, fd, c, c_len);
}

/* TODO: READ_LINE, READ_INT? */

/* Incorporate generated tables */
#include "cfd.h"
