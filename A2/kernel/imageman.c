/*
 * A2 Virtual Machine - image manager
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
 *
 * ------------------------------------------------------------------------
 * Loads files... how about codepage sharing?
 * XXX: due to sloth needs contiguous codepages, mempages at end
 *
 * This is actually both manager & cpu code - just consider
 * it vcpu_start code to be safe. (no exceptions)
 */

#include "autocfg.h"
#include "compat/bzero.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "taskman.h"
#include "printk.h"
#include "data_blk.h"
#include "amemory.h"
#include "bcode.h"
#include "cmd.h"
#include "module.h"
#include "hhac.h"
#include "vfd.h"

#include "imageman.h"
#include "symman.h"

#define frread(a, b, c, d) if (!fread(a, b, c, d)) { perror("fread"); return 1; }
#define ffree(x) if(x) free(x); x=NULL

#define IMLI_PROGSPEC	1
#define IMLI_CODE	2
#define IMLI_SYM	8
#define IMLI_EOF	128

static int imageman_loadimage_al(char *from, struct vcpu *into, unsigned alib_id);

/* Now we return alib_id - 0 for failure */
unsigned imageman_loadimage(char *from, struct vcpu *into) {
	unsigned id;
	/* Find a suitable alib_id - if it fails clean up.
	 * alib_id 0 is reserved for run-time allocated data,
	 * so first load takes id 1. Watch for wrapround. */
	if (into->alib_max >= A2_MAX_ALID) {
		unsigned j;
		for(id=1;id < A2_MAX_ALID;id++) {
			/* it can't have symbols if it has no code or data. */
			for(j=0;j<into->memblks;j++)
				if(into->mem[j].alib_id==id) goto continue2;
			for(j=0;j<into->cp_count;j++)
				if(into->cp_all[j].alib_id==id) goto continue2;
			break;
continue2:
		}
		if (id == A2_MAX_ALID) {
			printk2(PRINTK_CRIT, "Library limit reached\n");
			return 0;
		}
	} else {
		id=++into->alib_max;
	}
	if (imageman_loadimage_al(from, into, id)) {
		/* Damn. We'll have to clean up the mess. */
		imageman_unload_al(into, id);
		return 0;
	} else {
		return id;
	}
}

