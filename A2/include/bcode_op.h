/*
 * A2 Virtual Machine - Argante V2 bytecode format.
 * Copyright (c) 2001	James Kehl <ecks@optusnet.com.au>
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; version 2 of the License, with the
 * added restriction that it may only be converted to the version 2 of the
 * GNU General Public License.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _HAVE_BCODEOP
#define _HAVE_BCODEOP

#include "anyval.h"

/* Everyone's a bit sleepy? Ptr fix: 20/6/01 */
#define TYPE_UNSIGNED	000
#define TYPE_SIGNED	001
#define TYPE_FLOAT	002
#define TYPE_IMMEDIATE	000 /* It's either an IMM or a REG, innit? */
#define TYPE_REGISTER	004
#define TYPE_POINTER	010

#define TYPE_A1(a)	((a) << 0)
#define TYPE_A2(a)	((a) << 4)
#define TYPE_VALMASK	(TYPE_UNSIGNED | TYPE_SIGNED | TYPE_FLOAT)

struct bcode_op {
	unsigned char bcode;
	unsigned char type;
	short reserved; /* To make the arguments 32-aligned, for speed */
	anyval a1;
	anyval a2;
};

/* linux/stddef.h */
#define bcode_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/* (2 * sizeof(char) + sizeof(short)) */
#define OFFSET_OP_A1 bcode_offsetof(struct bcode_op, a1)
/* (2 * sizeof(char) + sizeof(short) + sizeof(anyval)) */
#define OFFSET_OP_A2 bcode_offsetof(struct bcode_op, a2)

#ifdef __BORLANDC__
/* Turn off alignment for ((packed)) stuff */
#pragma option -a1
#define __attribute__(a)
#endif

struct bcode_op_packed {
	unsigned char bcode;
	unsigned char type;
	anyval a1;
	anyval a2;
} __attribute__ ((packed));

#ifdef __BORLANDC__
#pragma option -a.
#undef __attribute__
#endif

/* (2 * sizeof(char)) */
#define OFFSET_PACK_OP_A1 bcode_offsetof(struct bcode_op_packed, a1)
/* (2 * sizeof(char) + sizeof(anyval)) */
#define OFFSET_PACK_OP_A2 bcode_offsetof(struct bcode_op_packed, a1)

#endif

