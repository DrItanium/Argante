/*
 * The New ArGante Compiler (NAG)
 * James Kehl <ecks@optusnet.com.au>
 * 
 * Use under GPL; just like GCC. I have no claim
 * over the images you produce with this.
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
#include "language.h"

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

struct defines 		*defined = NULL;


/*
 * Syntax:
 * exactly like the old one(!)
 * 
 * Extensions:
 * !VERSION 2 enables NAG extensions; not that there are any! yet!
 *
 * { } opens/closes context bracket.
 */

#define REF_PTR 1
#define REF_SIZE_CHAR 2
#define REF_SIZE_DWORD 3

typedef struct _depend depend;
struct _depend {
	char reftype;
	char stortype;
	off_t location;
	depend *next;
};

typedef struct _symbol symbol;
struct _symbol {
	char name[62];
	char type;
	char stortype;
	char defined;
	int location;
	int size;
	depend *dep;
	symbol *next;
};

typedef struct _context context;
struct _context {
	symbol *syms;
	context *prev;
};

context *context_head;
symbol *curr_symbol=NULL;
context *curr_symbol_context;

FILE *in, *out_bcode, *out_data, *out_pure, *out_syms;

#define STATE_OPTION 0
#define STATE_DATA 1
#define STATE_CODE 2

struct bformat bfmt;
int state=0;
int lineno=0;
int version=1;

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

void end_context()
{
	symbol *s, *d;
	context *x;
	int count;
	s=context_head->syms;
	count=0;
	while(s)
	{
		d=s->next;
		if (!s->defined)
		{
			fprintf(stderr, "<+> Symbol %s undefined\n", s->name);
			count++;
		} else
			free(s);
		s=d;
	}

	if (count)
	{
		fprintf(stderr, "<!> Line %d: %d symbols are undefined at context termination\n", lineno, count);
		exit(1);
	}

	x=context_head->prev;
	free(context_head);
	context_head=x;
}

/* BlEh! Defines. A Hack... */
int subst_define(char *in, char *s1, char *s2, char **ret)
{
	char *t, *l;
	t=strstr(in, s1);
	if (!t) return 0;

	*ret=malloc(strlen(in) - strlen(s1) + strlen(s2) + 1);
	memcpy(*ret, in, t-in);
	l=*ret+(t-in);
	memcpy(l, s2, strlen(s2));
	l+=strlen(s2);
	memcpy(l, t+strlen(s1), strlen(t) - strlen(s1));

	return 1;
}

char *run_define(char *in)
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

	if (to->defined) return;
	new=malloc(sizeof(new));
	if (!new) error("Malloc failure (out of memory!)");
	
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

	if (strlen(symname) >= sizeof(d->name)) warn("Symbol name too long, clipping enforced.");
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
	if ((curr_symbol->stortype==STATE_CODE))
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
		if (curr_symbol->stortype==STATE_DATA)
			curr_symbol->location/=4;

		curr_symbol->defined=1;
	//	fprintf(stderr, "<-> defining %s @ %d, %d long...\n", curr_symbol->name, curr_symbol->location, curr_symbol->size);
		/* Hunt for refs. */
		d=curr_symbol->dep;
		while (d)
		{
			int i;
			FILE *f;
	//		fprintf(stderr, "<-> ... %s adjusting %d %d %d\n", curr_symbol->name, d->location, d->stortype, d->reftype);
			n=d->next;
			f=(d->stortype==STATE_CODE) ? out_bcode : out_data;
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
					error("Unknown reftype! (internal error)");
			}
			fwrite(&i, sizeof(int), 1, f);
			fseek(f, 0, SEEK_END);

			/* Free deps & clear chain */
			free(d);
			d=n;
		}
		curr_symbol->dep=NULL;

		/* Now write name->address symbol table if requested */
		if (out_syms)
		{
			/* NAME (dynamic len), ADDRESS, SIZE, TYPE */
			fwrite(&curr_symbol->name, strlen(curr_symbol->name) + 1, 1, out_syms);
			fwrite(&curr_symbol->location, sizeof(curr_symbol->location), 1, out_syms);
			fwrite(&curr_symbol->size, sizeof(curr_symbol->size), 1, out_syms);
			fwrite(&curr_symbol->stortype, sizeof(curr_symbol->stortype), 1, out_syms);
		}
	}
	curr_symbol=NULL;
}

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
		curr_symbol->dep=NULL;
		curr_symbol->stortype=state;
		strncpy(curr_symbol->name, ln, sizeof(curr_symbol->name));
	}
