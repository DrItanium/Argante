/*
 * Proposed Argante V2 bytecode format.
 * James Kehl, 19/06/01.
 * 
 * All copyright + warranty disclaimed, ON THIS FILE ONLY.
 * So you can use this under LGPL.
 */

/* Everyone's a bit sleepy? Ptr fix: 20/6/01 */
#define TYPE_UNSIGNED	000
#define TYPE_SIGNED	001
#define TYPE_FLOAT	002
#define TYPE_IMMEDIATE	000 /* It's either an IMM or a REG, innit? */
#define TYPE_REGISTER	004
#define TYPE_POINTER	010

#define TYPE_A1(a)	(a << 0)
#define TYPE_A2(a)	(a << 4)
#define TYPE_VALMASK	(TYPE_UNSIGNED | TYPE_SIGNED | TYPE_FLOAT)

struct _bcode_op {
	char bcode;
	char type;
	short reserved; /* To make the arguments 32-aligned, remove if you are happy with 16-align */
	long a1;
	long a2;
};

