/*

   Argante virtual OS release 2
   ----------------------------

   Common FileDescriptor Operations

   Status: done

   Author:      James Kehl <ecks@optusnet.com.au>
*/

#ifndef _HAVE_CFD
#define _HAVE_CFD

#include "taskman.h"

#define A2_CFDDESC_LEN 4
#define A2_CFDDESC_NONE "\x00\x00\x00\x00"

#include "flexsock.h"

typedef void cfdop_close_fd (struct vcpu *curr_cpu, void *vfd);
/* for agents to create VFDs. */
typedef int cfdop_create_fd (struct vcpu *curr_cpu, const char *desc, flexsock io);
/* returns "Block size" - maximum size to read/write at once. */
typedef int cfdop_start (struct vcpu *curr_cpu, void *vfd);
/* returns bytes read/written */
typedef int cfdop_write_block (struct vcpu *curr_cpu, void *vfd, const char *buf, int size);
typedef int cfdop_read_block (struct vcpu *curr_cpu, void *vfd, char *buf, int size);

/* CFD operations table version 1.01 */
struct cfdop_1 {
	cfdop_start		*read_start;
	cfdop_start		*write_start;
	cfdop_read_block	*read_block;
	cfdop_write_block	*write_block;
	cfdop_close_fd		*fd_close;
	char fd_desc[A2_CFDDESC_LEN];	/* Unique endian-independant code (ie a string) for
					   accepting agent VFD's. Only used for fd_create. */
	cfdop_create_fd		*fd_create;
};

/* syscall.c */
extern void cfdop1_lid_set(unsigned lid, const struct cfdop_1 *a); /* Get/Set by lid */
extern const struct cfdop_1 *cfdop1_lid_get(unsigned lid);
extern const struct cfdop_1 *cfdop1_fddesc_get(char fddesc[A2_CFDDESC_LEN]); /* Get by fd_desc */
/* vfd.c */
extern const struct cfdop_1 *cfdop1_vfd_get(struct vcpu *curr_cpu, unsigned handle); /* Get for a vfd */
#endif
