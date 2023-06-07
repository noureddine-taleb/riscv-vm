#ifndef RISCV_PLIC_H
#define RISCV_PLIC_H

#define NR_IRQ_COUNT 256
#define NR_PENDING_REGS 8
#define NR_CONTEXTS 2

#include <types.h>
#include <types.h>

struct plic
{
	u32 priority[NR_IRQ_COUNT];
	u32 pending_bits[NR_PENDING_REGS];

	/*
	 * ctxs
	 */
	struct {
		u32 ctx_enable_bits[NR_PENDING_REGS];
		u32 ctx_priority_threshold_reg;
		/*
		* internal: used to mask claimed interrupts (gateway)
		*/
		u32 ctx_claim_complete_reg;
		u32 ctx_masked_bits[NR_PENDING_REGS];
	} contexts [NR_CONTEXTS];
};

void plic_set_pending_interrupt(struct plic *plic, u32 interrupt_id, u8 pending);
u8 plic_check_interrupts(struct plic *plic, int context);
int plic_bus_access(struct plic *plic, privilege_level priv_level,
					bus_access_type access_type, uxlen address,
					void *value, u8 len);

#endif /* RISCV_PLIC_H */
