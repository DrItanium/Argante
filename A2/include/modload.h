/*

   Argante virtual OS release 2
   ----------------------------

   Loadable modules support
   SHOULD NOT BE INCLUDED IN USER CODE!
   MANAGER + AUTOGENERATOR STUFF ONLY!

*/

#include "module.h"

/* All manager only stuff */
extern void module_static_init(void);
extern int module_dyn_load(char *a);
extern int module_dyn_unload(unsigned lid);
extern int module_dyn_reload(unsigned lid);

typedef int moduleinitfunc(unsigned lid);
typedef void modulevcpufunc(struct vcpu *cpu);
typedef void moduleshutdownfunc(void);

extern int register_syscall(unsigned id, syscallfunc *f);
extern int unregister_syscall(unsigned id);

extern unsigned lid_create(void);
extern void lid_destroy(unsigned lid);
extern void lid_assign(unsigned lid, char *name, modulevcpufunc *start, modulevcpufunc *stop);

extern void *vcpu_modules_start(void *);
extern void *vcpu_modules_stop(void *);
extern void vcpu_set_moduleid(unsigned);
extern void *vcpu_module_start(void *);
extern void *vcpu_module_stop(void *);

