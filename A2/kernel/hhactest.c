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
#include "compat/bzero.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include "taskman.h"
#include "hhac.h"

extern int validate_access(struct vcpu *curr_cpu, char *dir, char *atype); 
extern int hac_init(struct vcpu *cpu);
extern int hac_loadfile(struct vcpu *cpu, FILE *from);
extern int hac_unload(struct vcpu *cpu);

void memrep() {
#ifdef HAVE_MALLINFO
	int i;
	i=mallinfo().uordblks;
	printf("%d bytes in mallocated memory\n", i);
#endif
}

int main(int argc, char **argv) {
	struct vcpu base;
	FILE *f;
	char buf[1000];
	char *a, *b, *m;
	
	bzero(&base, sizeof(base));

	memrep();
	if (hac_init(&base)) {
		fprintf(stderr, "<!> hac_init failed. Aborting.\n");
		return 1;
	}

	if (argc != 2) {
		fprintf(stderr, "Usage: hhactest <hacfile>\n");
		return 1;
	}

	memrep();
	f=fopen(argv[1], "r");
	if (!f) {
		perror("fopen");
		return 1;
	}

	memrep();
	if (hac_loadfile(&base, f)) {
		fprintf(stderr, "<!> hac_loadfile failed. Aborting.\n");
		return 1;
	}
	memrep();

	printf("hhactest orange. (c) 2001 James Kehl <ecks@optusnet.com.au>\n"
		"Enter test phrase '/dir/' '/acl/' on prompt, or ^D to exit.\n");
	while(1) {
#ifndef USE_READLINE
		printf("<hhactest>: ");
		fflush(stdout);
		if (!fgets(buf,sizeof(buf),stdin)) break;
		a=strchr(buf, '\n');
		if (a) *a=0;
		m=buf;
#else
		m=readline("<hhactest>: ");
#endif
		a=strtok(m, " \t");
		b=strtok(NULL, " \t");
		if (!b) {
			printf("You said 'quit', right?\n");
			break;
		}
		printf("Analysing dir %s, atype %s...\n", a, b);
		if (validate_access(&base, a, b))
			printf("permission denied.\n");
		else
			printf("access granted.\n");
#ifdef USE_READLINE
		free(m);
#endif
	}
	
	memrep();
	hac_unload(&base);
	memrep();
	return 0;
}
