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
extern void lid_assign(unsigned lid, const char *filename, const char *otherdesc,
		modulevcpufunc *start, modulevcpufunc *stop);
extern unsigned lid_getdata(unsigned lid, const char **filename, const char **otherdesc); 

/* changed 0.010 */
extern void vcpu_modules_start(struct vcpu *curr_cpu);
extern void vcpu_modules_stop(struct vcpu *curr_cpu);
extern void vcpu_module_start(struct vcpu *curr_cpu, unsigned moduleid);
extern void vcpu_module_stop(struct vcpu *curr_cpu, unsigned moduleid);

