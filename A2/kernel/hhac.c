/*
 * A2 Virtual Machine - #HAC support
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
#include "compat/strtok_r.h"
#include "compat/strcmpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "taskman.h"
#include "printk.h"

#define PERM_DENY 00
#define PERM_ALLOW 01
/* UNSPECIFIED is used in empty ACL's, it does not alter a permission
 * brought from a lower level. It's used as a default, not a setting */
#define PERM_UNSPECIFIED 02

/*
 * and ACL for access type control
 */
struct acl {
	unsigned entries;
	unsigned size;
	struct acltable **table;
	int mode;
};

struct acltable {
	struct acl acl;
	char *name;
	struct acltable *next;
};

/*
 * HAC for directory heirarchy...
 */

struct hac {
	unsigned entries;
	unsigned size;
	struct hactable **table;
	struct acl acl;
	struct hactable *parent;
};

struct hactable {
	struct hac hac;
	char *name;
	struct hactable *next;
};

static int hash(const char *name) {
	int x;
	x=name[0];
	if (!name[0]) return x;
	x|=name[1] << 8;
	if (!name[1]) return x;
	x|=name[2] << 16;
	if (!name[2]) return x;
	x|=name[3] << 24;
	return x;
}

/*
 * ACL notes:
 * .. and . aren't supported, hopefully nobody will be stupid enough
 * to use user-specified ACLs!
 *
 * This gets called a lot, so please make it nice and clean and fast.
 * I wish I'd done a better job / that there was a better way :)
 */
static int search_acl(struct acl *acl, char *name) {
	int mode;
	int i;
	char *x, *reent;
	struct acltable *m;
	ALLOCA_STACK;

	mode=acl->mode;

	/* Fake strdupa */
	i=strlen(name) + 1;
	x=alloca(i);
	memcpy(x, name, i);
	name=x;

	x=strtok_r(name, "/", &reent);
	while(acl && x) {
		if (!acl->size) break;
		i=hash(x) % acl->size;
		m=acl->table[i];
		while (m && strcmp(m->name, x)) m=m->next; 
		if (!m) break;
		
		acl=&m->acl;
		if (acl->mode != PERM_UNSPECIFIED) mode=acl->mode;
		
		x=strtok_r(NULL, "/", &reent);
	}
	/* We're out of ACL or out of string. Return best match */
	return mode;
}

/*
 * HAC notes:
 * On dealing with ..'s:
 * a) it's very hard, and very ugly (requires quintuply-nested if's and 2 counters)
 * b) it's pointless because the module should fold() them first.
 * 
 * We 100% ignore them now. If the module author expects HAC to
 * clean up dodgy input we have more problems than path traversal!
 */
static int search_hac(struct hac *hac, char *dir, char *atype) {
	int mode=PERM_DENY; /* Secure By Default */
	int i;
	char *x, *reent;
	struct hactable *m;
	ALLOCA_STACK;

	/* Fake strdupa */
	i=strlen(dir) + 1;
	x=alloca(i);
	memcpy(x, dir, i);
	dir=x;
	/* Fingers crossed for the performance... */
	i=search_acl(&hac->acl, atype); 
	if (i != PERM_UNSPECIFIED) mode=i;
	x=strtok_r(dir, "/", &reent);
	while(hac && x) {
		if (!hac->size) return mode;
		i=hash(x) % hac->size;
		m=hac->table[i];
		while (m && strcmp(m->name, x)) m=m->next; 
		if (!m) return mode;
		hac=&m->hac;
		i=search_acl(&hac->acl, atype); 
		if (i != PERM_UNSPECIFIED) mode=i;
		x=strtok_r(NULL, "/", &reent);
	}
	/* We're out of HAC or out of string. Return best match */
	return mode;
}

/* The actual export function: 
 * returns 0 on auth success, nonzero else */
int validate_access(struct vcpu *curr_cpu, char *dir, char *atype) {
	int x;
	/* Uninitalized HAC. Shouldn't happen, really,
	 * but if it does, why crash and burn without a trace? */
	if (!curr_cpu->hac) {
		printk2(PRINTK_CRIT, "HAC uninitialized: internal error?\n");
		return 1;
	}
	x=search_hac(&curr_cpu->hac->hac, dir, atype);
	return (x == PERM_ALLOW) ? 0 : 1;
}

/* Now: the HAC update functions <EEEEEEK>
 * As we don't need the semaphore anymore, the external functions
 * are a little redundant; but they still check for initialization. */

/* When we alter a VCPU's HAC, we do it all at once.
 * No namby-pamby updating, we tear the whole thing
 * down and start from scratch. Got a problem with it? */
static int destroy_acl_i(struct acl *acl) {
	unsigned i;
	int s=0;
	struct acltable *m, *n;
	for(i=0;i<acl->size;i++) {
		m=acl->table[i];
		while (m) {
			s+=destroy_acl_i(&m->acl);
			n=m->next;
			s+=sizeof(struct acltable);
			s+=strlen(m->name) + 1; /* NUL */
			free(m->name);
			free(m);
			m=n;
		}
	}
	if (acl->table) {
		free(acl->table);
		s+=sizeof(struct acltable *) * acl->size;
	}
	acl->size=0;
	return s; /* Track bytes recovered. Just for fun. */
}

