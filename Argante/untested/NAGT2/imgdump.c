/*
 * "imgdump" - AOSr2 Image Disassembler
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

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

#include "nagt.h"

FILE *in;
symbol *code_symbol, *data_symbol, *curr_symbol;

int disassemble_code;
int disassemble_data;
int revert_symbols;

void section_dump();
void disassemble();
void dump_code(struct _bcode_op *arg, unsigned offs);
void dump_data(struct data_pkt *arg, char *last_type, unsigned offs);

/* Print d in hex or decimal, whichever is better. */
void represent_int(FILE *to, int d)
{
	int old_d=d;
	int hex_moves=0, dec_moves=0;
	
	if (d) {
	while(!(d & 15))
	{
		d>>=2;
		hex_moves++;
	}
	d=old_d;
	while(!(d % 10))
	{
		d/=10;
		dec_moves++;
	}
	}
	if (dec_moves >= hex_moves)
		fprintf(to, "%d", old_d);
	else
		fprintf(to, "0x%x", old_d);
}

symbol *find_symbol(symbol *head, unsigned int addr)
{
	symbol *x;
	x=head;
	while (x)
	{
		if (!(x->s.place & SYM_UNDEFINED) && x->s.addr == addr) return x;
		x=x->next;
	}
	return NULL;
}

#define find_symbol_data(a) find_symbol(data_symbol, a)
#define find_symbol_code(a) find_symbol(code_symbol, a)

/* Inefficient as 'ss... would you rather keep 2 copies? */
symbol *find_reloc_in(symbol *head, unsigned int place, unsigned int addr, areloc **ret)
{
	symbol *x;
	areloc *r;
	x=head;
	while (x)
	{
		r=x->reloc;
		while (r)
		{
			if (r->r.place == place && r->r.addr == addr)
			{
				*ret=r;
				return x;
			}
			r=r->next;
		}
		x=x->next;
	}
	return NULL;
}

symbol *find_reloc(unsigned int place, unsigned addr, areloc **ret)
{
	symbol *s;
	s=find_reloc_in(data_symbol, place, addr, ret);
	if (s) return s;
	s=find_reloc_in(code_symbol, place, addr, ret);
	return s;
}

void represent_reloc(FILE *to, symbol *s, areloc *r)
{
	switch(r->r.type)
	{
		case RELOC_ADDR:
			fputc(':', to); break;
		case RELOC_SIZE_DWORD:
			fputc('%', to); break;
		case RELOC_SIZE_BYTE:
			fputc('^', to); break;
	}
	printf("%s", s->s.fn);
}

void usage() {
		fprintf(stderr, 
		"imgdump - AOSr2 image disassembler.\nCopyright (C) 2001 James Kehl <ecks@optusnet.com.au>\n"
		"This comes with ABSOLUTELY NO WARRANTY; but this is free software, and you are\n"
		"welcome to redistribute it under certain conditions. See COPYING for details.\n\n");

		fprintf(stderr, "Usage: imgdump <-s|-D> filename.in\n"
				"Options:\n"
				"-s Section dump (no disasm)       -S Section dump (with disasm)\n"
				"-d .agt disassembly               -D .agt disassembly (symbolic)\n");
		exit(1);
}

int main(int argc, char *argv[])
{
	if (argc < 3) usage();
	
	/* Open file */
	in=fopen(argv[2], "rb");
	if (!in) {
		perror("fopen:");
		exit(1);
	}
	/* Check flag */
	if (*argv[1] != '-') usage();

	disassemble_code=disassemble_data=revert_symbols=0;

	switch(argv[1][1])
	{
		case 's':
			section_dump();
			break;
		case 'S':
			disassemble_code=disassemble_data=revert_symbols=1;
			section_dump();
			break;
		case 'd':
			disassemble_code=disassemble_data=1;
			disassemble();
			break;
		case 'D':
			disassemble_code=disassemble_data=revert_symbols=1;
			disassemble();
			break;
		default: usage();
	}
	return 0;
}

