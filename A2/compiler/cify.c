/*
 * "cify" - A2 Image -> C converter. 
 * Copyright (c) 2001	James Kehl <ecks@optusnet.com.au>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
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
 * Please note I disclaim any responsibility for or claim to the files
 * this software outputs. If you want to sell them for some exorbitant sum,
 * you can, just like a GCC'd program. If they go haywire and erase all your
 * files (or your boss', etc. etc.), that's also your problem.
 * 
 * -------------------------------------------------------------------------
 * 
 */

/*
 * If you cared, this is a very, very, very hacked up disassembler.
 */
#include "autocfg.h"
#include "compat/strtok_r.h"
#include "compat/strcmpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "bformat.h"
#include "lang2.h"
#include "opcodes.h"

#include "nagt.h"

/*
 * Thoughts/ToDo:
 * - Machine-code output? Nonportable... but nicer than intermediate C?
 * - detect which modules are needed
 */

void usage(void) {
		fprintf(stderr,
		"cify - A2 native compiler.\nCopyright (C) 2001 James Kehl <ecks@optusnet.com.au>\n"
		"This comes with ABSOLUTELY NO WARRANTY; but this is free software, and you are\n"
		"welcome to redistribute it under certain conditions. See COPYING for details.\n\n");

		fprintf(stderr, "Usage: cify image.in hac.in\n");
		exit(1);
}

FILE *in;

void disassemble(void);
void dump_hac(FILE *from);

int main(int argc, char *argv[])
{
	FILE *hac;
	if (argc < 3) usage();
	
	/* Open file */
	in=fopen(argv[1], "rb");
	if (!in) {
		perror("fopen:");
		exit(1);
	}
	hac=fopen(argv[2], "r");
	if (!hac) {
		perror("fopen:");
		exit(1);
	}
	printf(
		"#include <stdlib.h>\n"
		"#include <string.h>\n"
		"#include <signal.h>\n"
		"#include \"autocfg.h\"\n"
		"#include \"taskman.h\"\n"
	        "#include \"printk.h\"\n"
		"#include \"bcode.h\"\n"
		"#include \"hhac.h\"\n"
		"#include \"amemory.h\"\n"
		"#include \"modload.h\"\n"
		"#include \"exception.h\"\n"
		"#include \"cfdop.h\"\n"
		);
	printf("/* HAC from %s */\n", argv[2]);
	dump_hac(hac);
	fclose(hac);
	disassemble();

	/* Reminds me of Orange */
	printf(
		"int main(int argc, char **argv) {\n"
		"\tstruct vcpu cpu;\n"
		"\tint i;\n"
		"\tconst struct cfdop_1 *cfdtbl;\n"
		"#ifdef SIGPIPE\n"
		"\tsignal(SIGPIPE, SIG_IGN);\n"
		"#endif\n"
		"\tmodule_static_init();\n"
/* Removed 0.007: standalone cify stuff should not RTL */
/* "\twhile(argc > 1) { argc--; module_dyn_load(argv[argc]); }\n" */
		"\tfixed_load_code(&cpu);\n"
		"\thac_init(&cpu);\n"
		"\tfixed_hac(&cpu);\n"
		"\tvcpu_modules_start(&cpu);\n"
		/* Volunteer a VFD...? 0.007 */
		"\tcfdtbl=cfdop1_fddesc_get(*((int *) \"TCON\"));\n"
		"\tif (cfdtbl) cpu.reg[0].val.s=\n"
		"\tcfdtbl->fd_create(&cpu, NULL, fileno(stdin), fileno(stdout));\n"
		"\ti=do_fixed_bcode(&cpu);\n"
		"\tvcpu_modules_stop(&cpu);\n"
		"\treturn i;\n"
		"}\n"
		);
	return 0;
}

char type_of_a1(const struct bcode_op *bop) {
	if (bop->type & TYPE_A1(TYPE_SIGNED))
		return 's';
	else if (bop->type & TYPE_A1(TYPE_FLOAT))
		return 'f';
	else
		return 'u';
}

