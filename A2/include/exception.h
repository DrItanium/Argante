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
#define ERR_NOPERM		0xc /* HAC perm */
#define ERR_RESULT_TOOLONG	0xd
#define ERR_BAD_FD		0xe
#define ERR_TOOMANY_FDS		0xf
#define ERR_FILE_EXISTS		0x10
#define ERR_FILE_NOT_EXIST	0x11
#define ERR_ARG_TOOLONG		0x12
#define ERR_INTERNAL		0x13
#define ERR_MATHERROR		0x14

/* Syscall exceptions start at 0x1000 */
#define ERR_ALIB_FAIL		0x1000
#define ERR_ALIB_NOSYM		0x1001
#define ERR_STRFD_BOUNDS	0x1010
#define ERR_STRFD_SEARCHFAIL	0x1011


#define X_CPUSHUTDOWN		((unsigned) -1)

extern void throw_except(struct vcpu *curr_cpu, int except)
#ifdef __GNUC__
 __attribute__ ((noreturn))
#endif
;