void dump_codearg(int type, anyval *a1, int offs)
{
	char isreg, ispoint;
	symbol *s;
	areloc *r;

	ispoint=type & TYPE_POINTER;
	isreg=type & TYPE_REGISTER;
	if (ispoint) printf("*");
	switch(type & TYPE_VALMASK)
	{
		case TYPE_UNSIGNED: printf("u"); break;
		case TYPE_SIGNED: printf("s"); break;
		case TYPE_FLOAT: printf("f"); break;
		default: printf("?"); break;
	}
	printf(":");
	if (isreg)
		printf("r%d", a1->val.u);
	else if (revert_symbols && (s=find_reloc(SYM_CODE, offs, &r)))
		represent_reloc(stdout, s, r);
	else if (ispoint)
		represent_int(stdout, a1->val.u);
	else switch(type & TYPE_VALMASK)
	{
		case TYPE_SIGNED: represent_int(stdout, a1->val.s); break;
		case TYPE_FLOAT: printf("%g", a1->val.f); break;
		case TYPE_UNSIGNED:
		default:
				 represent_int(stdout, a1->val.u);
	}
}	


void dump_code(struct _bcode_op *arg, unsigned offs)
{
	int i;
	int args;

	if (!disassemble_code) return;

	if (arg->bcode>=OPS)
	{
		printf("\t???");
		args=2;
	}
	else
	{
		printf("\t%s", op[(int) arg->bcode].name);
		args=op[(int) arg->bcode].params;
	}

	if (revert_symbols && !strcasecmp(op[(int) arg->bcode].name, "SYSCALL"))
	{
		for (i=0;i<SCNUM;i++)
		{
			if (sys[i].num == arg->a1.val.u)
			{
				printf("\t$%s\n", sys[i].name);
				return;
			}
		}
	}

	if (args < 1) { printf("\n"); return; }
	printf("\t");
	dump_codearg(arg->type, &arg->a1, offs * sizeof(struct _bcode_op) + OFFSET_OP_A1);

	if (args < 2) { printf("\n"); return; }
	printf(", ");
	dump_codearg(arg->type >> 4, &arg->a2, offs * sizeof(struct _bcode_op) + OFFSET_OP_A2);
	printf("\n"); 
}

void dump_data(struct data_pkt *arg, char *last_type, unsigned offs)
{
	int i;

	if (!disassemble_data) return;

	/* Hey, so what if current NAGT doesn't allow :^% in data? I might fix it! */
	if (revert_symbols)
	{
		symbol *s;
		areloc *r;
		s=find_reloc(SYM_DATA, offs, &r);
		if (s)
		{
			switch(arg->type) {
				case DTYPE_SIGNED: printf("s:");
				case DTYPE_FLOAT: printf("f:");
				case DTYPE_STRING: printf("t:");
				case DTYPE_UNSIGNED: printf("u:");
			}
			represent_reloc(stdout, s, r);
			return;
		}
	}
	
	if (*last_type != DTYPE_STRING)
		printf("\t");
	else if (*last_type == DTYPE_STRING && arg->type != DTYPE_STRING)
		printf("\"\n");
	switch(arg->type)
	{
		case DTYPE_SIGNED:
			printf("s:");
			represent_int(stdout, arg->u.ds_int);
			break;
		case DTYPE_FLOAT:
			printf("%g", arg->u.df_float);
			break;
		case DTYPE_STRING:
			if (*last_type!=DTYPE_STRING)
				printf("\"");
			for (i=0;i<4;i++)
			{
				if (isprint(arg->u.dt_string[i]) && arg->u.dt_string[i]!='\"' && arg->u.dt_string[i]!='\'')
					fputc(arg->u.dt_string[i], stdout);
				else {
					fputc('\\', stdout);
					switch(arg->u.dt_string[i]) {
						case '\n': fputc('n', stdout); break;
						case '\r': fputc('r', stdout); break;
						case '\e': fputc('e', stdout); break;
						case '\b': fputc('b', stdout); break;
						case '\t': fputc('t', stdout); break;
						case '\"': fputc('\"', stdout); break;
						case '\'': fputc('\'', stdout); break;
						default: printf("x%2.2x", arg->u.dt_string[i]);
					}
				}
			}
			break;
		case DTYPE_UNSIGNED:
		default:
			printf("u:");
			represent_int(stdout, arg->u.ds_int);
	}
	*last_type = arg->type;
	if (arg->type != DTYPE_STRING)
		printf("\n");
}

