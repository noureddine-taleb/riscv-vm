#include <stdio.h>
#include <string.h>

#include <trap.h>
#include <hart.h>
#include <helpers.h>

// #define TRAP_TRACE

void hart_update_ip(struct hart *hart, u8 mei, u8 mti, u8 msi)
{
	/*
	 * IRQ coming from the clint are only assigned to machine mode bits,
	 * all other bits are actually pure SW interrupts, set for e.g. by
	 * m-mode context
	 */

	/*
	 * "Additionally, the platformlevel interrupt controller may generate
	 * supervisor-level external interrupts. The logical-OR of the
	 * software-writeable bit and the signal from the external interrupt
	 * controller is used to generate external interrupts to the
	 * supervisor."
	 */

	/*
	 * Get seip, if it is possibly set to 1 by SW
	 */
	UPDATE_BIT(hart->csr_store.ip, trap_cause_machine_exti, mei);

	/*
	 * Special seip handling
	 */
	UPDATE_BIT(hart->csr_store.ip, trap_cause_super_exti, mei);

	UPDATE_BIT(hart->csr_store.ip, trap_cause_machine_ti, mti);

	UPDATE_BIT(hart->csr_store.ip, trap_cause_machine_swi, msi);
}

int interrupt_check_pending(struct hart *hart,
							privilege_level curr_priv_mode,
							enum interrupt_cause irq,
							privilege_level *serving_priv_level)
{
	uxlen interrupt_bit = (1 << irq);
	privilege_level delegation_level = 0;
	*serving_priv_level = machine_mode;

	if (!((hart->csr_store.status >> curr_priv_mode) & 1)) // xIE = disabled
		return 0;

	if (!(hart->csr_store.ip & interrupt_bit & hart->csr_store.ie))
		return 0;

	if (hart->csr_store.mideleg & interrupt_bit)
		delegation_level = supervisor_mode;
	else
		delegation_level = machine_mode;

	if (delegation_level < curr_priv_mode)
		return 0;

	*serving_priv_level = delegation_level;
	return 1;
}

privilege_level
trap_get_serving_priv_level(struct hart *hart,
							privilege_level curr_priv_mode,
							enum trap_cause cause)
{
	uxlen exception_bit = (1 << cause);
	privilege_level serving_priv_mode;

	if (hart->csr_store.medeleg & exception_bit)
		serving_priv_mode = supervisor_mode;
	else
		serving_priv_mode = machine_mode;

	if (serving_priv_mode < curr_priv_mode)
		return curr_priv_mode;

	return serving_priv_mode;
}

void serve_exception(struct hart *hart,
					 privilege_level serving_priv_mode,
					 privilege_level previous_priv_mode,
					 uxlen is_interrupt, uxlen cause, uxlen curr_pc, uxlen tval)
{
	uxlen ie = 0;

	if (serving_priv_mode == machine_mode)
	{
		hart->csr_store.mepc = curr_pc;

		// set MPP
		UPDATE_BIT(hart->csr_store.status, TRAP_XSTATUS_MPP_BITS, previous_priv_mode & 1);
		UPDATE_BIT(hart->csr_store.status, TRAP_XSTATUS_MPP_BITS + 1, (previous_priv_mode >> 1) & 1);

		ie = (hart->csr_store.status >> serving_priv_mode) & 0x1;
		UPDATE_BIT(hart->csr_store.status, TRAP_XSTATUS_UPIE_BIT + serving_priv_mode, ie);

		CLEAR_BIT(hart->csr_store.status, serving_priv_mode);

		hart->csr_store.mcause = ((is_interrupt << (XLEN - 1)) | cause);
		hart->csr_store.mtval = tval;

		hart->pc = (hart->csr_store.mtvec & 0xFFFFFFFFFFFFFFFC) + ((hart->csr_store.mtvec & 0x3) == 1 && is_interrupt ? cause*4 : 0);
	}
	else if (serving_priv_mode == supervisor_mode)
	{
		hart->csr_store.sepc = curr_pc;

		// SET SPP
		UPDATE_BIT(hart->csr_store.status, TRAP_XSTATUS_SPP_BIT, previous_priv_mode & 1);

		ie = (hart->csr_store.status >> serving_priv_mode) & 0x1;
		UPDATE_BIT(hart->csr_store.status, TRAP_XSTATUS_UPIE_BIT + serving_priv_mode, ie);
		CLEAR_BIT(hart->csr_store.status, serving_priv_mode);

		hart->csr_store.stval = tval;
		hart->csr_store.scause = ((is_interrupt << (XLEN - 1)) | cause);

		hart->pc = (hart->csr_store.stvec & 0xFFFFFFFFFFFFFFFC) + ((hart->csr_store.stvec & 0x3) == 1 && is_interrupt ? cause*4 : 0);
	}
	else
	{
		die("handling exceptions by unsupported priv level = %d",
			serving_priv_mode);
	}
	hart->curr_priv_mode = serving_priv_mode;
}

