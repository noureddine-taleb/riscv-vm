#include <stdio.h>
#include <types.h>
#include <string.h>

#include <plic.h>
#include <helpers.h>
#include <assert.h>

#define PLIC_MAX_INTERRUPTS 255

#define PLIC_PRIORITY_ADDR_OFFS 0x0
#define PLIC_PRIORITY_SIZE_BYTES 0x400

#define PLIC_PENDING_ADDR_OFFS 0x1000
#define PLIC_PENDING_SIZE_BYTES 0x20

// Context 0: machine mode
#define PLIC_CTX0_IRQ_ENABLE_ADDR_OFFS 0x2000
#define PLIC_CTX0_IRQ_ENABLE_SIZE_BYTES 0x20

#define PLIC_CTX0_PRIO_THRESH_ADDR_OFFS 0x200000
#define PLIC_CTX0_PRIO_THRESH_SIZE_BYTES 0x4

#define PLIC_CTX0_PRIO_CLAIM_ADDR_OFFS 0x200004
#define PLIC_CTX0_PRIO_CLAIM_SIZE_BYTES 0x4

// Context 1: supervisor mode
#define PLIC_CTX1_IRQ_ENABLE_ADDR_OFFS 0x2080
#define PLIC_CTX1_IRQ_ENABLE_SIZE_BYTES 0x20

#define PLIC_CTX1_PRIO_THRESH_ADDR_OFFS 0x201000
#define PLIC_CTX1_PRIO_THRESH_SIZE_BYTES 0x4

#define PLIC_CTX1_PRIO_CLAIM_ADDR_OFFS 0x201004
#define PLIC_CTX1_PRIO_CLAIM_SIZE_BYTES 0x4

// TODO: bound checking
static u8 *get_u8_reg_ptr(struct plic *plic, uxlen address,
						  u32 **is_claim_complete)
{
	u8 *ret_ptr = NULL;
	uxlen tmp_address = 0;

	if (address < PLIC_PRIORITY_SIZE_BYTES)
	{
		assert(address % 4 == 0);
		tmp_address = address - PLIC_PRIORITY_ADDR_OFFS;
		/*
		 * first one is actually reserved
		 */
		if (tmp_address >= 0x4)
		{
			ret_ptr = (u8 *)plic->priority;
			return &ret_ptr[tmp_address];
		}
	}
	else if (ADDR_WITHIN_RANGE(address, PLIC_PENDING_ADDR_OFFS, PLIC_PENDING_SIZE_BYTES))
	{
		tmp_address = address - PLIC_PENDING_ADDR_OFFS;
		ret_ptr = (u8 *)plic->pending_bits;
		return &ret_ptr[tmp_address];
	}
	// ctx0
	else if (ADDR_WITHIN_RANGE(address, PLIC_CTX0_IRQ_ENABLE_ADDR_OFFS,
						 PLIC_CTX0_IRQ_ENABLE_SIZE_BYTES))
	{
		tmp_address = address - PLIC_CTX0_IRQ_ENABLE_ADDR_OFFS;
		ret_ptr = (u8 *)plic->contexts[0].ctx_enable_bits;
		return &ret_ptr[tmp_address];
	}
	else if (ADDR_WITHIN_RANGE(address, PLIC_CTX0_PRIO_THRESH_ADDR_OFFS,
						 PLIC_CTX0_PRIO_THRESH_SIZE_BYTES))
	{
		assert(address % 4 == 0);
		tmp_address = address - PLIC_CTX0_PRIO_THRESH_ADDR_OFFS;
		ret_ptr = (u8 *)&plic->contexts[0].ctx_priority_threshold_reg;
		return &ret_ptr[tmp_address];
	}
	else if (ADDR_WITHIN_RANGE(address, PLIC_CTX0_PRIO_CLAIM_ADDR_OFFS,
						 PLIC_CTX0_PRIO_CLAIM_SIZE_BYTES))
	{
		assert(address % 4 == 0);
		tmp_address = address - PLIC_CTX0_PRIO_CLAIM_ADDR_OFFS;
		ret_ptr = (u8 *)&plic->contexts[0].ctx_claim_complete_reg;
		*is_claim_complete = (u32 *)&plic->contexts[0].ctx_masked_bits;
		return &ret_ptr[tmp_address];
	} 
	// ctx1
	else if (ADDR_WITHIN_RANGE(address, PLIC_CTX1_IRQ_ENABLE_ADDR_OFFS,
						 PLIC_CTX1_IRQ_ENABLE_SIZE_BYTES))
	{
		tmp_address = address - PLIC_CTX1_IRQ_ENABLE_ADDR_OFFS;
		ret_ptr = (u8 *)plic->contexts[1].ctx_enable_bits;
		return &ret_ptr[tmp_address];
	}
	else if (ADDR_WITHIN_RANGE(address, PLIC_CTX1_PRIO_THRESH_ADDR_OFFS,
						 PLIC_CTX1_PRIO_THRESH_SIZE_BYTES))
	{
		assert(address % 4 == 0);
		tmp_address = address - PLIC_CTX1_PRIO_THRESH_ADDR_OFFS;
		ret_ptr = (u8 *)&plic->contexts[1].ctx_priority_threshold_reg;
		return &ret_ptr[tmp_address];
	}
	else if (ADDR_WITHIN_RANGE(address, PLIC_CTX1_PRIO_CLAIM_ADDR_OFFS,
						 PLIC_CTX1_PRIO_CLAIM_SIZE_BYTES))
	{
		assert(address % 4 == 0);
		tmp_address = address - PLIC_CTX1_PRIO_CLAIM_ADDR_OFFS;
		ret_ptr = (u8 *)&plic->contexts[1].ctx_claim_complete_reg;
		*is_claim_complete = (u32 *)&plic->contexts[1].ctx_masked_bits;
		return &ret_ptr[tmp_address];
	} else {
		debug("plic accessing unvailable region %#lx", address);
	}
	return ret_ptr;
}