#define SKIP while(mess_desc.size > 0) { fgetc(in); mess_desc.size--; } break
#define frread(a, b, c, d) if (!fread(a, b, c, d)) { perror("fread"); return; }

void section_dump(int with_disasm)
{
	int eof=-1;
	struct message_desc mess_desc;
	struct stable sym;
	int i;

	while(fread(&mess_desc, sizeof(mess_desc), 1, in))
	{
		if (eof < 0) {
			if (mess_desc.type != BFMT_PROGSPEC)
				fprintf(stderr, "<!> Image does not start with progspec\n");
			eof=0;
		}
		else if (eof) fprintf(stderr, "<!> Overterminated image (EOF tag with data afterward)\n");
		
		switch(mess_desc.type)
		{
			case BFMT_PROGSPEC: {
				struct progspec pspec;
				int i;
				printf("TYPE: %16.16s, SIZE: 0x%x\n", "BFMT_PROGSPEC", mess_desc.size);
				if (sizeof(pspec) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					SKIP;
				}
				frread(&pspec, mess_desc.size, 1, in);

				printf("\tMagic ID:0x%x\n\tSignature: %32.32s\n\tPriority: %d\n", pspec.magic,
						pspec.name, pspec.priority);
				printf("\tInitial domain: 0x%x\n\tDomains: ", pspec.init_domain);
				for(i=0;i<MAX_DOMAINS;i++) {
					if (pspec.domains[i])
					{
						if (i) printf(", %d", pspec.domains[i]);
						else printf("%d", pspec.domains[i]);
					} else {
						if (!i) printf("none");
						break;
					}
				}
				printf("\n");
	
				break; }
			case BFMT_CODE: {
				struct _bcode_op bop;
				/* Note that for BFMT_CODE, we count mess_desc.size in
				 * sizeof(bop)'s not bytes. Well... I hate myself :)
				 */
				printf("TYPE: %16.16s, SIZE: 0x%x\n", "BFMT_CODE", mess_desc.size);
				i=0;
				while (i < mess_desc.size)
				{
					frread(&bop, sizeof(bop), 1, in);
					dump_code(&bop, i);
					i++;
				}
				break; }
			case BFMT_DATA:
			case BFMT_RODATA: {
				struct data_pkt pkt;
				char last_type=-1;
				/* And for BFMT_DATA, the same thing applies; mess_desc.size
				 * is not in bytes. Worth changing? */
				printf("TYPE: %16.16s, SIZE: 0x%x\n", (mess_desc.type == BFMT_DATA) ?
					"BFMT_DATA" : "BFMT_RODATA", mess_desc.size);

				i=0;
				while (i < mess_desc.size)
				{
					frread(&pkt, sizeof(pkt), 1, in);
					dump_data(&pkt, &last_type, i);
					i++;
				}
				if (last_type == DTYPE_STRING) printf("\"\n");
				break; }
			case BFMT_SYM:
				printf("TYPE: %16.16s, SIZE: 0x%x\n", "BFMT_SYM", mess_desc.size);
				if (sizeof(sym) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					SKIP;
				}
				frread(&sym, mess_desc.size, 1, in);
				printf("\tSymbol name: %48.48s\n\tPlaceflags: %s%s%s\n\tAddress: 0x%x\n\tSize: 0x%x\n",
						sym.fn, ((sym.place & SYM_PLACEMASK) == SYM_CODE) ? "SYM_CODE" : "SYM_DATA",
						(sym.place & SYM_UNDEFINED) ? " SYM_UNDEFINED" : "",
						(sym.place & SYM_UNNAMED) ? " SYM_UNNAMED" : "",
						sym.addr, sym.size);

				break;
			case BFMT_RELOC: {
				struct reloc r;
				printf("TYPE: %16.16s, SIZE: 0x%x\n", "BFMT_RELOC", mess_desc.size);
				if (sizeof(r) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					SKIP;
				}
				frread(&r, mess_desc.size, 1, in);

				printf("\tPlace: %s\n\tAddress: 0x%x\n\tReftype: ",
						((r.place & SYM_PLACEMASK) == SYM_CODE) ? "SYM_CODE" : "SYM_DATA",
						r.addr);
				switch(r.type) {
					case RELOC_ADDR:
						puts("ADDR"); break;
					case RELOC_SIZE_DWORD:
						puts("SIZE_DWORD"); break;
					case RELOC_SIZE_BYTE:
						puts("SIZE_BYTE"); break;
					default:
						printf("UNKNOWN (%d)\n", r.type);
				}

				break; }
			case BFMT_EOF:
				printf("TYPE: %16.16s, SIZE: 0x%x\n", "BFMT_EOF", mess_desc.size);
				eof=1;
				break;
			default:
				printf("TYPE: 0x%14.14x, SIZE: 0x%x\n", mess_desc.type, mess_desc.size);
		}
	}
	
	if (!eof) fprintf(stderr, "<!> Unterminated image (no EOF tag)\n");
}