static int imageman_loadimage_al(char *from, struct vcpu *into, unsigned alib_id) {
	FILE *in;
	struct message_desc mess_desc;
	int flags=0;
	struct symbol *sdata=NULL;
	struct stable sym;
	unsigned code_base=0, mem_base, code_max=0, mem_max; /* pageof(mem) >= mem_base, mem < mem_max */
	
	if (!(in=fopen(from, "rb"))) {
		perror("image fopen");
		return 1;
	}

	/* We don't know how many memblks we need in advance. So use ones next to the 'top' */
	for(mem_base=into->memblks;mem_base && !into->mem[mem_base-1].memory;mem_base--);
	mem_max=mem_base;

	while(fread(&mess_desc, sizeof(mess_desc), 1, in))
	{
		if (!(flags & IMLI_PROGSPEC)) {
			if (mess_desc.type != BFMT_PROGSPEC) {
				printk2(PRINTK_CRIT, "Image does not start with progspec\n");
				fclose(in);
				return 1;
			}
		}
		else if (flags & IMLI_EOF) printk2(PRINTK_CRIT, "Overterminated image (EOF tag with data afterward)\n");
		else if (mess_desc.type == BFMT_PROGSPEC) {
			printk2(PRINTK_CRIT, "Multiple progspecs?\n");
			fclose(in);
			return 1;
		}

		if (mess_desc.type != BFMT_SYM && mess_desc.type != BFMT_RELOC)		
			flags&=~IMLI_SYM; /* reloc entries come immediately after symbols */

		switch(mess_desc.type)
		{
			case BFMT_PROGSPEC: {
				struct progspec pspec;
				int i;

				if (sizeof(pspec) != mess_desc.size) {
					printk2(PRINTK_CRIT, "Incomprehensible element size for pspec (%d/%d)\n",
							mess_desc.size, sizeof(pspec));
					fclose(in);
					return 1;
				}
				frread(&pspec, mess_desc.size, 1, in);
				flags|=IMLI_PROGSPEC;

				/* If we're loading a library, better not
				 * change domains and priority! */
				if (alib_id > 1) break;
				
				strncpy(into->pname, pspec.name, A2_MAX_PNAME);
				into->priority=pspec.priority;
				/* Usleep's min sleep is actually closer to .1 ms, right?
				 * So we don't want to be too stingy with the cycles... */
				if (into->priority < 10) into->priority=10;
				if (into->priority > A2_MAX_PRIORITY) into->priority=A2_MAX_PRIORITY;
				into->priority*=100;
				
				into->domain = pspec.init_domain;

				for(i=0;i<A2_MAX_DOMAINS;i++) {
					if (!pspec.domains[i])
						break;
				}

				into->dlist=malloc(i * sizeof(unsigned));
				memcpy(into->dlist, pspec.domains, i * sizeof(unsigned));
	
				break;
			}
			case BFMT_CODE: {
				struct bcode_op_packed bopp;
				struct bcode_op *bop;
				unsigned pages, i;

				/* Multiple code segments end up at implementation-specific
				 * addresses, so we have to disallow them. */
				if (flags & IMLI_CODE) {
					printk2(PRINTK_CRIT, "Multiple code segments\n");
					fclose(in);
					return 1;
				}
#ifdef NO_CODE_PAGES
				/* Well, we can load code-less libraries anyway... */
				if (into->cp_count) {
					printk2(PRINTK_CRIT, "This copy of A2 has been compiled without shlib (code pages) support.\n");
					return 1;
				}
#endif

				flags|=IMLI_CODE;

				/* if there's 512 packets and a page is 256 packs big,
				 * the last packet is IP 511 and it's on page 1 - so we need 2 pages.
				 * if it's 513 packets we need 3 pages and IP 512 is on page 2... */
				pages=CP_PAGE_OF(mess_desc.size - 1) + 1;

				printk2(PRINTK_INFO, "%d code packets on %d pages\n", mess_desc.size, pages);

				if (!into->cp_count) {
					if (!(into->cp_all=malloc(pages * sizeof(struct codepage)))) {
						perror("malloc");
						fclose(in);
						return 1;
					};
					into->cp_count=pages;
					code_base=0;
				} else {
					unsigned begin;
					/* So now we gotta find 'pages' free pages. Make 'em contiguous. */
					for(code_base=0;code_base < into->cp_count;code_base++) {
						if (into->cp_all[code_base].bcode) continue;
						/* ok, space after? */
						begin=code_base;
						for(code_base++;code_base < into->cp_count &&
								code_base < (begin + pages);code_base++)
							if (into->cp_all[code_base].bcode) goto nospace;
						code_base=begin;
						break;
nospace: /* 2nd level continue */
					}
					/* now code_base is the 1st available page - do we need more? */
					if (into->cp_count < code_base + pages) {
						struct codepage *tmp;
						printk2(PRINTK_DEBUG, "had %d pages. now got %d\n", into->cp_count, code_base+pages);
						if (!(tmp = realloc(into->cp_all, (code_base + pages) *
								sizeof(struct codepage)))) {
							perror("malloc");
							fclose(in);
							return 1;
						};
						into->cp_all=tmp;
						into->cp_count=code_base + pages;
					} else {
						printk2(PRINTK_DEBUG, "reusing pages %d-%d\n", code_base, code_base+pages-1);
					}
				}
				code_max=code_base+pages;
			
				for(pages=code_base;pages<code_max;pages++) {
					into->cp_all[pages].size = (mess_desc.size < CP_PAGE_SIZE) ?
						mess_desc.size : CP_PAGE_SIZE;
					into->cp_all[pages].alib_id = alib_id;
					/* brown paper bag! */
					into->cp_all[pages].jitoffs = NULL;
					if (!(into->cp_all[pages].bcode =
						malloc(into->cp_all[pages].size * sizeof(struct bcode_op))))
					{
						perror("malloc");
						into->cp_all[pages].size=0;
						fclose(in);
						return 1;
					}

					/* load the code */
					for(i=0;i<into->cp_all[pages].size;i++) {
						if (!fread(&bopp, sizeof(struct bcode_op_packed), 1, in)) {
							perror("fread");
							fclose(in);
							return 1;
						}
						bop=&into->cp_all[pages].bcode[i];
						bop->bcode=bopp.bcode;
						bop->type=bopp.type;
						bop->a1.val.u=bopp.a1.val.u;
						bop->a2.val.u=bopp.a2.val.u;
					}
					mess_desc.size-=CP_PAGE_SIZE;
				}

				break;
			}
			case BFMT_DATA:
			case BFMT_RODATA: {
				struct memblk *x;
				struct data_pkt bl;
				unsigned i=0;

				if (mess_desc.size > A2_MAX_BLKSIZ) {
					printk2(PRINTK_CRIT, "Image uses excessively big segment.\n");
					fclose(in);
					return 1;
				}
				/* have any unused memblks? */
				if (mem_max >= into->memblks) {
					if (into->memblks >= A2_MAX_MEMBLK) {
						printk2(PRINTK_CRIT, "Memory cap reached.\n");
						fclose(in);
						return 1;
					}
					into->memblks++;
					if (into->mem)
						x=realloc(into->mem, sizeof(struct memblk) * into->memblks);
					else
						x=malloc(sizeof(struct memblk) * into->memblks);
					if (!x) {
						perror("malloc");
						fclose(in);
						return 1;
					}
					into->mem=x;
				}
				x=&into->mem[mem_max];
				mem_max++;

				x->memory=malloc(sizeof(anyval) * mess_desc.size);
				x->alib_id=alib_id;
				x->size=mess_desc.size;
				x->destroy_scnum=0; /* Meaning 0 isn't a good syscall to use for a destroy handler. */
				
				x->mode=A2_MEM_READ;
				if (mess_desc.type != BFMT_RODATA)
					x->mode|=A2_MEM_WRITE;

				if (!x->memory) {
					perror("malloc");
					fclose(in);
					return 1;
				}
				
				while(i < mess_desc.size)
				{
					frread(&bl, sizeof(struct data_pkt), 1, in);
					/* XXX: Endianness check */
					x->memory[i].val.u=bl.u.du_int;
					i++;
				}
				break;
					  }
			case BFMT_SYM:
				if (mess_desc.size != sizeof(sym)) {
					printk2(PRINTK_CRIT, "Incomprehensible sym size (%d/%d)\n", mess_desc.size, sizeof(sym));
					fclose(in);
					return 1;
				}
				frread(&sym, sizeof(sym), 1, in);
				/* Check if address actually lies within this alib's domain,
				 * and adjust the addresses with code_base etc. */ 
				flags|=IMLI_SYM;
				if (sym.place & SYM_DATA) {
					sym.addr+=mem_base * A2_MAX_BLKSIZ;
					if (sym.addr < mem_base * A2_MAX_BLKSIZ ||
						sym.addr >= mem_max * A2_MAX_BLKSIZ) {
							printk2(PRINTK_CRIT, "Symbol defined outside our domain\n");
							fclose(in);
							return 1;
					}
				} else if (sym.place & SYM_CODE) {
					sym.addr+=code_base * CP_PAGE_SIZE;
					if (sym.addr < code_base * CP_PAGE_SIZE ||
						sym.addr >= code_max * CP_PAGE_SIZE) {
							printk2(PRINTK_CRIT, "Symbol defined outside our domain\n");
							fclose(in);
							return 1;
					}
				} /* else ?! */

				/* Local symbols don't go into the table - reloc' em right up
				 * (ever heard of an external local?! :) */
				if (sym.place & SYM_UNNAMED) sdata=NULL;
				else sdata=symman_add_symbol(into, &sym, alib_id);
				break;
			case BFMT_RELOC: {
				struct reloc r;
				if (!(flags & IMLI_SYM)) {
					printk2(PRINTK_CRIT, "Reloc entry must follow sym\n");
					fclose(in);
					return 1;
				}
				if (mess_desc.size != sizeof(r)) {
					printk2(PRINTK_CRIT, "Incomprehensible reloc size (%d/%d)\n", mess_desc.size, sizeof(r));
					fclose(in);
					return 1;
				}
				frread(&r, sizeof(r), 1, in);
				/* Have we just been asked to overwrite another alib's code?! */
				if (r.place & SYM_DATA) {
					r.addr+=mem_base * A2_MAX_BLKSIZ;
					if (r.addr < mem_base * A2_MAX_BLKSIZ ||
						r.addr >= mem_max * A2_MAX_BLKSIZ) {
							printk2(PRINTK_CRIT, "Reloc defined outside our domain\n");
							fclose(in);
							return 1;
					}
				} else if (r.place & SYM_CODE) {
					if (!(flags & IMLI_CODE)) {
						printk2(PRINTK_CRIT, "Reloc defined outside our domain\n");
						fclose(in);
						return 1;
					}
					r.addr+=code_base * CP_PAGE_SIZE * sizeof(struct bcode_op);
					if (CP_PAGE_OF(r.addr / sizeof(struct bcode_op)) < code_base ||
						CP_PAGE_OF(r.addr / sizeof(struct bcode_op)) >= code_max) {
							printk2(PRINTK_CRIT, "Reloc defined outside our domain\n");
							fclose(in);
							return 1;
					}
				} /* else ?! */
				/* 'k, gofer it */
				symman_add_reloc(into, &sym, sdata, &r, alib_id);
				break;
				 }
			case BFMT_EOF:
				flags|=IMLI_EOF;
				break;
			default:
				printk2(PRINTK_WARN, "Image contains extra data, attempting to skip.\n");
				while(mess_desc.size > 0) { fgetc(in); mess_desc.size--; }
		}
	}
	fclose(in);
	
	if (!(flags & IMLI_EOF)) {
		printk2(PRINTK_CRIT, "Unterminated image (no EOF tag)\n");
		return 1;
	}

	if (validate_bcode(into)) {
		printk2(PRINTK_CRIT, "Image's bytecode is corrupt/evil\n");
		return 1;
	}

	if (CP_PAGE_OF(into->ip) < into->cp_count) {
		/* for loading 1st image this is essential, it can't hurt the rest either */
		memcpy(&into->cp_curr, &into->cp_all[CP_PAGE_OF(into->ip)], sizeof(struct codepage));
		into->cp_curr_id=CP_PAGE_OF(into->ip);
	} else {
		/* I don't know why you would first load a data-only library,
		 * but it could be useful - say, using different domains
		 * on the same file - so we'll allow it for now */
/*		printk2(PRINTK_WARN, "IP on currently nonexistent page\n"); */
		bzero(&into->cp_curr, sizeof(struct codepage));
	}


	return 0;
}

