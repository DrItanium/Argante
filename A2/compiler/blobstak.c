/*
 * "Blobstak" - Poor man's obstacks
 * Copyright (c) 2001	James Kehl <ecks@optusnet.com.au>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
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

#include "autocfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blobstak.h"

#define BLOBSTAK_CHUNK 4096

void blobstak_init(blobstak *n)
{
	n->size=n->alloc_size=0;
	n->data=NULL;
}

void blobstak_free(blobstak *n)
{
	if (!n->alloc_size) return;

	free(n->data);
}

int blobstak_add(blobstak *n, void *data, size_t size)
{
	size_t news, nas;
	void *newdata;

	news=n->size+size;
	
	/* Widen blobstak if needed */
	nas=n->alloc_size;
	while (news >= nas) nas+=BLOBSTAK_CHUNK;

	if (nas != n->alloc_size)
	{
		if (n->data)
			newdata=realloc(n->data, nas);
		else
			newdata=malloc(nas);
		
		if (newdata)
		{
			n->data=newdata;
			n->alloc_size=nas;
		} else {
			perror("blobstak_add malloc");
			return 1;
		}
	}
	/* Copy in data */
	memcpy((char *) n->data + n->size, data, size);
	n->size=news;

	return 0;
}

