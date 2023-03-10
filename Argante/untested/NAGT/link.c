/*
 * NAGT static linker.
 * James Kehl <ecks@optusnet.com.au>
 * 
 * Use under GPL; just like GCC. I have no claim
 * over the images you produce with this.
 * 
 * Inputs:
 * one or more relocatable Argante images (a.k.a object files).
 * Entry point must be within first image.
 * 
 * Output:
 * Single relocatable Argante image.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "bformat.h"

#include "symtab.h"
#include "bcode_op.h"

symbol *sym_top=NULL;
struct bformat header;

struct _bcode_op *bytecode;
int *data;
int this_bsec=0;
int this_dsec=0;

symbol *find_symbol(char *symname)
{
	symbol *x;
	
	x=sym_top;
	while (x)
	{
		if (!(x->stortype & SYM_NO_CLASH) && !strncmp(x->name, symname, sizeof(x->name)))
			return x;
		x=x->next;
	}
	return NULL;
}

/* Deps have char offsets within the file.
 * Syms have offsets integral with the type */
int adjust_sym_location(int location, char type)
{
	if (type & STATE_DATA)
		return location + this_dsec;
	else if (type & STATE_CODE)
		return location + this_bsec;
	
	fprintf(stderr, "<+> Unknown symbol type (%d)... bad image?\n", type);
	return location;
}

int adjust_dep_location(int location, char type)
{
	if (type & STATE_DATA)
		return location + this_dsec * sizeof(int);
	else if (type & STATE_CODE)
		return location + this_bsec * sizeof(struct _bcode_op);
	
	fprintf(stderr, "<+> Unknown dep type (%d)... bad image?\n", type);
	return location;
}

void read_syms(FILE *sym)
{
	int ct, ci, n;
	symbol sym_tmp, *s;
	depend dep_tmp, *d;

	ct=0;
	while(1)
	{
		n=0;
		
		while ((ci=fgetc(sym)) > 0)
		{
			if (n < (sizeof(sym_tmp.name) - 1))
				sym_tmp.name[n]=ci;
			else
			{
				fprintf(stderr, "<!> Overlong symbol name (corrupted image or new compiler). Aborting\n");
				exit(-1);
			}
			n++;
		}
		sym_tmp.name[n]=0;

		/* Read no name? No more symbols, then! */
		if (n==0)
		{
			fprintf(stderr, "<-> %d symbols read from table\n", ct);
			break;
		}
		ct++;

		/* Read in symbol specs */
		if (
			fread(&sym_tmp.location, sizeof(sym_tmp.location), 1, sym) != 1 ||
			fread(&sym_tmp.size, sizeof(sym_tmp.size), 1, sym) != 1 ||
			fread(&sym_tmp.stortype, sizeof(sym_tmp.stortype), 1, sym) != 1
		)
		{
			fprintf(stderr, "<+> Couldn't get stats for %s", sym_tmp.name);
			break;
		}


		sym_tmp.defined=((sym_tmp.size!=0) ? 1 : 0);
		/* Try and find an existing symbol like this.
		 * If we find one, amalgamate them if possible or abort if not.
		 * If it's new, duplicate sym_tmp.
		 */

		if (!(sym_tmp.stortype & SYM_NO_CLASH)) {
			s=find_symbol(sym_tmp.name);
		} else {
			if (!sym_tmp.defined)
			{
				fprintf(stderr, "<!> Symbol %s undefined and undefinable. Image corrupted, aborting.\n", sym_tmp.name);
				exit(-1);
			}

			s=NULL;
		}

		if (!s) /* Phew, new one. */
		{
			s=malloc(sizeof(sym_tmp));
			memcpy(s, &sym_tmp, sizeof(sym_tmp));
		
			if (s->defined) s->location=adjust_sym_location(s->location, s->stortype);

			s->dep=NULL;
			s->next=sym_top;
			sym_top=s;
		} else {
			if (sym_tmp.defined)
			{ /* Merge or conflict? */
				if (s->defined)
				{ /* Conflict */
					fprintf(stderr, "<!> Symbol %s doubly defined. Aborting\n", s->name);
					exit(-1);
				}
				/* Merge */
				s->location=adjust_sym_location(sym_tmp.location, sym_tmp.stortype);
				s->size=sym_tmp.size;
				s->defined=sym_tmp.defined;
			} /* else both undefined, no problems */
		}

		/* Now append dep-chain to chained symbol */
		while (1)
		{
			if (
				fread(&dep_tmp.reftype, sizeof(dep_tmp.reftype), 1, sym) != 1 ||
				fread(&dep_tmp.stortype, sizeof(dep_tmp.stortype), 1, sym) != 1 ||
				fread(&dep_tmp.location, sizeof(dep_tmp.location), 1, sym) != 1
			)
			{
				fprintf(stderr, "<+> Couldn't get dependencies for %s\n", s->name);
				break;
			}
			/* -1 to break chain */
			if (dep_tmp.reftype < 0) break;
			/* Relocate */
			dep_tmp.location=adjust_dep_location(dep_tmp.location, dep_tmp.stortype);
			
			/* Got a valid one, so add it! */
			d=malloc(sizeof(dep_tmp));
			memcpy(d, &dep_tmp, sizeof(dep_tmp));
			d->next=s->dep;
			s->dep=d;
		}
	}
}

