/*

   Argante virtual OS 
   ------------------

   Bytecode interpreter.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#include "exception.h"

void do_cycles(int,int);

// Instruction size: 12 bytes.

#define TYPE_IMMEDIATE		0x0
#define TYPE_UREG		0x1
#define TYPE_SREG		0x2
#define TYPE_FREG		0x3
#define TYPE_IMMPTR		0x4
#define TYPE_UPTR		0x5

#define CMD_NOP			0x0
#define CMD_JMP			0x1
#define CMD_IFEQ		0x2
#define CMD_IFNEQ		0x3
#define CMD_IFABO		0x4
#define CMD_IFBEL		0x5
#define CMD_CALL		0x6
#define CMD_RET			0x7
#define CMD_HALT		0x8
#define CMD_SYSCALL		0x9
#define CMD_ADD			0xa
#define CMD_SUB			0xb
#define CMD_MUL			0xc
#define CMD_DIV			0xd
#define CMD_MOD			0xe
#define CMD_XOR			0xf
#define CMD_REV			0x10
#define CMD_NOT			0x11
#define CMD_AND			0x12
#define CMD_OR			0x13
#define CMD_MOV			0x14
#define CMD_SLEEPFOR		0x15
#define CMD_WAITTILL		0x16
#define CMD_ALLOC		0x17 // memsize; ret: 1 = chnum, 2 = memptr
#define CMD_REALLOC		0x18 // chnum, newsize
#define CMD_DEALLOC		0x19 // chnum
#define CMD_CMPCNT		0x1a // srcaddr, dstaddr, 1 = cnt, res => 1
#define CMD_CPCNT		0x1b // srcaddr, dstaddr, 1 = cnt, res => 1
#define CMD_ONFAIL		0x1c
#define CMD_NOFAIL		0x1d
#define CMD_LOOP		0x1e
#define CMD_RAISE		0x1f

#define CMD_LDB 		0x20

#define CMD_SETSTACK		0x21
#define CMD_PUSHS		0x22
#define CMD_POPS		0x23

#define CMD_STOB		0x24

// Everything with opcode higher than CMD_INVALID is invalid ;) Please
// increase CMD_INVALID if adding new features...
#define CMD_INVALID		0x25


extern void non_fatal(int,char*,int);
extern void jit_init();

#ifndef __I_AM_THE_BCODE
extern int failure;
#endif

