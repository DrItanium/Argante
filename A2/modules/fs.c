/*
 * A2 syscall module: fs
 *
 * Manipulate filesystem objects.
 * Substantially modified + rewritten for CVFD and A2.
 *
 * Nonblocking IO is not handled in this module... after
 * all this is dealing with real files only...
 *
 * Author:                   Michal Zalewski <lcamtuf@ids.pl>
 * A2 (total) conversion:    James Kehl <ecks@optusnet.com.au>
 * A2 maintainer:            James Kehl <ecks@optusnet.com.au>
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
#include "compat/bzero.h"
#include "compat/limits.h"
#include "compat/io.h"

#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>

#ifdef HAVE_DIR_H
/* XXX HACK */
#include <dir.h>
#define mkdir(name, perm) mkdir(name)
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "exception.h"
#include "amemory.h"
#include "module.h"
#include "hhac.h"
#include "file.h"
#include "vfd.h"
#include "cfdop.h"

#include <dirent.h>

#define CREAT_MODE (S_IRUSR | S_IWUSR)
#define DIR_CREAT_MODE S_IRWXU

#ifndef O_BINARY
#define O_BINARY 0
/* and hope. */
#endif

/* Bitflag explanation for FS_OPEN etc:
 * (1) READ: file is opened for reading.
 * 	ACL: /fs/open/read/linear (without FSEEK)
 * 	ACL: /fs/open/read/random (with FSEEK)
 * (2) WRITE: file is opened for writing. Without READ | FSEEK,
 * 	erases file contents first.
 * 	ACL: /fs/open/write/overwrite (without R|F)
 * 	ACL: /fs/open/write/modify (with R|F)
 * (0) APPEND: file is opened for write-append. Excludes all
 * 	other flags (reading is not allowed, fseeking is useless)
 * 	ACL: /fs/open/write/append
 * (4) FSEEK: allows seeking on file.
 *
 * FS_OPEN creates the file if it doesn't exist. (under all flags, even READ!)
 * 	if the file needs creation, /fs/create/file is checked.
 * FS_OPEN_EXISTING fails if the file does not exist.
 * FS_OPEN_CREATE creates the file, and aborts if it already exists.
 */

/* NOTE: HAC is now only checked on file open, stuff like read() only checks
 * permflags on filehandle. It's also My Humble Opinion that close() is not a
 * security threat. */

#define FS_FLAG_READ	1
#define FS_FLAG_WRITE	2
#define FS_FLAG_FSEEK	4

#define FD_TYPE_FILE	1
#define FD_TYPE_DIR	2

struct fs_fd {
	int type;
	int protflags;
	union {
		FILE *f;
		DIR *d;
	} u;
#ifndef HAVE_SEEKDIR
	unsigned offs;
#endif
};

struct fs_vcpu {
	char *wd;
};

/* Reent-OK because only used at init time... I hope */
static int stored_lid; 

static size_t fs_cio_startr(struct vcpu *curr_cpu, struct fs_fd *vfd);
static size_t fs_cio_startw(struct vcpu *curr_cpu, struct fs_fd *vfd);
static size_t fs_cio_read(struct vcpu *curr_cpu, struct fs_fd *vfd, char *buf, size_t size);
static size_t fs_cio_write(struct vcpu *curr_cpu, struct fs_fd *vfd, const char *buf, size_t size);
static void fs_cio_close (struct vcpu *curr_cpu, struct fs_fd *vfd);

/* CFD ops table */
static const struct cfdop_1 fs_ops = {
	(cfdop_start *) fs_cio_startr,
	(cfdop_start *) fs_cio_startw,
	(cfdop_read_block *) fs_cio_read,
	(cfdop_write_block *) fs_cio_write,
	(cfdop_close_fd *) fs_cio_close,
	A2_CFDDESC_NONE, NULL /* No Agent-FD's, though an ArganteNFS would be cool. */
};

static inline int module_internal_init(int lid)
{
	stored_lid=lid;
	cfdop1_lid_set(lid, &fs_ops);

	printk2(PRINTK_INFO, "A2 Filesystem module loaded.\n");
	return 0;
}

