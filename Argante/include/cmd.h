/*

   Argante virtual OS
   ------------------

   Functions for binary code interpreter.

   Status: optimalization ongoing

   Author:     Mariusz Woloszyn (Kil3r) <kil3r@dione.ids.pl>
   Maintainer: Mariusz Woloszyn (Kil3r) <kil3r@dione.ids.pl>

*/

extern void cmd_invalid ();

extern void cmd_nop ();

extern void cmd_mov_ureg_immediate ();
extern void cmd_mov_ureg_ureg ();
extern void cmd_mov_ureg_sreg ();
extern void cmd_mov_ureg_freg ();
extern void cmd_mov_ureg_immptr ();
extern void cmd_mov_ureg_uptr ();

extern void cmd_mov_sreg_immediate ();
extern void cmd_mov_sreg_ureg ();
extern void cmd_mov_sreg_sreg ();
extern void cmd_mov_sreg_freg ();
extern void cmd_mov_sreg_immptr ();
extern void cmd_mov_sreg_uptr ();

extern void cmd_mov_freg_immediate ();
extern void cmd_mov_freg_ureg ();
extern void cmd_mov_freg_sreg ();
extern void cmd_mov_freg_freg ();
extern void cmd_mov_freg_immptr ();
extern void cmd_mov_freg_uptr ();

extern void cmd_mov_immptr_immediate ();
extern void cmd_mov_immptr_ureg ();
extern void cmd_mov_immptr_sreg ();
extern void cmd_mov_immptr_freg ();
extern void cmd_mov_immptr_immptr ();
extern void cmd_mov_immptr_uptr ();

extern void cmd_mov_uptr_immediate ();
extern void cmd_mov_uptr_ureg ();
extern void cmd_mov_uptr_sreg ();
extern void cmd_mov_uptr_freg ();
extern void cmd_mov_uptr_immptr ();
extern void cmd_mov_uptr_uptr ();

extern void cmd_add_ureg_immediate ();
extern void cmd_add_ureg_ureg ();
extern void cmd_add_ureg_sreg ();
extern void cmd_add_ureg_freg ();
extern void cmd_add_ureg_immptr ();
extern void cmd_add_ureg_uptr ();

extern void cmd_add_sreg_immediate ();
extern void cmd_add_sreg_ureg ();
extern void cmd_add_sreg_sreg ();
extern void cmd_add_sreg_freg ();
extern void cmd_add_sreg_immptr ();
extern void cmd_add_sreg_uptr ();

extern void cmd_add_immptr_immediate ();
extern void cmd_add_immptr_ureg ();
extern void cmd_add_immptr_sreg ();
extern void cmd_add_immptr_freg ();
extern void cmd_add_immptr_immptr ();
extern void cmd_add_immptr_uptr ();

extern void cmd_add_uptr_immediate ();
extern void cmd_add_uptr_ureg ();
extern void cmd_add_uptr_sreg ();
extern void cmd_add_uptr_freg ();
extern void cmd_add_uptr_immptr ();
extern void cmd_add_uptr_uptr ();

extern void cmd_add_freg_immediate ();
extern void cmd_add_freg_ureg ();
extern void cmd_add_freg_sreg ();
extern void cmd_add_freg_freg ();
extern void cmd_add_freg_immptr ();
extern void cmd_add_freg_uptr ();

extern void cmd_sub_ureg_immediate ();
extern void cmd_sub_ureg_ureg ();
extern void cmd_sub_ureg_sreg ();
extern void cmd_sub_ureg_freg ();
extern void cmd_sub_ureg_immptr ();
extern void cmd_sub_ureg_uptr ();

extern void cmd_sub_sreg_immediate ();
extern void cmd_sub_sreg_ureg ();
extern void cmd_sub_sreg_sreg ();
extern void cmd_sub_sreg_freg ();
extern void cmd_sub_sreg_immptr ();
extern void cmd_sub_sreg_uptr ();

extern void cmd_sub_immptr_immediate ();
extern void cmd_sub_immptr_ureg ();
extern void cmd_sub_immptr_sreg ();
extern void cmd_sub_immptr_freg ();
extern void cmd_sub_immptr_immptr ();
extern void cmd_sub_immptr_uptr ();

extern void cmd_sub_uptr_immediate ();
extern void cmd_sub_uptr_ureg ();
extern void cmd_sub_uptr_sreg ();
extern void cmd_sub_uptr_freg ();
extern void cmd_sub_uptr_immptr ();
extern void cmd_sub_uptr_uptr ();

extern void cmd_sub_freg ();

extern void cmd_mul_ureg ();
extern void cmd_mul_sreg ();
extern void cmd_mul_immptr ();
extern void cmd_mul_uptr ();
extern void cmd_mul_freg ();

extern void cmd_xor_ureg ();
extern void cmd_xor_sreg ();
extern void cmd_xor_immptr ();
extern void cmd_xor_uptr ();

extern void cmd_or_ureg ();
extern void cmd_or_sreg ();
extern void cmd_or_immptr ();
extern void cmd_or_uptr ();

extern void cmd_and_ureg ();
extern void cmd_and_sreg ();
extern void cmd_and_immptr ();
extern void cmd_and_uptr ();

extern void cmd_div_ureg ();
extern void cmd_div_sreg ();
extern void cmd_div_immptr ();
extern void cmd_div_uptr ();
extern void cmd_div_freg ();

