#ifndef RISCV_TRAP_H
#define RISCV_TRAP_H

#include <types.h>
#include <types.h>

#define TRAP_XSTATUS_SIE_BIT 1
#define TRAP_XSTATUS_MIE_BIT 3
#define TRAP_XSTATUS_UPIE_BIT 4
#define TRAP_XSTATUS_SPIE_BIT 5
#define TRAP_XSTATUS_MPIE_BIT 7
#define TRAP_XSTATUS_SPP_BIT 8
#define TRAP_XSTATUS_MPP_BITS 11 /* and 12 */
#define TRAP_XSTATUS_MPRV_BIT 17
#define TRAP_XSTATUS_SUM_BIT 18
#define TRAP_XSTATUS_MXR_BIT 19
#define TRAP_XSTATUS_TSR_BIT 22

enum interrupt_cause
{
	trap_cause_irq_min = -1,

	trap_cause_user_swi = 0,
	trap_cause_super_swi,
	trap_cause_reserved,
	trap_cause_machine_swi,

	trap_cause_user_ti,
	trap_cause_super_ti,
	trap_cause_reserved_1,
	trap_cause_machine_ti,

	trap_cause_user_exti,
	trap_cause_super_exti,
	trap_cause_reserved_2,
	trap_cause_machine_exti
};

enum trap_cause
{
	trap_cause_instr_addr_misalign = 0,
	trap_cause_instr_access_fault,
	trap_cause_illegal_instr,
	trap_cause_breakpoint,
	trap_cause_load_addr_misalign,
	trap_cause_load_access_fault,
	trap_cause_store_amo_addr_fault,
	trap_cause_store_amo_access_fault,
	trap_cause_user_ecall,
	trap_cause_super_ecall,
	trap_cause_reserved_3,
	trap_cause_machine_ecall,
	trap_cause_instr_page_fault,
	trap_cause_load_page_fault,
	trap_cause_reserved_4,
	trap_cause_store_amo_page_fault
};

void hart_update_ip(struct hart *hart, u8 ext_int, u8 tim_int,
					u8 sw_int);
int interrupt_check_pending(struct hart *hart, privilege_level curr_priv_mode,
							enum interrupt_cause irq,
							privilege_level *serving_priv_level);
privilege_level trap_get_serving_priv_level(struct hart *hart,
											privilege_level curr_priv_mode,
											enum trap_cause cause);
void serve_exception(struct hart *hart, privilege_level serving_priv_mode,
					 privilege_level previous_priv_mode, uxlen is_interrupt,
					 uxlen cause, uxlen curr_pc, uxlen tval);
void return_from_exception(struct hart *hart,
						   privilege_level serving_priv_mode);

#endif /* RISCV_TRAP_H */