static inline void module_internal_vcpu_start(struct vcpu *cpu) {
	struct fs_vcpu *x;
	x=module_get_reserved(cpu, stored_lid);
	if (x)
		printk2(PRINTK_CRIT,
			"FS: vcpu_start: just got a pre-used reserved struct!?\n");

	x=malloc(sizeof(struct fs_vcpu));
	bzero(x, sizeof(struct fs_vcpu));
	if (module_set_reserved(cpu, stored_lid, x)) {
		printk2(PRINTK_CRIT, "FS: vcpu_start: module_set_reserved failed!\n");
		free(x);
		return;
	}
	x->wd=NULL;
}

static inline void module_internal_vcpu_stop(struct vcpu *cpu) {
	struct fs_vcpu *x;
	struct fs_fd *y;
	int fdh, c=0;

	/* The difference between >0 and >=0 is amazing. */
	while((fdh=vfd_find_mine(cpu, stored_lid)) >= 0) {
		y=vfd_get_data(cpu, stored_lid, fdh);
		switch(y->type) {
			case FD_TYPE_FILE: fclose(y->u.f); break;
			case FD_TYPE_DIR: closedir(y->u.d); break;
			default: printk2(PRINTK_CRIT, "FS: internal error: unknown fd type %d\n", y->type);
		}
		free(y); /* yeeps! */
		vfd_dealloc(cpu, stored_lid, fdh);
		c++;
	}
	if (c) printk2(PRINTK_WARN, "FS: freed %d unclosed VFD's\n", c);
	
	x=module_get_reserved(cpu, stored_lid);
	if (x) {
		if (x->wd) free(x->wd);
		free(x);
		module_set_reserved(cpu, stored_lid, NULL);
	}
}

static inline void module_internal_shutdown(void) {
}

/*! assigned 200 - 300 */ 

/* This callback charade is for alloca freeing on ret.
 * filename is the actual real filename to access,
 * HACname is the HAC path for it. */
typedef void fsfunc(struct vcpu *curr_cpu, const anyval *arg, void *extraflags,
		char *HACname, char *filename);

static void fs_resolve_buffer(struct vcpu *curr_cpu, const anyval *arg, void *extraflags,
		unsigned bufaddr, unsigned bufsize, fsfunc *callback)
{
	char *HACname;
	char *filename, *username, *btarget;
	const char *t;
	ALLOCA_STACK;

	/* Ok. Step 1, retrieve the file path. */
	/* Alloca segfaults if it runs out of memory, but real filenames
	 * can't exceed PATH_MAX or so anyway. Test it. */
	if (bufsize >= PATH_MAX) throw_except(curr_cpu, ERR_ARG_TOOLONG);

	/* ... check that mem is good for what it claims */
	t=(const char *) mem_ro_block(curr_cpu, bufaddr, dwords_of_bytes(bufsize));
	if (*t == '/') /* Absolute path */ {
		filename=alloca(bufsize + 5); /* Remember Astrings aren't 0-term */
		strcpy(filename, "./fs");
		HACname=filename + 1;
		btarget=username=filename + 4;
	} else /* Relative path */ {
		int s;
		struct fs_vcpu *x;
		x=module_get_reserved(curr_cpu, stored_lid);
		if (!x) throw_except(curr_cpu, ERR_OOM);
		if (x->wd) {
			s=strlen(x->wd);
			filename=alloca(s + bufsize + 6); /* Forget Not the wd's / */
			strcpy(filename, "./fs");
			HACname=filename + 1;
			username=filename + 4;
			strcpy(username, x->wd);
			btarget=username + s;
		} else {
			filename=alloca(bufsize + 6);
			strcpy(filename, "./fs");
			HACname=filename + 1;
			btarget=username=filename + 4;
		}
		*btarget='/'; btarget++;
	}

	/* Now an atosys strcpy */
	memcpy(btarget, t, bufsize);
	btarget[bufsize]=0; /* NT-it */

	/* Now fold it */
	fold(username);
	/* Return it */
	callback(curr_cpu, arg, extraflags, HACname, filename);
}	

