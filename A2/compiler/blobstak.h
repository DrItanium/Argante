/*
 * "Blobstak" - Poor man's obstacks
 * Copyright (c) 2001	James Kehl <ecks@optusnet.com.au>
 *
 * Please see COPYING and blobstak.c for licence/warranty
 * details.
 */

struct _blobstak {
	size_t size;
	size_t alloc_size;
	void *data;
};
typedef struct _blobstak blobstak;

extern void blobstak_init(blobstak *n);
extern void blobstak_free(blobstak *n);
extern int blobstak_add(blobstak *n, void *data, size_t size);
#define blobstak_data(a) (a.data)
#define blobstak_size(a) (a.size)