/*	else
		warn("Prototype ref found here"); */

	if (strlen(ln) >= sizeof(curr_symbol->name)) warn("Symbol name too long, clipping enforced.");
	curr_symbol->name[sizeof(curr_symbol->name) - 1]=0; /* It's easier if it's null-terminated */

	curr_symbol->defined=0;
	curr_symbol->location=ftello((state==STATE_CODE) ? out_bcode : out_data)
		/ ((state==STATE_CODE) ? 12 : 1);
}

void do_option(char *ln)
{
	char *tmp, *arg2;
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
	else if (!strcasecmp(ln, "VERSION"))
	{
		version=atoi(arg2);
		warn("version is currently a no-op");
	}
	else
		error("Unknown option");
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
				error("Unterminated string constant.");
			else if (*ln=='\\')
			{
				ln++;
				switch(*ln)
				{ /* There's a few I missed, I'm sure */
					case 'n': *ln='\n'; break;
					case 'r': *ln='\r'; break;
					case 'e': *ln='\e'; break;
					case 'b': *ln='\b'; break;
					case 't': *ln='\t'; break;
					case 0: error("Unterminated escape sequence.");
				}
			}
			fputc(*ln, out_data);
		}
		ln++;
		/* Recurse; we may have "stuff" 0x81 "stuff" to deal with */
//		puts(ln); // XXX DEBUG
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
		FILE *ft;
		/* Now we have multi-element data symbols, so the whole deal of what
		 * 'repeat' does is kinda thorny.
		 * 
		 * So, let's just say repeat repeats all data on the same line as
		 * repeat is given on. Ugly, yes; but also the most useful possibility.
		 */
	
		/* Get repct */
		ct=strtol(next, &ret, 0) - 1; /* We already wrote one block of data */
		if (ret==next) { warn(next); error("Garbage numeric"); }

		if (ct<0 || ct>MAX_ALLOC_MEMBLK)
			error("Senseless repeat count");

		/* Get data to repeat */
		sz=ftello(out_data) - rep_data_offs; /* Amount of data to repeat */
		if (!sz) error("No data given to repeat");
		
		next=malloc(sz);
		/* Read data... */
		ft=fopen("nag_data.tmp", "rb");
		if (!ft) { perror("fopen(r)"); exit(1); }
		fseek(ft, rep_data_offs, SEEK_SET);
		if ((z=fread(next, sizeof(char), sz, ft)) < sz)
			perror("fread");
		fclose(ft);
		
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
		if (sizeof(float) != sizeof(long)) error("sizeof(float) is somewhat odd");
		f=strtod(ln, &ret);
		fwrite(&f, sizeof(float), 1, out_data);
	}
	else
	{
		int i;
		i=strtol(ln, &ret, 0);
		fwrite(&i, sizeof(int), 1, out_data);
	}

	if (ret==ln) error("Garbage numeric.");
	return next;
}