/* arg is file mode, r1 is file buffer, r2 is buffer size. */
static void fs_open_gen(struct vcpu *curr_cpu, const anyval *arg, void *extraflagsv, char *HACname, char *filename) {
	struct fs_fd *newvfd;
	FILE *f;
	int flags, i, j;
	int extraflags=*((int *) extraflagsv);

	/* Try to work out what mode we're askin 4 */
	if (arg->val.u & FS_FLAG_READ) {
		if (arg->val.u & FS_FLAG_FSEEK) {
			VALIDATE(HACname, "/open/read/random");
		} else {
			VALIDATE(HACname, "/open/read/linear");
		}
	}

	if (arg->val.u & FS_FLAG_WRITE) {
		if (arg->val.u & (FS_FLAG_FSEEK | FS_FLAG_READ)) {
			VALIDATE(HACname, "/open/write/modify");
		} else {
			VALIDATE(HACname, "/open/write/overwrite");
		}
	}

	if (!(arg->val.u & (FS_FLAG_WRITE | FS_FLAG_READ))) {
		if (arg->val.u & FS_FLAG_FSEEK) {
			VALIDATE(HACname, "/open/write/append-bug");
		} else {
			VALIDATE(HACname, "/open/write/append");
		}
	}
	
	/* Guess that succeeded... let's OPEN! */
	if (arg->val.u & FS_FLAG_READ && arg->val.u & FS_FLAG_WRITE) flags=O_RDWR;
	else if (arg->val.u & FS_FLAG_READ) flags=O_RDONLY;
	else if (arg->val.u & FS_FLAG_WRITE) flags=O_WRONLY;
	else flags=O_WRONLY | O_APPEND;

	if (extraflags & O_EXCL) {
		VALIDATE(HACname, "/create/file");
		i=open(filename, flags | O_CREAT | O_EXCL | O_BINARY, CREAT_MODE);
		if (i < 0) {
			perror("open O_EXCL");
			throw_except(curr_cpu, ERR_FILE_EXISTS);
		}
	} else if (extraflags & O_CREAT) {
		
		i=open(filename, flags | O_BINARY, CREAT_MODE);
		if (i < 0) {
			VALIDATE(HACname, "/create/file");
			i=open(filename, flags | O_CREAT | O_BINARY, CREAT_MODE);
		}
		if (i < 0) throw_except(curr_cpu, ERR_GENERIC); /* ??? */
	} else {
		i=open(filename, flags | O_BINARY, CREAT_MODE);
		if (i < 0) throw_except(curr_cpu, ERR_FILE_NOT_EXIST);
	}

	if (flags==O_RDWR) f=fdopen(i, "rwb");
	else if (flags==O_RDONLY) f=fdopen(i, "rb");
	else if (flags==O_WRONLY) f=fdopen(i, "wb");
	else if (flags & O_APPEND) f=fdopen(i, "ab");
	else {
		printk2(PRINTK_CRIT, "FS: internal error (something rotten in the state of flags)\n");
		f=NULL;
	}

	if (!f) {
		perror("fdopen");
		close(i);
		throw_except(curr_cpu, ERR_GENERIC); /* ??? */
	}
	
	j=vfd_alloc_new(curr_cpu, stored_lid);
	newvfd=malloc(sizeof(struct fs_fd));
	if (!newvfd) {
		vfd_dealloc(curr_cpu, stored_lid, j);
		throw_except(curr_cpu, ERR_OOM);
	}
	newvfd->type=FD_TYPE_FILE;
	newvfd->protflags=arg->val.u;
	newvfd->u.f=f;
	vfd_set_data(curr_cpu, stored_lid, j, newvfd);
	curr_cpu->reg[0].val.u=j;
}

/*! FS_OPEN 200 SYS2 = fs_open */
static void fs_open(SYSCALL_ARGS) {
	int flags=O_CREAT;
	fs_resolve_buffer(curr_cpu, arg, &flags, curr_cpu->reg[1].val.u,
		curr_cpu->reg[2].val.u, fs_open_gen);
}

/*! FS_OPEN_EXISTING 201 SYS2 = fs_open_existing */
static void fs_open_existing(SYSCALL_ARGS) {
	int flags=0;
	fs_resolve_buffer(curr_cpu, arg, &flags, curr_cpu->reg[1].val.u,
		curr_cpu->reg[2].val.u, fs_open_gen);
}

/*! FS_OPEN_CREATE 202 SYS2 = fs_open_create */
static void fs_open_create(SYSCALL_ARGS) {
	int flags=O_CREAT | O_EXCL;
	fs_resolve_buffer(curr_cpu, arg, &flags, curr_cpu->reg[1].val.u,
		curr_cpu->reg[2].val.u, fs_open_gen);
}

/* ARG is filehandle for: close, read, write, seek, tell, flush
 * close, seek, tell work equally well on dirhandles. */
