/* Autogenerated, please edit console.c */
#include "modload.h"
#ifndef STATIC
int module_init
#else
int console_init
#endif
(unsigned lid) {
	if (module_internal_init(lid)) return 1;
	return 0;
}

#ifndef STATIC
void module_vcpu_start
#else
void console_vcpu_start
#endif
(struct vcpu *cpu) { module_internal_vcpu_start(cpu); }

#ifndef STATIC
void module_vcpu_stop
#else
void console_vcpu_stop
#endif
(struct vcpu *cpu) { module_internal_vcpu_stop(cpu); }

#ifndef STATIC
void module_shutdown
#else
void console_shutdown
#endif
() {
	module_internal_shutdown();}