/* For unloading just a single alid, not destroying a CPU */
int imageman_unload_al(struct vcpu *from, unsigned alib_id) {
	unsigned i;
	int unloaded=0;
	/* Code */
	for(i=0;i<from->cp_count;i++) {
		if (from->cp_all[i].alib_id==alib_id) {
			ffree(from->cp_all[i].bcode);
			ffree(from->cp_all[i].jitoffs);
			from->cp_all[i].size=0;
			from->cp_all[i].alib_id=0;
			unloaded++;
		}
	}
	/* It would be dumb to unload yourself but hey */
	if (CP_PAGE_OF(from->ip) < from->cp_count) {
		memcpy(&from->cp_curr, &from->cp_all[CP_PAGE_OF(from->ip)], sizeof(struct codepage));
		from->cp_curr_id=CP_PAGE_OF(from->ip);
	} else {
		bzero(&from->cp_curr, sizeof(struct codepage));
	}
	/* Data */
	for(i=0;i<from->memblks;i++) {
		if (from->mem[i].alib_id==alib_id) {
			if ((from->mem[i].mode & A2_MEM_MAPPED) || (from->mem[i].destroy_scnum != 0))
				printk2(PRINTK_CRIT,
					"special mem page 0x%x somehow belonged to a library!\n",
					i * A2_MAX_BLKSIZ);
			else {
				unloaded++;
				free(from->mem[i].memory);
				bzero(&from->mem[i], sizeof(struct memblk));
			}
		}
	}
	/* Symbols */
	symman_unload_al(from, alib_id);
	/* Reduce alib if possible */
	if (alib_id == from->alib_max)
		from->alib_max--;
	/* Did we just unload nothing at all? */
	if (unloaded) return 0; else return 1;
}


