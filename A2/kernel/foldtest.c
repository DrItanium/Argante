/*
 * A2 Virtual Machine - #HAC tester
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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include "file.h"

int main() {
	char buf[4000];
	char *a, *m;
	printf("foldtest orange. (c) 2001 James Kehl <ecks@optusnet.com.au>\n"
		"Enter test path or ^D to exit\n");
	while(1) {
#ifndef USE_READLINE
		printf("<foldtest>: ");
		fflush(stdout);
		if (!fgets(buf,sizeof(buf),stdin)) break;
		a=strchr(buf, '\n');
		if (a) *a=0;
		m=buf;
#else
		m=readline("<foldtest>: ");
#endif
		fold(m);
		puts(m);
#ifdef USE_READLINE
		free(m);
#endif
	}
	
	return 0;
}
