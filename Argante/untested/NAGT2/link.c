/*
 * "link" - AOSr2 Image Static Linker
 * Copyright (c) 2001	James Kehl <ecks@optusnet.com.au>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "bformat.h"
#include "lang2.h"

#include "blobstak.h"

#define blobstak_pkt_data(a) ((struct data_pkt *) blobstak_data(a))
#define blobstak_pkt_size(a) (blobstak_size(a) / sizeof(struct data_pkt))
#define blobstak_pkt_add(a, x) blobstak_add(a, x, sizeof(struct data_pkt))

#define blobstak_bcode_data(a) ((struct _bcode_op *) blobstak_data(a))
#define blobstak_bcode_size(a) (blobstak_size(a) / sizeof(struct _bcode_op))
#define blobstak_bcode_add(a, x) blobstak_add(a, x, sizeof(struct _bcode_op))

#include "nagt.h"

symbol *curr_symbol;

static blobstak *data_segment, code_segment;
static int data_segments=0, curr_data_base=0, curr_data_segment=0;

#define dataseg_page(a) (data_segment[a / MAX_BLKSIZ])
#define dataseg_offs(a) (a % MAX_BLKSIZ)
#define dataseg_now (data_segment[curr_data_segment])

static FILE *in, *out_pure;
static struct progspec pspec;


