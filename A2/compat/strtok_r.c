/*
 * "strtok_r" - reentrant strtok
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
#include <string.h>
#include "compat/strtok_r.h"

char *strtok_r(char *buf, const char *sep, char **reent) {
	char *thistok, *next;
	int d;
	thistok=(buf) ? buf : *reent;
	if (!thistok) return NULL;
	while (strchr(sep, *thistok)) thistok++;
	if (!*thistok) return NULL;
	next=thistok;
	d=strcspn(next, sep);
	next+=d;
	if (*next) {
		*next=0;
		next++;
	} else next=NULL;

	*reent=next;
	return thistok;
}
