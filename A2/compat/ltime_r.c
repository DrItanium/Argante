/*
 * "ltime_r" - reentrant localtime emulation
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
#include "compat/semaphr.h"
#include "compat/time.h"
#include "compat/ltime_r.h"
#include <stdio.h>
#include "printk.h"

/* The deal: use the nonreentrant one but get a lock on it. */
static sem_t ltime_sem;
static int inited=0;

/* Should only be called during single-thread... */
void init_localtime_r(void) {
	if (inited) return;
	sem_init(&ltime_sem, 0, 1);
	inited=1;
}

struct tm *localtime_r(const time_t *time, struct tm *buf) {
	struct tm *ret;

	if (!inited) printk2(PRINTK_CRIT, "localtime_r called without localtime_r_init!\n"); 
	else sem_wait(&ltime_sem);
	
	ret=localtime(time);
	if (ret) memcpy(buf, ret, sizeof(struct tm));
	else buf=NULL;

	if (inited) sem_post(&ltime_sem);

	return buf;
}
