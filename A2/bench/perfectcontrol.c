/*
 * IN CASE YOU WERE WONDERING:
 * This is a hand-tuned version of the control benchmark,
 * originally produced by cify. Effectively, it evaluates
 * the speed limit for cify's output. (No matter how clever
 * cify gets it can't beat this.)
 *
 * James K. 15/8/01.
 */
#include "autocfg.h"
#include "compat/bzero.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "taskman.h"
#include "printk.h"
#include "bcode.h"
#include "hhac.h"
#include "amemory.h"
#include "modload.h"
#include "exception.h"
/* HAC from ../conf/bench-HAC */
#define ADDHAC_ERROR "add_hac_i failed: DIR: %s ATYPE: %s TYPE: %s"
static void fixed_hac(struct vcpu *cpu) {
	if (add_hac_entry(cpu, "display/", "/write/", PERM_ALLOW))
		fprintf(stderr, ADDHAC_ERROR, "display/", "/write/", "PERM_ALLOW");
	if (add_hac_entry(cpu, "localsys/", "/time/get", PERM_ALLOW))
		fprintf(stderr, ADDHAC_ERROR, "localsys/", "/time/get", "PERM_ALLOW");
	if (add_hac_entry(cpu, "fs/", "/", PERM_ALLOW))
		fprintf(stderr, ADDHAC_ERROR, "fs/", "/", "PERM_ALLOW");
	if (add_hac_entry(cpu, "localsys/", "/time/tostr", PERM_ALLOW))
		fprintf(stderr, ADDHAC_ERROR, "localsys/", "/time/tostr", "PERM_ALLOW");
}
static void fixed_load_code(struct vcpu *cpu) {
	bzero(cpu, sizeof(struct vcpu));
	strncpy(cpu->pname, "Flow control benchmark", A2_MAX_PNAME);
	cpu->priority=10000;
	cpu->domain=0;
	cpu->dlist=malloc(1 * sizeof(unsigned));
	cpu->dlist[0]=0;
/* Load data segments */
	cpu->memblks=3;
	cpu->mem=calloc(3, sizeof(struct memblk));
	cpu->mem[0].mode=A2_MEM_READ|A2_MEM_WRITE;
	cpu->mem[0].size=1;
	cpu->mem[0].memory=malloc(1 * sizeof(anyval));
	memcpy(cpu->mem[0].memory,
"\x0a\x00\x00\x00", 1 * sizeof(anyval));
	cpu->mem[1].mode=A2_MEM_READ|A2_MEM_WRITE;
	cpu->mem[1].size=7;
	cpu->mem[1].memory=malloc(7 * sizeof(anyval));
	memcpy(cpu->mem[1].memory,
"\x46\x75\x6e\x63\x74\x69\x6f\x6e\x20\x63\x61\x6c\x6c\x73\x20\x2f\x20\x73\x65\x63\x6f\x6e\x64\x3a\x20\x00\x00\x00", 7 * sizeof(anyval));
	cpu->mem[2].mode=A2_MEM_READ|A2_MEM_WRITE;
	cpu->mem[2].size=4;
	cpu->mem[2].memory=malloc(4 * sizeof(anyval));
	memcpy(cpu->mem[2].memory,
"\x4c\x6f\x6f\x70\x73\x20\x2f\x20\x73\x65\x63\x6f\x6e\x64\x3a\x20", 4 * sizeof(anyval));
/* End */
}

#define PUSH_STACK(a) if(cpu->mstack_ptr >= cpu->mstack_size) \
throw_except(cpu, ERR_MSTACK_OVER); \
a=mem_rw(cpu, cpu->mstack_ptr + cpu->mstack); \
cpu->mstack_ptr++
#define POP_STACK(a) if(cpu->mstack_ptr == 0) \
throw_except(cpu, ERR_MSTACK_UNDER); \
cpu->mstack_ptr--; \
a=mem_ro(cpu, cpu->mstack_ptr + cpu->mstack)

