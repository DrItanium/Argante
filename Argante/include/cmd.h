/*

   Argante virtual OS
   ------------------

   Functions for binary code interpreter.

   Status: optimalization ongoing

   Author:     Mariusz Woloszyn (Kil3r) <kil3r@dione.ids.pl>
   Maintainer: Mariusz Woloszyn (Kil3r) <kil3r@dione.ids.pl>

*/

extern void cmd_invalid (int c);

extern void cmd_nop (int c);

extern void cmd_mov_ureg_immediate (int c);
extern void cmd_mov_ureg_ureg (int c);
extern void cmd_mov_ureg_sreg (int c);
extern void cmd_mov_ureg_freg (int c);
extern void cmd_mov_ureg_immptr (int c);
extern void cmd_mov_ureg_uptr (int c);

extern void cmd_mov_sreg_immediate (int c);
extern void cmd_mov_sreg_ureg (int c);
extern void cmd_mov_sreg_sreg (int c);
extern void cmd_mov_sreg_freg (int c);
extern void cmd_mov_sreg_immptr (int c);
extern void cmd_mov_sreg_uptr (int c);

extern void cmd_mov_freg_immediate (int c);
extern void cmd_mov_freg_ureg (int c);
extern void cmd_mov_freg_sreg (int c);
extern void cmd_mov_freg_freg (int c);
extern void cmd_mov_freg_immptr (int c);
extern void cmd_mov_freg_uptr (int c);

extern void cmd_mov_immptr_immediate (int c);
extern void cmd_mov_immptr_ureg (int c);
extern void cmd_mov_immptr_sreg (int c);
extern void cmd_mov_immptr_freg (int c);
extern void cmd_mov_immptr_immptr (int c);
extern void cmd_mov_immptr_uptr (int c);

extern void cmd_mov_uptr_immediate (int c);
extern void cmd_mov_uptr_ureg (int c);
extern void cmd_mov_uptr_sreg (int c);
extern void cmd_mov_uptr_freg (int c);
extern void cmd_mov_uptr_immptr (int c);
extern void cmd_mov_uptr_uptr (int c);

extern void cmd_add_ureg_immediate (int c);
extern void cmd_add_ureg_ureg (int c);
extern void cmd_add_ureg_sreg (int c);
extern void cmd_add_ureg_freg (int c);
extern void cmd_add_ureg_immptr (int c);
extern void cmd_add_ureg_uptr (int c);

extern void cmd_add_sreg_immediate (int c);
extern void cmd_add_sreg_ureg (int c);
extern void cmd_add_sreg_sreg (int c);
extern void cmd_add_sreg_freg (int c);
extern void cmd_add_sreg_immptr (int c);
extern void cmd_add_sreg_uptr (int c);

extern void cmd_add_immptr_immediate (int c);
extern void cmd_add_immptr_ureg (int c);
extern void cmd_add_immptr_sreg (int c);
extern void cmd_add_immptr_freg (int c);
extern void cmd_add_immptr_immptr (int c);
extern void cmd_add_immptr_uptr (int c);

extern void cmd_add_uptr_immediate (int c);
extern void cmd_add_uptr_ureg (int c);
extern void cmd_add_uptr_sreg (int c);
extern void cmd_add_uptr_freg (int c);
extern void cmd_add_uptr_immptr (int c);
extern void cmd_add_uptr_uptr (int c);

extern void cmd_add_freg_immediate (int c);
extern void cmd_add_freg_ureg (int c);
extern void cmd_add_freg_sreg (int c);
extern void cmd_add_freg_freg (int c);
extern void cmd_add_freg_immptr (int c);
extern void cmd_add_freg_uptr (int c);

extern void cmd_sub_ureg_immediate (int c);
extern void cmd_sub_ureg_ureg (int c);
extern void cmd_sub_ureg_sreg (int c);
extern void cmd_sub_ureg_freg (int c);
extern void cmd_sub_ureg_immptr (int c);
extern void cmd_sub_ureg_uptr (int c);

