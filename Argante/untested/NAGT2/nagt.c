/*
 * The New ArGanTe compiler, Version 2 (NAGTv2)
 * Copyright (c) 2001	James Kehl <ecks@optusnet.com.au>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 * -------------------------------------------------------------------------
 * 
 * Please note I disclaim any responsibility for or claim to the images 
 * this software outputs. If you want to sell them for some exorbitant sum,
 * you can, just like a GCC'd program. If they go haywire and erase all your
 * files (or your boss', etc. etc.), that's also your problem.
 * 
 * -------------------------------------------------------------------------
 * 19/06/01: shykta
 * - change everything in NAGT1 for AOSr2 bformat.
 * 24/06/01: shykta
 * - tiny change to use 'anyval' unions like my proposed kernel
 *
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "blobstak.h"

#define blobstak_pkt_data(a) ((struct data_pkt *) blobstak_data(a))
#define blobstak_pkt_size(a) (blobstak_size(a) / sizeof(struct data_pkt))
#define blobstak_pkt_add(a, x) blobstak_add(a, x, sizeof(struct data_pkt))

#define blobstak_bcode_data(a) ((struct _bcode_op *) blobstak_data(a))
#define blobstak_bcode_size(a) (blobstak_size(a) / sizeof(struct _bcode_op))
#define blobstak_bcode_add(a, x) blobstak_add(a, x, sizeof(struct _bcode_op))

#include "config.h"
#include "bformat.h"
#include "lang2.h"

/* Syscalls */
struct sdes {
  char* name;
  int num;
};

struct sdes sys[]={
#include "autogen.h"
};

#define SCNUM (sizeof(sys)/sizeof(struct sdes))

/* Defines - lifted, as you can see... */
struct defines {
    char 		*name;
    char 		*value;
    struct defines	* next;
};

static struct defines	*defined = NULL;

#include "nagt.h"

/*
 * Syntax:
 * Data's type is given by u: (unsigned) s: (signed) f: (float)
 * t: (string, not valid in code, only for endian preservation)
 *
 * In code, type is given by u: s: f: or defaulted sensibly,
 * a pointer is given by sticking a * in front. So, these are valid:
 * 0x1020
 * f:12349138 <--- != 12349138.0!!! Is an unsigned used as float
 * f:12349138.0 <--- == 12349138.0, is an actual float
 * *f:r0 <--- a UPTR to a float
 * *r0 <--- another UPTR, but an unsigned result this time
 * *0x1020 <--- a IMMPTR
 * u:r0 <--- unsigned reg.
 * 
 * Contexts are staying.
 */

static context *context_head, *curr_symbol_context;
static symbol *curr_symbol=NULL, *all_symbols=NULL;

static blobstak *data_segment, code_segment;
static int data_segments=0, curr_data_base=0, curr_data_segment=0;

#define dataseg_page(a) (data_segment[a / MAX_BLKSIZ])
#define dataseg_offs(a) (a % MAX_BLKSIZ)
#define dataseg_now (data_segment[curr_data_segment])

static FILE *in, *out_pure;
static struct progspec pspec;

static int state=STATE_OPTION;
static int lineno=0;
static int linking=0;

#define WHITESPACE " \t\n\r"
#define my_isspace(a) strchr(WHITESPACE, a)

void error(char *message)
{
	fprintf(stderr, "<!> Line %d: %s\n", lineno, message);
	exit(1);
}

void warn(char *message)
{
	fprintf(stderr, "<+> Line %d: %s\n", lineno, message);
}

/* BlEh! Defines. A Hack... */
static int subst_define(char *in, char *s1, char *s2, char **ret)
{
	char *t, *l;
	t=strstr(in, s1);
	if (!t) return 0;

	*ret=malloc(strlen(in) - strlen(s1) + strlen(s2) + 1);
	memcpy(*ret, in, t-in);
	l=*ret+(t-in);
	memcpy(l, s2, strlen(s2) + 1);
	l+=strlen(s2);
	memcpy(l, t+strlen(s1), strlen(t) - strlen(s1));

	return 1;
}

static char *run_define(char *in)
{
	char *old, *new;
	struct defines *d;

	old=strdup(in);
	d=defined;

	while(d)
	{
		while (subst_define(old, d->name, d->value, &new))
		{
			free(old);
			old=new;
		}
		d=d->next;
	}
	return old;
}

