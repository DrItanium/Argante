/*
 * A2 syscall module: locallib
 *
 * Simple, useful functions: get machine name, get time,
 * get OS name, get VS statistics, get real system stats etc.
 *
 * Author:           Michal Zalewski <lcamtuf@ids.pl>
 * A2 conversion:    James Kehl <ecks@optusnet.com.au>
 * A2 maintainer:    James Kehl <ecks@optusnet.com.au>
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
#include "compat/time.h"
#include "compat/ltime_r.h"
#include "compat/limits.h"
#include "compat/io.h"

#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
/* for hostname */
#include <unistd.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>

#ifndef HAVE_GETTIMEOFDAY
#ifdef HAVE_FTIME
#include <sys/timeb.h>
#endif
#endif

#ifdef HAVE_SYSINFO
#include <sys/sysinfo.h>
#endif

#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "module.h"
#include "amemory.h"
#include "exception.h"
#include "hhac.h"

#ifdef __WIN32__
#include <io.h>
#include <windows.h>
#endif

/*! assigned 300 - 400 */
static inline int module_internal_init(int lid)
{
#ifndef HAVE_DEV_URANDOM
	/* Sure, not top quality entropy... but who cares */
	srand(time(0));
#endif
#ifndef HAVE_LOCALTIME_R
	init_localtime_r();
#endif
	printk2(PRINTK_INFO, "LocalLibrary module loaded.\n");
	return 0;
}

static inline void module_internal_vcpu_start(struct vcpu *cpu) {}
static inline void module_internal_vcpu_stop(struct vcpu *cpu) {}
static inline void module_internal_shutdown(void) {}

/*! LOCAL_GETTIME 301 = local_gettime */
/* Ret: u:r0 - current time, u:r1 - current time, microseconds */
static void local_gettime(SYSCALL_ARGS) {
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;
#else
#ifdef HAVE_FTIME
	struct timeb buf;
#endif
#endif
	
	VALIDATE("/localsys/","/time/get");

#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&tv, NULL);
	curr_cpu->reg[0].val.u=tv.tv_sec;
	curr_cpu->reg[1].val.u=tv.tv_usec;
#else
#ifdef HAVE_FTIME
	/* ftime is what does the job under Borland... */
	ftime(&buf);
	curr_cpu->reg[0].val.u=buf.time;
	curr_cpu->reg[1].val.u=buf.millitm;
#else
	/* Resort to inaccuracy... */
	curr_cpu->reg[0].val.u=time();
	curr_cpu->reg[1].val.u=0;
#endif
#endif

}

/*! LOCAL_TIMETOSTR 302 SYS2 = local_timetostr */
/* Input:  u:r0 - GETTIME result, u:r1 - buf, u:r2 - buf size
   Return: u:r2 - bytes stored */
static void local_timetostr(SYSCALL_ARGS) {
	/* It's ugly, it's LibC :) */
	char buf[64];
	struct tm ltbuf;

	VALIDATE("/localsys/","/time/tostr");

	if (!strftime(buf, sizeof(buf) - 1, "%a %b %d %H:%M:%S %Y",
			localtime_r((time_t *) &arg->val.u, &ltbuf))) {
		throw_except(curr_cpu, ERR_GENERIC); /* XXX FIXME */
	}

	curr_cpu->reg[2].val.u=kerntoa_strcpy(curr_cpu, curr_cpu->reg[1].val.u, curr_cpu->reg[2].val.u, buf);
}

/*! LOCAL_GETHOSTNAME 303 = local_gethostname */
/* Input:  u:r0 - buf, u:r1 - size
   Return: u:r1 - bytes stored */
