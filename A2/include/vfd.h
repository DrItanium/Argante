/*

   Argante virtual OS release 2
   ----------------------------

   Common Virtual FileDescriptor stuff

   Status: done

   Author:      James Kehl <ecks@optusnet.com.au>
*/

#ifndef _HAVE_VFD
#define _HAVE_VFD

#define A2_LID_ANY (A2_MAX_RSRVD + 42)

extern void *vfd_get_data(struct vcpu *curr_cpu, unsigned lid, unsigned handle);
extern void vfd_set_data(struct vcpu *curr_cpu, unsigned lid, unsigned handle, void *newd);
extern int vfd_alloc_new(struct vcpu *curr_cpu, unsigned lid);
extern void vfd_dealloc(struct vcpu *curr_cpu, unsigned lid, unsigned handle);
/* For vcpu_stop stuff */
extern int vfd_find_mine(struct vcpu *curr_cpu, unsigned lid);
/* Forgets ALL VFD's. Modules, don't touch. */
extern void vfd_obliviate(struct vcpu *curr_cpu);

#endif 