void return_from_exception(struct hart *hart, privilege_level serving_priv_mode)
{
	privilege_level previous_priv_level = 0;
	uxlen pie = 0;

	if (serving_priv_mode == machine_mode)
	{
		previous_priv_level =
			GET_BIT_RANGE(hart->csr_store.status, TRAP_XSTATUS_MPP_BITS, 2);
		hart->curr_priv_mode = previous_priv_level;
		hart->override_pc = hart->csr_store.mepc;

		pie =
			(hart->csr_store.status >> (TRAP_XSTATUS_UPIE_BIT +
										serving_priv_mode)) &
			0x1;
		UPDATE_BIT(hart->csr_store.status, serving_priv_mode, pie);
		CLEAR_BIT(hart->csr_store.status,
				  (TRAP_XSTATUS_UPIE_BIT + serving_priv_mode));
	}
	else if (serving_priv_mode == supervisor_mode)
	{
		previous_priv_level =
			GET_BIT(hart->csr_store.status, TRAP_XSTATUS_SPP_BIT);
		hart->curr_priv_mode = previous_priv_level;
		hart->override_pc = hart->csr_store.sepc;

		pie =
			(hart->csr_store.status >> (TRAP_XSTATUS_UPIE_BIT +
										serving_priv_mode)) &
			0x1;
		UPDATE_BIT(hart->csr_store.status, serving_priv_mode, pie);
		CLEAR_BIT(hart->csr_store.status,
				  (TRAP_XSTATUS_UPIE_BIT + serving_priv_mode));
	}
	else
	{
		die("returning from exception by unsupported priv level = %d",
			serving_priv_mode);
	}
}

void prepare_sync_trap(struct hart *hart, uxlen cause, uxlen tval)
{
	if (!hart->sync_trap_pending)
	{
		hart->sync_trap_pending = 1;
		hart->sync_trap_cause = cause;
		hart->sync_trap_tval = tval;
#ifdef CSR_TRACE
		debug("trap: cause %ld tval=%#lx\n", cause, tval);
#endif
	} else {
		die("double fault (trap while handling another trap)");
	}
}

u8 hart_handle_pending_interrupts(struct hart *hart)
{
	privilege_level serving_priv_level = machine_mode;

	if (hart->sync_trap_pending)
	{
		serving_priv_level =
			trap_get_serving_priv_level(hart,
										hart->curr_priv_mode,
										hart->sync_trap_cause);

		serve_exception(hart, serving_priv_level,
						hart->curr_priv_mode, 0,
						hart->sync_trap_cause, hart->pc - 4,
						hart->sync_trap_tval);

		hart->sync_trap_pending = 0;
		hart->sync_trap_cause = 0;
		hart->sync_trap_tval = 0;
		return 1;
	}

	for (enum interrupt_cause interrupt_cause =
			 trap_cause_machine_exti;
		 interrupt_cause >= trap_cause_user_swi; interrupt_cause--)
	{
		int trap_retval = interrupt_check_pending(hart,
												  hart->curr_priv_mode,
												  interrupt_cause,
												  &serving_priv_level);
		if (trap_retval)
		{
			serve_exception(hart,
							serving_priv_level,
							hart->curr_priv_mode, 1,
							interrupt_cause, hart->pc,
							hart->sync_trap_tval);
			return 1;
		}
	}

	return 0;
}
