/*

   Argante virtual OS release 2
   ----------------------------

   Exception numbers

   Status: blank

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#define ERR_GENERIC     	0x1
#define ERR_OUTSIDE_CODE	0x2
#define ERR_OUTSIDE_MEM		0x3
#define ERR_CORRUPT_CODE	0x4
#define ERR_PROTFAULT		0x5
#define ERR_MSTACK_OVER		0x6
#define ERR_MSTACK_UNDER	0x7
#define ERR_OOM			0x8
#define ERR_CSTACK_OVER		0x9
#define ERR_CSTACK_UNDER	0xa
#define ERR_NOSYSCALL		0xb

extern void throw_except(struct vcpu *curr_cpu, int except) __attribute__ ((noreturn));

