/*
 * The New ArGante Compiler (NAG)
 * James Kehl <ecks@optusnet.com.au>
 * 
 * Use under GPL; just like GCC. I have no claim
 * over the images you produce with this.
 * ---------------------------------------------
 *
 * 30/4/02. Backported bugfixes and blue-defines from A2.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "memory.h"
#include "config.h"
#include "bcode.h"
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

/* Defines - strange passing resembance to AGTC */
struct defines {
	 char 		*name;
	 char 		*value;
	 struct defines	* next;
	 int isblue; /* Yes! It's TRUE! */
	 int val_len;
	 int nam_len;
};

struct defines 		*defined = NULL;

#include "symtab.h"
#include "bcode_op.h"

/*
 * Syntax:
 * exactly like the old one(!)
 * 
 * Extensions:
 * NONE. I've given up on 'embracing + extending' assembly language. :)
 *
 * Despite this, contexts are staying in. They are the best way I can
 * see of keeping the namespace clean, which is essential for successful
 * linking.
 */

context *context_head;
symbol *curr_symbol=NULL;
context *curr_symbol_context;

FILE *in, *out_bcode, *out_data, *out_pure, *out_syms;

struct bformat bfmt;
int state=STATE_OPTION;
int lineno=0;
int version=1;

int linking=0;

#define WHITESPACE " \t\n\r"
#define my_isspace(a) strchr(WHITESPACE, a)

void Nerror(char *message)
{
	fprintf(stderr, "<!> Line %d: %s\n", lineno, message);
	exit(1);
}

void Nwarn(char *message)
{
	fprintf(stderr, "<+> Line %d: %s\n", lineno, message);
}

/* Defines. A little less of a hack, 25/1/02 JSK */
static int subst_define(char **in, size_t *size, struct defines *d)
{
	char *t, *last;
	size_t newsz, rmdr;

	last=*in;
	newsz=0;

	while((t=strstr(last, d->name))) {
		rmdr=strlen(t + d->nam_len);
		newsz=(t - *in) + d->val_len + rmdr + 1;
		if (*size < newsz) {
			int offs;
			/* Should we use Power-of-Two here? */
			offs=last-*in;
			t=realloc(*in, newsz);
			if (!t) Nerror("Malloc failure (out of memory!)");
			*in=t;
			*size=newsz;
			last=*in+offs;
			t=strstr(last, d->name);
			if (!t) Nerror("Macro name vanished! (internal error?)");
		}

		/* The first part of the string's OK, but
			the last mustn't be overwritten. */
		memmove(t+d->val_len, t+d->nam_len, rmdr + 1);
		memcpy(t, d->value, d->val_len);
		last=t + d->val_len;
	}

	return (newsz > 0) ? 1 : 0;
}

static void run_define(char **in, size_t *size)
{
	struct defines *d;

	d=defined;
	while(d)
	{
		if (!d->isblue && subst_define(in, size, d)) {
			d->isblue=1;
			d=defined;
		} else d=d->next;
	}
	d=defined;
	while(d) {
		d->isblue=0;
		d=d->next;
	}
}

/* Symbols */
symbol *find_symbol_in(char *symname, context *in)
{
	symbol *x;
	x=in->syms;
	while (x)
	{
		if (!strncmp(x->name, symname, sizeof(x->name)))
			return x;
		x=x->next;
	}
	return NULL;
}

void add_depend(int type, int loc, symbol *to)
{
	depend *new;

	new=malloc(sizeof(depend));
	if (!new) Nerror("Malloc failure (out of memory!)");
	
	new->next=to->dep;
	new->reftype=type;
	new->stortype=state;
	new->location=loc;
	to->dep=new;

}

symbol *find_symbol(char *symname)
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

