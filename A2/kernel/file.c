/*
 * A2 Virtual Machine - fs util functions
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"

/* fold(x) converts x from a string of the form /fs/../cow/moo/file/../moo
 * into what it really is (eg. /cow/moo/moo). It should be used on all paths
 * that contain a user-specified component before HAC-checking.
 *
 * Note: /.. transforms to /, like under Linux (at least mine.)
 * So hackers who try /fs/../../anotherdir/ get /anotherdir not ../anotherdir
 *
 * Oh, and it returns the result in the same memory x was in.
 */
void fold(char *x) {
	char *p;
	char *n;
	char *inptr;
	char *outptr, initial;
	int len, len2;
	ALLOCA_STACK;
	/* We can hardly lengthen the string, can we? */
	len2=strlen(x) + 1;
	outptr=p=alloca(len2); /* alloca's faster */
	*outptr=0;
	inptr=x;
	initial=*x;
	while (inptr) {
		n=strchr(inptr, '/');
		if (n) {
			/* Strsep-alike... */
			len=n-inptr;
			*n=0; n++;
		} else {
			len=strlen(inptr);
		}
		/* and possibly a memcpy? */
		if (inptr[0] == '.' && !inptr[1]) { /* . - do nothing */
		} else if (inptr[0] == '.' && inptr[1] == '.' && !inptr[2]) {
			/* .. - trim back to last dir bit */
			while (*outptr != '/' && outptr > p) outptr--;
			*outptr=0;
		} else if (len) {
			/* a normal directory - lcamtuf sayeth: "you lucky asshole" */
			if (outptr > p || initial=='/') { /* can't lengthen it! */
				*outptr='/'; outptr++;
			}
			memcpy(outptr, inptr, len);
			outptr+=len;
			*outptr=0;
		}
		inptr=n;
	}
	memcpy(x, p, len2);
}