extern void cmd_sub_sreg_immediate (int c);
extern void cmd_sub_sreg_ureg (int c);
extern void cmd_sub_sreg_sreg (int c);
extern void cmd_sub_sreg_freg (int c);
extern void cmd_sub_sreg_immptr (int c);
extern void cmd_sub_sreg_uptr (int c);

extern void cmd_sub_immptr_immediate (int c);
extern void cmd_sub_immptr_ureg (int c);
extern void cmd_sub_immptr_sreg (int c);
extern void cmd_sub_immptr_freg (int c);
extern void cmd_sub_immptr_immptr (int c);
extern void cmd_sub_immptr_uptr (int c);

extern void cmd_sub_uptr_immediate (int c);
extern void cmd_sub_uptr_ureg (int c);
extern void cmd_sub_uptr_sreg (int c);
extern void cmd_sub_uptr_freg (int c);
extern void cmd_sub_uptr_immptr (int c);
extern void cmd_sub_uptr_uptr (int c);

extern void cmd_sub_freg (int c);

extern void cmd_mul_ureg (int c);
extern void cmd_mul_sreg (int c);
extern void cmd_mul_immptr (int c);
extern void cmd_mul_uptr (int c);
extern void cmd_mul_freg (int c);

extern void cmd_xor_ureg (int c);
extern void cmd_xor_sreg (int c);
extern void cmd_xor_immptr (int c);
extern void cmd_xor_uptr (int c);

extern void cmd_or_ureg (int c);
extern void cmd_or_sreg (int c);
extern void cmd_or_immptr (int c);
extern void cmd_or_uptr (int c);

extern void cmd_and_ureg (int c);
extern void cmd_and_sreg (int c);
extern void cmd_and_immptr (int c);
extern void cmd_and_uptr (int c);

extern void cmd_div_ureg (int c);
extern void cmd_div_sreg (int c);
extern void cmd_div_immptr (int c);
extern void cmd_div_uptr (int c);
extern void cmd_div_freg (int c);

extern void cmd_mod_ureg (int c);
extern void cmd_mod_sreg (int c);
extern void cmd_mod_immptr (int c);
extern void cmd_mod_uptr (int c);
extern void cmd_mod_freg (int c);

extern void cmd_jmp_immediate (int c);
extern void cmd_jmp_ureg (int c);
extern void cmd_jmp_immptr (int c);
extern void cmd_jmp_uptr (int c);

extern void cmd_call_immediate (int c);
extern void cmd_call_ureg (int c);
extern void cmd_call_immptr (int c);
extern void cmd_call_uptr (int c);

extern void cmd_loop_immediate (int c);
extern void cmd_loop_ureg (int c);
extern void cmd_loop_immptr (int c);
extern void cmd_loop_uptr (int c);

extern void cmd_onfail_immediate (int c);
extern void cmd_onfail_ureg (int c);
extern void cmd_onfail_immptr (int c);
extern void cmd_onfail_uptr (int c);

extern void cmd_syscall_immediate (int c);
extern void cmd_syscall_ureg (int c);
extern void cmd_syscall_immptr (int c);
extern void cmd_syscall_uptr (int c);

extern void cmd_ret_immediate (int c);
extern void cmd_ret_ureg (int c);
extern void cmd_ret_immptr (int c);
extern void cmd_ret_uptr (int c);

extern void cmd_raise_immediate (int c);
extern void cmd_raise_ureg (int c);
extern void cmd_raise_immptr (int c);
extern void cmd_raise_uptr (int c);

extern void cmd_ifeq_immediate (int c);
extern void cmd_ifeq_ureg (int c);
extern void cmd_ifeq_sreg (int c);
extern void cmd_ifeq_freg (int c);
extern void cmd_ifeq_immptr (int c);
extern void cmd_ifeq_uptr (int c);