/* Symbols */
static symbol *find_symbol_in(char *symname, context *in)
{
	symbol *x;
	x=in->syms;
	while (x)
	{
		if (!strncmp(x->s.fn, symname, sizeof(x->s.fn)))
			return x;
		x=x->next;
	}
	return NULL;
}

static void add_depend(int type, int loc, symbol *to)
{
	areloc *new;

	new=malloc(sizeof(new));
	if (!new) error("Malloc failure (out of memory!)");
	
	new->next=to->reloc;
	new->r.type=type;
	new->r.place=(state==STATE_CODE) ? SYM_CODE : SYM_DATA;
	new->r.addr=loc;

	to->reloc=new;
}

static symbol *find_symbol(char *symname)
{
	context *x;
	symbol *s;

	x=context_head;
	while (x)
	{
		if ((s=find_symbol_in(symname, x)))
			return s;
		x=x->prev;
	}
	return NULL;
}

static symbol *find_symbol_f(char *symname)
{
	symbol *s, *d;

	if (curr_symbol && !strncmp(symname, curr_symbol->s.fn, sizeof(curr_symbol->s.fn)))
		return curr_symbol;

	s=find_symbol(symname);
	if (s) return s;

	/* No symbol! Better implicitly define one then */
	d=malloc(sizeof(symbol));
	d->reloc=NULL;
	d->s.place=SYM_UNDEFINED;
	d->s.addr=0;
	d->next=context_head->syms;
	context_head->syms=d;
	strncpy(d->s.fn, symname, sizeof(d->s.fn));

	if (strlen(symname) >= sizeof(d->s.fn)) warn("Symbol name too long, clipping enforced.");
	d->s.fn[sizeof(d->s.fn) - 1]=0; /* It's easier if it's null-terminated */
	
	return d;
}

/*
 * define_symbol takes curr_symbol and puts it in existence.
 */
static void define_symbol()
{
	off_t sz;

	if (!curr_symbol) return;
	
	curr_symbol->s.place=(state==STATE_CODE) ? SYM_CODE : SYM_DATA;
	if (state == STATE_CODE)
		sz=blobstak_bcode_size(code_segment);
	else
		sz=blobstak_pkt_size(dataseg_now) + curr_data_base;
	
	/* Linker needs to relocate every symbol,
	 * and not clash symbols in contexts. */
	if (curr_symbol_context->prev)
		curr_symbol->s.place|=SYM_UNNAMED;

	curr_symbol->s.size=sz-curr_symbol->s.addr;

	/* XXX: What does this do?
	 * Apparently it adds the symbol if a symbol with that name
	 * (hopefully the same symbol) can already be found. I hope.
	 * Does it work?
	 */
	if (!find_symbol(curr_symbol->s.fn))
	{
		curr_symbol->next=curr_symbol_context->syms;
		curr_symbol_context->syms=curr_symbol;
	}

	if (!curr_symbol->s.size)
	{
		curr_symbol->s.addr=0;
		curr_symbol->s.place|=SYM_UNDEFINED;
	} else {
		areloc *d;

		curr_symbol->s.place&=~SYM_UNDEFINED;
		/* Hunt for refs. */
		d=curr_symbol->reloc;
		while (d)
		{
			int i;
			switch(d->r.type)
			{
				case RELOC_ADDR:
					i=curr_symbol->s.addr;
					break;
				case RELOC_SIZE_BYTE:
					i=curr_symbol->s.size * sizeof(int);
					break;
				case RELOC_SIZE_DWORD:
					i=curr_symbol->s.size;
					break;
				default:
					error("Unknown reftype! (internal error)");
					return;
			}
			if ((d->r.place & SYM_PLACEMASK) == SYM_DATA)
			{
/*				fprintf(stderr, "Adjusting data addr 0x%x\n", d->r.addr); */
				blobstak_pkt_data(&dataseg_page(d->r.addr))->u.du_int=i;
			}
			else
			{
/*				fprintf(stderr, "Adjusting code addr 0x%x from 0x%x to 0x%x\n",
					d->r.addr,
					(*(int *) (&((char *) blobstak_data(code_segment))[d->r.addr])),
					i); */

				((anyval *) (&((char *) blobstak_data(code_segment))[d->r.addr]))->val.u=i;
			}

			d=d->next;
		}
	}

	/* We now save symbol-table output till the last moment, so we can
	 * keep a relocation table */
	curr_symbol=NULL;
}

