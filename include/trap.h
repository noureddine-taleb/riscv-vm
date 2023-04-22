/*
 * Trap setup and trap handling registers seem to be shared across privilege
 * levels but with different access permissions. So we try to unify those
 * registers here. 
 */
#ifndef RISCV_TRAP_H
#define RISCV_TRAP_H

#include <types.h>
#include <types.h>

// /* XSTATUS BITS */
#define TRAP_XSTATUS_SIE_BIT 1
#define TRAP_XSTATUS_MIE_BIT 3
#define TRAP_XSTATUS_UPIE_BIT 4
#define TRAP_XSTATUS_SPIE_BIT 5
#define TRAP_XSTATUS_MPIE_BIT 7
#define TRAP_XSTATUS_SPP_BIT 8
#define TRAP_XSTATUS_MPP_BIT 11	/* and 12 */
#define TRAP_XSTATUS_MPBIT 17
#define TRAP_XSTATUS_SUM_BIT 18
#define TRAP_XSTATUS_MXR_BIT 19
#define TRAP_XSTATUS_TSR_BIT 22

#define GET_GLOBAL_IRQ_BIT(priv_level) (1<<priv_level)
#define GET_LOCAL_IRQ_BIT(priv_level, trap_type) ( (1<<(priv_level_max*trap_type)) << priv_level )
#define GET_EXCEPTION_BIT(trap_cause) ( 1 << trap_cause )

enum interrupt_cause {
	trap_cause_irq_min = -1,

	trap_cause_user_swi = 0,
	trap_cause_super_swi,
	trap_cause_rsvd_0,
	trap_cause_machine_swi,

	trap_cause_user_ti,
	trap_cause_super_ti,
	trap_cause_rsvd_1,
	trap_cause_machine_ti,

	trap_cause_user_exti,
	trap_cause_super_exti,
	trap_cause_rsvd_2,
	trap_cause_machine_exti
};

enum trap_cause {
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
	trap_cause_rsvd_3,
	trap_cause_machine_ecall,
	trap_cause_instr_page_fault,
	trap_cause_load_page_fault,
	trap_cause_rsvd_4,
	trap_cause_store_amo_page_fault
};

enum interrupt_type {
	trap_type_unknown = -1,
	trap_type_swi = 0,
	trap_type_ti,
	trap_type_exti,
};

void hart_set_pending_bits(struct hart *hart, u8 ext_int, u8 tim_int,
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