static int do_fixed_bcode(struct vcpu *cpu) {
	unsigned xp, origip;
	anyval *a1, *a2;
/* X-handler */
	if((xp=setjmp(cpu->onexcept))) {
	if (xp==X_CPUSHUTDOWN) {
	printk2(PRINTK_WARN, "Task committed suicide (HALT or RAISE -1).\n");
	return 0;
	}
	origip=cpu->ip;
	while (!cpu->xip) {
	if(setjmp(cpu->onexcept))
	{
	fprintf(stderr, "<+> Unhandled exception %d, origin 0x%x. Murdered.\n", xp, origip);
	return 1;
	}
	pop_ip_from_stack(cpu, 1);
	}
	cpu->ip=cpu->xip;
	cpu->xip=0;
	}
/* Code Handler */

	while(1) {
	switch(cpu->ip) {
	case 0: sS0:
		cpu->reg[0].val.u=A2_MAX_BLKSIZ;
		cpu->ip++;
	case 1: sS1:
		cpu->reg[1].val.u=28u;
		cpu->ip++;
	case 2: sS2:
		cpu->next_ip=cpu->ip + 1;
		agt_syscall(cpu, 1, NULL);
		cpu->ip=cpu->next_ip;
		break;
	case 3: sS3:
 		cpu->reg[8].val.s=10000000u;
		cpu->ip++;
	case 4: sS4:
		cpu->reg[8].val.s-=1u;
		cpu->ip++;
	case 5: sS5:
		cpu->next_ip=cpu->ip + 1;
		agt_syscall(cpu, 301, NULL);
		cpu->ip=cpu->next_ip;
		break;
	case 6: sS6:
		cpu->reg[10].val.u=cpu->reg[0].val.u;
		cpu->ip++;
	case 7: sS7:
		cpu->reg[11].val.u=cpu->reg[1].val.u;
		cpu->ip++;
	case 8: sS8:
		cpu->next_ip=cpu->ip + 1;
		push_ip_on_stack(cpu);
		cpu->ip=36u;
		cpu->xip=0;
		goto sS36;
	case 9: sS9:
		if (cpu->reg[8].val.s > 0) {
		cpu->reg[8].val.s--;
		cpu->ip=8u;
		goto sS8;
		} else { cpu->ip++; goto sS10; }
	case 10: sS10:
		cpu->next_ip=cpu->ip + 1;
		push_ip_on_stack(cpu);
		cpu->ip=37u;
		cpu->xip=0;
		goto sS37;
	case 11: sS11:
		cpu->reg[8].val.f=10000000u;
		cpu->ip++;
	case 12: sS12:
		cpu->reg[8].val.f/=cpu->reg[0].val.f;
		cpu->ip++;
	case 13: sS13:
		cpu->next_ip=cpu->ip + 1;
		agt_syscall(cpu, 4, &cpu->reg[8]);
		cpu->ip=cpu->next_ip;
		break;
	case 14: sS14:
		cpu->reg[0].val.u=0;
		cpu->ip++;
	case 15: sS15:
		cpu->reg[1].val.u=4u;
		cpu->ip++;
	case 16: sS16:
		cpu->next_ip=cpu->ip + 1;
		agt_syscall(cpu, 1, NULL);
		cpu->ip=cpu->next_ip;
		break;
	case 17: sS17:
		cpu->reg[0].val.u=A2_MAX_BLKSIZ * 2;
		cpu->ip++;
	case 18: sS18:
		cpu->reg[1].val.u=16u;
		cpu->ip++;
	case 19: sS19:
		cpu->next_ip=cpu->ip + 1;
		agt_syscall(cpu, 1, NULL);
		cpu->ip=cpu->next_ip;
		break;
	case 20: sS20:
		cpu->reg[8].val.s=10000000u;
		cpu->ip++;
	case 21: sS21:
		cpu->reg[8].val.s*=20u;
		cpu->ip++;
	case 22: sS22:
		cpu->reg[8].val.s-=1u;
		cpu->ip++;
	case 23: sS23:
		cpu->next_ip=cpu->ip + 1;
		agt_syscall(cpu, 301, NULL);
		cpu->ip=cpu->next_ip;
		break;
	case 24: sS24:
		cpu->reg[10].val.u=cpu->reg[0].val.u;
		cpu->ip++;
	case 25: sS25:
		cpu->reg[11].val.u=cpu->reg[1].val.u;
		cpu->ip++;
	case 26: sS26:
		if (cpu->reg[8].val.s > 0) {
			cpu->reg[8].val.s--;
			cpu->ip=26u;
			goto sS26;
		} else { cpu->ip++; goto sS27; }
	case 27: sS27:
		cpu->next_ip=cpu->ip + 1;
		push_ip_on_stack(cpu);
		cpu->ip=37u;
		cpu->xip=0;
		goto sS37;
	case 28: sS28:
		cpu->reg[8].val.f=10000000u;
		cpu->ip++;
	case 29: sS29:
		cpu->reg[8].val.f*=20u;
		cpu->ip++;
	case 30: sS30:
		cpu->reg[8].val.f/=cpu->reg[0].val.f;
		cpu->ip++;
	case 31: sS31:
		cpu->next_ip=cpu->ip + 1;
		agt_syscall(cpu, 4u, &cpu->reg[8]);
		cpu->ip=cpu->next_ip;
		break;
	case 32: sS32:
		cpu->reg[0].val.u=0u;
		cpu->ip++;
	case 33: sS33:
		cpu->reg[1].val.u=4u;
		cpu->ip++;
	case 34: sS34:
		cpu->next_ip=cpu->ip + 1;
		agt_syscall(cpu, 1u, NULL);
		cpu->ip=cpu->next_ip;
		break;
	case 35: sS35:
		throw_except(cpu, X_CPUSHUTDOWN);
		break;
	case 36: sS36:
		pop_ip_from_stack(cpu, 1);
		cpu->ip=cpu->next_ip;
		break;
	case 37: sS37:
		cpu->next_ip=cpu->ip + 1;
		agt_syscall(cpu, 301u, NULL);
		cpu->ip=cpu->next_ip;
		break;
	case 38: sS38:
		cpu->reg[0].val.u-=cpu->reg[10].val.u;
		cpu->ip++;
	case 39: sS39:
		cpu->reg[1].val.s-=cpu->reg[11].val.u;
		cpu->ip++;
	case 40: sS40:
		cpu->reg[0].val.f=cpu->reg[0].val.u;
		cpu->ip++;
	case 41: sS41:
		cpu->reg[1].val.f=cpu->reg[1].val.s;
		cpu->ip++;
	case 42: sS42:
		cpu->reg[1].val.f/=1000000.000000;
		cpu->ip++;
	case 43: sS43:
		cpu->reg[0].val.f+=cpu->reg[1].val.f;
		cpu->ip++;
	case 44: sS44:
		pop_ip_from_stack(cpu, 1);
		cpu->ip=cpu->next_ip;
		break;
	default:
throw_except(cpu, ERR_OUTSIDE_CODE);
}
}
}
int main(int argc, char **argv) {
	struct vcpu cpu;
	int i;
#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif
	module_static_init();
	while(argc > 1) { argc--; module_dyn_load(argv[argc]); }
	fixed_load_code(&cpu);
	hac_init(&cpu);
	fixed_hac(&cpu);
	vcpu_modules_start(&cpu);
	i=do_fixed_bcode(&cpu);
	vcpu_modules_stop(&cpu);
	return i;
}