/*
 * Manages debugger output and relocation table stuff
 */
static void symbol_track(symbol *s)
{
	s->next=all_symbols;
	all_symbols=s;
}

static void begin_context()
{
	context *new;

	new=malloc(sizeof(context));
	if (!new) error("Out of memory while creating context");

	new->syms=NULL;
	new->prev=context_head;
	context_head=new;
}

static void end_context()
{
	symbol *s, *d;
	context *x;
	int count;

	/* Symbols shouldn't finish in a higher context than they started in
	 * (other way round is OK) */
	define_symbol();
	s=context_head->syms;
	count=0;

	x=context_head->prev;
	while(s)
	{
		d=s->next;
		/* Top level and linking? Don't worry about undefined symbols then */
		if ((s->s.place & SYM_UNDEFINED) && (!linking || x)) 
		{
			/* If we aren't top level, well, it must be defined in
			 * a higher context */
			if (x)
			{
				s->next=x->syms;
				x->syms=s;
				/* Don't bother fixing up context_head->syms
				 * because we won't use it anymore anyway. */
			} else {
				fprintf(stderr, "<+> Symbol %s undefined\n", s->s.fn);
				count++;
			}
		} else
		{
			/* Save the data */
			symbol_track(s);
		}
		s=d;
	}

	if (count)
	{
		fprintf(stderr, "<!> Line %d: %d symbols are undefined at end-of-file\n", lineno, count);
		exit(1);
	}

	x=context_head->prev;
	free(context_head);
	context_head=x;
}

/* grunt work starts here */
static void switch_state(int to)
{
	if (to==state) return;
	state=to;
}

static void do_symbol(char *ln)
{
	context *x;
	symbol *s;

	ln++; /* Ignore : */

	/* Are we in data segment? If so, we need to add a new block */
	if (state==STATE_DATA)
	{
		curr_data_segment=data_segments;
		curr_data_base=curr_data_segment * MAX_BLKSIZ;
		data_segments++;
		if (data_segment)
			data_segment=realloc(data_segment, data_segments * sizeof(blobstak));
		else
			data_segment=malloc(data_segments * sizeof(blobstak));

		if (!data_segment) error("Malloc error (Out of memory!)");
		blobstak_init(&data_segment[curr_data_segment]);
	}

	x=context_head;
	while (x)
	{
		if ((s=find_symbol_in(ln, x)))
		{
			if (!(s->s.place & SYM_UNDEFINED))
				error("Symbol doubly defined");

			curr_symbol=s;
			curr_symbol_context=x;
			break;
		}

		x=x->prev;
	}

	if (x==NULL)
	{
		curr_symbol=malloc(sizeof(symbol));
		curr_symbol_context=context_head;
		curr_symbol->reloc=NULL;
		curr_symbol->s.place=(state==STATE_CODE) ? SYM_CODE : SYM_DATA;
		curr_symbol->s.place|=SYM_UNDEFINED;
		strncpy(curr_symbol->s.fn, ln, sizeof(curr_symbol->s.fn));
	}
/*	else
		warn("Prototype ref found here"); */

	if (strlen(ln) >= sizeof(curr_symbol->s.fn)) warn("Symbol name too long, clipping enforced.");
	curr_symbol->s.fn[sizeof(curr_symbol->s.fn) - 1]=0; /* It's easier if it's null-terminated */

	curr_symbol->s.addr=(state & STATE_CODE) ?
		blobstak_bcode_size(code_segment):blobstak_pkt_size(dataseg_now) + curr_data_base;
}

