/*
 * Proposed Argante V2 data segment format.
 * James Kehl, 19/06/01.
 *
 * All copyright + warranty disclaimed, ON THIS FILE ONLY.
 * So you can use this under LGPL.
 */

struct data_hdr {
	unsigned int size;	/* Note - this is the dwords that should be allocated for this block.
				   NOT the number of dwords given. So, you can 'compress' trailing zeroes. */
};

/* This is for endian swapping. */
#define DTYPE_UNSIGNED	0x01
#define DTYPE_SIGNED	0x02
#define DTYPE_FLOAT	0x03
#define DTYPE_STRING	0x04

struct data_pkt {
	union {
		unsigned du_int;
		signed ds_int;
		float df_float;
		char dt_string[4];
	} u;
	char type;
} __attribute__ ((packed));