/* NO EXCEPTIONS, PLEEEZ!
 * Also this should be prepared for being cancelled halfway through and
 * restarted. */
void imageman_unload(struct vcpu *from) {
	unsigned i;
	/* Code */
	for(i=0;i<from->cp_count;i++) {
		ffree(from->cp_all[i].bcode);
		ffree(from->cp_all[i].jitoffs);
	}
	from->cp_count=0;
	ffree(from->cp_all);
	/* not useful but hey */
	bzero(&from->cp_curr, sizeof(struct codepage));

	/* Data */
	for(i=0;i<from->memblks;i++) {
		if (from->mem[i].memory) {
			if (from->mem[i].mode & A2_MEM_MAPPED)
				printk2(PRINTK_CRIT, "Mapped page 0x%x neglected.\n",
					i * A2_MAX_BLKSIZ);
			else if (from->mem[i].destroy_scnum)
				printk2(PRINTK_CRIT, "Special-destroy page 0x%x neglected.\n",
					i * A2_MAX_BLKSIZ);
			else {
				free(from->mem[i].memory);
				from->mem[i].memory=NULL;
			}
		}
	}
	from->memblks=0;
	ffree(from->mem);
	/* Syms */
	symman_unload(from);
	/* Modules */
	for(i=0;i<from->reserved_ct;i++) /* I ain't cleaning that s%#@ up! */
		if (from->reserved[i])
			printk2(PRINTK_CRIT, "Library %d left a reserved behind!\n", i);
	ffree(from->reserved);
	/* VFD */
	vfd_obliviate(from);
	/* Misc */
	ffree(from->dlist);
	ffree(from->stack);
	ffree(from->xstack);
	if(from->hac) hac_unload(from);
}