void arg_parse(char *ln, long *out, char *type, int offs)
{
	int i;
	char *z, *ret;
	symbol *s;
	
	z=ln+1;
	*type=TYPE_IMMEDIATE;
	
	/* While all immediates are integers, for some reason
	 * that doesn't stop us using floating immediates. Hmm. */
	if(z[strlen(z)-1]=='f' || strchr(z, '.'))
	{
		float f;
		long *z;
		if (sizeof(float) != sizeof(long)) error("sizeof(float) is somewhat odd");
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
			error("Unknown syscall");
			return;
		case '*': /* uptr */
			arg_parse(z, out, type, offs);
			if (*type==TYPE_UREG)
				*type=TYPE_UPTR;
			else if (*type==TYPE_IMMEDIATE)
				*type=TYPE_IMMPTR;
			else
				error("Pointer requires immediate or ureg");
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
	i=strtol(z, &ret, 0);
	*out=i;
	if (ret==z) error("Garbage numeric.");
}

/* Stuff from task.c */
struct _bcode_op {
	char bcode;
	char t1;
	char t2;
	char rsrvd;
	long a1;
	long a2;
};

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

//	fprintf(stderr, "-%s-%s-%s-\n", ln, arg1, arg2);
	/* Ok, we now have mnem in ln, and the args. */
	/* Try and comprehend the args. */
	ict=0;
	bop.t1=bop.t2=-1;
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
				error("Wrong number of parameters");
			}
			bop.bcode=op[itmp].bcode;
			break;
		}
	}
	if (itmp>=OPS) error("Unknown mnemonic!");

	/* Check types. Ick; this should have been a bytearray. */
	if (
	( bop.t1==TYPE_IMMEDIATE && !op[itmp].imm1) ||
	( bop.t2==TYPE_IMMEDIATE && !op[itmp].imm2))
		error("Type mismatch with immediate.");

	if (
	( bop.t1==TYPE_UREG && !op[itmp].ureg1) ||
	( bop.t2==TYPE_UREG && !op[itmp].ureg2))
		error("Type mismatch with ureg.");

	if (
	( bop.t1==TYPE_SREG && !op[itmp].sreg1) ||
	( bop.t2==TYPE_SREG && !op[itmp].sreg2))
		error("Type mismatch with sreg.");

	if (
	( bop.t1==TYPE_FREG && !op[itmp].freg1) ||
	( bop.t2==TYPE_FREG && !op[itmp].freg2))
		error("Type mismatch with freg.");

	if (
	( bop.t1==TYPE_IMMPTR && !op[itmp].immptr1) ||
	( bop.t2==TYPE_IMMPTR && !op[itmp].immptr2))
		error("Type mismatch with immptr.");

	if (
	( bop.t1==TYPE_UPTR && !op[itmp].uptr1) ||
	( bop.t2==TYPE_UPTR && !op[itmp].uptr2))
		error("Type mismatch with uptr.");

	/* It seems that the Kernel does not like invalid
	 * types even for unused parms. Sigh. */
	if (bop.t1==-1) bop.t1=0;
	if (bop.t2==-1) bop.t2=0;

	/* Phew. */
	fwrite(&bop, sizeof(bop), 1, out_bcode);
}

int main(int argc, char *argv[])
{
	char *inl, *o;
	size_t n;
	int z;
	char *ln, *tmp;
	
	if (argc < 3)
	{
		fprintf(stderr, "Usage: nag filename.in filename.out [filename.sym]\n");
		exit(1);
	}

	in=fopen(argv[1], "r");
	if (!in) { perror("fopen(r)"); return 1; }
	out_bcode=fopen("nag_bcode.tmp", "wb");
	out_data=fopen("nag_data.tmp", "wb");
	out_pure=fopen(argv[2], "wb");
	
	if (argc == 4) 
	{
		out_syms=fopen(argv[3], "wb");
		if (!out_syms) perror("error opening syms file");
	} else out_syms=NULL;
	
	if (!out_bcode || !out_data || !out_pure) { perror("fopen(w)"); return 1; }

	context_head=calloc(sizeof(context),1);
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
		if (tmp==ln) continue; /* Eh? Blank line? */

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
		} else {
			if (state==STATE_DATA)
			{
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

	/* Just in case */
	define_symbol();
	end_context();

	/* Finished, at last. Merge files into one. */
	fseek(out_bcode, 0, SEEK_END);
	fseek(out_data, 0, SEEK_END);
	/* Calculate data and code sizes */
	bfmt.datasize=ftello(out_data) / 4;
	bfmt.bytesize=ftello(out_bcode) / sizeof(struct _bcode_op);
	fprintf(stderr, "<-> %d data packets, %d bytecode packets\n", bfmt.datasize, bfmt.bytesize);
	
	/* Write header out */
	fwrite(&bfmt, sizeof(bfmt), 1, out_pure);
	
	/* The rest */
	fclose(out_bcode); 
	fclose(out_data); 
	out_bcode=fopen("nag_bcode.tmp", "rb");
	out_data=fopen("nag_data.tmp", "rb");
	if (!out_bcode || !out_data) { perror("fopen(r)"); return 1; }

	while (fread(&z, sizeof(z), 1, out_bcode) > 0) 
		fwrite(&z, sizeof(z), 1, out_pure);
	while (fread(&z, sizeof(z), 1, out_data) > 0) 
		fwrite(&z, sizeof(z), 1, out_pure);
	
	return 0;
}