static void plic_check_sanity(struct plic *plic)
{
	u32 i = 0;

	for (i = 0; i < NR_IRQ_COUNT; i++)
	{
		plic->priority[i] = plic->priority[i] & 0x7;
	}

	plic->contexts[0].ctx_priority_threshold_reg &= 0x7;
	plic->contexts[1].ctx_priority_threshold_reg &= 0x7;
}

void plic_set_pending_interrupt(struct plic *plic, u32 interrupt_id, u8 pending)
{
	u32 irq_reg = interrupt_id / 32;
	u32 irq_bit = interrupt_id % 32;

	UPDATE_BIT(plic->pending_bits[irq_reg], irq_bit, pending);

	for (int i=0; i < NR_CONTEXTS; i++) {
		plic->pending_bits[irq_reg] &= ~plic->contexts[i].ctx_masked_bits[irq_reg];
	}
}

u8 plic_check_interrupts(struct plic *plic, int context)
{
	u32 i, j = 0;
	u32 irq = 0;
	u32 irq_to_trigger = 0;
	u8 highest_prio = 0;

	/*
	 * check for any enabled interrupt and threshold
	 */
	for (i = 0; i < NR_PENDING_REGS; i++)
	{
		if ((plic->contexts[context].ctx_enable_bits[i] & plic->pending_bits[i]) == 0)
			continue;

		irq = i * (sizeof(plic->contexts[context].ctx_enable_bits[0]) * 8);
		for (j = 0; j < sizeof(plic->contexts[context].ctx_enable_bits[0]) * 8; j++, irq++)
		{
			if (GET_BIT(plic->contexts[context].ctx_enable_bits[i], j) &&
				GET_BIT(plic->pending_bits[i], j) &&
				plic->priority[irq] > plic->contexts[context].ctx_priority_threshold_reg &&
				!GET_BIT(plic->contexts[context].ctx_masked_bits[i], j) // is it already claimed ?
			  )
			{				
				/*
				* find irq with highest prio
				*/
				if (plic->priority[irq] > highest_prio)
				{
					highest_prio = plic->priority[irq];
					irq_to_trigger = irq;
				}
			}
		}
	}

	if (irq_to_trigger > 0)
	{
		plic->contexts[context].ctx_claim_complete_reg = irq_to_trigger;
		return 1;
	}
	else
	{
		plic->contexts[context].ctx_claim_complete_reg = 0;
	}

	return 0;
}

int plic_bus_access(struct plic *plic, privilege_level __maybe_unused priv_level,
					bus_access_type access_type, uxlen address, void *value, u8 len)
{
	// serves as a flag and as container for context's masked irqs from gateway
	u32 *is_claim_complete___ctx_mask_bits = 0;
	u32 irq_reg = 0;
	u32 irq_bit = 0;
	u8 *u8_ptr = get_u8_reg_ptr(plic, address, &is_claim_complete___ctx_mask_bits);
	u32 tmp_val = 0;

	if (u8_ptr)
	{
		if (access_type == bus_write_access)
		{
			memcpy(u8_ptr, value, len);
			tmp_val = *(u32 *)value;
			/*
			 * check if it is the claim complete reg
			 */
			if (is_claim_complete___ctx_mask_bits)
			{
				irq_reg = tmp_val / 32;
				irq_bit = tmp_val % 32;
				CLEAR_BIT(is_claim_complete___ctx_mask_bits[irq_reg], irq_bit);
			}
			/*
			 * be sure that all updated values are sane
			 */
			plic_check_sanity(plic);
		}
		else
		{
			memcpy(value, u8_ptr, len);
			tmp_val = *(u32 *)u8_ptr;
			/*
			 * check if it is the claim complete reg
			 */
			if (is_claim_complete___ctx_mask_bits)
			{
				irq_reg = tmp_val / 32;
				irq_bit = tmp_val % 32;
				if (GET_BIT(plic->pending_bits[irq_reg], irq_bit)) {
					SET_BIT(is_claim_complete___ctx_mask_bits[irq_reg], irq_bit);
					CLEAR_BIT(plic->pending_bits[irq_reg], irq_bit);
				}
			}
		}
	}

	return 0;
}