symbol *find_symbol_f(char *symname)
{
	symbol *s, *d;

	if (curr_symbol && !strcmp(symname, curr_symbol->name))
		return curr_symbol;

	s=find_symbol(symname);
	if (s) return s;

	/* No symbol! Better implicitly define one then */
	d=malloc(sizeof(symbol));
	d->dep=NULL;
	d->stortype=0;
	d->defined=0;
	d->location=0;
	d->next=context_head->syms;
	context_head->syms=d;
	strncpy(d->name, symname, sizeof(d->name));

	if (strlen(symname) >= sizeof(d->name)) Nwarn("Symbol name too long, clipping enforced.");
	d->name[sizeof(d->name) - 1]=0; /* It's easier if it's null-terminated */
	
	return d;
}

/*
 * define_symbol takes curr_symbol and puts it in existance.
 */
void define_symbol()
{
	off_t sz;

	if (!curr_symbol) return;
	
	curr_symbol->stortype=state;
	if ((curr_symbol->stortype & STATE_CODE))
		sz=ftello(out_bcode) / 12;
	else
	{
		int n;
		/* XXX XXX XXX MEGA FIXME XXX XXX!
		 * The kernel is RETARDED and needs all
		 * accesses padded to a dword boundary.
		 */
		sz=ftello(out_data);
		n=(4 - (sz % 4)) % 4;
		
		while(n)
		{
			fputc(0, out_data);
			n--;
		}
	}
	
	/* Brown paper bag mistake: linker needs to relocate every symbol.
	 * However, it needs to not clash symbols in contexts. */
	if (curr_symbol_context->prev)
		curr_symbol->stortype|=SYM_NO_CLASH;

	curr_symbol->size=sz-curr_symbol->location;

	if (!find_symbol(curr_symbol->name))
	{
		curr_symbol->next=curr_symbol_context->syms;
		curr_symbol_context->syms=curr_symbol;
	}

	if (!curr_symbol->size)
	{
		curr_symbol->location=0;
		curr_symbol->defined=0;
	} else {
		depend *d, *n;
		/* Arrrrrgh. Kernel braindeath. */
		if (curr_symbol->stortype & STATE_DATA)
			curr_symbol->location/=4;

		curr_symbol->defined=1;
		/* Hunt for refs. */
		d=curr_symbol->dep;
		while (d)
		{
			int i;
			FILE *f;
			n=d->next;
			f=(d->stortype & STATE_CODE) ? out_bcode : out_data;
			fseek(f, d->location, SEEK_SET);
			switch(d->reftype)
			{
				case REF_PTR:
					i=curr_symbol->location;
					break;
				case REF_SIZE_CHAR:
					i=curr_symbol->size;
					break;
				case REF_SIZE_DWORD:
					i=curr_symbol->size / 4;
					break;
				default:
					Nerror("Unknown reftype! (internal error)");
			}
			fwrite(&i, sizeof(int), 1, f);
			fseek(f, 0, SEEK_END);

			d=n;
		}
	}

	/* We now save symbol-table output till the last moment, so we can
	 * keep a relocation table */
	curr_symbol=NULL;
}

/*
 * Manages debugger output and relocation table stuff
 */
void symbol_track(symbol *s)
{
	depend *r;
	const int n1=-1;
	const char c1=-1;
	
	if (!out_syms) return;

//	fprintf(stderr, "<.> storing symbol %s, %s %s\n", s->name, (s->defined) ? "defined" : "undef", (s->stortype & SYM_NO_CLASH) ? "private": "public");
	
	/* NAME (dynamic len), ADDRESS, SIZE, TYPE.
	 * Size is zero for undefined symbols, of course. */
	fwrite(&s->name, strlen(s->name) + 1, 1, out_syms);
	fwrite(&s->location, sizeof(s->location), 1, out_syms);
	fwrite(&s->size, sizeof(s->size), 1, out_syms);
	fwrite(&s->stortype, sizeof(s->stortype), 1, out_syms);
	/* Reloc table output. Sorry, I broke the format. *sniff*/
	r=s->dep;
	while (r)
	{
		/* We write char reftype; char stortype; off_t location; for each depend. */
		fwrite(&r->reftype, sizeof(r->reftype), 1, out_syms);
		fwrite(&r->stortype, sizeof(r->stortype), 1, out_syms);
		fwrite(&r->location, sizeof(r->location), 1, out_syms);
		r=r->next;
	}
	/* -1 for reftype terminates the chain. */ 
	fwrite(&c1, sizeof(r->reftype), 1, out_syms);
	fwrite(&c1, sizeof(r->stortype), 1, out_syms);
	fwrite(&n1, sizeof(r->location), 1, out_syms);
}