extern void cmd_ifneq_immediate (int c);
extern void cmd_ifneq_ureg (int c);
extern void cmd_ifneq_sreg (int c);
extern void cmd_ifneq_freg (int c);
extern void cmd_ifneq_immptr (int c);
extern void cmd_ifneq_uptr (int c);

extern void cmd_ifabo_immediate (int c);
extern void cmd_ifabo_ureg (int c);
extern void cmd_ifabo_sreg (int c);
extern void cmd_ifabo_freg (int c);
extern void cmd_ifabo_immptr (int c);
extern void cmd_ifabo_uptr (int c);

extern void cmd_ifbel_immediate (int c);
extern void cmd_ifbel_ureg (int c);
extern void cmd_ifbel_sreg (int c);
extern void cmd_ifbel_freg (int c);
extern void cmd_ifbel_immptr (int c);
extern void cmd_ifbel_uptr (int c);

extern void cmd_halt (int c);

extern void cmd_nofail (int c);

extern void cmd_not_ureg (int c);
extern void cmd_not_sreg (int c);
extern void cmd_not_immptr (int c);
extern void cmd_not_uptr (int c);

extern void cmd_sleepfor_immediate (int c);
extern void cmd_sleepfor_ureg (int c);
extern void cmd_sleepfor_sreg (int c);
extern void cmd_sleepfor_freg (int c);
extern void cmd_sleepfor_immptr (int c);
extern void cmd_sleepfor_uptr (int c);

extern void cmd_waittill_immediate (int c);
extern void cmd_waittill_ureg (int c);
extern void cmd_waittill_sreg (int c);
extern void cmd_waittill_freg (int c);
extern void cmd_waittill_immptr (int c);
extern void cmd_waittill_uptr (int c);

extern void cmd_ldb_ureg (int c);
extern void cmd_ldb_sreg (int c);
extern void cmd_ldb_freg (int c);
extern void cmd_ldb_immptr (int c);
extern void cmd_ldb_uptr (int c);

extern void cmd_stob_ureg (int c);
extern void cmd_stob_sreg (int c);
extern void cmd_stob_immptr (int c);
extern void cmd_stob_uptr (int c);

extern void cmd_alloc_immediate (int c);
extern void cmd_alloc_ureg (int c);
extern void cmd_alloc_sreg (int c);
extern void cmd_alloc_immptr (int c);
extern void cmd_alloc_uptr (int c);

extern void cmd_realloc_immediate (int c);
extern void cmd_realloc_ureg (int c);
extern void cmd_realloc_sreg (int c);
extern void cmd_realloc_immptr (int c);
extern void cmd_realloc_uptr (int c);

extern void cmd_dealloc_immediate (int c);
extern void cmd_dealloc_ureg (int c);
extern void cmd_dealloc_sreg (int c);
extern void cmd_dealloc_immptr (int c);
extern void cmd_dealloc_uptr (int c);

extern void cmd_cmpcnt_immediate (int c);
extern void cmd_cmpcnt_ureg (int c);
extern void cmd_cmpcnt_sreg (int c);
extern void cmd_cmpcnt_immptr (int c);
extern void cmd_cmpcnt_uptr (int c);

extern void cmd_cpcnt_immediate (int c);
extern void cmd_cpcnt_ureg (int c);
extern void cmd_cpcnt_sreg (int c);
extern void cmd_cpcnt_immptr (int c);
extern void cmd_cpcnt_uptr (int c);

extern void cmd_setstack_immediate (int c);
extern void cmd_setstack_ureg (int c);
extern void cmd_setstack_immptr (int c);
extern void cmd_setstack_uptr (int c);

extern void cmd_push_immediate (int c);
extern void cmd_push_ureg (int c);
extern void cmd_push_sreg (int c);
extern void cmd_push_freg (int c);
extern void cmd_push_immptr (int c);
extern void cmd_push_uptr (int c);

extern void cmd_pop_ureg (int c);
extern void cmd_pop_sreg (int c);
extern void cmd_pop_freg (int c);
extern void cmd_pop_immptr (int c);
extern void cmd_pop_uptr (int c);