/*! FS_CLOSE 210 SYS2 = fs_close */
static void fs_cio_close(struct vcpu *curr_cpu, struct fs_fd *vfd) {
	switch(vfd->type) {
		case FD_TYPE_FILE: fclose(vfd->u.f); break;
		case FD_TYPE_DIR: closedir(vfd->u.d); break;
		default: printk2(PRINTK_CRIT,
			"FS: internal error: unknown fd type %d\n", vfd->type);
	}
	free(vfd);
}

static void fs_close(SYSCALL_ARGS) {
	struct fs_fd *fd;
	fd=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	fs_cio_close(curr_cpu, fd);
	vfd_dealloc(curr_cpu, stored_lid, arg->val.u);
}

/*! FS_READ 211 SYS2 = fs_read */
static size_t fs_cio_startr(struct vcpu *curr_cpu, struct fs_fd *vfd) {
	if (vfd->type != FD_TYPE_FILE) throw_except(curr_cpu, ERR_BAD_FD);
	/* Shouldn't it be a HAC error? */
	if (!(vfd->protflags & FS_FLAG_READ)) throw_except(curr_cpu, ERR_BAD_FD);
	/* Could get it from stat, but this is easier. Round it to a dword. */
	return ((unsigned) (BUFSIZ / sizeof(anyval))) * sizeof(anyval);
}

/* Hey, cfdop actually makes this simpler!!! */
static size_t fs_cio_read(struct vcpu *curr_cpu, struct fs_fd *vfd, char *buf, size_t size) {
	return fread(buf, sizeof(char), size, vfd->u.f);
}

/* r1 buffer, r2 buffer size. */
static void fs_read(SYSCALL_ARGS) {
	struct fs_fd *y;
	size_t i, bs, siz, bs2;
	char *buf;

	y=vfd_get_data(curr_cpu, stored_lid, arg->val.u);

	siz=curr_cpu->reg[2].val.u;
	bs=fs_cio_startr(curr_cpu, y);

	for(i=0;i<siz;i+=bs) {
		if (bs > (siz - i)) bs=siz-i;
		buf=(char *) mem_rw_block(curr_cpu, curr_cpu->reg[1].val.u, dwords_of_bytes(bs));
		bs2=fs_cio_read(curr_cpu, y, buf, bs);
		curr_cpu->reg[1].val.u+=bs2 / sizeof(anyval);
		curr_cpu->reg[2].val.u-=bs2;
		if (bs2 != bs) return;
	}
}

/*! FS_WRITE 212 SYS2 = fs_write */
static size_t fs_cio_startw(struct vcpu *curr_cpu, struct fs_fd *vfd) {
	if (vfd->type != FD_TYPE_FILE) throw_except(curr_cpu, ERR_BAD_FD);
	if (!(vfd->protflags & FS_FLAG_WRITE) && vfd->protflags)
		throw_except(curr_cpu, ERR_BAD_FD);
	return ((unsigned) (BUFSIZ / sizeof(anyval))) * sizeof(anyval);
}

static size_t fs_cio_write(struct vcpu *curr_cpu, struct fs_fd *vfd, const char *buf, size_t size) {
	return fwrite(buf, sizeof(char), size, vfd->u.f);
}

static void fs_write(SYSCALL_ARGS) {
	struct fs_fd *y;
	size_t i, bs, siz, bs2;
	const char *buf;

	y=vfd_get_data(curr_cpu, stored_lid, arg->val.u);

	siz=curr_cpu->reg[2].val.u;
	bs=fs_cio_startw(curr_cpu, y);

	for(i=0;i<siz;i+=bs) {
		if (bs > (siz - i)) bs=siz-i;
		buf=(char *) mem_ro_block(curr_cpu, curr_cpu->reg[1].val.u, dwords_of_bytes(bs));
		bs2=fs_cio_write(curr_cpu, y, buf, bs);
		curr_cpu->reg[1].val.u+=bs2 / sizeof(anyval);
		curr_cpu->reg[2].val.u-=bs2;
		if (bs2 != bs) return;
	}
}

/*! FS_FLUSH 213 SYS2 = fs_flush */
static void fs_flush(SYSCALL_ARGS) {
	struct fs_fd *y;

	y=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	if (y->type != FD_TYPE_FILE) throw_except(curr_cpu, ERR_BAD_FD);
	fflush(y->u.f);
}

