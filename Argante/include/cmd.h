/*

   Argante virtual OS
   ------------------

   Functions for binary code interpreter.

   Status: optimalization ongoing

   Author:     Mariusz Woloszyn (Kil3r) <kil3r@dione.ids.pl>
   Maintainer: Mariusz Woloszyn (Kil3r) <kil3r@dione.ids.pl>

*/

extern void cmd_invalid (void);

extern void cmd_nop (void);

extern void cmd_mov_ureg_immediate (void);
extern void cmd_mov_ureg_ureg (void);
extern void cmd_mov_ureg_sreg (void);
extern void cmd_mov_ureg_freg (void);
extern void cmd_mov_ureg_immptr (void);
extern void cmd_mov_ureg_uptr (void);

extern void cmd_mov_sreg_immediate (void);
extern void cmd_mov_sreg_ureg (void);
extern void cmd_mov_sreg_sreg (void);
extern void cmd_mov_sreg_freg (void);
extern void cmd_mov_sreg_immptr (void);
extern void cmd_mov_sreg_uptr (void);

extern void cmd_mov_freg_immediate (void);
extern void cmd_mov_freg_ureg (void);
extern void cmd_mov_freg_sreg (void);
extern void cmd_mov_freg_freg (void);
extern void cmd_mov_freg_immptr (void);
extern void cmd_mov_freg_uptr (void);

extern void cmd_mov_immptr_immediate (void);
extern void cmd_mov_immptr_ureg (void);
extern void cmd_mov_immptr_sreg (void);
extern void cmd_mov_immptr_freg (void);
extern void cmd_mov_immptr_immptr (void);
extern void cmd_mov_immptr_uptr (void);

extern void cmd_mov_uptr_immediate (void);
extern void cmd_mov_uptr_ureg (void);
extern void cmd_mov_uptr_sreg (void);
extern void cmd_mov_uptr_freg (void);
extern void cmd_mov_uptr_immptr (void);
extern void cmd_mov_uptr_uptr (void);

extern void cmd_add_ureg_immediate (void);
extern void cmd_add_ureg_ureg (void);
extern void cmd_add_ureg_sreg (void);
extern void cmd_add_ureg_freg (void);
extern void cmd_add_ureg_immptr (void);
extern void cmd_add_ureg_uptr (void);

extern void cmd_add_sreg_immediate (void);
extern void cmd_add_sreg_ureg (void);
extern void cmd_add_sreg_sreg (void);
extern void cmd_add_sreg_freg (void);
extern void cmd_add_sreg_immptr (void);
extern void cmd_add_sreg_uptr (void);

extern void cmd_add_immptr_immediate (void);
extern void cmd_add_immptr_ureg (void);
extern void cmd_add_immptr_sreg (void);
extern void cmd_add_immptr_freg (void);
extern void cmd_add_immptr_immptr (void);
extern void cmd_add_immptr_uptr (void);

extern void cmd_add_uptr_immediate (void);
extern void cmd_add_uptr_ureg (void);
extern void cmd_add_uptr_sreg (void);
extern void cmd_add_uptr_freg (void);
extern void cmd_add_uptr_immptr (void);
extern void cmd_add_uptr_uptr (void);

extern void cmd_add_freg_immediate (void);
extern void cmd_add_freg_ureg (void);
extern void cmd_add_freg_sreg (void);
extern void cmd_add_freg_freg (void);
extern void cmd_add_freg_immptr (void);
extern void cmd_add_freg_uptr (void);

extern void cmd_sub_ureg_immediate (void);
extern void cmd_sub_ureg_ureg (void);
extern void cmd_sub_ureg_sreg (void);
extern void cmd_sub_ureg_freg (void);
extern void cmd_sub_ureg_immptr (void);
extern void cmd_sub_ureg_uptr (void);

extern void cmd_sub_sreg_immediate (void);
extern void cmd_sub_sreg_ureg (void);
extern void cmd_sub_sreg_sreg (void);
extern void cmd_sub_sreg_freg (void);
extern void cmd_sub_sreg_immptr (void);
extern void cmd_sub_sreg_uptr (void);

extern void cmd_sub_immptr_immediate (void);
extern void cmd_sub_immptr_ureg (void);
extern void cmd_sub_immptr_sreg (void);
extern void cmd_sub_immptr_freg (void);
extern void cmd_sub_immptr_immptr (void);
extern void cmd_sub_immptr_uptr (void);

extern void cmd_sub_uptr_immediate (void);
extern void cmd_sub_uptr_ureg (void);
extern void cmd_sub_uptr_sreg (void);
extern void cmd_sub_uptr_freg (void);
extern void cmd_sub_uptr_immptr (void);
extern void cmd_sub_uptr_uptr (void);

extern void cmd_sub_freg (void);

extern void cmd_mul_ureg (void);
extern void cmd_mul_sreg (void);
extern void cmd_mul_immptr (void);
extern void cmd_mul_uptr (void);
extern void cmd_mul_freg (void);

extern void cmd_xor_ureg (void);
extern void cmd_xor_sreg (void);
extern void cmd_xor_immptr (void);
extern void cmd_xor_uptr (void);

extern void cmd_or_ureg (void);
extern void cmd_or_sreg (void);
extern void cmd_or_immptr (void);
extern void cmd_or_uptr (void);

extern void cmd_and_ureg (void);
extern void cmd_and_sreg (void);
extern void cmd_and_immptr (void);
extern void cmd_and_uptr (void);

extern void cmd_div_ureg (void);
extern void cmd_div_sreg (void);
extern void cmd_div_immptr (void);
extern void cmd_div_uptr (void);
extern void cmd_div_freg (void);