void begin_context()
{
	context *new;

	new=malloc(sizeof(context));
	if (!new) Nerror("Out of memory while creating context");

	new->syms=NULL;
	new->prev=context_head;
	context_head=new;
}

void end_context()
{
	symbol *s, *d;
	context *x;
	int count;
	s=context_head->syms;
	count=0;

	/* Symbols shouldn't finish in a higher context than they started in
	 * (other way round is OK) */
	define_symbol();
	
	x=context_head->prev;
	while(s)
	{
		d=s->next;
		/* Top level and linking? Don't worry about undefined symbols then */
		if (!s->defined && (!linking || x)) 
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
				fprintf(stderr, "<+> Symbol %s undefined\n", s->name);
				count++;
			}
		} else
		{
			depend *r, *n;
			/* Save the data */
			symbol_track(s);

			/* Free the chain */
			r=s->dep;
			while (r)
			{
				n=r->next;
				free(r);
				r=n;
			}
			free(s);
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
void switch_state(int to)
{
	if (to==state) return;
	state=to;
}

void do_symbol(char *ln)
{
	context *x;
	symbol *s;

	ln++; /* Ignore : */
	
	x=context_head;
	while (x)
	{
		if ((s=find_symbol_in(ln, x)))
		{
			if (s->defined)
				Nerror("Symbol doubly defined");

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
		curr_symbol->dep=NULL;
		curr_symbol->stortype=state;
		strncpy(curr_symbol->name, ln, sizeof(curr_symbol->name));
	}
/*	else
		Nwarn("Prototype ref found here"); */

	if (strlen(ln) >= sizeof(curr_symbol->name)) Nwarn("Symbol name too long, clipping enforced.");
	curr_symbol->name[sizeof(curr_symbol->name) - 1]=0; /* It's easier if it's null-terminated */

	curr_symbol->defined=0;
	curr_symbol->location=ftello((state & STATE_CODE) ? out_bcode : out_data)
		/ ((state & STATE_CODE) ? 12 : 1);
}

void do_option(char *ln)
{
	char *tmp, *arg2;
	int itmp, itmp2;
	
	if (state != STATE_OPTION)
		Nerror("Option not allowed in .code or .data block");
	
	ln++; /* Ignore ! */

	itmp=strcspn(ln, WHITESPACE);
	ln[itmp]=0;
	tmp=&ln[itmp+1];
	
	itmp=strspn(tmp, WHITESPACE);
	arg2=&tmp[itmp];
	if (!*arg2)
		Nerror("Option without arguments");
	
	if (!strcasecmp(ln, "SIGNATURE"))
	{
		if (*arg2 != '"') {
			Nwarn("!signature not enclosed in quotes");
			itmp=0;
		} else {
			itmp=1;
			arg2++;
		}

		itmp2=0;
		while (*arg2 && itmp2 < (sizeof(bfmt.signature) - 1))
		{
			if (itmp && *arg2=='"') break;
			bfmt.signature[itmp2]=*arg2;
			itmp2++; arg2++;
		}
		bfmt.signature[itmp2]=0; /* Null-terminate. Note we ensure enough room here */
	}
	else if (!strcasecmp(ln, "PRIORITY"))
	{
		bfmt.priority=atoi(arg2);
	}
	else if (!strcasecmp(ln, "IPCREG"))
	{
		bfmt.ipc_reg=atoi(arg2);
	}
	else if (!strcasecmp(ln, "INITDOMAIN"))
	{
		bfmt.current_domain=atoi(arg2);
	}
	else if (!strcasecmp(ln, "INITUID"))
	{
		bfmt.domain_uid=atoi(arg2);
	}
	else if (!strcasecmp(ln, "DOMAINS"))
	{
		itmp=0;
		bzero(bfmt.domains, sizeof(bfmt.domains));
		
		tmp=strtok(arg2, " \t,");
		while(tmp && itmp < MAX_EXEC_DOMAINS)
		{
			bfmt.domains[itmp]=atoi(tmp);
			tmp=strtok(NULL, " \t,");
			itmp++;
		}
	}
	else
		Nerror("Unknown option");
}

void do_pragma(char *ln)
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
		if (!arg) Nerror("Define without arguments");
		name=strtok(arg, " \t\n\r");
		val=strtok(NULL, " \t\n\r");

		if (!name || !val) Nerror("Define requires two arguments");

		/* Add a definition! */
		d=malloc(sizeof(struct defines));
		d->name=strdup(name);
		d->value=strdup(val);
		d->isblue=0;
		d->val_len=strlen(val);
		d->nam_len=strlen(name);
		d->next=defined;
		defined=d;
	}
	else if (!strcasecmp(ln, "END")) /* What the?! */
	{
	}
	else
		Nerror("Unknown pragma");

}