static int validate_bcode_page(struct codepage *page)
{
	unsigned i;
	struct bcode_op *bop;

	/* Shouldn't be any point recompiling something */
	if (page->jitoffs) return 0;
	/* Now this is an ugly cheat, but I so want to beat AOS1's speeds */
	page->jitoffs=malloc(page->size * sizeof(short));
	if (!page->jitoffs) {
		perror("malloc");
		return 1;
	}
	
	for(i=0;i<page->size;i++)
	{
		bop=&page->bcode[i];

		if (((unsigned) bop->bcode) >= CMD_INVALID) {
			printk2(PRINTK_CRIT, "Invalid opcode\n");
			return 1;
		}
		if (jit_protflags[(unsigned) bop->bcode] & A1_READ_WRITE
			&& !(bop->type & TYPE_A1(TYPE_POINTER | TYPE_REGISTER))) {
			printk2(PRINTK_CRIT, "Selfmodifying code\n");
			return 1; /* Code tried to selfmodify */
		}
		if (bop->type & TYPE_A1(TYPE_REGISTER) && bop->a1.val.u >= A2_REGISTERS) {
			printk2(PRINTK_CRIT, "Too many registers\n");
			return 1; /* Code tried to selfmodify */
		}
		if (jit_protflags[(unsigned) bop->bcode] & A2_READ_WRITE
			&& !(bop->type & TYPE_A2(TYPE_POINTER | TYPE_REGISTER))) {
			printk2(PRINTK_CRIT, "Selfmodifying code\n");
			return 1;
		}
		if (bop->type & TYPE_A2(TYPE_REGISTER) && bop->a2.val.u >= A2_REGISTERS) {
			printk2(PRINTK_CRIT, "<!> Too many registers\n");
			return 1; /* Code tried to selfmodify */
		}
		/* (0 or 1 or 2) + (0 or 1 or 2) * 3 = range 0-8. */
		page->jitoffs[i]=(bop->type & TYPE_A1(TYPE_VALMASK))
			+ ((bop->type & TYPE_A2(TYPE_VALMASK)) >> 4) * 3
			+ (unsigned) bop->bcode * 9;
		if (page->jitoffs[i] > (CMD_INVALID - 1) * 9) {
			printk2(PRINTK_CRIT, "Evil tablesmasher code\n");
			return 1; /* throw_except(curr_cpu, ERR_CORRUPT_CODE); */
		}
	}
	return 0;
}

int validate_bcode(struct vcpu *curr_cpu)
{
	unsigned i;
	int j;
	for (i=0;i<curr_cpu->cp_count;i++) {
		if ((j=validate_bcode_page( &curr_cpu->cp_all[i]) )) return j;
	}
	return 0;
}
	