static void do_option(char *ln)
{
	char *tmp, *arg2, *ret;
	int itmp, itmp2;
	
	if (state != STATE_OPTION)
		error("Option not allowed in .code or .data block");
	
	ln++; /* Ignore ! */

	itmp=strcspn(ln, WHITESPACE);
	ln[itmp]=0;
	tmp=&ln[itmp+1];
	
	itmp=strspn(tmp, WHITESPACE);
	arg2=&tmp[itmp];
	if (!*arg2)
		error("Option without arguments");
	
	if (!strcasecmp(ln, "SIGNATURE"))
	{
		if (*arg2 != '"') {
			warn("!signature not enclosed in quotes");
			itmp=0;
		} else {
			itmp=1;
			arg2++;
		}

		itmp2=0;
		while (*arg2 && itmp2 < (sizeof(pspec.name) - 1))
		{
			if (itmp && *arg2=='"') break;
			pspec.name[itmp2]=*arg2;
			itmp2++; arg2++;
		}
		pspec.name[itmp2]=0; /* Null-terminate. Note we ensure enough room here */
	}
	else if (!strcasecmp(ln, "PRIORITY"))
	{
		/* Erm... hehehe, I really didn't see this one... */
		pspec.priority=strtoul(arg2, &ret, 0);
		if (*ret) error("Trailing garbage.");
	}
	else if (!strcasecmp(ln, "INITDOMAIN"))
	{
		pspec.init_domain=strtoul(arg2, &ret, 0);
		if (*ret) error("Trailing garbage.");
	}
	else if (!strcasecmp(ln, "DOMAINS"))
	{
		itmp=0;
		bzero(pspec.domains, sizeof(pspec.domains));
		
		tmp=strtok(arg2, " \t,");
		while(tmp && itmp < MAX_DOMAINS)
		{
			pspec.domains[itmp]=strtoul(tmp, &ret, 0);
			if (*ret) error("Trailing garbage.");
			if (!pspec.domains[itmp]) warn("Domains listed after zero will be ignored");
			tmp=strtok(NULL, " \t,");
			itmp++;
		}
	}
	else
		error("Unknown option");
}

static void do_pragma(char *ln)
{
	int itmp;
	char *arg, *tmp;
	ln++; /* Ignore . */

	itmp=strcspn(ln, WHITESPACE);
	ln[itmp]=0;
	tmp=&ln[itmp+1];
	
	itmp=strspn(tmp, WHITESPACE);
	
	arg=&tmp[itmp];
	if (!*arg) arg=NULL;

	if (!strcasecmp(ln, "CODE"))
	{
		switch_state(STATE_CODE);
	}
	else if (!strcasecmp(ln, "DATA"))
	{
		switch_state(STATE_DATA);
	}
	else if (!strcasecmp(ln, "DEFINE"))
	{
		char *name, *val;
		struct defines *d;
		if (!arg) error("Define without arguments");
		name=strtok(arg, " \t\n\r");
		val=strtok(NULL, " \t\n\r");
		
		if (!name || !val) error("Define requires two arguments");

		/* Add a definition! */
		d=malloc(sizeof(struct defines));
		d->name=strdup(name);
		d->value=strdup(val);
		d->next=defined;
		defined=d;
	}
	else if (!strcasecmp(ln, "END")) /* What the?! */
	{
	}
	else
		error("Unknown pragma");

}

static int rep_data_offs; /* Data offset as of line start, for repeat */