char type_of_a2(const struct bcode_op *bop) {
	if (bop->type & TYPE_A2(TYPE_SIGNED))
		return 's';
	else if (bop->type & TYPE_A2(TYPE_FLOAT))
		return 'f';
	else
		return 'u';
}

/* Mostly borrowed from the kernel's HHAC.c. checking etc? */
void dump_hac(FILE *from) {
	int i;
	char buf[1000];
	char *x, *s;
	char *dir, *atype, *mode;

	printf("#define ADDHAC_ERROR \"add_hac_i failed: DIR: %%s ATYPE: %%s TYPE: %%s\"\n");
	printf("static void fixed_hac(struct vcpu *cpu) {\n");
	while (fgets(buf,sizeof(buf), from)) {
		s=&buf[0];
		if ((x=strchr(buf,'#'))) *x=0;
		if ((x=strchr(buf,'\n'))) *x=0;
		if (!*s) continue;
		/* I couldn't help but make improvements... */
		while (*s && isspace(*s)) s++;
		if (!*s) continue;
		/* like this... */
		x=strchr(s, 0);
		while (isspace(*x)) { *x=0; x--; }
		
		if (!*s) continue;
		/* against all odds we have a valid line... */
		dir=strtok_r(s, " \t", &x);
		if (*dir == '/') dir++;
		atype=strtok_r(NULL, " \t", &x);
		mode=strtok_r(NULL, " \t", &x);
		/* Don't think it's safe/useful to allow setting PERM_UNSPEC */
		i=strcasecmp(mode, "allow");
		printf("\tif (add_hac_entry(cpu, \"%s\", \"%s\", %s))\n", dir, atype,
				i ? "PERM_DENY" : "PERM_ALLOW" );
		printf("\t\tfprintf(stderr, ADDHAC_ERROR, \"%s\", \"%s\", \"%s\");\n",
				dir, atype, i ? "PERM_DENY" : "PERM_ALLOW" );
	}
	printf("}\n");
}
#define SKIP while(mess_desc.size > 0) { fgetc(in); mess_desc.size--; } break
#define frread(a, b, c, d) if (!fread(a, b, c, d)) { perror("fread"); return; }