int main(int argc, char **argv)
{
	struct bformat bfmt;
	symbol *s;
	depend *r;
	int i, l;
	FILE *in;
	FILE *out;
	const int n1=-1;
	const char c1=-1;

	if (argc == 1)
	{
		fprintf(stderr,
			"Link - Argante static linker. (C) 2001 James Kehl <ecks@optusnet.com.au>\n"
			"No warranty yada yada yada.\n\n");

		fprintf(stderr, "usage: %s <main.img> [2.img] [3.img] ...\nProduces lout.img\n", argv[0]);
		exit(-1);
	}
	
	fprintf(stderr, "<-> Producing lout.img\n"); 

	out=fopen("lout.img", "wb");
	if (!out)
	{
		perror("<!> fopen");
		fprintf(stderr, "<!> Couldn't open output file. Aborting\n");
	}


	for(i=1;i<argc;i++)
	{
		in=fopen(argv[i], "rb");
		
		/* Some basic sanity checks */
		if(fread(&bfmt, sizeof(bfmt), 1, in) != 1)
		{
			perror("<!> fread");
			fprintf(stderr, "<!> %s has no header. Aborting\n", argv[i]); 
			exit(-1);
		}
		
		if (bfmt.magic1 != BFMT_MAGIC1 || bfmt.magic2 != BFMT_MAGIC2)
		{
			fprintf(stderr, "<!> %s is corrupted or not native to this platform. Aborting\n", argv[i]);
			exit(-1);
		}

		/* Adjust our compound image */
		if (i==1)
		{
			memcpy(&header, &bfmt, sizeof(bfmt));
			data=malloc(header.datasize * sizeof(int));
			bytecode=malloc(header.bytesize * sizeof(struct _bcode_op));
		}
		else
		{
			this_dsec=header.datasize;
			this_bsec=header.bytesize;

			header.datasize+=bfmt.datasize;
			header.bytesize+=bfmt.bytesize;
			
			data=realloc(data, header.datasize * sizeof(int));
			bytecode=realloc(bytecode, header.bytesize * sizeof(struct _bcode_op));
		}

		/* Just check if malloc worked... */
		/* It's forgivable to have an empty section for the first file */
		if ((!data && header.datasize > 0) || (!bytecode && header.bytesize > 0))
		{
			perror("<!> malloc");
			fprintf(stderr, "<!> Malloc/realloc failed. Either %s or your memory is broken.\n", argv[i]);
			exit(-1);
		}
		/* Load in the data + bytecode */
		l=fread(&bytecode[this_bsec], sizeof(struct _bcode_op), bfmt.bytesize, in);
		if(l != bfmt.bytesize)
		{
			perror("<!> fread");
			fprintf(stderr, "<+> Could only read %d of %d bytecode packets from %s.\n", l, bfmt.bytesize, argv[i]);
		}
		
		l=fread(&data[this_dsec], sizeof(int), bfmt.datasize, in);
		if(l != bfmt.datasize)
		{
			perror("<!> fread");
			fprintf(stderr, "<+> Could only read %d of %d data packets from %s.\n", l, bfmt.datasize, argv[i]);
		}

		/* Load in reloc table. Ick! */
		read_syms(in);

		fclose(in);
	}

	/* Adjust image using reloc symbols. */
	s=sym_top;
	while (s)
	{
		if (!s->defined)
		{
			fprintf(stderr, "<!> %s: unresolved symbol\n", s->name);
			exit(-1);
		}
		r=s->dep;
		while (r)
		{
			switch(r->reftype)
			{
				case REF_PTR:
					i=s->location;
					break;
				case REF_SIZE_CHAR:
					i=s->size;
					break;
				case REF_SIZE_DWORD:
					i=s->size / 4;
					break;
			}
			
			/* Constructs like this almost make me hate C */
			if (r->stortype & STATE_DATA) /* Be comprehensive */
				*((int *) &(((char *) data)[r->location]))=i;
			else if (r->stortype & STATE_CODE)
				*((int *) &(((char *) bytecode)[r->location]))=i;
			else fprintf(stderr, "<+> Unknown storage type for symbol dependency.\n");
			
			r=r->next;
		}
		s=s->next;
	}

#define xfwrite(a, b, c, d) if (fwrite(a,b,c,d) != c) goto writerror
	
	/* Write compound image */
	xfwrite(&header, sizeof(header), 1, out);
	xfwrite(bytecode, sizeof(struct _bcode_op), header.bytesize, out);
	xfwrite(data, sizeof(int), header.datasize, out);
	/* Write reloc symbols. Recursive linking, anyone? */
	s=sym_top;
	while (s)
	{
		xfwrite(&s->name, strlen(s->name) + 1, 1, out);
		xfwrite(&s->location, sizeof(s->location), 1, out);
		xfwrite(&s->size, sizeof(s->size), 1, out);
		xfwrite(&s->stortype, sizeof(s->stortype), 1, out);
		r=s->dep;
		while (r)
		{
			xfwrite(&r->reftype, sizeof(r->reftype), 1, out);
			xfwrite(&r->stortype, sizeof(r->stortype), 1, out);
			xfwrite(&r->location, sizeof(r->location), 1, out);
			r=r->next;
		}
		/* -1 for reftype terminates the chain. */ 
		xfwrite(&c1, sizeof(r->reftype), 1, out);
		xfwrite(&c1, sizeof(r->stortype), 1, out);
		xfwrite(&n1, sizeof(r->location), 1, out);
		
		s=s->next;
	}

	fprintf(stderr, "<-> Linking complete.\n");
	return 0;

writerror:
	perror("<!> fwrite");
	fprintf(stderr, "<!> Production of output image failed. \n");
	return -1;
}