/* ICK #1 */
static char *do_data(char *ln)
{
	char *next, *ret;
	struct data_pkt pkt;
	char enforced_type=0;

	bzero(&pkt, sizeof(pkt));
	/* Do we need to add a default data segment? */
	if (!data_segments)
	{
		curr_data_segment=data_segments;
		curr_data_base=curr_data_segment * MAX_BLKSIZ;
		data_segments++;
		data_segment=malloc(data_segments * sizeof(blobstak));
		blobstak_init(&data_segment[curr_data_segment]);

		if (!data_segment) error("Malloc error (Out of memory!)");
	}
	
	/* Trim beginning - we're recursive after all */
	while (*ln && my_isspace(*ln))
		ln++;

	if(*ln==0)
		return NULL;

	/* First, does it have a type specifier? u: s: f: t: */
	if (ln[1]==':' && ln[0]!='"')
	{
		enforced_type=1;
		switch(ln[0])
		{
			case 'u': pkt.type = DTYPE_UNSIGNED; break;
			case 's': pkt.type = DTYPE_SIGNED; break;
			case 'f': pkt.type = DTYPE_FLOAT; break;
			case 't': pkt.type = DTYPE_STRING; break;
			default: error("Unrecognized type specifier (must be u: s: f: or t:)");
		}

		ln+=2;

		/* If they put whitespace after typespec */
		while (*ln && my_isspace(*ln))
			ln++;

		if(*ln==0) {
			warn("Typespecifier specifies no data");
			return NULL;
		}
	}

	/* Is it a string? */
	if(ln[0]=='"')
	{
		int n=0;
		if (!enforced_type) pkt.type = DTYPE_STRING;
		
		while (*(++ln)!='"')
		{
			if (!(*ln))
				error("Unterminated string constant.");
			else if (*ln=='\\')
			{
				ln++;
				switch(*ln)
				{ /* There's a few I missed, I'm sure */
					case 'x':
					{ /* Shellcode in Argante? Cool! */
						static char nu[3]; /* Ick... like the only static buffer here */
						ln++;
						if (!(nu[0]=*ln)) error("Bad \\x sequence.");
						ln++;
						if (!(nu[1]=*ln)) error("Bad \\x sequence.");
						nu[2]=0;

						*ln=strtol(&nu[0], &ret, 0x10); /* Hex radix */
						if (ret && *ret) error("Bad \\x sequence."); /* Trailing garbage? */
						break;
					}
					case 'n': *ln='\n'; break;
					case 'r': *ln='\r'; break;
					case 'e': *ln='\e'; break;
					case 'b': *ln='\b'; break;
					case 't': *ln='\t'; break;
					case 0: error("Unterminated escape sequence.");
				}
			}
			pkt.u.dt_string[n]=*ln;
			n++;
			/* Filled this packet? */
			if (n >= sizeof(pkt.u.dt_string)) {
				blobstak_pkt_add(&dataseg_now, &pkt);
				n=0;
			}
		}
		ln++;
		/* ummm - duh... */
		if (n) {
			/* Fill the rest of the packet with NUL's */
			while(n % sizeof(pkt.u.dt_string)) {
				pkt.u.dt_string[n]=0;
				n++;
			}
			blobstak_pkt_add(&dataseg_now, &pkt);
		}
		/* Recurse; we may have "stuff" 0x81 "stuff" to deal with */
		return ln;
	}

	/* For simplicity's sake, break up the string. */
	next=ln;
	while (*next && !my_isspace(*next))
		next++;
	
	if (*next) {
		*next=0; next++;
	} else
		next=NULL;

	/* Is it a special op? */
	if (!strcasecmp(ln, "REPEAT"))
	{
		int sz, ct, z;
		/*
		 * Repeat repeats all data on the same line as the repeat is given on. 
		 * It's the most useful possibility.
		 */
		if (enforced_type) warn("Typespecifier does nothing to REPEAT");
	
		/* Get repct */
		ct=strtol(next, &ret, 0) - 1; /* We already wrote one block of data */
		if (ret==next) { warn(next); error("Garbage numeric"); }

		if (ct<0 || ct> MAX_BLKSIZ)
			error("Senseless repeat count");

		/* Get data to repeat */
		/* Amount of data to repeat */
		sz=blobstak_pkt_size(dataseg_now) - rep_data_offs;
		if (!sz) error("No data given to repeat");
		
		/* Ok... repeat it */
		while(ct > 0)
		{
			for(z=0;z<sz;z++)
			{
				blobstak_pkt_add(&dataseg_now,
					&blobstak_pkt_data(dataseg_now)[rep_data_offs+z]);
			}
			ct--;
		}
		
		return ret;
	}

	/* Now check it for a number. */
		
	/* I guess we just assume it's floating point if it contains . or ends in f.
	 * Course if it starts with 0x, it's not a float... agh */
	if((ln[strlen(ln)-1]=='f' && strncmp(ln, "0x", 2)) || strchr(ln, '.'))
	{
		float f;

		f=strtod(ln, &ret);
		if (!enforced_type) pkt.type = DTYPE_FLOAT;
		pkt.u.df_float=f;
	}
	else
	{
		if (enforced_type && pkt.type == DTYPE_SIGNED)
		{
			pkt.u.ds_int=strtol(ln, &ret, 0);
		} else {
			pkt.u.du_int=strtoul(ln, &ret, 0);
			
			if (!enforced_type) pkt.type = DTYPE_UNSIGNED;
		}
	}

	if (ret==ln) error("Garbage numeric.");
	
	blobstak_pkt_add(&dataseg_now, &pkt);
	return next;
}

