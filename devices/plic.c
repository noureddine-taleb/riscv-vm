#include <stdio.h>
#include <types.h>
#include <string.h>

#include <riscv_helper.h>
#include <plic.h>

#define PLIC_MAX_INTERRUPTS 255

#define PLIC_PRIORITY_ADDR_OFFS 0x0
#define PLIC_PRIORITY_SIZE_BYTES 0x400

#define PLIC_PENDING_ADDR_OFFS 0x1000
#define PLIC_PENDING_SIZE_BYTES 0x20

#define PLIC_IRQ_ENABLE_ADDR_OFFS 0x2000
#define PLIC_IRQ_ENABLE_SIZE_BYTES 0x20

#define PLIC_PRIO_THRESH_ADDR_OFFS 0x200000
#define PLIC_PRIO_THRESH_SIZE_BYTES 0x4

#define PLIC_PRIO_CLAIM_ADDR_OFFS 0x200004
#define PLIC_PRIO_CLAIM_SIZE_BYTES 0x4

static u8 *get_u8_reg_ptr(struct plic *plic, uxlen address,
						  u8 *is_claim_complete)
{
	u8 *ret_ptr = NULL;
	uxlen tmp_address = 0;

	if (address < PLIC_PRIORITY_SIZE_BYTES)
	{
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
	else if (ADDR_WITHIN(address, PLIC_PENDING_ADDR_OFFS, PLIC_PENDING_SIZE_BYTES))
	{
		tmp_address = address - PLIC_PENDING_ADDR_OFFS;
		ret_ptr = (u8 *)plic->pending_bits;
		return &ret_ptr[tmp_address];
	}
	else if (ADDR_WITHIN(address, PLIC_IRQ_ENABLE_ADDR_OFFS,
						 PLIC_IRQ_ENABLE_SIZE_BYTES))
	{
		tmp_address = address - PLIC_IRQ_ENABLE_ADDR_OFFS;
		ret_ptr = (u8 *)plic->enable_bits;
		return &ret_ptr[tmp_address];
	}
	else if (ADDR_WITHIN(address, PLIC_PRIO_THRESH_ADDR_OFFS,
						 PLIC_PRIO_THRESH_SIZE_BYTES))
	{
		tmp_address = address - PLIC_PRIO_THRESH_ADDR_OFFS;
		ret_ptr = (u8 *)&plic->priority_threshold;
		return &ret_ptr[tmp_address];
	}
	else if (ADDR_WITHIN(address, PLIC_PRIO_CLAIM_ADDR_OFFS,
						 PLIC_PRIO_CLAIM_SIZE_BYTES))
	{
		tmp_address = address - PLIC_PRIO_CLAIM_ADDR_OFFS;
		ret_ptr = (u8 *)&plic->claim_complete;
		*is_claim_complete = 1;
		return &ret_ptr[tmp_address];
	}

	return ret_ptr;
}

static void plic_check_sanity(struct plic *plic)
{
	u32 i = 0;

	for (i = 0; i < NR_PRIO_MEM_REGS; i++)
	{
		plic->priority[i] = plic->priority[i] & 0x7;
	}

	plic->priority_threshold = plic->priority_threshold & 0x7;
}

void plic_set_pending_interrupt(struct plic *plic, u32 interrupt_id, u8 pending)
{
	u32 irq_reg = interrupt_id / 32;
	u32 irq_bit = interrupt_id % 32;
	// todo: change this to something better
	UPDATE_BIT(plic->pending_bits[irq_reg], irq_bit, pending);
}

u8 plic_check_interrupts(struct plic *plic)
{
	u32 i, j = 0;
	u32 irq_id_count = 0;
	u32 irq_to_trigger = 0;
	u8 highest_prio = 0;

	/*
	 * check for any enabled interrupt and threshold
	 */
	for (i = 0; i < NR_ENABLE_REGS; i++)
	{
		if ((!plic->enable_bits[i]) || (!plic->pending_bits[i]))
			continue;

		for (j = 0; j < sizeof(plic->enable_bits[0]) * 8; j++)
		{
			if (GET_BIT(plic->enable_bits[i], j) &&
				GET_BIT(plic->pending_bits[i], j) &&
				(plic->priority[irq_id_count] >=
				 plic->priority_threshold))
			{
				if (GET_BIT(plic->claimed_bits[i], j))
				{
					/*
					 * qemu also seems to clear pending bit
					 * if it was already claimed
					 */
					UPDATE_BIT(plic->pending_bits[i], j, 0);
				}
				else if ((plic->priority[irq_id_count] >
						  highest_prio))
				{
					/*
					 * find irq with highest prio
					 */
					highest_prio =
						plic->priority[irq_id_count];
					irq_to_trigger = irq_id_count;
				}
			}

			irq_id_count++;
		}
	}

	if (irq_to_trigger > 0)
	{
		plic->claim_complete = irq_to_trigger;
		return 1;
	}
	else
	{
		plic->claim_complete = 0;
	}

	return 0;
}

int plic_bus_access(struct plic *plic, privilege_level priv_level,
					bus_access_type access_type, uxlen address, void *value, u8 len)
{
	(void)priv_level;
	u8 is_claim_complete = 0;
	u32 irq_reg = 0;
	u32 irq_bit = 0;
	u8 *u8_ptr = get_u8_reg_ptr(plic, address, &is_claim_complete);
	u32 tmp_val = 0;

	if (u8_ptr)
	{
		if (access_type == bus_write_access)
		{
			memcpy(u8_ptr, value, len);
			tmp_val = *(u32 *) value;
			/*
			 * check if it is the claim complete reg
			 */
			if (is_claim_complete)
			{
				irq_reg = tmp_val / 32;
				irq_bit = tmp_val % 32;
				UPDATE_BIT(plic->claimed_bits[irq_reg], irq_bit, 0);
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
			if (is_claim_complete)
			{
				irq_reg = tmp_val / 32;
				irq_bit = tmp_val % 32;
				if (GET_BIT(plic->pending_bits[irq_reg], irq_bit))
					UPDATE_BIT(plic->claimed_bits[irq_reg], irq_bit, 1);
			}
		}
	}

	return 0;
}
