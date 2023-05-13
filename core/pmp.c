#include <stdio.h>
#include <string.h>

#include <pmp.h>
#include <riscv_helper.h>
#include <types.h>
#include <hart.h>

int pmp_write_csr_cfg(__maybe_unused u16 address, struct csr_mapping *map, uxlen val)
{
	u8 *old_pmpcfgs = (u8 *)map->value;
	u8 *new_pmpcfgs = (u8 *)&val;
	int pmpcfg_off = address - CSR_PMPCFG0;

	for (u8 i = 0; i < sizeof(uxlen); i++)
	{
		/*
		 * check if it is locked, this can only be cleared by a reset
		 */
		if (GET_BIT(old_pmpcfgs[i], PMP_CFG_L_BIT))
			continue;

		/*
		 * check top of a range case
		 */
		int is_last_cfg = i == sizeof(uxlen) - 1 && pmpcfg_off == (PMP_NR_CFG_REGS - 1);

		if (is_last_cfg)
			continue;

		if (PMP_CFG_ADDRESS_MATCHING(old_pmpcfgs[i + 1]) == pmp_a_tor && PMP_CFG_LOCKED(old_pmpcfgs[i + 1]))
			continue;

		/*
		 * all good we safely change the cfg value now
		*/
		old_pmpcfgs[i] = new_pmpcfgs[i];
	}

	return 0;
}

int pmp_write_csr_addr(u16 address, struct csr_mapping *map, uxlen val)
{
	u8 *pmpcfgs = (u8 *)map->cookie;
	int pmpaddr_off = address - CSR_PMPADDR0;

	/*
	 * updating the reg is only permitted if it is not locked
	 */
	if (GET_BIT(pmpcfgs[pmpaddr_off], PMP_CFG_L_BIT))
		return 0;

	/*
	 * check tor
	 */
	int is_last_cfg = pmpaddr_off == (PMP_NR_ADDR_REGS - 1);

	if (is_last_cfg)
		return 0;

	if (PMP_CFG_ADDRESS_MATCHING(pmpcfgs[pmpaddr_off + 1]) == pmp_a_tor && PMP_CFG_LOCKED(pmpcfgs[pmpaddr_off + 1]))
		return 0;

	*map->value = val;

	return 0;
}

int pmp_mem_check(struct hart *hart, privilege_level curr_priv, uxlen addr,
				  u8 len, bus_access_type access_type)
{
	/*
	 * check if the address matches any enabled config
	 * lower cfgs have precedence over higher ones
	 */
	bool at_least_one_pmp_active = false;

	/*
	 * loop over all the pmps
	*/
	for (int i = 0; i < PMP_NR_ADDR_REGS; i++)
	{
		u8 *pmpcfgs = (u8 *)&hart->csr_store.pmpcfg;
		uxlen *pmpaddrs = (uxlen *)&hart->csr_store.pmpaddr;
		enum pmp_addr_matching pmp_address_matching = PMP_CFG_ADDRESS_MATCHING(pmpcfgs[i]);
		u8 allowed_access = PMP_CFG_ACCESS(pmpcfgs[i]);
		uxlen pmp_addr_start = 0, pmp_addr_size = 0;
		u8 pmp_addr_size_shift = 0;
		
		/*
		* We only have to check the locked regions in machine
		* mode
		*/
		if (curr_priv == machine_mode && !PMP_CFG_LOCKED(pmpcfgs[i]))
			continue;

		if (pmp_address_matching == pmp_a_off)
			continue;

		at_least_one_pmp_active = true;

		switch (pmp_address_matching)
		{
			case pmp_a_tor:
				if (i == 0)
				{
					pmp_addr_start = 0;
					pmp_addr_size = pmpaddrs[i] << 2;
				}
				else
				{
					pmp_addr_start = pmpaddrs[i - 1] << 2;
					pmp_addr_size = (pmpaddrs[i] << 2) - pmp_addr_start;
				}
				break;
			case pmp_a_napot:
					/**
					 * checkout the spec to understand this
					 * riscv priveleged isa 20211203 p 59
					*/
					pmp_addr_size_shift = FIND_FIRST_BIT_SET(~pmpaddrs[i]) + 2;
					if (pmp_addr_size_shift == 2) // FIND_FIRST_BIT_SET(~pmpaddrs[i]) == 0
						pmp_addr_size_shift = XLEN + 3;

					if (pmp_addr_size_shift >= XLEN)
						pmp_addr_size = -1; // max size
					else
						pmp_addr_size = 1 << pmp_addr_size_shift;

					if (pmp_addr_size_shift >= XLEN)
						pmp_addr_start = 0;
					else
						pmp_addr_start = (pmpaddrs[i] >> (pmp_addr_size_shift - 2)) << pmp_addr_size_shift;
				break;
			case pmp_a_na4:
				pmp_addr_start = pmpaddrs[i] << 2;
				pmp_addr_size = 4;
				break;
			default:
				// unreachable
				break;
		}

		/*
		 * Check if the access partially overlaps with
		 * configured mem regions
		 */
		bool lower_addr_match = ADDR_WITHIN(addr, pmp_addr_start, pmp_addr_size);
		bool upper_addr_match = ADDR_WITHIN(addr + (len - 1), pmp_addr_start, pmp_addr_size);
		u8 curr_access_flags = (1 << access_type);

		/*
		* the accessed range is within the pmp range
		*/
		if (upper_addr_match && lower_addr_match)
			return (curr_access_flags & allowed_access) ? 0 : -1;

		/*
		* partial access is only allowed for machine mode
		*/
		if (lower_addr_match ^ upper_addr_match)
			return (
					(curr_priv == machine_mode) 
					&& (curr_access_flags & allowed_access)
				) ? 0 : -1;
	}

	/*
	 * machine mode has access by default
	 */
	if (curr_priv == machine_mode)
		return 0;

	/*
	 * non machine mode
	 * pmp is used default to deny
	 */
	if (at_least_one_pmp_active)
		return -1;

	/*
	 * pmp is not used default to allow
	 */
	return 0;
}

int pmp_check(struct hart *hart, privilege_level __maybe_unused priv_level,
			  bus_access_type access_type, uxlen *addr, void *__maybe_unused value, u8 len)
{
	uxlen trap_cause;
	switch (access_type)
	{
	case bus_instr_access:
		trap_cause = trap_cause_instr_access_fault;
		break;
	case bus_read_access:
		trap_cause = trap_cause_load_access_fault;
		break;
	case bus_write_access:
		trap_cause = trap_cause_store_amo_access_fault;
		break;
	default:
		die("unreachable");
	}

	if (pmp_mem_check(hart, priv_level, *addr, len, access_type))
	{
		prepare_sync_trap(hart, trap_cause, *addr);
		return -1;
	}

	return 0;
}