/* Types of data declaration:
 * 1. Numbers; any kind.
 * 2. Strings. ""
 * 3. TODO: special types repeat and block.
 */

int rep_data_offs; /* Data offset as of line start, for repeat */

char *do_data(char *ln)
{
	char *next, *ret;

	/* Trim beginning - we're recursive after all */
	while (*ln && my_isspace(*ln))
		ln++;

	if(*ln==0)
		return NULL;


	/* Is it a string? */
	if(ln[0]=='"')
	{
		while (*(++ln)!='"')
		{
			if (!(*ln))
				Nerror("Unterminated string constant.");
			else if (*ln=='\\')
			{
				ln++;
				switch(*ln)
				{ /* There's a few I missed, I'm sure */
					case 'x':
					{ /* Shellcode in Argante? Cool! */
						static char nu[3]; /* Ick... like the only static buffer here */
						ln++;
						if (!(nu[0]=*ln)) Nerror("Bad \\x sequence.");
						ln++;
						if (!(nu[1]=*ln)) Nerror("Bad \\x sequence.");
						nu[2]=0;

						*ln=strtol(&nu[0], &ret, 0x10); /* Hex radix */
						if (ret && *ret) Nerror("Bad \\x sequence."); /* Trailing garbage? */
						break;
					}
					case 'n': *ln='\n'; break;
					case 'r': *ln='\r'; break;
					case 'e': *ln='\e'; break;
					case 'b': *ln='\b'; break;
					case 't': *ln='\t'; break;
					case 0: Nerror("Unterminated escape sequence.");
				}
			}
			fputc(*ln, out_data);
		}
		ln++;
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
	if (!strcasecmp(ln, "BLOCK"))
		return NULL; /* Abandon it. We are in block mode always. I think. */
	if (!strcasecmp(ln, "REPEAT"))
	{
		int sz, ct, z;
		/* Now we have multi-element data symbols, so the whole deal of what
		 * 'repeat' does is kinda thorny.
		 * 
		 * So, let's just say repeat repeats all data on the same line as
		 * repeat is given on. Ugly, yes; but also the most useful possibility.
		 */
	
		/* Get repct */
		ct=strtol(next, &ret, 0) - 1; /* We already wrote one block of data */
		if (ret==next) { Nwarn(next); Nerror("Garbage numeric"); }

		if (ct<0 || ct>MAX_ALLOC_MEMBLK)
			Nerror("Senseless repeat count");

		/* Get data to repeat */
		sz=ftello(out_data) - rep_data_offs; /* Amount of data to repeat */
		if (!sz) Nerror("No data given to repeat");
		
		next=malloc(sz);
		/* Read data... */
		fseek(out_data, rep_data_offs, SEEK_SET);
		if ((z=fread(next, sizeof(char), sz, out_data)) < sz)
			perror("fread");
		fseek(out_data, 0, SEEK_END);
		
		/* Ok, we recovered the data to repeat... repeat it */
		while(ct > 0)
		{
			fwrite(next, sizeof(char), sz, out_data);
			ct--;
		}
		
		free(next);
		return ret;
	}

	/* Now check it for a number. */
		
	/* I guess we just assume it's floating point if it contains . or ends in f */
	if(ln[strlen(ln)-1]=='f' || strchr(ln, '.'))
	{
		float f;
		if (sizeof(float) != sizeof(long)) Nerror("sizeof(float) is somewhat odd");
		f=strtod(ln, &ret);
		fwrite(&f, sizeof(float), 1, out_data);
	}
	else
	{
		int i;
		i=strtoul(ln, &ret, 0);
		fwrite(&i, sizeof(int), 1, out_data);
	}

	if (ret==ln) Nerror("Garbage numeric.");
	return next;
}

void arg_parse(char *ln, unsigned long *out, char *type, int offs)
{
	int i;
	char *z, *ret;
	symbol *s;
	
	z=ln+1;
	*type=TYPE_IMMEDIATE;
	
	/* While all immediates are integers, for some reason
	 * that doesn't stop us using floating immediates. Hmm. */
	/* 0xf is not a float... DOH! */
	if((z[strlen(z)-1]=='f' && strncmp(ln, "0x", 2)) || strchr(z, '.'))
	{
		float f;
		long *z;
		if (sizeof(float) != sizeof(long)) Nerror("sizeof(float) is somewhat odd");
		f=strtod(ln, &ret);
		z=(long *) &f;
		*out=*z;
		return;
	}

	switch(*ln) 
	{

		case '$': /* Syscall */
			/* TYPE_IMMEDIATE */
			for (i=0;i<SCNUM;i++)
			{
				if (!strcasecmp(z,sys[i].name))
				{
					*out=sys[i].num;
					return;
				}
			}
			Nerror("Unknown syscall");
			return;
		case '*': /* uptr */
			arg_parse(z, out, type, offs);
			if (*type==TYPE_UREG)
				*type=TYPE_UPTR;
			else if (*type==TYPE_IMMEDIATE)
				*type=TYPE_IMMPTR;
			else
				Nerror("Pointer requires immediate or ureg");
			return;
		case ':': /* Pointer */
			s=find_symbol_f(z);
			add_depend( REF_PTR, ftello(out_bcode) + offs, s);
			*out=s->location;
			return;
		case '^': /* char size */
			s=find_symbol_f(z);
			add_depend( REF_SIZE_CHAR, ftello(out_bcode) + offs, s);
			*out=s->size;
			return;
		case '%': /* dword size */
			s=find_symbol_f(z);
			add_depend( REF_SIZE_DWORD, ftello(out_bcode) + offs, s);
			*out=s->size / 4;
			return;
		case 'u': /* Cool, registers. */
			*type=TYPE_UREG;
			break;
		case 's':
			*type=TYPE_SREG;
			break;
		case 'f':
			*type=TYPE_FREG;
			break;
		default:
			z=ln;
	}
	
	 /* Convert an integer (this applies to the 15 in u15 too) */
	i=strtoul(z, &ret, 0);
	*out=i;
	printf("'%s' -> 0x%lx\n", z, *out);
	if (ret==z) Nerror("Garbage numeric.");
}

/*
 * DANGER: EARTHMOVING IN PROGRESS
 * Beware of flying debris.
 */
void do_code(char *ln)
{
	struct _bcode_op bop;
	char *arg1, *arg2, *tmp;
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
	bop.t1=bop.t2=0;
	/* Clear the struct properly. Oops. */
	bop.rsrvd=bop.a1=bop.a2=0;
	if (arg1) {
		ict=1;
		while (*arg1 && my_isspace(*arg1))
			arg1++;
		arg_parse(arg1, &bop.a1, &bop.t1, 4);
	} 
	if (arg2) {
		ict=2;
		while (*arg2 && my_isspace(*arg2))
			arg2++;
		arg_parse(arg2, &bop.a2, &bop.t2, 8);
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
				Nerror("Wrong number of parameters");
			}
			bop.bcode=op[itmp].bcode;
			break;
		}
	}
	if (itmp>=OPS) Nerror("Unknown mnemonic!");

	/* Check types. */
	if (ict > 0 && !((1 << bop.t1) & op[itmp].tparam1))
		Nerror("Type mismatch for argument 1");

	if (ict > 1 && !((1 << bop.t2) & op[itmp].tparam2))
		Nerror("Type mismatch for argument 2");

	/* Phew. */
	fwrite(&bop, sizeof(bop), 1, out_bcode);
}