/*! FS_SEEK 214 SYS2 = fs_seek */
/* This implements SEEK_SET. Other calls may be added for SEEK_END, SEEK_CUR. */
/* Takes: fd in arg, offset in r1. Returns: nothing.
 * 'sparse files' mean there aren't too many things that can break here. Even 
 * SEEK -1 will work (creating a 2^32-1 byte file, but that only scares people :) */
static void fs_seek(SYSCALL_ARGS) {
	struct fs_fd *y;
	off_t r;

	y=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	if (!(y->protflags & FS_FLAG_FSEEK)) throw_except(curr_cpu, ERR_BAD_FD);

	r=curr_cpu->reg[1].val.u;
	switch(y->type) {
		case FD_TYPE_FILE: fseek(y->u.f, r, SEEK_SET); break;
		case FD_TYPE_DIR:
#ifdef HAVE_SEEKDIR
		seekdir(y->u.d, r);
#else
		/* Ok, we don't have seekdir... Emulate! */
		{
			unsigned i;
			rewinddir(y->u.d);
			y->offs=0;
			for(i=0;i < r;i++) {
				/* readdir_r isn't needed, we don't check contents */
				if (readdir(y->u.d)) y->offs++;
			}
		}
#endif
		break;
		default: printk2(PRINTK_CRIT, "FS: internal error: unknown fd type %d\n", y->type);
	}
}


/*! FS_TELL 215 SYS2 = fs_tell */
/* Takes: arg. Returns: offset in r0. */
static void fs_tell(SYSCALL_ARGS) {
	struct fs_fd *y;
	off_t r;

	y=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	if (!(y->protflags & FS_FLAG_FSEEK)) throw_except(curr_cpu, ERR_BAD_FD);

	switch(y->type) {
		case FD_TYPE_FILE: r=ftell(y->u.f); break;
		case FD_TYPE_DIR:
#ifdef HAVE_SEEKDIR
			r=telldir(y->u.d);
#else
			r=y->offs;
#endif
			break;
		default: printk2(PRINTK_CRIT, "FS: internal error: unknown fd type %d\n", y->type);
			 r=0;
	}
	curr_cpu->reg[0].val.u=r;
}


/*! FS_WD_SET 220 = fs_wd_cwd */
/* r0 is file buffer, r1 is buffer size. */
static void fs_wd_cwd_now(struct vcpu *curr_cpu, const anyval *arg, void *extraflags, char *HACname, char *filename) {
	struct stat buf;
	struct fs_vcpu *x;

	VALIDATE(HACname, "/wd/set");

	/* Check for dir existence */
	if (stat(filename, &buf)) throw_except(curr_cpu, ERR_FILE_NOT_EXIST);
	if (!S_ISDIR(buf.st_mode)) throw_except(curr_cpu, ERR_BAD_FD);

	/* Finally, everything's cool, so set WD. */
	x=module_get_reserved(curr_cpu, stored_lid);
	if (!x) throw_except(curr_cpu, ERR_OOM);
	if (x->wd) free(x->wd);
	
	filename+=4; /* strip off ./fs ***XXX PROBLEM SPOT XXX**** */
	x->wd=strdup(filename);
}

static void fs_wd_cwd(SYSCALL_ARGS) {
	fs_resolve_buffer(curr_cpu, NULL, NULL,	curr_cpu->reg[0].val.u,
		curr_cpu->reg[1].val.u,	fs_wd_cwd_now);
}

/*! FS_WD_GET 221 = fs_wd_pwd */
static void fs_wd_pwd(SYSCALL_ARGS) {
	char *nn;
	struct fs_vcpu *x;
	int i;
	ALLOCA_STACK;

	x=module_get_reserved(curr_cpu, stored_lid);
	if (!x) throw_except(curr_cpu, ERR_OOM);
	if (!x->wd) {
		VALIDATE("/fs/","/wd/get");
		curr_cpu->reg[1].val.u=kerntoa_strcpy(curr_cpu, curr_cpu->reg[0].val.u, curr_cpu->reg[1].val.u, "/");
		return;
	}
	i=strlen(x->wd);

	nn=alloca(i + 4);
	strcpy(nn, "/fs");
	strcpy(nn + 3, x->wd);
	
	VALIDATE(nn,"/wd/get");
	
	curr_cpu->reg[1].val.u=kerntoa_strcpy(curr_cpu, curr_cpu->reg[0].val.u, curr_cpu->reg[1].val.u, x->wd);
}

