/*
 * A2 Virtual Machine - symbol manager / dynamic linker
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
 * ------------------------------------------------------------------------
 *
 * This is actually both manager & cpu code - just consider
 * it vcpu_start code to be safe. (no exceptions)
 */
#include "autocfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "bformat.h"
#include "bcode_op.h"
#include "amemory.h"
#include "symman.h"

/*
 * We have to add symbols to the end of a hash -
 * i.e. first added, first found (so LD_PRELOAD works,
 * if you consider it a feature).
 *
 * We also must track the references of undefined
 * symbols and relocate them when the symbol is defined.
 * We'll dispose of the references after that. So you can't
 * load a library, have :Tekst point to it, unload it, load
 * another, and expect :Tekst to point to the new one.
 * Get real :)
 *
 * We also have to free relocs when an alib is unloaded.
 * Otherwise Bad Things Happen when the symbol is defined.
 * When an undefined symbol has no relocs, it is cleared.
 *
 * alib_id 0 reserved for undefined symbols. Surprised?
 */

#define ST_INITIAL 50
#define ST_NEXT(prev) 2*prev

struct symhash {
	struct symhashent *symhash;
	unsigned entries;
	unsigned size;
};

struct relocent {
	struct reloc r;
	unsigned alib_id;
	struct relocent *next;
};

struct symhashent {
	struct symbol *first;
	struct symbol *last;
};

/* same as in hhac.c... lame eh? */
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

struct symbol *symman_find_symbol(struct vcpu *in, char *name, unsigned alib_id) {
	struct symbol *x;
	/* Won't find it in an empty table. */
	if (!in->st || !in->st->entries) return NULL;
	x=(in->st->symhash[hash(name) % in->st->size]).first;
	while(x) {
		if ((!alib_id || alib_id == x->alib_id) && !strncmp(name, x->s.fn, A2_MAX_SNAME)) break;
		x=x->next;
	}
	return x;
}

/* Leave nothing! */
void symman_unload(struct vcpu *in) {
	unsigned i;
	struct symbol *x, *next;
	struct relocent *r, *rnext;
	
	if (!in->st) return;
	
	for(i=0;i<in->st->size;i++) {
		x=in->st->symhash[i].first;
		while(x) {
			next=x->next;
			r=x->reloc;
			if (r && x->alib_id) printk2(PRINTK_CRIT, "Symbol owned but with relocs?\n");
			while(r) {
				rnext=r->next;
				free(r);
				r=rnext;
			}
			free(x);
			x=next;
			in->st->symhash[i].first=x;
		}
	}
	if (in->st->size) {
		free(in->st->symhash);
		in->st->symhash=NULL;
		in->st->size=0;
	}
	free(in->st); in->st=NULL;
}

void symman_unload_al(struct vcpu *in, unsigned alib_id) {
	unsigned i;
	struct symbol *x, *prev, *next;
	if (!alib_id) return;
	if (!in->st || !in->st->entries) return;
	for(i=0;i<in->st->size;i++) {
		prev=NULL;
		x=in->st->symhash[i].first;
		while(x) {
			next=x->next;
			if (x->alib_id == alib_id) goto clear;
			else if (!x->alib_id) { /* search for relocs */
				int ct=0;
				struct relocent *r, *rprev=NULL, *rnext;
				r=x->reloc;
				while(r) {
					rnext=r->next;
					if (r->alib_id != alib_id) {
						ct++;
						rprev=r;
					} else {
						free(r);
						if (rprev) rprev->next=rnext;
						else x->reloc=rnext;
					}
					r=rnext;
				}
				if (!ct) goto clear;
			}
			prev=x;
			x=next;
			continue;
clear:
#ifdef SYMMAN_DEBUG
			printk2(PRINTK_DEBUG, "Unloading %32.32s\n", x->s.fn);
#endif
			in->st->entries--;
			free(x);
			if (prev) prev->next=next;
			else in->st->symhash[i].first=next;
			x=next;
		}
#ifdef SYMMAN_DEBUG
		printk("<+> First%d is now %32.32s\n", i,(in->st->symhash[i].first) ?
			in->st->symhash[i].first->s.fn : "null");
		printk("<+>  Last%d is now %32.32s\n", i, (prev) ? prev->s.fn : "null");
#endif
		in->st->symhash[i].last=prev;
	}
}

static int symman_reloc_now(struct vcpu *in, struct stable *sym, struct reloc *r) {
	unsigned page, addr, i;
	switch(r->type)
	{
		case RELOC_ADDR:
			i=sym->addr;
			break;
		case RELOC_SIZE_BYTE:
			i=sym->size * sizeof(anyval);
			break;
		case RELOC_SIZE_DWORD:
			i=sym->size;
			break;
		default:
			printk2(PRINTK_CRIT, "Unknown reftype while relocating.\n");
			return 1;
	}

	switch(r->place & SYM_PLACEMASK) {
		/* If you change the paging rules in memory.c adjust this */
		case SYM_DATA:
			page=r->addr / A2_MAX_BLKSIZ;
			addr=r->addr % A2_MAX_BLKSIZ;
			if (page >= in->memblks || addr >= in->mem[page].size) {
				printk2(PRINTK_CRIT, "Reloc data target out of bounds.\n");
				return 1;
			}
			in->mem[page].memory[addr].val.u=i;
			break;
		case SYM_CODE:
			addr=CP_DATA_OF(r->addr / sizeof(struct bcode_op));
			page=CP_PAGE_OF(r->addr / sizeof(struct bcode_op));
			if (page >= in->cp_count || addr >= in->cp_all[page].size) {
				printk2(PRINTK_CRIT, "Reloc code target out of bounds.\n");
				return 1;
			}
			/* On the off chance someone's relocating opcodes :) */
			switch(r->addr % sizeof(struct bcode_op)) {
				case (OFFSET_OP_A1):
					in->cp_all[page].bcode[addr].a1.val.u=i;
					break;
				case (OFFSET_OP_A2):
					in->cp_all[page].bcode[addr].a2.val.u=i;
					break;
				default:
					printk2(PRINTK_CRIT, "Reloc target is not a1 or a2. (%d)\n",
							r->addr % sizeof(struct bcode_op));
					return 1;
			}
			break;
		default:
			printk2(PRINTK_CRIT, "Unknown place to relocate.\n");
			return 1;
	}
	return 0;
}