void disassemble() {
	/* Have you ever seen such a sight in your life... */
	int eof=-1;
	struct message_desc mess_desc;
	struct data_pkt **data_segment=NULL;
	unsigned *data_segment_size=NULL;
	int data_segments=0;
	struct _bcode_op *code_segment=NULL;
	unsigned code_segment_size=0;
	symbol *s;
	int i, j, hidden;
	char last_type;

	while(fread(&mess_desc, sizeof(mess_desc), 1, in))
	{
		if (eof < 0) {
			if (mess_desc.type != BFMT_PROGSPEC)
				fprintf(stderr, "<!> Image does not start with progspec\n");
			eof=0;
		}
		else if (eof) fprintf(stderr, "<!> Overterminated image (EOF tag with data afterward)\n");
		
		switch(mess_desc.type)
		{
			case BFMT_PROGSPEC: {
				struct progspec pspec;
				int i;
				if (sizeof(pspec) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					SKIP;
				}
				frread(&pspec, mess_desc.size, 1, in);
				printf("!SIGNATURE \"%s\"\n", pspec.name); 
				printf("!PRIORITY %d\n", pspec.priority);
				printf("!INITDOMAIN %d\n", pspec.init_domain);
				printf("!DOMAINS ");

				for(i=0;i<MAX_DOMAINS;i++) {
					if (pspec.domains[i])
					{
						if (i) printf(", %d", pspec.domains[i]);
						else printf("%d", pspec.domains[i]);
					}
				}
				printf("\n");
	
				break;
			}
			case BFMT_CODE:
				if (code_segment) {
					fprintf(stderr, "<!> Multiple code segments\n");
					mess_desc.size*=sizeof(struct _bcode_op);
					SKIP;
				}

				code_segment=malloc(mess_desc.size * sizeof(struct _bcode_op));
				if (!code_segment) {
					perror("malloc");
					return;
				}

				if (fread(code_segment, sizeof(struct _bcode_op), mess_desc.size, in) !=
						mess_desc.size) {
					perror("fread");
					return;
				}

				code_segment_size=mess_desc.size;

				break;
			case BFMT_DATA:
			case BFMT_RODATA:
				data_segments++;
				if (data_segment)
				{
					data_segment=realloc(data_segment, sizeof(struct data_pkt *) * data_segments);
					data_segment_size=realloc(data_segment_size, sizeof(unsigned) * data_segments);
				} else {
					data_segment=malloc(sizeof(struct data_pkt *) * data_segments);
					data_segment_size=malloc(sizeof(unsigned) * data_segments);
				}

				if (!data_segment || !data_segment_size) {
					perror("malloc");
					return;
				}

				data_segment[data_segments - 1]=malloc(sizeof(struct data_pkt) * mess_desc.size);
				data_segment_size[data_segments - 1]=mess_desc.size;

				if (!data_segment[data_segments - 1]) {
					perror("malloc");
					return;
				}
				
				if (fread(data_segment[data_segments - 1],
						sizeof(struct data_pkt), mess_desc.size, in) !=
						mess_desc.size) {
					perror("fread");
					return;
				}
				break;
			case BFMT_SYM:
				if (sizeof(struct stable) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					SKIP;
				}
				
				curr_symbol=malloc(sizeof(symbol));
				if (!curr_symbol) {
					perror("malloc");
					return;
				}

				frread(&curr_symbol->s, mess_desc.size, 1, in);
				if ((curr_symbol->s.place & SYM_PLACEMASK) == SYM_CODE)
				{
					curr_symbol->next=code_symbol;
					code_symbol=curr_symbol;
				} else {
					curr_symbol->next=data_symbol;
					data_symbol=curr_symbol;
				}
				break;
			case BFMT_RELOC:
			{
				areloc *r;
				if (!curr_symbol) {
					fprintf(stderr, "<!> Reloc without preceding symbol\n");
					SKIP;
				}

				if (sizeof(struct reloc) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					SKIP;
				}
				
				r=malloc(sizeof(areloc));
				if (!r) {
					perror("malloc");
					return;
				}
				
				frread(&r->r, mess_desc.size, 1, in);
				r->next=curr_symbol->reloc;
				curr_symbol->reloc=r;

				break;
			}
			case BFMT_EOF:
				eof=1;
				break;
		}
	}
	
	if (!eof) fprintf(stderr, "<!> Unterminated image (no EOF tag)\n");

	/* GASP... well at least we've got the file loaded now... */
	/* Data first? */
	printf("\n.DATA\n");
	hidden=0;

	for (j=0;j<data_segments;j++) {
		printf("# Data segment %d, logical address 0x%x\n", j, MAX_BLKSIZ * j);
		last_type=-1;

		for (i=0;i<data_segment_size[j];i++) {
			if (revert_symbols)
				s=find_symbol_data(i + MAX_BLKSIZ * j);
			else
				s=NULL;
				
			if (s) {
				if (last_type == DTYPE_STRING) printf("\"\n");
				if (hidden && !s->s.place & SYM_UNNAMED) {
					printf("}\n");
					hidden=0;
				} else if (!hidden && s->s.place & SYM_UNNAMED)
				{
					printf("{\n");
					hidden=1;
				}
				
				printf(":%s\n", s->s.fn);
			} else if (i==0) {
				if (last_type == DTYPE_STRING) printf("\"\n");
				printf(":DATA_0x%x\t# DS boundary marker\n", MAX_BLKSIZ * j);
			}
			dump_data(&data_segment[j][i], &last_type, i + MAX_BLKSIZ * j);
		}
		if (last_type == DTYPE_STRING) printf("\"\n");
	}
	if (hidden) printf("}\n");

	/* Code */
	printf("\n.CODE\n");
	hidden=0;
	for (i=0;i<code_segment_size;i++)
	{
		if (revert_symbols) {
			s=find_symbol_code(i);
		if (s) {
			/* NOTE: This is a dirty hack. It's quite probable
			 * that code will end up in a different context to
			 * the data it uses.
			 * If anyone sees an algorithm that can work this,
			 * I'd love to see it... under 1000 lines, thanks.
			 */ 
			if (hidden && !s->s.place & SYM_UNNAMED)
			{
				printf("}\n");
				hidden=0;
			} else if (!hidden && s->s.place & SYM_UNNAMED)
			{
				printf("{\n");
				hidden=1;
			}

			printf(":%s\n", s->s.fn);
		}
		}
		dump_code(&code_segment[i], i);
	}
	if (hidden) printf("}\n");
}

