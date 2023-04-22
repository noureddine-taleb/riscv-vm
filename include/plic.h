#ifndef RISCV_PLIC_H
#define RISCV_PLIC_H

#define NR_PRIO_MEM_REGS 256
#define NR_PENDING_REGS 8
#define NR_ENABLE_REGS 8
#define NR_CLAIMED_BITS_REGS 8

#include <types.h>
#include <types.h>

struct plic {
	/*
	 * 0x0C00 0000 - 0x0C00 0400 priority reg 0 is reserved interrupt
	 * source id[0] refers to priority 1 7 priority levels the higher the
	 * level the higher the priority if two ids have same prio, the lower
	 * interrupt id has a higher prio Addressing: BASE_ADDRESS +
	 * 4*Interrupt ID Bits Field Name Description [2:0] Priority Sets the
	 * priority for a given global interrupt. [31:3] Reserved WIRI 
	 */
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
	 * [2:0] Threshold Sets the priority threshold for the E31 Coreplex.
	 * [31:3] Reserved WIRI
	 */
	u32 priority_threshold;

	u32 claim_complete;

	/*
	 * internal 
	 */
	u32 claimed_bits[NR_CLAIMED_BITS_REGS];

};

void plic_update_pending(struct plic *plic, u32 interrupt_id, u8 pending);
u8 plic_update(struct plic *plic);
int plic_bus_access(struct plic *plic, privilege_level priv_level,
		    bus_access_type access_type, uxlen address,
		    void *value, u8 len);

#endif /* RISCV_PLIC_H */