int symman_add_reloc(struct vcpu *in, struct stable *sym, struct symbol *sdata, struct reloc *r, unsigned alib_id) {
	if (sym->place & SYM_UNDEFINED) {
		if (!sdata) { printk2(PRINTK_CRIT, "undefined local?\n"); return 1; }
		sym=&sdata->s;
	}
	
	if (sym->place & SYM_UNDEFINED) {
		/* Still nothing, apparently. */
		struct relocent *re;
		re=malloc(sizeof(struct relocent));
		if (!re) {
			perror("malloc");
			return 1;
		}
		memcpy(&re->r, r, sizeof(struct reloc));
		re->alib_id=alib_id;
		re->next=sdata->reloc;
		sdata->reloc=re;
		return 0;
	} else {
		/* So let's relocate it now. */
		return symman_reloc_now(in, sym, r);
	}
}

static void symman_add_symbol_i(struct vcpu *to, struct symbol *s) {
	int h;

	to->st->entries++;
	h=hash(s->s.fn) % to->st->size;
	if (to->st->symhash[h].last) {
		to->st->symhash[h].last->next=s;
	} else {
		to->st->symhash[h].first=s;
	}

	to->st->symhash[h].last=s;
	s->next=NULL;
}

struct symbol *symman_add_symbol(struct vcpu *to, struct stable *sym, unsigned alib_id) {
	struct symbol *x;

	/* Local symbols are easy */
	if (sym->place & SYM_UNNAMED) return NULL;
	
#ifdef SYMMAN_DEBUG
	printk2(PRINTK_DEBUG, "adding sym %32.32s for %d\n", sym->fn, alib_id);
#endif

	if (sym->place & SYM_UNDEFINED) {
#ifdef SYMMAN_DEBUG
		printk2(PRINTK_DEBUG, "\tundefined\n");
#endif
		/* Find a defined complement */
		if ((x=symman_find_symbol(to, sym->fn, 0))) {
#ifdef SYMMAN_DEBUG
			printk2(PRINTK_DEBUG, "\tbut doubled: %32.32s\n", x->s.fn);
#endif
			return x;
		}
	} else {
#ifdef SYMMAN_DEBUG
		printk2(PRINTK_DEBUG, "\tdefined\n");
#endif
		/* Find an undefined complement if possible */
		if ((x=symman_find_symbol(to, sym->fn, 0))) {
			if (x->s.place & SYM_UNDEFINED) {
				struct relocent *r, *p;
				/* h'ray! */
#ifdef SYMMAN_DEBUG
				printk2(PRINTK_DEBUG, "defining %32.32s\n", sym->fn);
#endif
				memcpy(&x->s, sym, sizeof(struct stable));
				x->alib_id=alib_id;
				r=x->reloc;
				while(r) {
					p=r->next;
					symman_reloc_now(to, sym, &r->r);
					free(r);
					r=p;
				}
				x->reloc=NULL;
				return x;
			}
		}
	}
#ifdef SYMMAN_DEBUG
	printk2(PRINTK_DEBUG, "\tand original\n");
#endif

	if (!to->st) {
		if (!(to->st=malloc(sizeof(struct symhash)))) {
			perror("malloc");
			return NULL;
		}
		to->st->entries=0;
		to->st->size=ST_INITIAL;
		to->st->symhash=calloc(sizeof(struct symhashent), to->st->size);
		if (!to->st->symhash) {
			perror("malloc");
			return NULL;
		}
	}

	if (!(x=malloc(sizeof(struct symbol)))) {
		perror("malloc");
		return NULL;
	}
	memcpy(&x->s, sym, sizeof(struct stable));
	if (sym->place & SYM_UNDEFINED) x->alib_id=0;
	else x->alib_id=alib_id;
	x->reloc=NULL;

	if (to->st->entries >= 0.8 * to->st->size) {
		struct symhashent *old, *new;
		struct symbol *s, *p;
		int newsize, oldsize, i;
		newsize=ST_NEXT(to->st->size);
		new=calloc(sizeof(struct symhashent), newsize);
		if (!new) {
			perror("malloc");
			return NULL;
		}

		/* swap in new hash */
		oldsize=to->st->size;
		to->st->size=newsize;
		to->st->entries=0;
		old=to->st->symhash;
		to->st->symhash=new;

		for(i=0;i<oldsize;i++) {
			s=old[i].first;
			while(s) {
				p=s->next;
				symman_add_symbol_i(to, s);
				s=p;
			}
		}
		free(old);
	}

	symman_add_symbol_i(to, x);
	return x;
}