/* ICK #2 */
static void arg_parse(char *ln, anyval *out, char *type, int offs)
{
	int i;
	char *z, *ret;
	symbol *s;
	int type_set=0;
	int type_high=0;
	
	/* Find type specifiers */
	if (*ln == '*') /* Pointer spec. */
	{
		type_high|=TYPE_POINTER;
		
		ln++;
		while (*ln && my_isspace(*ln)) ln++; 

		if (!*ln) error("Nothing after pointer sign?");
	}
	if (ln[1]==':')
	{
		type_set=1;
		switch(ln[0])
		{
			case 'u': *type = TYPE_UNSIGNED; break;
			case 's': *type = TYPE_SIGNED; break;
			case 'f': *type = TYPE_FLOAT; break;
			default: error("Unrecognized type specifier (must be u: s: or f:)");
		}
		ln+=2;

		while (*ln && my_isspace(*ln)) ln++; 

		if (!*ln) error("Nothing after typespec?");
	}
	
	z=ln+1;

	/* 0xf is not a float... DOH! */
	if((ln[strlen(z)]=='f' && strncmp(ln, "0x", 2)) || strchr(ln, '.'))
	{
		float f;
		if (sizeof(float) != sizeof(long)) error("sizeof(float) is somewhat odd");
		f=strtod(ln, &ret);
		out->val.f=f;

		if (!type_set) *type=TYPE_FLOAT;
		type_high|=TYPE_IMMEDIATE;
		
		*type|=type_high; return;
	}

	switch(*ln) 
	{

		case '$': /* Syscall */
			if (!type_set) *type=TYPE_UNSIGNED;
			type_high|=TYPE_IMMEDIATE;

			for (i=0;i<SCNUM;i++)
			{
				if (!strcasecmp(z,sys[i].name))
				{
					out->val.u=sys[i].num;
					*type|=type_high; return;
				}
			}
			error("Unknown syscall");
			return;
		case ':': /* Pointer */
			if (!type_set) *type=TYPE_UNSIGNED;
			type_high|=TYPE_IMMEDIATE;

			s=find_symbol_f(z);
			add_depend( RELOC_ADDR, blobstak_size(code_segment) + offs, s);
			out->val.u=s->s.addr;
			*type|=type_high; return;
		case '^': /* char size */
			if (!type_set) *type=TYPE_UNSIGNED;
			type_high|=TYPE_IMMEDIATE;
			
			s=find_symbol_f(z);
			add_depend( RELOC_SIZE_BYTE, blobstak_size(code_segment) + offs, s);
			out->val.u=s->s.size * sizeof(int);
			*type|=type_high; return;
		case '%': /* dword size */
			if (!type_set) *type=TYPE_UNSIGNED;
			type_high|=TYPE_IMMEDIATE;

			s=find_symbol_f(z);
			add_depend( RELOC_SIZE_DWORD, blobstak_size(code_segment) + offs, s);
			out->val.u=s->s.size;
			*type|=type_high; return;
		case 'r': /* Cool, registers. */
			// if (type_high & TYPE_POINTER)
			if (!type_set) {
				warn("You haven't given a type for accessing this register, defaulting to u:");
				*type=TYPE_UNSIGNED;
			}

			type_high|=TYPE_REGISTER;
			break;
		default:
			type_high|=TYPE_IMMEDIATE;
			z=ln;
	}
	
	 /* Convert an integer (this applies to the 15 in r15 too) */
	if (type_set && *type & TYPE_SIGNED && !(type_high & TYPE_REGISTER))
		out->val.s=strtol(z, &ret, 0);
	else
	{
		if (!type_set) *type=TYPE_UNSIGNED;
		out->val.u=strtoul(z, &ret, 0);
	}

	if (ret==z) error("Garbage numeric.");

	*type|=type_high;
}

