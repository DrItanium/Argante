/*
 * "imgstrip" - A2 Image Stripper
 * Copyright (c) 2002	James Kehl <ecks@optusnet.com.au>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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

/*
 * This is intended for use on properly linked images.
 * -1 signize jmps & calls to SYM_UNNAMEDs in code and removes them
 *    if there's no other references elsewhere. Image is still relocatable.
 * -2 signize jmps & calls to all code symbols and removes them if
 *    there's no other references elsewhere. Image is still relocatable,
 *    but functions won't be available externally. 
 * -3 All symbols (code and data) are removed. Image is NOT relocatable.
 *    jmps and calls are NOT signized.
 */

/* oh yeh - LOOPs are also signized. */

#include "autocfg.h"
#include "compat/strcmpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "bformat.h"
#include "opcodes.h"

#include "nagt.h"

static FILE *in;
static char *outfile;

static struct progspec pspec;

static struct data_pkt **data_segment=NULL;
static unsigned *data_segment_size=NULL, data_segments=0;
static char *ds_readonly=NULL; 

static struct bcode_op *code_segment=NULL;
static unsigned code_segment_size=0;
static symbol *curr_symbol=NULL;

static int usage(void) {
	fprintf(stderr, 
	"imgstrip - A2 image stripper.\nCopyright (C) 2002 James Kehl <ecks@optusnet.com.au>\n"
	"This comes with ABSOLUTELY NO WARRANTY; but this is free software, and you are\n"
	"welcome to redistribute it under certain conditions. See COPYING for details.\n\n");
	fprintf(stderr, "Usage: imgstrip <-1|-2|-3> filename.in [filename.out]\n"
			"Options:\n"
			"-1 Remove debugging symbols - file relocatable\n"
			"-2 Remove code symbols - file relocatable but bad for libraries.\n"
			"-3 Remove ALL symbols - file NOT relocatable but compatible with pre-0.007\n"
			);
	exit(1);
}

static int disassemble(int level);

int main(int argc, char *argv[])
{
	if (argc < 3) usage();
	
	/* Open file */
	in=fopen(argv[2], "rb");
	if (!in) {
		perror("fopen:");
		exit(1);
	}

	if (argc == 4) outfile=argv[3];
	else outfile=argv[2];

	/* Check flag */
	if (*argv[1] != '-') usage();

	switch(argv[1][1])
	{
		case '1': return disassemble(1);
		case '2': return disassemble(2);
		case '3': return disassemble(3);
		default: return usage();
	}
}

#define frread(a, b, c, d) if (!fread(a, b, c, d)) { perror("fread"); return 1; }
#define frwrite(a, b, c, d) if (!fwrite(a, b, c, d)) { perror("fwrite"); return 1; }

#define TYPE_WHOLEFIELD (TYPE_UNSIGNED | TYPE_SIGNED | TYPE_FLOAT | TYPE_REGISTER | TYPE_POINTER)

