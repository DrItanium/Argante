/*

   Argante virtual OS release 2
   ----------------------------

   Bytecode constants

   Status: undone

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

extern void push_ip_on_stack_mt(struct vcpu *curr_cpu);
extern void pop_ip_from_stack_mt(struct vcpu *curr_cpu, unsigned count);

extern void push_ip_on_stack_st();
extern void pop_ip_from_stack_st(unsigned count);

/* Don't get confused and use the real syscall()... */
extern void agt_syscall(struct vcpu *curr_cpu, unsigned callno, anyval *arg);
extern void do_cycle(struct vcpu *curr_cpu);
extern int validate_bcode(struct vcpu *curr_cpu);

//printk
//syscall