/*! FS_OPEN_DIR 230 = fs_open_dir */
/* Returns dirhandle in r0, takes buf in r1/r2 */

static void fs_open_dir_now(struct vcpu *curr_cpu, const anyval *arg, void *extraflags, char *HACname, char *filename) {
	struct fs_fd *newvfd;
	DIR *d;
	int j;

	/* Try to work out what mode we're askin 4 */
	VALIDATE(HACname, "/open/directory");

	d=opendir(filename);

	if (!d) {
		perror("opendir");
		throw_except(curr_cpu, ERR_FILE_NOT_EXIST); /* right? */
	}

	j=vfd_alloc_new(curr_cpu, stored_lid);
	newvfd=malloc(sizeof(struct fs_fd));
	if (!newvfd) {
		vfd_dealloc(curr_cpu, stored_lid, j);
		closedir(d);
		throw_except(curr_cpu, ERR_OOM);
	}
	newvfd->type=FD_TYPE_DIR;
	newvfd->protflags=0;
	newvfd->u.d=d;
#ifndef HAVE_SEEKDIR
	newvfd->offs=0;
#endif
	vfd_set_data(curr_cpu, stored_lid, j, newvfd);
	curr_cpu->reg[0].val.u=j;
}

static void fs_open_dir(SYSCALL_ARGS) {
	fs_resolve_buffer(curr_cpu, NULL, NULL,	curr_cpu->reg[1].val.u,
		curr_cpu->reg[2].val.u,	fs_open_dir_now);
}

/*! FS_READ_DIR 231 SYS2 = fs_read_dir */
/* Buffer in r1/r2. Size of name in r2. */
static void fs_read_dir(SYSCALL_ARGS) {
	struct fs_fd *y;
	/* Under Borland at least, readdir's static buffer
		is per-dirstream... so it's OK!!! Threads shouldn't share streams */
#ifdef HAVE_READDIR_R
	struct dirent dent;
#endif
	struct dirent *res;

	y=vfd_get_data(curr_cpu, stored_lid, arg->val.u);
	if (y->type != FD_TYPE_DIR) throw_except(curr_cpu, ERR_BAD_FD);
#ifdef HAVE_READDIR_R
	readdir_r(y->u.d, &dent, &res);
#else
	res=readdir(y->u.d);
#endif
	if (!res) {
		curr_cpu->reg[2].val.u=0;
		return;
	}

#ifndef HAVE_SEEKDIR
	y->offs++;
#endif
	curr_cpu->reg[2].val.u=kerntoa_strcpy(curr_cpu, curr_cpu->reg[1].val.u, curr_cpu->reg[2].val.u, res->d_name);
}

/*! FS_MAKE_DIR 232 = fs_make_dir */
/* FN buffer in r0/r1. */
static void fs_mkdir_now(struct vcpu *curr_cpu, const anyval *arg, void *extraflags, char *HACname, char *filename) {
	VALIDATE(HACname, "/create/directory");
	if (mkdir(filename, DIR_CREAT_MODE)) throw_except(curr_cpu, ERR_GENERIC);
}

static void fs_make_dir(SYSCALL_ARGS) {
	fs_resolve_buffer(curr_cpu, NULL, NULL,	curr_cpu->reg[0].val.u,
		curr_cpu->reg[1].val.u,	fs_mkdir_now);
}

/*! FS_STAT 233 = fs_stat */
/* FN buffer in r0/r1.
 * r0	-> file type: 1=regular file, 2=directory, 3+=other. 0=error.
 * r1	-> st_size
 * r2	-> st_mtime
 * */

static void fs_stat_now(struct vcpu *curr_cpu, const anyval *arg, void *extraflags, char *HACname, char *filename) {
	struct stat buf;

	VALIDATE(HACname, "/stat");
	curr_cpu->reg[0].val.u=0;

	if (stat(filename, &buf)) throw_except(curr_cpu, ERR_FILE_NOT_EXIST);

	if (S_ISREG(buf.st_mode)) curr_cpu->reg[0].val.u=1;
	else if (S_ISDIR(buf.st_mode)) curr_cpu->reg[0].val.u=2;
	else curr_cpu->reg[0].val.u=3; /* S_ISBLK, S_ISCHAR, S_ISPIPE...? */

	curr_cpu->reg[1].val.u=buf.st_size;
	curr_cpu->reg[2].val.u=buf.st_mtime; /* time_t is signed, right? */
}