static int destroy_hac_i(struct hac *hac) {
	unsigned i;
	int s;
	struct hactable *m, *n;

	s=destroy_acl_i(&hac->acl);
	
	for(i=0;i<hac->size;i++) {
		m=hac->table[i];
		while (m) {
			s+=destroy_hac_i(&m->hac);
			n=m->next;
			s+=sizeof(struct hactable);
			s+=strlen(m->name) + 1; /* NUL */
			free(m->name);
			free(m);
			m=n;
		}
	}
	if (hac->table) {
		free(hac->table);
		s+=sizeof(struct hactable *) * hac->size;
	}
	hac->size=0;
	return s; /* Track bytes recovered. Just for fun. */
}

static int add_acl_i(struct acl *parent, char *atype, int amode) {
	char *dn, *d2;
	struct acltable *m;
	int y;

	if (*atype == '/') atype++; /* strchr differs a bit from strtok */
	while (1) {
	/* Any more ACL-dirs to add? */
	if (!atype || !*atype) {
		parent->mode=amode;
		return 0;
	}
	/* Ok, what ACL-dir do we have to change to next? */
	dn = strchr(atype, '/');
	if (dn) {
		y=dn-atype; /* Len of this dir */
		d2=malloc(y + 1); /* Working copy of this segment */
		strncpy(d2, atype, y);
		d2[y]=0;
		atype=d2;
		dn++; /* The next dir */
	} else atype=strdup(atype);
	/* No ..'s or .'s in ACL's to worry about */
	/* We aren't the leafnode. So we have to find or create the next directory. */
	if (parent->size) {
		y=hash(atype) % parent->size;
		m=parent->table[y];
		while (m && strcmp(m->name, atype)) m=m->next; 
	} else m=NULL;
	if (m) { /* Found it! */
		free(atype); /* Don't need the copy */
	} else {
		/* Damn. Work. */
		if (parent->entries >= 0.8 * parent->size) {
			unsigned s, i;
			struct acltable **new, *q;
			if (parent->size) 
				printk2(PRINTK_DEBUG, "Rehashing ACL... (Eeek?)\n");
	
			/* ACL hashsizes will be even less than HAC's */
			s=parent->size + 5;
			
			new=calloc(sizeof(struct acltable *), s);
			if (!new) {
				perror("malloc");
				return 1;
			}
		
			/* Rehash */
			for(i=0;i<parent->size;i++)
			{
				while (parent->table[i]) {
					q=parent->table[i];
					/* Something nice about moving the entries
					 * rather than half-copying them */
					parent->table[i]=q->next;
					y=hash(q->name) % s;
					/* SLL linkin */
					q->next=new[y];
					new[y]=q;
				}
			}
			
			/* Now away with the old, empty table! And in with the new! */
			parent->size=s;
			free(parent->table);
			parent->table=new;
		}
		parent->entries++;
		y=hash(atype) % parent->size;
		m=malloc(sizeof(struct acltable));
		if (!m) {
			perror("malloc");
			return 1;
		}
		m->name=atype; /* Malloc'd string */
		m->next=parent->table[y];
		parent->table[y]=m;
		/* Setup new ACL */
		m->acl.entries=m->acl.size=0;
		m->acl.table=NULL;
		m->acl.mode=PERM_UNSPECIFIED;
	}
	parent=&m->acl; /* Non-recursive recursion :) */
	atype=dn;
	}
}

