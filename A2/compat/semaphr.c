/*
 * "semaphr" - portable semaphores
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
#include "printk.h"

#ifdef NO_THREADS
#include "compat/usleep.h"

/* Under NO_THREADS, how we implement it doesn't really matter. */
void sem_wait(sem_t *sem) {
	if (!(*sem)) printk2(PRINTK_CRIT, "Probable deadlock");
	while (!(*sem)) usleep(1000000);
	(*sem)--;
}

void sem_post(sem_t *sem) {
	(*sem)++;
}

void sem_init(sem_t *sem, int unused, int val) {
	(*sem)=val;
}
#else
#ifdef __WIN32__
/* Under Win32 we have the builtin functions
	InterlockedIncrement and InterlockedDecrement;
	perfect for semaphores.  */
#include <windows.h>
void sem_wait(sem_t *sem) {
	/* XXX: Can this possibly work in MT? */
	while (InterlockedDecrement(sem) < 0) {
		InterlockedIncrement(sem);
	}
}
void sem_post(sem_t *sem) {
	InterlockedIncrement(sem);
}

void sem_init(sem_t *sem, int unused, int val) {
	InterlockedExchange(sem, val);
}
#else
#error No semaphore support available
#endif
#endif