static void fs_stat(SYSCALL_ARGS) {
	fs_resolve_buffer(curr_cpu, NULL, NULL,	curr_cpu->reg[0].val.u,
		curr_cpu->reg[1].val.u,	fs_stat_now);
}

/*! FS_RENAME 240 = fs_rename */
/* Source buffer in r0/r1, target buffer in r2/r3...
 * double buffer resolve... BleH.*/

static void fs_ren_2file(struct vcpu *curr_cpu, const anyval *arg, void *extra, char *HACname, char *filename) {
	char *source;
	int i;

	source=(char *) extra;

	/* I find it scary that renaming a file can replace one. I don't like it.
	 * So create the file/directory FIRST, which effectively locks that filename
	 * unless something EXCEPTIONALLY wierd is going on. Like TWO race attacks...
	 * XXX: Am I being retarded?
	 *
	 * This has the side effect of mostly blocking any race on earlier stat.
	 * A directory can't be moved onto a file or vice versa, so nothing can happen. */
	VALIDATE(HACname, "/create/file");
	i=open(filename, O_WRONLY | O_CREAT | O_EXCL, 0 /* So nobody writes it in meantime */);
	if (i < 0) throw_except(curr_cpu, ERR_FILE_EXISTS);
	close(i);
	if (rename(source, filename)) {
		perror("rename");
		unlink(filename); /* does this create a hole? */
		throw_except(curr_cpu, ERR_GENERIC);
	}
}

static void fs_ren_2dir(struct vcpu *curr_cpu, const anyval *arg, void *extra, char *HACname, char *filename) {
	char *source;

	source=(char *) extra;

	VALIDATE(HACname, "/create/directory");
	if (mkdir(filename, S_IWUSR /* Absolut minimal perms */)) throw_except(curr_cpu, ERR_FILE_EXISTS);
	if (rename(source, filename)) {
		perror("rename");
		rmdir(filename); /* Remove our temp directory - does this cause holes? */
		throw_except(curr_cpu, ERR_GENERIC);
	}
}

static void fs_ren_1(struct vcpu *curr_cpu, const anyval *arg, void *extra, char *HACname, char *filename) {
	struct stat buf;

	if (stat(filename, &buf)) throw_except(curr_cpu, ERR_FILE_NOT_EXIST);

	if (S_ISDIR(buf.st_mode)) { /* Maybe equivalent to deletion... maybe not */
		VALIDATE(HACname, "/rename/directory"); 
		fs_resolve_buffer(curr_cpu, NULL, filename,
			curr_cpu->reg[2].val.u, curr_cpu->reg[3].val.u,
			fs_ren_2dir);
	} else {
		VALIDATE(HACname, "/rename/file");
		fs_resolve_buffer(curr_cpu, NULL, filename,
			curr_cpu->reg[2].val.u, curr_cpu->reg[3].val.u,
			fs_ren_2file);
	}
}

static void fs_rename(SYSCALL_ARGS) {
	fs_resolve_buffer(curr_cpu, NULL, NULL,	curr_cpu->reg[0].val.u,
		curr_cpu->reg[1].val.u,	fs_ren_1);
}


/*! FS_DELETE 241 = fs_delete */
/* FN buffer in r0/r1. */

static void fs_delete_now(struct vcpu *curr_cpu, const anyval *arg, void *extraflags, char *HACname, char *filename) {
	struct stat buf;

	if (stat(filename, &buf)) throw_except(curr_cpu, ERR_FILE_NOT_EXIST);

	/* BLEH. Are we removing a file or a directory? */
	if (!S_ISDIR(buf.st_mode)) {
		VALIDATE(HACname, "/delete/file");
		if (unlink(filename)) throw_except(curr_cpu, ERR_GENERIC);
	} else {
		VALIDATE(HACname, "/delete/directory");
		if (rmdir(filename)) throw_except(curr_cpu, ERR_GENERIC);
	}
}

static void fs_delete(SYSCALL_ARGS) {
	fs_resolve_buffer(curr_cpu, NULL, NULL,	curr_cpu->reg[0].val.u,
		curr_cpu->reg[1].val.u,	fs_delete_now);
}

/* Incorporate generated tables */
#include "fs.h"
