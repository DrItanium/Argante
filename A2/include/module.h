/*

   Argante virtual OS release 2
   ----------------------------

   Loadable modules support

*/

#ifndef SYSCALL_ARGS
#define SYSCALL_ARGS struct vcpu *curr_cpu, const anyval *arg

typedef void syscallfunc (SYSCALL_ARGS);

/* module only funcs */
extern void *module_get_reserved(struct vcpu *cpu, unsigned lid);
extern int module_set_reserved(struct vcpu *cpu, unsigned lid, void *newdata);

#endif