extern void cmd_mod_ureg ();
extern void cmd_mod_sreg ();
extern void cmd_mod_immptr ();
extern void cmd_mod_uptr ();
extern void cmd_mod_freg ();

extern void cmd_jmp_immediate ();
extern void cmd_jmp_ureg ();
extern void cmd_jmp_immptr ();
extern void cmd_jmp_uptr ();

extern void cmd_call_immediate ();
extern void cmd_call_ureg ();
extern void cmd_call_immptr ();
extern void cmd_call_uptr ();

extern void cmd_loop_immediate ();
extern void cmd_loop_ureg ();
extern void cmd_loop_immptr ();
extern void cmd_loop_uptr ();

extern void cmd_onfail_immediate ();
extern void cmd_onfail_ureg ();
extern void cmd_onfail_immptr ();
extern void cmd_onfail_uptr ();

extern void cmd_syscall_immediate ();
extern void cmd_syscall_ureg ();
extern void cmd_syscall_immptr ();
extern void cmd_syscall_uptr ();

extern void cmd_ret_immediate ();
extern void cmd_ret_ureg ();
extern void cmd_ret_immptr ();
extern void cmd_ret_uptr ();

extern void cmd_raise_immediate ();
extern void cmd_raise_ureg ();
extern void cmd_raise_immptr ();
extern void cmd_raise_uptr ();

extern void cmd_ifeq_immediate ();
extern void cmd_ifeq_ureg ();
extern void cmd_ifeq_sreg ();
extern void cmd_ifeq_freg ();
extern void cmd_ifeq_immptr ();
extern void cmd_ifeq_uptr ();

extern void cmd_ifneq_immediate ();
extern void cmd_ifneq_ureg ();
extern void cmd_ifneq_sreg ();
extern void cmd_ifneq_freg ();
extern void cmd_ifneq_immptr ();
extern void cmd_ifneq_uptr ();

extern void cmd_ifabo_immediate ();
extern void cmd_ifabo_ureg ();
extern void cmd_ifabo_sreg ();
extern void cmd_ifabo_freg ();
extern void cmd_ifabo_immptr ();
extern void cmd_ifabo_uptr ();

extern void cmd_ifbel_immediate ();
extern void cmd_ifbel_ureg ();
extern void cmd_ifbel_sreg ();
extern void cmd_ifbel_freg ();
extern void cmd_ifbel_immptr ();
extern void cmd_ifbel_uptr ();

extern void cmd_halt ();

extern void cmd_nofail ();

extern void cmd_not_ureg ();
extern void cmd_not_sreg ();
extern void cmd_not_immptr ();
extern void cmd_not_uptr ();

extern void cmd_sleepfor_immediate ();
extern void cmd_sleepfor_ureg ();
extern void cmd_sleepfor_sreg ();
extern void cmd_sleepfor_freg ();
extern void cmd_sleepfor_immptr ();
extern void cmd_sleepfor_uptr ();

extern void cmd_waittill_immediate ();
extern void cmd_waittill_ureg ();
extern void cmd_waittill_sreg ();
extern void cmd_waittill_freg ();
extern void cmd_waittill_immptr ();
extern void cmd_waittill_uptr ();

extern void cmd_ldb_ureg ();
extern void cmd_ldb_sreg ();
extern void cmd_ldb_freg ();
extern void cmd_ldb_immptr ();
extern void cmd_ldb_uptr ();

extern void cmd_stob_ureg ();
extern void cmd_stob_sreg ();
extern void cmd_stob_immptr ();
extern void cmd_stob_uptr ();

extern void cmd_alloc_immediate ();
extern void cmd_alloc_ureg ();
extern void cmd_alloc_sreg ();
extern void cmd_alloc_immptr ();
extern void cmd_alloc_uptr ();

extern void cmd_realloc_immediate ();
extern void cmd_realloc_ureg ();
extern void cmd_realloc_sreg ();
extern void cmd_realloc_immptr ();
extern void cmd_realloc_uptr ();

extern void cmd_dealloc_immediate ();
extern void cmd_dealloc_ureg ();
extern void cmd_dealloc_sreg ();
extern void cmd_dealloc_immptr ();
extern void cmd_dealloc_uptr ();

extern void cmd_cmpcnt_immediate ();
extern void cmd_cmpcnt_ureg ();
extern void cmd_cmpcnt_sreg ();
extern void cmd_cmpcnt_immptr ();
extern void cmd_cmpcnt_uptr ();

extern void cmd_cpcnt_immediate ();
extern void cmd_cpcnt_ureg ();
extern void cmd_cpcnt_sreg ();
extern void cmd_cpcnt_immptr ();
extern void cmd_cpcnt_uptr ();

extern void cmd_setstack_immediate ();
extern void cmd_setstack_ureg ();
extern void cmd_setstack_immptr ();
extern void cmd_setstack_uptr ();

extern void cmd_push_immediate ();
extern void cmd_push_ureg ();
extern void cmd_push_sreg ();
extern void cmd_push_freg ();
extern void cmd_push_immptr ();
extern void cmd_push_uptr ();

extern void cmd_pop_ureg ();
extern void cmd_pop_sreg ();
extern void cmd_pop_freg ();
extern void cmd_pop_immptr ();
extern void cmd_pop_uptr ();

