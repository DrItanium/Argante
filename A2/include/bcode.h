/*

   Argante virtual OS release 2
   ----------------------------

   Bytecode constants

   Status: undone

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

extern void push_ip_on_stack(struct vcpu *curr_cpu);
extern void pop_ip_from_stack(struct vcpu *curr_cpu, unsigned count);

/* Don't get confused and use the real syscall()... */
extern void agt_syscall(struct vcpu *curr_cpu, unsigned callno, const anyval *arg);