static void local_gethostname(SYSCALL_ARGS) {
#ifdef HAVE_GETHOSTNAME
	char buf[128]; /* Linux max is 64, so this is pretty safe. */

	VALIDATE("/localsys/","/hostname/get");

	if (gethostname(buf, sizeof(buf))) {
		printk2(PRINTK_ERR, "Error getting hostname. Using \'bogus\' instead\n");
		curr_cpu->reg[1].val.u=kerntoa_strcpy(curr_cpu, curr_cpu->reg[0].val.u, curr_cpu->reg[1].val.u, "bogus");
	} else {
		curr_cpu->reg[1].val.u=kerntoa_strcpy(curr_cpu, curr_cpu->reg[0].val.u, curr_cpu->reg[1].val.u, buf);
	}
#else
#ifdef __WIN32__
	char buf[MAX_COMPUTERNAME_LENGTH + 1];
	int sz=MAX_COMPUTERNAME_LENGTH + 1;
	/* Unicode?! */
	if (!GetComputerName(&buf, &sz)) {
		printk2(PRINTK_ERR, "Error getting hostname. Using \'bogus\' instead\n");
		curr_cpu->reg[1].val.u=kerntoa_strcpy(curr_cpu, curr_cpu->reg[0].val.u, curr_cpu->reg[1].val.u, "bogus");
	} else {
		curr_cpu->reg[1].val.u=kerntoa_strcpy(curr_cpu, curr_cpu->reg[0].val.u, curr_cpu->reg[1].val.u, buf);
	}
#else
	/* Just Bogus It */
	curr_cpu->reg[1].val.u=kerntoa_strcpy(curr_cpu, curr_cpu->reg[0].val.u, curr_cpu->reg[1].val.u, "bogus");
#endif
#endif
}

/*! LOCAL_GETRANDOM 306 = local_getrandom */
/* Return: u0 - random number */
static void local_getrandom(SYSCALL_ARGS) {
	int x,y;
	unsigned int l;

	VALIDATE("/localsys/","/random/get");

#ifdef HAVE_DEV_URANDOM
	x=open("/dev/urandom", O_RDONLY|O_NONBLOCK);
	if (!x) throw_except(curr_cpu, ERR_GENERIC); /* XXX FIXME - ERR_DEADLOCK!? */
	y=read(x,&l,sizeof(l));
	if (!y) throw_except(curr_cpu, ERR_GENERIC); /* XXX FIXME - ERR_DEADLOCK!? */
	close(x);
	curr_cpu->reg[0].val.u=l;
#else
	/* Goofball way of getting a full int even if RAND_MAX is short.
	 * using rand here can never be other than a hack. */
	x=(INT_MAX / RAND_MAX);
	l=0;
	for(y=0;y<x;y++) {
		l*=RAND_MAX;
		l+=rand();
	}
	curr_cpu->reg[0].val.u=l;
#endif
}

/*! LOCAL_VS_STAT 304 = local_stat_vs */
static void local_stat_vs(SYSCALL_ARGS) {
	VALIDATE("/localsys/","/stat/virtual");
	printk2(PRINTK_WARN, "VS_STAT is not yet implemented for A2\n");
	throw_except(curr_cpu, ERR_NOSYSCALL);
}

/*! LOCAL_RS_STAT 305 = local_stat_rs */
static void local_stat_rs(SYSCALL_ARGS)
#ifndef HAVE_SYSINFO
{
	printk2(PRINTK_WARN, "RS_STAT for current OS is not implemented.\n");
	throw_except(curr_cpu, ERR_NOSYSCALL);
}
#else
{
	struct sysinfo i;
	VALIDATE("/localsys/","/stat/real");
	sysinfo(&i);

	curr_cpu->reg[0].val.u=i.uptime;
	curr_cpu->reg[1].val.u=i.loads[0];
	curr_cpu->reg[2].val.u=i.totalram/1024;
	curr_cpu->reg[3].val.u=i.freeram/1024;
	curr_cpu->reg[4].val.u=i.totalswap/1024;
	curr_cpu->reg[5].val.u=i.freeswap/1024;
	curr_cpu->reg[6].val.u=i.procs;
}
#endif /* __linux__ */

/* Incorporate generated tables */
#include "locallib.h"