int main(int argc, char *argv[])
{
	char *inl;
	size_t n;
	int z=1;
	char *ln, *tmp;
	
	if (argc < 3)
	{
		fprintf(stderr, 
			"NAGT - RSIS assembler. (C) 2001 James Kehl <ecks@optusnet.com.au>\n"
			"No warranty yada yada yada.\n\n");

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
	
	out_bcode=tmpfile(); //fopen("nag_bcode.tmp", "wb+");
	out_data=tmpfile(); //fopen("nag_data.tmp", "wb+");
	out_syms=tmpfile(); //fopen("nag_syms.tmp", "wb+");

	out_pure=fopen(argv[z], "wb");

	if (!out_syms) {
		perror("error opening syms file");
		if (linking) exit(-1);
	}
	
	if (!out_bcode || !out_data || !out_pure) { perror("fopen(w)"); return 1; }

	context_head=NULL;
	begin_context();
	
	bfmt.magic1=BFMT_MAGIC1;
	bfmt.magic2=BFMT_MAGIC2;
	bfmt.memflags=MEM_FLAG_READ | MEM_FLAG_WRITE;
	 
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

		run_define(&inl, &n);
		/* Strip off leading whitespace. */
		tmp=ln=inl;
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

		rep_data_offs=ftello(out_data);
	
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
				Nerror("Global context undefined: you don't want to do that");
		} else {
			if (state==STATE_DATA)
			{
				tmp=ln;
				while(tmp) tmp=do_data(tmp);
			}
			else if (state==STATE_CODE)
				do_code(ln);
			else
				Nerror("Command not understood in this state");
		}
	}

	define_symbol();
	end_context();

	/* Finished, at last. Merge files into one. */
	fseek(out_bcode, 0, SEEK_END); /* This also flushes the stream */
	fseek(out_data, 0, SEEK_END);
	/* Calculate data and code sizes */
	bfmt.datasize=ftello(out_data) / 4;
	bfmt.bytesize=ftello(out_bcode) / sizeof(struct _bcode_op);
	fprintf(stderr, "<-> %d data packets, %d bytecode packets\n", bfmt.datasize, bfmt.bytesize);
	
	/* Write header out */
	fwrite(&bfmt, sizeof(bfmt), 1, out_pure);
	
	/* The rest */

	/* Now we open files w+, so just seek to start and get reading */
	fseek(out_bcode, 0, SEEK_SET);
	fseek(out_data, 0, SEEK_SET);
	fseek(out_syms, 0, SEEK_SET);
	
	/* Stick reloc table at EOF. Argante complains but will
	 * still run the image. */

	while (fread(&z, sizeof(z), 1, out_bcode) > 0) 
		fwrite(&z, sizeof(z), 1, out_pure);
	while (fread(&z, sizeof(z), 1, out_data) > 0) 
		fwrite(&z, sizeof(z), 1, out_pure);
	while (fread(&z, 1, 1, out_syms) > 0) /* syms are not always 4-align */
		fwrite(&z, 1, 1, out_pure);
	
	return 0;
}