/* I like this better. */
static void do_code(char *ln)
{
	struct _bcode_op bop;
	char *arg1, *arg2, *tmp;
	char t1, t2;
	int itmp;
	int ict;

	/* I almost long for sscanf. */
	while (*ln && my_isspace(*ln))
		ln++;
	itmp=strcspn(ln, WHITESPACE);

	if (ln[itmp]!=0)
	{
		ln[itmp]=0;
		tmp=&ln[itmp+1];
	} else
		tmp="";

	arg1=strtok(tmp, ",");
	arg2=strtok(NULL, ",");

	/* Ok, we now have mnem in ln, and the args. */
	/* Try and comprehend the args. */
	ict=0;

	/* Clear the struct properly. */
	bzero(&bop, sizeof(bop));
	
	if (arg1) {
		ict=1;
		while (*arg1 && my_isspace(*arg1))
			arg1++;
		arg_parse(arg1, &bop.a1, &t1, 4); /* Fingers crossed! */
		bop.type|=TYPE_A1(t1);
	} 
	if (arg2) {
		ict=2;
		while (*arg2 && my_isspace(*arg2))
			arg2++;
		arg_parse(arg2, &bop.a2, &t2, 8);
		bop.type|=TYPE_A2(t2);
	}
	
	/* Find the mnem... */
	for(itmp=0;itmp<OPS;itmp++)
	{
		if (!strcasecmp(op[itmp].name, ln))
		{
			/* What about opcode overloading?
			 * One day we will want 0,1 and 2-arg versions of halt... */
			if (ict!=op[itmp].params)
			{
				error("Wrong number of parameters");
			}
			bop.bcode=op[itmp].bcode;
			break;
		}
	}
	if (itmp>=OPS) error("Unknown mnemonic!");

	/* Check types.
	 * <sigh>, I thought I'd fixed this &&!&&|| with lang2...
	 *
	 * Checks:
	 * - whether op accepts u/s/f.
	 * - whether op won't accept an IMM. (ImmPtrs still ok)
	 * - whether op won't accept a PTR.
	 */
	if (ict > 0)
	{
		if (!((1 << (t1 & TYPE_VALMASK)) & op[itmp].tparam1))
			error("Type mismatch for argument 1 (u/s/f)");
		if ((op[itmp].tparam1 & (1 << TYPE_REGISTER)) && !(t1 & TYPE_REGISTER) && !(t1 & TYPE_POINTER))
			error("Type mismatch for argument 1 (need reg)");
		if (!(op[itmp].tparam1 & (1 << TYPE_POINTER)) && (t1 & TYPE_POINTER))
			error("Type mismatch for argument 1 (can't use ptr)");
		
		if (ict > 1)
		{
			if (!((1 << (t2 & TYPE_VALMASK)) & op[itmp].tparam2))
				error("Type mismatch for argument 2 (u/s/f)");
			if ((op[itmp].tparam2 & (1 << TYPE_REGISTER)) && !(t2 & TYPE_REGISTER) && !(t2 & TYPE_POINTER))
				error("Type mismatch for argument 2 (need reg)");
			if (!(op[itmp].tparam2 & (1 << TYPE_POINTER)) && (t2 & TYPE_POINTER))
				error("Type mismatch for argument 2 (can't use ptr)");
		}
	}

	/* Phew. */
	blobstak_bcode_add(&code_segment, &bop);
}

static void finish_output();

int main(int argc, char *argv[])
{
	char *inl, *o;
	size_t n;
	int z=1;
	char *ln, *tmp;

	if (sizeof(anyval) != sizeof(int))
	{
		fprintf(stderr, "Nasty architecture you have there.\n");
		exit(-1);
	}
	
	if (argc < 3)
	{
		fprintf(stderr, 
		"NAGTv2 - RSISv2 assembler. Copyright (C) 2001 James Kehl <ecks@optusnet.com.au>\n"
		"NAGT comes with ABSOLUTELY NO WARRANTY; this is free software, and you are\n"
		"welcome to redistribute it under certain conditions. See COPYING for details.\n\n");

		fprintf(stderr, "Usage: %s [-c] filename.in filename.out\n", argv[0]);
		exit(1);
	}

	if (!strcmp(argv[1], "-c"))
	{
		linking=1;
		z++;
	}
	
	fprintf(stderr, "<-> Producing %s from %s in %s mode.\n", argv[z+1], argv[z], linking ? "object" : "standalone"); 

	in=fopen(argv[z], "r");
	if (!in) { perror("fopen(r)"); return 1; }
	z++;
	
	out_pure=fopen(argv[z], "wb");
	if (!out_pure) { perror("fopen(w)"); return 1; }

	context_head=NULL;
	begin_context();
	
	pspec.magic=BMAGIC;
	bzero(pspec.name, sizeof(pspec.name));
	bzero(pspec.domains, sizeof(pspec.domains));
	pspec.init_domain=0;
	pspec.priority=1;
	 
	inl=NULL; n=0;
	/* Get lines while they exist */
	while(getline(&inl, &n, in) > 0)
	{
		lineno++;
		/* Exterminate comments. */
		tmp=strchr(inl, '#');
		if (tmp) *tmp=0;
		tmp=strchr(inl, '\n');
		if (tmp) *tmp=0;

		/* Yick. */
		o=ln=run_define(inl);
		/* Strip off leading whitespace. */
		tmp=ln;
		while (*tmp && my_isspace(*tmp))
		{
			ln=tmp;
			tmp++;
		}
		/* Strip off trailing whitespace - do we need this?! */
		if (!*tmp) continue;
		tmp=strchr(tmp, 0); tmp--;
		while(tmp > ln && my_isspace(*tmp))
		{
			*tmp=0;
			tmp--;
		}
		if (!*ln) continue; /* Eh? Blank line? */

		/* Parse the Code!!! */
		if (ln[0] == '!') {
			define_symbol();
			do_option(ln);
		} else if (ln[0]=='.') {
			define_symbol();
			do_pragma(ln);
		} else if (ln[0]==':') {
			define_symbol();
			do_symbol(ln);
		/* Contexts - added v1.6 Apr 18 2001 */
		} else if (ln[0]=='{') {
			begin_context();
		} else if (ln[0]=='}') {
			end_context();
			if (!context_head)
				error("Global context undefined: you don't want to do that");
		} else {
			if (state==STATE_DATA)
			{
				/* Not neccessary to add DATA_BASE, REPEAT's always in one segment */
				rep_data_offs=blobstak_pkt_size(data_segment[curr_data_segment]);
	
				tmp=ln;
				while(tmp) tmp=do_data(tmp);
			}
			else if (state==STATE_CODE)
				do_code(ln);
			else
				error("Command not understood in this state");
		}
		free(o);
	}

	define_symbol();
	end_context();

	finish_output();

	return 0;
}