extern void cmd_mod_ureg (void);
extern void cmd_mod_sreg (void);
extern void cmd_mod_immptr (void);
extern void cmd_mod_uptr (void);
extern void cmd_mod_freg (void);

extern void cmd_jmp_immediate (void);
extern void cmd_jmp_ureg (void);
extern void cmd_jmp_immptr (void);
extern void cmd_jmp_uptr (void);

extern void cmd_call_immediate (void);
extern void cmd_call_ureg (void);
extern void cmd_call_immptr (void);
extern void cmd_call_uptr (void);

extern void cmd_loop_immediate (void);
extern void cmd_loop_ureg (void);
extern void cmd_loop_immptr (void);
extern void cmd_loop_uptr (void);

extern void cmd_onfail_immediate (void);
extern void cmd_onfail_ureg (void);
extern void cmd_onfail_immptr (void);
extern void cmd_onfail_uptr (void);

extern void cmd_syscall_immediate (void);
extern void cmd_syscall_ureg (void);
extern void cmd_syscall_immptr (void);
extern void cmd_syscall_uptr (void);

extern void cmd_ret_immediate (void);
extern void cmd_ret_ureg (void);
extern void cmd_ret_immptr (void);
extern void cmd_ret_uptr (void);

extern void cmd_raise_immediate (void);
extern void cmd_raise_ureg (void);
extern void cmd_raise_immptr (void);
extern void cmd_raise_uptr (void);

extern void cmd_ifeq_immediate (void);
extern void cmd_ifeq_ureg (void);
extern void cmd_ifeq_sreg (void);
extern void cmd_ifeq_freg (void);
extern void cmd_ifeq_immptr (void);
extern void cmd_ifeq_uptr (void);

extern void cmd_ifneq_immediate (void);
extern void cmd_ifneq_ureg (void);
extern void cmd_ifneq_sreg (void);
extern void cmd_ifneq_freg (void);
extern void cmd_ifneq_immptr (void);
extern void cmd_ifneq_uptr (void);

extern void cmd_ifabo_immediate (void);
extern void cmd_ifabo_ureg (void);
extern void cmd_ifabo_sreg (void);
extern void cmd_ifabo_freg (void);
extern void cmd_ifabo_immptr (void);
extern void cmd_ifabo_uptr (void);

extern void cmd_ifbel_immediate (void);
extern void cmd_ifbel_ureg (void);
extern void cmd_ifbel_sreg (void);
extern void cmd_ifbel_freg (void);
extern void cmd_ifbel_immptr (void);
extern void cmd_ifbel_uptr (void);

extern void cmd_halt (void);

extern void cmd_nofail (void);

extern void cmd_not_ureg (void);
extern void cmd_not_sreg (void);
extern void cmd_not_immptr (void);
extern void cmd_not_uptr (void);

extern void cmd_sleepfor_immediate (void);
extern void cmd_sleepfor_ureg (void);
extern void cmd_sleepfor_sreg (void);
extern void cmd_sleepfor_freg (void);
extern void cmd_sleepfor_immptr (void);
extern void cmd_sleepfor_uptr (void);

extern void cmd_waittill_immediate (void);
extern void cmd_waittill_ureg (void);
extern void cmd_waittill_sreg (void);
extern void cmd_waittill_freg (void);
extern void cmd_waittill_immptr (void);
extern void cmd_waittill_uptr (void);

extern void cmd_ldb_ureg (void);
extern void cmd_ldb_sreg (void);
extern void cmd_ldb_freg (void);
extern void cmd_ldb_immptr (void);
extern void cmd_ldb_uptr (void);

extern void cmd_stob_ureg (void);
extern void cmd_stob_sreg (void);
extern void cmd_stob_immptr (void);
extern void cmd_stob_uptr (void);

extern void cmd_alloc_immediate (void);
extern void cmd_alloc_ureg (void);
extern void cmd_alloc_sreg (void);
extern void cmd_alloc_immptr (void);
extern void cmd_alloc_uptr (void);

extern void cmd_realloc_immediate (void);
extern void cmd_realloc_ureg (void);
extern void cmd_realloc_sreg (void);
extern void cmd_realloc_immptr (void);
extern void cmd_realloc_uptr (void);

extern void cmd_dealloc_immediate (void);
extern void cmd_dealloc_ureg (void);
extern void cmd_dealloc_sreg (void);
extern void cmd_dealloc_immptr (void);
extern void cmd_dealloc_uptr (void);

extern void cmd_cmpcnt_immediate (void);
extern void cmd_cmpcnt_ureg (void);
extern void cmd_cmpcnt_sreg (void);
extern void cmd_cmpcnt_immptr (void);
extern void cmd_cmpcnt_uptr (void);

extern void cmd_cpcnt_immediate (void);
extern void cmd_cpcnt_ureg (void);
extern void cmd_cpcnt_sreg (void);
extern void cmd_cpcnt_immptr (void);
extern void cmd_cpcnt_uptr (void);

extern void cmd_setstack_immediate (void);
extern void cmd_setstack_ureg (void);
extern void cmd_setstack_immptr (void);
extern void cmd_setstack_uptr (void);

extern void cmd_push_immediate (void);
extern void cmd_push_ureg (void);
extern void cmd_push_sreg (void);
extern void cmd_push_freg (void);
extern void cmd_push_immptr (void);
extern void cmd_push_uptr (void);

extern void cmd_pop_ureg (void);
extern void cmd_pop_sreg (void);
extern void cmd_pop_freg (void);
extern void cmd_pop_immptr (void);
extern void cmd_pop_uptr (void);

