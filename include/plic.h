#ifndef RISCV_PLIC_H
#define RISCV_PLIC_H

#define NR_PRIO_MEM_REGS 256
#define NR_PENDING_REGS 8
#define NR_ENABLE_REGS 8
#define NR_CLAIMED_BITS_REGS 8

#include <types.h>
#include <types.h>

struct plic
{
	u32 priority[NR_PRIO_MEM_REGS];

	/*
	 * bits correspronding to the interrupt ids
	 */
	u32 pending_bits[NR_PENDING_REGS];

	/*
	 * bits correspronding to the interrupt ids
	 */
	u32 enable_bits[NR_ENABLE_REGS];

	/*
	 * Interrupts with a lower prio setting than threshold will be masked out
	 * Bits Field Name Description
	 */
	u32 priority_threshold;

	u32 claim_complete;

	/*
	 * internal: used to mask claimed interrupts (gateway)
	 */
	u32 claimed_bits[NR_CLAIMED_BITS_REGS];
};

void plic_set_pending_interrupt(struct plic *plic, u32 interrupt_id, u8 pending);
u8 plic_check_interrupts(struct plic *plic);
int plic_bus_access(struct plic *plic, privilege_level priv_level,
					bus_access_type access_type, uxlen address,
					void *value, u8 len);

#endif /* RISCV_PLIC_H */