/* Finished, at last. Create the One File to Bind Them All. */
static void finish_output()
{
	struct message_desc mess_desc;
	symbol *s;
	areloc *r;
	int z;

	fprintf(stderr, "<-> Writing file: ");
	/* Write progheader */
	mess_desc.type=BFMT_PROGSPEC;
	mess_desc.size=sizeof(pspec);
	fwrite(&mess_desc, sizeof(mess_desc), 1, out_pure);
	fwrite(&pspec, sizeof(pspec), 1, out_pure);

	/* The code... */
	mess_desc.type=BFMT_CODE;
	mess_desc.size=blobstak_bcode_size(code_segment);
	fprintf(stderr, "%d code packets, ", mess_desc.size);
	fwrite(&mess_desc, sizeof(mess_desc), 1, out_pure);
	
	fwrite(blobstak_bcode_data(code_segment), sizeof(struct _bcode_op), mess_desc.size, out_pure);

	/* The data... */
	fprintf(stderr, "%d data segments (size ", data_segments);
	mess_desc.type=BFMT_DATA;

	for (z=0;z<data_segments;z++)
	{
		mess_desc.size=blobstak_pkt_size(data_segment[z]);
		fwrite(&mess_desc, sizeof(mess_desc), 1, out_pure);
		
		if (z != 0) fprintf(stderr, ", "); fprintf(stderr, "%d", mess_desc.size);
		
		fwrite(blobstak_pkt_data(data_segment[z]), sizeof(struct data_pkt), mess_desc.size, out_pure);
	}
	fprintf(stderr, ")\n<-> Symbol/Reloc table info:\n");

	/* Syms and relocs */
	s=all_symbols;
	while(s)
	{
		mess_desc.type=BFMT_SYM;
		mess_desc.size=sizeof(struct stable);
		fwrite(&mess_desc, sizeof(mess_desc), 1, out_pure);
		fprintf(stderr, "    %s - size 0x%x place 0x%x addr 0x%x\n", s->s.fn, s->s.size,
				s->s.place, s->s.addr);

		fwrite(&s->s, sizeof(struct stable), 1, out_pure);

		mess_desc.type=BFMT_RELOC;
		mess_desc.size=sizeof(struct reloc);

		r=s->reloc;
		while (r)
		{
			fprintf(stderr, "    \treftype %d place 0x%x addr 0x%x\n", r->r.type,
				r->r.place, r->r.addr);
			
			fwrite(&mess_desc, sizeof(mess_desc), 1, out_pure);
			fwrite(&r->r, sizeof(struct reloc), 1, out_pure);
			r=r->next;
		}
		s=s->next;
	}
	
	/* End it */
	mess_desc.type=BFMT_EOF;
	mess_desc.size=0;
	fwrite(&mess_desc, sizeof(mess_desc), 1, out_pure);
}