static int disassemble(int level) {
	int eof=-1;
	FILE *out;
	struct message_desc mess_desc;
	unsigned i;
	struct bcode_op_packed bopp;
	struct bcode_op *op;

	while(fread(&mess_desc, sizeof(mess_desc), 1, in))
	{
		if (eof < 0) {
			if (mess_desc.type != BFMT_PROGSPEC) {
				fprintf(stderr, "<!> Image does not start with progspec\n");
				return 1;
			}
			eof=0;
		}
		else if (eof) {
			fprintf(stderr, "<!> Overterminated image (EOF tag with data afterward)\n");
			return 1;
		}
		
		switch(mess_desc.type)
		{
			case BFMT_PROGSPEC: {
				if (sizeof(pspec) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					return 1;
				}
				frread(&pspec, sizeof(pspec), 1, in);
				break;
			}
			case BFMT_CODE: {
				if (code_segment) {
					fprintf(stderr, "<!> Multiple code segments\n");
					return 1;
				}

				code_segment=malloc(mess_desc.size * sizeof(struct bcode_op));
				if (!code_segment) {
					perror("malloc");
					return 1;
				}

				for(i=0;i<mess_desc.size;i++) {
					if (!fread(&bopp, sizeof(struct bcode_op_packed), 1, in)) {
						perror("fread");
						return 1;
					}
					op=&code_segment[i];
					op->bcode=bopp.bcode;
					op->type=bopp.type;
					op->a1.val.u=bopp.a1.val.u;
					op->a2.val.u=bopp.a2.val.u;
				}

				code_segment_size=mess_desc.size;
				break;
			}
			case BFMT_DATA:
			case BFMT_RODATA:
				data_segments++;
				if (data_segment)
				{
					data_segment=realloc(data_segment, sizeof(struct data_pkt *) * data_segments);
					data_segment_size=realloc(data_segment_size, sizeof(unsigned) * data_segments);
					ds_readonly=realloc(ds_readonly, sizeof(char) * data_segments);
				} else {
					data_segment=malloc(sizeof(struct data_pkt *) * data_segments);
					data_segment_size=malloc(sizeof(unsigned) * data_segments);
					ds_readonly=malloc(sizeof(char) * data_segments);
				}

				if (!data_segment || !data_segment_size || !ds_readonly) {
					perror("malloc");
					return 1;
				}

				data_segment[data_segments - 1]=malloc(sizeof(struct data_pkt) * mess_desc.size);
				data_segment_size[data_segments - 1]=mess_desc.size;

				if (!data_segment[data_segments - 1]) {
					perror("malloc");
					return 1;
				}
				if (mess_desc.type == BFMT_RODATA)
					ds_readonly[data_segments - 1] = 1;
				else
					ds_readonly[data_segments - 1] = 0;
				
				if (fread(data_segment[data_segments - 1],
						sizeof(struct data_pkt), mess_desc.size, in) !=
						mess_desc.size) {
					perror("fread");
					return 1;
				}
				break;
			case BFMT_SYM: {
				symbol *s;
				if (sizeof(struct stable) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					return 1;
				}
				
				s=malloc(sizeof(symbol));
				if (!s) {
					perror("malloc");
					return 1;
				}
				s->reloc=NULL;

				frread(&s->s, mess_desc.size, 1, in);

				s->next=curr_symbol;
				curr_symbol=s;
				break;
			}
			case BFMT_RELOC:
			{
				unsigned ip;
				areloc *r;
				if (!curr_symbol) {
					fprintf(stderr, "<!> Reloc without preceding symbol\n");
					return 1;
				}

				if (sizeof(struct reloc) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					return 1;
				}
				
				r=malloc(sizeof(areloc));
				if (!r) {
					perror("malloc");
					return 1;
				}
			
				frread(&r->r, mess_desc.size, 1, in);

#define LEAVERELOC(a) goto addreloc
				/* Don't touch undefined symbols... */
				if (curr_symbol->s.place & SYM_UNDEFINED) LEAVERELOC("undefined");
				/* If we're level 3 we don't care about relocating */
				if (level >= 3) break;
				/* Who gives a rats about RELOC_SIZE even in data?! It doesn't change... */
				if (r->r.type != RELOC_ADDR) break;
				/* Only touch internal symbols on lev1 */
				if (level < 2 && !(curr_symbol->s.place & SYM_UNNAMED)) LEAVERELOC("named");
				if ((r->r.place & SYM_PLACEMASK) != SYM_CODE) LEAVERELOC("reloc not in code");
				if ((curr_symbol->s.place & SYM_PLACEMASK) != SYM_CODE) LEAVERELOC("not code sym");
				/* If it's JMP u: or CALL u: we'll signize it */
				ip=r->r.addr / sizeof(struct bcode_op);
				if (ip >= code_segment_size) {
					fprintf(stderr, "<!> Reloc code target out of bounds.\n");
					return 1;
				}
				op=&code_segment[ip];
				if (op->bcode == CMD_JMP || op->bcode == CMD_CALL) {
					if (op->type & TYPE_A1(TYPE_REGISTER | TYPE_POINTER)) LEAVERELOC("JMP with reg/ptr");
					if ((op->type & TYPE_A1(TYPE_VALMASK)) != TYPE_UNSIGNED)
						LEAVERELOC("JMP not unsigned"); 

					if (r->r.addr % sizeof(struct bcode_op) != OFFSET_OP_A1) {
						fprintf(stderr, "<!> JMP's address is not in a1!\n");
						return 1;
					}
					op->a1.val.s-=ip;
					op->type&=TYPE_A2(TYPE_WHOLEFIELD);
					op->type|=TYPE_A1(TYPE_SIGNED);
					break;
				}
				if (op->bcode == CMD_LOOP) {
					if (op->type & TYPE_A2(TYPE_REGISTER | TYPE_POINTER)) LEAVERELOC("LOOP with reg/ptr");
					if ((op->type & TYPE_A2(TYPE_VALMASK)) != TYPE_UNSIGNED)
						LEAVERELOC("LOOP not unsigned");

					if (r->r.addr % sizeof(struct bcode_op) != OFFSET_OP_A2) {
						fprintf(stderr, "<!> LOOP's code address is not in a2?\n");
						return 1;
					}
					op->a2.val.s-=ip;
					op->type&=TYPE_A1(TYPE_WHOLEFIELD);
					op->type|=TYPE_A2(TYPE_SIGNED);
					break;
				}
				LEAVERELOC("Wrong opcode");
				addreloc:
				r->next=curr_symbol->reloc;
				curr_symbol->reloc=r;

				break;
			}
			case BFMT_EOF:
				eof=1;
				break;
		}
	}
	
	if (!eof) {
		fprintf(stderr, "<!> Unterminated image (no EOF tag)\n");
		return 1;
	}

	fclose(in);

	out=fopen(outfile, "wb");
	if (!out) {
		perror("fopen:");
		exit(1);
	}

	/* DONE! WRITE OUTPUT! */
	mess_desc.type=BFMT_PROGSPEC;
	mess_desc.size=sizeof(pspec);
	frwrite(&mess_desc, sizeof(mess_desc), 1, out);
	frwrite(&pspec, sizeof(pspec), 1, out);

	if (code_segment_size) {
		mess_desc.type=BFMT_CODE;
		mess_desc.size=code_segment_size;
		frwrite(&mess_desc, sizeof(mess_desc), 1, out);

		/* Pack code */
		for(i=0;i<mess_desc.size;i++) {
			op=&code_segment[i];
			bopp.bcode=op->bcode;
			bopp.type=op->type;
			bopp.a1.val.u=op->a1.val.u;
			bopp.a2.val.u=op->a2.val.u;
			frwrite(&bopp, sizeof(bopp), 1, out);
		}
	}

	for (i=0;i<data_segments;i++)
	{
		if (ds_readonly[i] != 0)
			mess_desc.type=BFMT_RODATA;
		else
			mess_desc.type=BFMT_DATA;
		mess_desc.size=data_segment_size[i];
		frwrite(&mess_desc, sizeof(mess_desc), 1, out);
		frwrite(data_segment[i], sizeof(struct data_pkt), mess_desc.size, out);
	}

	{
	symbol *s; areloc *r;
	i=0;
	s=curr_symbol;
	while(s)
	{
		/* Level 3: write no symbols but ones with relocs. */
		/* Level 2: write no symbols but DATA ones or with relocs. */
		/* Level 1: write no symbols but DATA ones or with relocs or 'named'. */
		if (s->reloc || (level < 3 && (s->s.place & SYM_PLACEMASK) == SYM_DATA) ||
			(level < 2 && !(s->s.place & SYM_UNNAMED))) {
		mess_desc.type=BFMT_SYM;
		mess_desc.size=sizeof(struct stable);

		frwrite(&mess_desc, sizeof(mess_desc), 1, out);

		frwrite(&s->s, sizeof(struct stable), 1, out);

		mess_desc.type=BFMT_RELOC;
		mess_desc.size=sizeof(struct reloc);

		r=s->reloc;
		while (r)
		{
			frwrite(&mess_desc, sizeof(mess_desc), 1, out);
			frwrite(&r->r, sizeof(struct reloc), 1, out);
			r=r->next;
		}
		} else {
			i++;
		}
		s=s->next;
	}
	}

	fprintf(stderr, "<+> %d symbols omitted\n", i);
	/* End it */
	mess_desc.type=BFMT_EOF;
	mess_desc.size=0;
	frwrite(&mess_desc, sizeof(mess_desc), 1, out);
	return 0;
}

