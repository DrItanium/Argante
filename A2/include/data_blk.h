/*
 * A2 Virtual Machine - data segment format
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

/* This is for endian swapping. */
#define DTYPE_UNSIGNED	0x01
#define DTYPE_SIGNED	0x02
#define DTYPE_FLOAT	0x03
#define DTYPE_STRING	0x04

#ifdef __BORLANDC__
#pragma option -a1
#define __attribute__(a)
#endif

struct data_pkt {
	union {
		unsigned long du_int;
		signed long ds_int;
		a2float df_float;
		char dt_string[SIZEOF_LONG];
	} u;
	char type;
} __attribute__ ((packed));

#ifdef __BORLANDC__
#pragma option -a.
#undef __attribute__
#endif
