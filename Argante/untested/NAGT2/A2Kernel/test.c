/*
 * Argante2 'lemon' pre-alpha.
 * (c) 2001 James Kehl <ecks@optusnet.com.au>
 * Use under LGPL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include "config.h"
#include "taskman.h"
#include "bcode.h"
#include "data_blk.h"
#include "exception.h"
#include "memory.h"
#include "cmd.h"
#include <sys/time.h>

/* Ok, we are talking majorly testing only for this */
void agt_syscall(struct vcpu *curr_cpu, unsigned callno, anyval *arg)
{
	if (!arg) arg=&curr_cpu->reg[0];
	
	switch(callno)
	{
		case 0x1: {
			/* IO_PUTSTRING */
			int i;
			char *x=NULL;
			anyval *p;
			i=0;
//			printf("Printing %d chars from addr %d\n", curr_cpu->reg[1].val.u, arg->val.u);
			while(i < curr_cpu->reg[1].val.u)
			{
				if (!(i % 4)) {
					p=mem_ro(curr_cpu, arg->val.u + i / 4);
					x=(char *) &p->val.u;
				} else {
					x++;
				}
				i++;
//				printf("%d(%c) @ %d(%d)", *x, *x, arg->val.u + i / 4, p->val.u);
				fputc(*x, stdout);
			}
			
			break;
		}
		case 0x2: printf("%ld", arg->val.u); /* IO_PUTINT */ break;
		case 0x4: printf("%g", arg->val.f); /* IO_PUTFLOAT */ break;
		case 0x3: printf("%c", (char) arg->val.u); /* IO_PUTCHAR */ break;
		case 0x5: printf("%lx", arg->val.u); /* IO_PUTHEX */ break;
		case 301: {
			/* LOCAL_GETTIME */
				  struct timezone tz;
				  struct timeval tv;
				  gettimeofday(&tv,&tz);
				  
				  curr_cpu->reg[0].val.u=tv.tv_sec;
				  curr_cpu->reg[1].val.u=tv.tv_usec;
		} break;
		default: throw_except(curr_cpu, ERR_NOSYSCALL);
	}
}

#define frread(a, b, c, d) if (!fread(a, b, c, d)) { perror("fread"); return 1; }

int main(int argc, char **argv)
{
	struct vcpu aaa;
	FILE *in;
	struct message_desc mess_desc;
	int eof=-1;
	int xp;

	bzero(&aaa, sizeof(aaa));

	if (argc != 2) {
		fprintf(stderr, "<!> Usage: %s file.img\n\n", argv[0]);
		exit(1);
	}

	if (!(in=fopen(argv[1], "rb"))) {
		perror("fopen");
		exit(1);
	}

#define BRI  "\x1b[1m"
#define DARK "\x1b[2m"
#define NORM "\x1b[0m"

  printk(NORM "Booting AOSr2 'lemon' pre-alpha...\t" BRI "\"[We] use bad software and bad machines for "
              "the wrong things.\"\n\n");
  printk(NORM "(C) 2001 James Kehl <ecks@optusnet.com.au>\n"
 	"(C) 2001, 2001 Michal Zalewski <lcamtuf@bos.bindview.com>\n"
  	"(C) 2000, 2001 Argante Development Team <argante@linuxpl.org>\n\n");

	while(fread(&mess_desc, sizeof(mess_desc), 1, in))
	{
		if (eof < 0) {
			if (mess_desc.type != BFMT_PROGSPEC) {
				fprintf(stderr, "<!> Image does not start with progspec\n");
				exit(1);
			}
			eof=0;
		}
		else if (eof) fprintf(stderr, "<!> Overterminated image (EOF tag with data afterward)\n");
		else if (mess_desc.type == BFMT_PROGSPEC) {
			fprintf(stderr, "<!> Multiple progspecs?\n");
			exit(1);
		}
		
		switch(mess_desc.type)
		{
			case BFMT_PROGSPEC: {
				struct progspec pspec;
				int i;
				
				if (sizeof(pspec) != mess_desc.size) {
					fprintf(stderr, "<!> Incomprehensible element size for pspec (%d/%d)\n", mess_desc.size, sizeof(pspec));
					exit(1);
				}
				frread(&pspec, mess_desc.size, 1, in);
				strncpy(aaa.pname, pspec.name, MAX_PNAME);
				// pspec.priority???
				aaa.domain = pspec.init_domain;

				for(i=0;i<MAX_DOMAINS;i++) {
					if (!pspec.domains[i])
						break;
				}

				aaa.dlist=malloc(i * sizeof(unsigned));
				memcpy(aaa.dlist, pspec.domains, i * sizeof(unsigned));
	
				break;
			}
			case BFMT_CODE:
				if (aaa.bcode) {
					fprintf(stderr, "<!> Multiple code segments\n");
					exit(1);
				}

				aaa.bcode=malloc(mess_desc.size * sizeof(struct _bcode_op));
				if (!aaa.bcode) {
					perror("malloc");
					exit(1);
				}

				if (fread(aaa.bcode, sizeof(struct _bcode_op), mess_desc.size, in) !=
						mess_desc.size) {
					perror("fread");
					exit(1);
				}

				aaa.csize=mess_desc.size;

				break;
			case BFMT_DATA:
			case BFMT_RODATA: {
				struct memblk *x;
				struct data_pkt bl;
				int i=0;
				
				aaa.memblks++;
				if (aaa.mem)
					aaa.mem=realloc(aaa.mem, sizeof(struct memblk) * aaa.memblks);
				else
					aaa.mem=malloc(sizeof(struct memblk) * aaa.memblks);

				if (!aaa.mem) {
					perror("malloc");
					exit(1);
				}

				x=&aaa.mem[aaa.memblks - 1];

				x->memory=malloc(sizeof(anyval) * mess_desc.size);
				x->size=mess_desc.size;
				x->destroy=NULL;
				
				x->mode=MEM_READ;
				if (mess_desc.type != BFMT_RODATA)
					x->mode|=MEM_WRITE;

				if (!x->memory) {
					perror("malloc");
					exit(1);
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
			case BFMT_RELOC:
				while(mess_desc.size > 0) { fgetc(in); mess_desc.size--; } break;
			case BFMT_EOF:
				eof=1;
				break;
		}
	}
	
	if (!eof) {
		fprintf(stderr, "<!> Unterminated image (no EOF tag)\n");
		exit(1);
	}

	/* GASP... well at least we've got the file loaded now... */
	if (validate_bcode(&aaa)) {
		fprintf(stderr, "<!> Image's bytecode is corrupt/evil\n");
		exit(1);
	}

	while(1) {
	/* How To Handle Exceptions(tm):
	 * 1. throw_except returns here.
	 * 2. No xip for this level? Pop the stack until one appears.
	 * 3. If we have an xip, set new IP there, and zero old xip.
	 * 4. Out of stack? It didn't handle it, then.
	 */
		if((xp=setjmp(aaa.onexcept))) {
			while (!aaa.xip) {
				if(setjmp(aaa.onexcept))
				{
					fprintf(stderr, "Unhandled exception %d. Dying...\n", xp);
					exit(1);
				}
#ifndef NONREENTRANT
	pop_ip_from_stack_mt(&aaa, 1);
#else
	pop_ip_from_stack_st(1);
#endif
			}
			aaa.ip=aaa.xip;
			aaa.xip=0;
		}
		do_cycle(&aaa);
	}
	exit(1);
}