void disassemble(void) {
	/* Have you ever seen such a sight in your life... */
	int eof=-1;
	struct message_desc mess_desc;
	struct data_pkt **data_segment=NULL;
	unsigned *data_segment_size=NULL;
	unsigned data_segments=0;
	unsigned *data_prot=NULL;
	struct bcode_op *code_segment=NULL;
	unsigned code_segment_size=0;
	unsigned long i, j;

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
				int domsz, i;
				if (sizeof(pspec) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size\n");
					SKIP;
				}

				frread(&pspec, mess_desc.size, 1, in);
				printf("static void fixed_load_code(struct vcpu *cpu) {\n");
            /* Try not to include autocfg.h... we'd like portable code... */
				/* printf("\tbzero(cpu, sizeof(struct vcpu));\n"); */
				printf("\t{ int i; for(i=0;i<sizeof(struct vcpu);i++)"
							" ((char *) cpu)[i]=0; }");
				printf("\tstrncpy(cpu->pname, \"%s\", A2_MAX_PNAME);\n", pspec.name);
				printf("\tcpu->priority=%d;\n", pspec.priority);

				/* Domains. Hardly worthwhile with per-CPU hac but wtf. */
				printf("\tcpu->domain=%d;\n", pspec.init_domain);

				for(domsz=0;domsz<A2_MAX_DOMAINS;domsz++)
					if (!pspec.domains[domsz]) break;
				domsz++; /* MAY end up with more than MAX_DOMAINS? */

				/* XXX: error check */
				printf("\tcpu->dlist=malloc(%d * sizeof(unsigned));\n", domsz);
				for(i=0;i<domsz;i++) {
					printf("\tcpu->dlist[%d]=%d;\n", i, pspec.domains[i]);
				}
	
				break;
			}
			case BFMT_CODE: {
				struct bcode_op_packed bopp;
				struct bcode_op *bop;
				if (code_segment) {
					fprintf(stderr, "<!> Multiple code segments\n");
					mess_desc.size*=sizeof(struct bcode_op);
					SKIP;
				}

				code_segment=malloc(mess_desc.size * sizeof(struct bcode_op));
				if (!code_segment) {
					perror("malloc");
					return;
				}

				for(i=0;i<mess_desc.size;i++) {
					if (!fread(&bopp, sizeof(struct bcode_op_packed), 1, in)) {
						perror("fread");
						return;
					}
					bop=&code_segment[i];
					bop->bcode=bopp.bcode;
					bop->type=bopp.type;
					bop->a1.val.u=bopp.a1.val.u;
					bop->a2.val.u=bopp.a2.val.u;
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
					data_prot=realloc(data_prot, sizeof(unsigned) * data_segments);
				} else {
					data_segment=malloc(sizeof(struct data_pkt *) * data_segments);
					data_segment_size=malloc(sizeof(unsigned) * data_segments);
					data_prot=malloc(sizeof(unsigned) * data_segments);
				}

				if (!data_segment || !data_segment_size || !data_prot) {
					perror("malloc");
					return;
				}

				data_segment[data_segments - 1]=malloc(sizeof(struct data_pkt) * mess_desc.size);
				data_segment_size[data_segments - 1]=mess_desc.size;
				data_prot[data_segments - 1]=mess_desc.type;

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
			case BFMT_RELOC:
				SKIP;
			case BFMT_EOF:
				eof=1;
				break;
		}
	}
	
	if (!eof) fprintf(stderr, "<!> Unterminated image (no EOF tag)\n");

	/* GASP... well at least we've got the file loaded now... */
	
	printf("/* Load data segments */\n");
	printf("\tcpu->memblks=%d;\n", data_segments);
	printf("\tcpu->mem=calloc(%d, sizeof(struct memblk));\n", data_segments);
	for(i=0;i<data_segments;i++) {
		printf("\tcpu->mem[%lu].mode=A2_MEM_READ%s;\n", i,
			(data_prot[i]==BFMT_RODATA) ? "" : "|A2_MEM_WRITE");
		printf("\tcpu->mem[%lu].size=%d;\n", i, data_segment_size[i]); 
		
		for(j=0;j<data_segment_size[i];j++) {
			if (data_segment[i][j].u.du_int != 0) break;
		}

		if (j<data_segment_size[i]) {
			/* Oooh, _initialized_ memory! */
			printf("\tcpu->mem[%lu].memory=malloc(%d * sizeof(anyval));\n", i, data_segment_size[i]);
			printf("\tmemcpy(cpu->mem[%lu].memory,\n\"", i);
			for(j=0;j<data_segment_size[i];j++) { /* XXX: Endians? */
				printf("\\x%2.2x", (unsigned char) ((char *) &data_segment[i][j].u.du_int)[0]);
				printf("\\x%2.2x", (unsigned char) ((char *) &data_segment[i][j].u.du_int)[1]);
				printf("\\x%2.2x", (unsigned char) ((char *) &data_segment[i][j].u.du_int)[2]);
				printf("\\x%2.2x", (unsigned char) ((char *) &data_segment[i][j].u.du_int)[3]);
				if (j % 4 == 3) printf("\"\n\""); /* Line wrap */
			}
			printf("\", %d * sizeof(anyval));\n", data_segment_size[i]);
		} else { /* Empty memory. Just clear it. */
			printf("\tcpu->mem[%lu].memory=calloc(%d, sizeof(anyval));\n", i, data_segment_size[i]);
		}
	}
	printf("/* End */\n}\n\n");

	/* Now for the code... <shudder> */

	/* XXX: Validate?
	 * IF WE USE ANY DIFFERENT RULES TO THE INTERPRETER, ***WE HAVE A PROBLEM!*** */
	printf("#define PUSH_STACK(a) if(cpu->mstack_ptr >= cpu->mstack_size) \\\n"
		"throw_except(cpu, ERR_MSTACK_OVER); \\\n"
		"a=mem_rw(cpu, cpu->mstack_ptr + cpu->mstack); \\\n"
		"cpu->mstack_ptr++\n");
	printf("#define POP_STACK(a) if(cpu->mstack_ptr == 0) \\\n"
		"throw_except(cpu, ERR_MSTACK_UNDER); \\\n"
		"cpu->mstack_ptr--; \\\n"
		"a=mem_ro(cpu, cpu->mstack_ptr + cpu->mstack)\n\n");

	printf("static int do_fixed_bcode(struct vcpu *cpu) {\n\tunsigned xp, origip;\n\tanyval *a1, *a2;\n/* X-handler */\n");
	printf("\tif((xp=setjmp(cpu->onexcept))) {\n"
		"\tif (xp==X_CPUSHUTDOWN) {\n"
		"\tprintk2(PRINTK_WARN, \"Task committed suicide (HALT or RAISE -1).\\n\");\n"
		"\treturn 0;\n"
		"\t}\n"
		"\torigip=cpu->ip;\n"
		"\twhile (!cpu->xip) {\n"
		"\tif(setjmp(cpu->onexcept))\n"
		"\t{\n"
		"\tfprintf(stderr, \"<+> Unhandled exception %%d, origin 0x%%x. Murdered.\\n\", xp, origip);\n"
		"\treturn 1;\n"
		"\t}\n"
		"\tpop_ip_from_stack(cpu, 1);\n"
		"\t}\n"
		"\tcpu->ip=cpu->xip;\n"
		"\tcpu->reg[%d].val.u=xp;\n"
		"\tcpu->xip=0;\n"
		"\t}\n/* Code Handler */\n\n", A2_REGISTERS - 1);
	printf("\twhile(1) {\n");
	printf("\tswitch(cpu->ip) {\n");
	for(i=0;i<code_segment_size;i++) {
		/* Should the code go to next instruction, no q's asked? */
		int fallthru=1;

		printf("\tcase %lu: sS%lu:\n", i, i);
		if ((unsigned) code_segment[i].bcode >= OPS) {
			printf("\t\tthrow_except(cpu, ERR_CORRUPT_CODE);\n");
			continue;
		}

		if (code_segment[i].type & TYPE_A1(TYPE_REGISTER) && code_segment[i].a1.val.u >= A2_REGISTERS) {
			fprintf(stderr, "<!> Excessive register!\n");
			return;
		}

		if (code_segment[i].type & TYPE_A1(TYPE_POINTER)) {
			if (op[(unsigned) code_segment[i].bcode].tparam1 & (1 << TYPE_REGISTER))
				printf("\t\ta1=mem_rw(cpu, \n");
			else
				printf("\t\ta1=(anyval *) mem_ro(cpu, \n");
			printf((code_segment[i].type & TYPE_A1(TYPE_REGISTER)) ?
				"cpu->reg[%lu].val.u" : "%lu",
				code_segment[i].a1.val.u);
			printf(");\n");
		} else if (code_segment[i].type & TYPE_A1(TYPE_REGISTER)) {
			printf("\ta1=&cpu->reg[%lu];\n", code_segment[i].a1.val.u);
		} else { /* FIXME! We can do this better, surely */
			printf("\t{ static const ");
			if (code_segment[i].type & TYPE_A1(TYPE_FLOAT)) {
				printf("float ca1=%f;", code_segment[i].a1.val.f);
			} else if (code_segment[i].type & TYPE_A1(TYPE_SIGNED)) {
				printf("signed ca1=%ld;", code_segment[i].a1.val.u);
			} else {
				printf("unsigned ca1=%luu;", code_segment[i].a1.val.u);
			}
			printf(" a1=(anyval *) &ca1; }\n"); /* XXX POSSIBLY-BROKEN HACK */
		}
	
		if (code_segment[i].type & TYPE_A2(TYPE_REGISTER) && code_segment[i].a2.val.u >= A2_REGISTERS) {
			fprintf(stderr, "<!> Excessive register!\n");
			return;
		}

		if (code_segment[i].type & TYPE_A2(TYPE_POINTER)) {
			if (op[(unsigned) code_segment[i].bcode].tparam2 & (1 << TYPE_REGISTER))
				printf("\t\ta2=mem_rw(cpu, \n");
			else
				printf("\t\ta2=(anyval *) mem_ro(cpu, \n");
			printf((code_segment[i].type & TYPE_A2(TYPE_REGISTER)) ?
				"cpu->reg[%lu].val.u" : "%lu",
				code_segment[i].a2.val.u);
			printf(");\n");
		} else if (code_segment[i].type & TYPE_A2(TYPE_REGISTER)) {
			printf("\ta2=&cpu->reg[%lu];\n", code_segment[i].a2.val.u);
		} else {
			printf("\t{ static const ");
			if (code_segment[i].type & TYPE_A2(TYPE_FLOAT)) {
				printf("float ca2=%f;", code_segment[i].a2.val.f);
			} else if (code_segment[i].type & TYPE_A2(TYPE_SIGNED)) {
				printf("signed ca2=%ld;", code_segment[i].a2.val.u);
			} else {
				printf("unsigned ca2=%luu;", code_segment[i].a2.val.u);
			}
			printf(" a2=(anyval *) &ca2; }\n"); /* XXX POSSIBLY-BROKEN HACK */
		}

		/*
		 * We don't have to use the NEXT_IP hack because we know whether the op
		 * will change IP or not. I guess. Except maybe the syscalls: make provisions for that.
		 */
		switch(code_segment[i].bcode) {
			case CMD_NOP:
				break; /* TOO easy :) */
			case CMD_MOV:
				printf("a1->val.%c%s=a2->val.%c;\n",
					type_of_a1(&code_segment[i]),
					"",
					type_of_a2(&code_segment[i]));
				break;
			case CMD_ADD:
				printf("a1->val.%c%s=a2->val.%c;\n",
					type_of_a1(&code_segment[i]),
					"+",
					type_of_a2(&code_segment[i]));
				break;
			case CMD_SUB:
				printf("a1->val.%c%s=a2->val.%c;\n",
					type_of_a1(&code_segment[i]),
					"-",
					type_of_a2(&code_segment[i]));
				break;
			case CMD_MUL:
				printf("a1->val.%c%s=a2->val.%c;\n",
					type_of_a1(&code_segment[i]),
					"*",
					type_of_a2(&code_segment[i]));
				break;
			case CMD_DIV:
				printf("a1->val.%c%s=a2->val.%c;\n",
					type_of_a1(&code_segment[i]),
					"/",
					type_of_a2(&code_segment[i]));
				break;
			case CMD_AND: /* These will generate invalid instructions for invalid bytecode. Eh */
				printf("a1->val.u&=a2->val.u;\n");
				break;
			case CMD_OR:
				printf("a1->val.u|=a2->val.u;\n");
				break;
			case CMD_XOR:
				printf("a1->val.u^=a2->val.u;\n");
				break;
			case CMD_NOT:
				printf("a1->val.u=~a1->val.u;\n");
				break;
			case CMD_MOD:
				printf("a1->val.u%%=a2->val.u;\n");
				break;
			case CMD_ROL:
				printf("{\nunsigned i=a2->val.u; while(i) {\n"
				"a1->val.u=(a1->val.u << 1) | (a1->val.u >> (sizeof(unsigned) * 8 - 1));\n"
				"i--;\n"
				"}\n}\n");
				break;
			case CMD_ROR:
				printf("{\nunsigned i=a2->val.u; while(i) {\n"
				"a1->val.u=(a1->val.u << (sizeof(unsigned) * 8 - 1)) | (a1->val.u >> 1);\n"
				"i--;\n"
				"}\n}\n");
				break;
			case CMD_SHL:
				printf("a1->val.u<<=a2->val.u;\n");
				break;
			case CMD_SHR:
				printf("a1->val.u>>=a2->val.u;\n");
				break;
			case CMD_ALLOC:
				printf("a1->val.u=mem_alloc(cpu, a1->val.u, a2->val.u);\n");
				break;
			case CMD_REALLOC:
				if (code_segment[i].type & TYPE_A2(TYPE_SIGNED))
					printf("mem_changeperm(cpu, a1->val.u, a2->val.u);\n");
				else
					printf("mem_realloc(cpu, a1->val.u, a2->val.u);\n");
				break;
			case CMD_FREE:
				printf("mem_dealloc(cpu, a1->val.u);\n");
				break;
			case CMD_STACK:
				printf("cpu->mstack=a1->val.u;\n"
					"cpu->mstack_size=a2->val.u;\n");
				break;
			/* The following instructions don't usually fallthru,
				as they nearly always use goto; which makes break redundant.
				So set fallthru to -1 if 0/1 would create dead code. */
			case CMD_LOOP: {
				/* Can fallthru; but can also change IP. */
				/* XXX: won't compile if jump to excessive address */
				char s;
				s=(char) ((code_segment[i].type & TYPE_A1(TYPE_SIGNED)) ?
					's' : 'u');
				printf("if (a1->val.%c > 0) {\n"
					"a1->val.%c--;\n", s, s);
				if (code_segment[i].type & TYPE_A1(TYPE_SIGNED)) {
					printf("cpu->ip+=a2->val.s;\n");
					if (!(code_segment[i].type & (TYPE_A2(TYPE_REGISTER) | TYPE_A2(TYPE_POINTER)))) {
						printf("goto sS%lu;\n", i+code_segment[i].a2.val.s);
						fallthru=-1;
					} else fallthru=0;
				} else {
					printf("cpu->ip=a2->val.u;\n");
					if (!(code_segment[i].type & (TYPE_A2(TYPE_REGISTER) | TYPE_A2(TYPE_POINTER)))) {
						printf("goto sS%lu;\n", code_segment[i].a2.val.u);
						fallthru=-1;
					} else fallthru=0;
				}
				printf("} else { cpu->ip++; goto sS%lu; }\n", i + 1);
				break;
					 }
			case CMD_IFABO:
/*				printf("cpu->ip+=(a1->val.%c %s a2->val.%c) ? 1 : 2;\n",
					type_of_a1(&code_segment[i]),
					">",
					type_of_a2(&code_segment[i])); */
				printf("if (a1->val.%c %s a2->val.%c) {\n"
					"cpu->ip+=1; goto sS%lu;\n"
					"} else {\n"
					"cpu->ip+=2; goto sS%lu;\n}\n",
					type_of_a1(&code_segment[i]),
					">",
					type_of_a2(&code_segment[i]),
					i+1, i+2);
				fallthru=-1;
				break;
			case CMD_IFBEL:
				printf("if (a1->val.%c %s a2->val.%c) {\n"
					"cpu->ip+=1; goto sS%lu;\n"
					"} else {\n"
					"cpu->ip+=2; goto sS%lu;\n}\n",
					type_of_a1(&code_segment[i]),
					"<",
					type_of_a2(&code_segment[i]),
					i+1, i+2);
				fallthru=-1;
				break;
			case CMD_IFEQ:
				printf("if (a1->val.%c %s a2->val.%c) {\n"
					"cpu->ip+=1; goto sS%lu;\n"
					"} else {\n"
					"cpu->ip+=2; goto sS%lu;\n}\n",
					type_of_a1(&code_segment[i]),
					"==",
					type_of_a2(&code_segment[i]),
					i+1, i+2);
				fallthru=-1;
				break;
			case CMD_IFNEQ:
				printf("if (a1->val.%c %s a2->val.%c) {\n"
					"cpu->ip+=1; goto sS%lu;\n"
					"} else {\n"
					"cpu->ip+=2; goto sS%lu;\n}\n",
					type_of_a1(&code_segment[i]),
					"!=",
					type_of_a2(&code_segment[i]),
					i+1, i+2);
				fallthru=-1;
				break;
			case CMD_JMP:
				if (code_segment[i].type & TYPE_A1(TYPE_SIGNED)) {
					printf("cpu->ip+=a1->val.s;\n");
					if (!(code_segment[i].type & (TYPE_A1(TYPE_REGISTER) | TYPE_A1(TYPE_POINTER)))) {
						printf("goto sS%lu;\n", i+code_segment[i].a1.val.s);
						fallthru=-1;
					} else fallthru=0;
				} else {
					printf("cpu->ip=a1->val.u;\n");
					if (!(code_segment[i].type & (TYPE_A1(TYPE_REGISTER) | TYPE_A1(TYPE_POINTER)))) {
						printf("goto sS%lu;\n", code_segment[i].a1.val.u);
						fallthru=-1;
					} else fallthru=0;
				}
				break;
			case CMD_POP:
				printf("{ const anyval *z; POP_STACK(z); ");
				printf("a1->val.%c=z->val.%c",
					type_of_a1(&code_segment[i]),
					type_of_a1(&code_segment[i]));
				printf("; }\n");
				break;
			case CMD_PUSH:
				printf("{ anyval *z; PUSH_STACK(z); ");
				printf("z->val.%c=a1->val.%c",
					type_of_a1(&code_segment[i]),
					type_of_a1(&code_segment[i]));
				printf("; }\n");
				break;
			case CMD_SYSCALL:
				printf("cpu->next_ip=cpu->ip + 1;\n"
					"agt_syscall(cpu, a1->val.u, NULL);\n"
					"cpu->ip=cpu->next_ip;\n");
				fallthru=0;
				break;
			case CMD_SYSCALL2:
				printf("cpu->next_ip=cpu->ip + 1;\n"
					"agt_syscall(cpu, a1->val.u, a2);\n"
					"cpu->ip=cpu->next_ip;\n");
				fallthru=0;
				break;
			case CMD_HALT:
				printf("throw_except(cpu, X_CPUSHUTDOWN);\n");
				fallthru=0; /* Unreachable code, but wtf */
				break;
			case CMD_HANDLER:
				printf("cpu->xip=a1->val.u;\n");
				break;
			case CMD_RAISE:
				printf("throw_except(cpu, a1->val.u);\n");
				fallthru=0;
				break;
			case CMD_CALL:
				printf("cpu->next_ip=cpu->ip + 1;\n"
				"push_ip_on_stack(cpu);\n"
				"cpu->xip=0;\n"
				);
				if (code_segment[i].type & TYPE_A1(TYPE_SIGNED)) {
					printf("cpu->ip+=a1->val.s;\n");
					if (!(code_segment[i].type & (TYPE_A1(TYPE_REGISTER) | TYPE_A1(TYPE_POINTER)))) {
						printf("goto sS%lu;\n", i+code_segment[i].a1.val.s);
						fallthru=-1;
					} else fallthru=0;
				} else {
					printf("cpu->ip=a1->val.u;\n");
					if (!(code_segment[i].type & (TYPE_A1(TYPE_REGISTER) | TYPE_A1(TYPE_POINTER)))) {
						printf("goto sS%lu;\n", code_segment[i].a1.val.u);
						fallthru=-1;
					} else fallthru=0;
				}
				break;
			case CMD_RET:
				printf("{ unsigned work=a1->val.u;\n"
					"if (!work) work=1;\n"
					"pop_ip_from_stack(cpu, work);\n"
					"cpu->ip=cpu->next_ip;\n"
					"}\n");
				fallthru=0;
				break;
			case CMD_WAIT:
				printf("usleep(a1->val.u);\n");
				break;
			default:
				printf("throw_except(cpu, ERR_CORRUPT_CODE);\n");
		}
		if (fallthru > 0)
			printf("\t\tcpu->ip++;\n");
		else if (!fallthru)
			printf("\t\tbreak;\n");
		/* else both ways goto */
	}
	printf("\tdefault:\nthrow_except(cpu, ERR_OUTSIDE_CODE);\n}\n}\n}\n");
}

