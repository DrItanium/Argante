/*

   Argante virtual OS release 2
   ----------------------------

   HAC control

   Status: completed

   Author:     James Kehl <ecks@optusnet.com.au>

*/
#include <stdio.h>

extern int validate_access(struct vcpu *curr_cpu, char *dir, char *atype); 
extern int hac_init(struct vcpu *cpu);
extern int hac_loadfile(struct vcpu *cpu, FILE *from);
extern int hac_unload(struct vcpu *cpu);
extern int add_hac_entry(struct vcpu *cpu, char *dir, char *atype, int amode);

#define PERM_DENY 00
#define PERM_ALLOW 01

#define VALIDATE(res,act) if (validate_access(curr_cpu,res,act)) { printk2(PRINTK_WARN, "VALIDATE failure: %s for %s\n", res, act); throw_except(curr_cpu, ERR_NOPERM); }