static int add_hac_i(struct hactable *parent, char *dir, char *atype, int amode) {
	char *dn, *d2;
	struct hactable *m;
	int y;

	if (*dir == '/') dir++; /* strchr differs a bit from strtok */
	while(1) { /* Avoid recursing, it's hard on the stack */
	/* Are we at the final directory? */
	if (!dir || !*dir) return add_acl_i(&parent->hac.acl, atype, amode);
	/* Ok, what dir do we have to change to next? */
	dn = strchr(dir, '/');
	if (dn) {
		y=dn-dir; /* Len of this dir */
		d2=malloc(y + 1); /* Working copy of this segment */
		strncpy(d2, dir, y);
		d2[y]=0;
		dir=d2;
		dn++; /* The next dir */
	} else dir=strdup(dir);
	if (dir[0]=='.') {
		if (!dir[1]) printk2(PRINTK_WARN, "Your HAC contains a directory ., which is going to be pretty useless!\n");
		else if (dir[1] == '.' && !dir[2])
			printk2(PRINTK_WARN, "Your HAC contains a directory .., which is going to be pretty useless!\n");
	}
	/* We aren't the leafnode. So we have to find or create the next directory. */
	if (parent->hac.size) {
		y=hash(dir) % parent->hac.size;
		m=parent->hac.table[y];
		while (m && strcmp(m->name, dir)) m=m->next; 
	} else m=NULL;
	if (m) { /* Found it! */
		free(dir); /* Don't need the copy */
	} else {
		/* Damn. Work. */
		if (parent->hac.entries >= 0.8 * parent->hac.size) {
			unsigned s, i;
			struct hactable **new, *q;
			if (parent->hac.size) 
				printk2(PRINTK_DEBUG, "Rehashing HAC... (Eeek?)\n");
	
			/* Rehashing might be fairly expensive, but I would be
			 * surprised if a HACdir had more than 10 children.
			 * Every excess entry takes 4 bytes, and HAC modification
			 * does not happen at runtime (mostly) */
			s=parent->hac.size + 10;
			
			new=calloc(sizeof(struct hactable *), s);
			if (!new) {
				perror("malloc");
				return 1;
			}
		
			/* Rehash */
			for(i=0;i<parent->hac.size;i++)
			{
				while (parent->hac.table[i]) {
					q=parent->hac.table[i];
					/* Something nice about moving the entries
					 * rather than half-copying them */
					parent->hac.table[i]=q->next;
					y=hash(q->name) % s;
					/* SLL linkin */
					q->next=new[y];
					new[y]=q;
				}
			}
			
			/* Now away with the old, empty table! And in with the new! */
			parent->hac.size=s;
			free(parent->hac.table);
			parent->hac.table=new;
		}
		parent->hac.entries++;
		y=hash(dir) % parent->hac.size;
		m=malloc(sizeof(struct hactable));
		if (!m) {
			perror("malloc");
			return 1;
		}
		m->name=dir; /* Malloc'd string */
		m->next=parent->hac.table[y];
		parent->hac.table[y]=m;
		/* Setup new HAC */
		m->hac.entries=m->hac.size=0;
		m->hac.table=NULL;
		m->hac.parent=parent;
		/* and a minimal ACL */
		m->hac.acl.entries=m->hac.acl.size=0;
		m->hac.acl.table=NULL;
		m->hac.acl.mode=PERM_UNSPECIFIED;
	}
	parent=m;
	dir=dn;
	}
}

/* Export functions */

/* Readies a VCPU's #HAC table for use. */
int hac_init(struct vcpu *cpu) {
	cpu->hac=calloc(sizeof(struct hactable), 1);
	if (!cpu->hac) return 1;
	return 0;
}

/* Loads a new #HAC table from file, possibly unloading the old one. */
int hac_loadfile(struct vcpu *cpu, FILE *from) {
	int i;
	char buf[1000];
	char *x, *s;
	char *dir, *atype, *mode;
	int lineno=0;
	if (!cpu->hac) {
		printk2(PRINTK_CRIT, "loading file into uninitialised HAC!\n");
		return 1;
	}
	/* Can't use external hac_unload because things might get awful
	 * snarled between unloading the old table and loading the new one. */
	if (cpu->hac->hac.size) {
		i=destroy_hac_i(&cpu->hac->hac);
	}
	/* I deplore this, but it was ok for AOSr1. */
	while (fgets(buf,sizeof(buf),from)) {
		lineno++;
		s=&buf[0];
		if ((x=strchr(buf,'#'))) *x=0;
		if ((x=strchr(buf,'\n'))) *x=0;
		if (!*s) continue;
		/* I couldn't help but make improvements... */
		while (*s && isspace((unsigned) *s)) s++;
		if (!*s) continue;
		/* like this... */
		x=strchr(s, 0);
		while (isspace((unsigned) *x)) { *x=0; x--; }
		if (!*s) continue;
		/* against all odds we have a valid line... */
		dir=strtok_r(s, " \t", &x);
		if (*dir == '/') dir++;
		atype=strtok_r(NULL, " \t", &x);
		mode=strtok_r(NULL, " \t", &x);
		if (!strcasecmp(mode, "allow")) i=PERM_ALLOW;
		else if (!strcasecmp(mode, "deny")) i=PERM_DENY;
		else { /* Don't think it's safe/useful to allow setting PERM_UNSPEC */
			printk2(PRINTK_WARN, "#HAC: line %d: unknown permission %s\n", lineno, mode);
			continue;
		}
		if (add_hac_i(cpu->hac, dir, atype, i))
			printk2(PRINTK_ERR, "#HAC: line %d failed\n", lineno);
	}
	return 0;
}

/* Export edition of add_hac_i. For cify, mainly.
 * There might be some (perf) issues with using this repetitively. So avoid it. */
int add_hac_entry(struct vcpu *cpu, char *dir, char *atype, int amode) {
	int x;
	if (!cpu->hac) {
		printk2(PRINTK_CRIT, "loading file into uninitialised HAC!\n");
		return 1;
	}
	x=add_hac_i(cpu->hac, dir, atype, amode);
	return x;
}

/* Unloads a #HAC table, in preparation for VCPU shutdown */
int hac_unload(struct vcpu *cpu) {
	int i;
	if (cpu->hac) {
		i=destroy_hac_i(&cpu->hac->hac);
		i+=sizeof(struct hactable);
		free(cpu->hac);
		cpu->hac=NULL;
		printk2(PRINTK_DEBUG, "%d bytes (or more) freed with #HAC table\n", i);
	}
	return 0;
}
