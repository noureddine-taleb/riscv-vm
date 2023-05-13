#include <stdio.h>
#include <string.h>

#include <pmp.h>
#include <riscv_helper.h>
#include <types.h>
#include <hart.h>

int pmp_write_csr_cfg(__maybe_unused u16 address, struct csr_mapping *map, uxlen val)
{
	u8 *pmpcfg = (u8 *)map->value;
	u8 *new_pmpcfg = (u8 *)&val;

	for (u8 i = 0; i < sizeof(uxlen); i++)
	{
		/*
		 * check if it is locked, this can only be cleared by a reset
		 */
		if (GET_BIT(pmpcfg[i], PMP_CFG_L_BIT))
			continue;

		int is_last_cfg = map->value == pmpcfg[PMP_NR_CFG_REGS - 1] && i == sizeof(uxlen) - 1;
		if (!is_last_cfg && GET_RANGE(pmpcfg[i+1], PMP_CFG_A_BIT_OFFS, 2) == pmp_a_tor && GET_BIT(pmpcfg[i+1], PMP_CFG_L_BIT))
			continue;

		pmpcfg[i] = new_pmpcfg[i];
	}

	return 0;
}

int pmp_write_csr_addr(u16 address, struct csr_mapping *map, uxlen val)
{
	u8 *cfg_ptr = (u8 *)map->cookie;
	int pmpaddr_i = address - CSR_PMPADDR0;

	/*
	 * updating the reg is only permitted if it is not locked do nothing
	 * and return with OK if it is locked
	 */
	if (GET_BIT(cfg_ptr[pmpaddr_i], PMP_CFG_L_BIT))
		return 0;

	/*
	 * check if the next index is locked and set to tor
	 */
	int is_last_cfg = pmpaddr_i == (PMP_NR_ADDR_REGS - 1);
	if (!is_last_cfg && GET_RANGE(cfg_ptr[pmpaddr_i + 1], PMP_CFG_A_BIT_OFFS, 2) == pmp_a_tor && GET_BIT(cfg_ptr[pmpaddr_i + 1], PMP_CFG_L_BIT))
		return 0;

	*map->value = val;

	return 0;
}

static inline uxlen get_pmp_napot_size_from_pmpaddr(uxlen addr)
{
	/*
	 * +2 because it was shifted by 2 with encoding
	 */
	return (1 << (FIND_FIRST_BIT_SET(~addr) + 2));
}

static inline uxlen get_pmp_napot_addr_from_pmpaddr(uxlen addr)
{
	uxlen size = get_pmp_napot_size_from_pmpaddr(addr);
	uxlen mask = ((size / 2) - 1) >> 2;
	return (addr - mask) << 2;
}

int pmp_mem_check(struct hart *hart, privilege_level curr_priv, uxlen addr,
				  u8 len, bus_access_type access_type)
{
	/*
	 * check if the address matches any enabled config
	 */
	/*
	 * lower cfgs have precedence over higher ones
	 */
	u8 i = 0;
	u8 j = 0;
	u8 addr_count = 0;
	u8 *cfg_ptr = NULL;
	enum pmp_addr_matching addr_mode = pmp_a_off;
	uxlen addr_end = 0;
	uxlen pmp_addr_start = 0;
	uxlen pmp_addr_size = 0;
	int at_least_one_active = 0;
	u8 curr_access_flags = (1 << access_type);
	u8 allowed_access = 0;
	u8 lower_addr_match = 0;
	u8 upper_addr_match = 0;

	for (i = 0; i < PMP_NR_CFG_REGS; i++)
	{
		cfg_ptr = (u8 *)&hart->csr_store.pmpcfg[i];
		for (j = 0; j < sizeof(uxlen); j++)
		{
			lower_addr_match = 0;
			upper_addr_match = 0;

			/*
			 * We only have to check the locked regions in machine
			 * mode
			 */
			if ((curr_priv == machine_mode) && !GET_BIT(cfg_ptr[j], PMP_CFG_L_BIT))
				continue;

			addr_count = (i * sizeof(uxlen)) + j;
			addr_mode = GET_RANGE(cfg_ptr[j], PMP_CFG_A_BIT_OFFS, 2);

			if (!addr_mode)
				continue;

			allowed_access = cfg_ptr[j] & 0x7;

			at_least_one_active = 1;

			switch (addr_mode)
			{
			case pmp_a_tor:
				if (addr_count == 0)
				{
					pmp_addr_start = 0;
					pmp_addr_size =
						(hart->csr_store.pmpaddr[addr_count] << 2);
				}
				else
				{
					pmp_addr_start =
						(hart->csr_store.pmpaddr[addr_count - 1] << 2);
					pmp_addr_size =
						(hart->csr_store.pmpaddr[addr_count] << 2) - pmp_addr_start;
				}
				break;
			case pmp_a_napot:
				/*
				 * I couldn't find this case in the spec, but
				 * qemu seems to do it in a similar fashion
				 * https://github.com/qemu/qemu/blob/master/target/riscv/pmp.c
				 */
				if (hart->csr_store.pmpaddr[addr_count] ==
					(uxlen)-1)
				{
					pmp_addr_size = -1;
					pmp_addr_start = 0;
				}
				else
				{
					pmp_addr_size =
						get_pmp_napot_size_from_pmpaddr(hart->csr_store.pmpaddr[addr_count]);
					pmp_addr_start =
						get_pmp_napot_addr_from_pmpaddr(hart->csr_store.pmpaddr[addr_count]);
				}
				break;
			case pmp_a_na4:
				pmp_addr_start =
					(hart->csr_store.pmpaddr[addr_count] << 2);
				pmp_addr_size = 4;
				break;
			default:
				break;
			}

			addr_end = addr + (len - 1);

			/*
			 * Check if the access partially overlaps with
			 * configured mem regions
			 */
			lower_addr_match =
				ADDR_WITHIN(addr, pmp_addr_start, pmp_addr_size);
			upper_addr_match =
				ADDR_WITHIN(addr_end, pmp_addr_start,
							pmp_addr_size);

			/*
			 * lower addr is within, but upper not, so access not
			 * granted, except when we are in machine mode and RWX
			 * flags match
			 */
			if (lower_addr_match && !upper_addr_match)
				return ((curr_priv == machine_mode) && (curr_access_flags & allowed_access))
						   ? 0
						   : -1;

			/*
			 * upper addr is within, but lower not, so access not
			 * granted, except when we are in machine mode and RWX
			 * flags match
			 */
			if (upper_addr_match && !lower_addr_match)
				return ((curr_priv == machine_mode) && (curr_access_flags & allowed_access))
						   ? 0
						   : -1;

			/*
			 * Both are within the range, return with OK
			 */
			if (upper_addr_match && lower_addr_match)
				return (curr_access_flags & allowed_access) ? 0 : -1;
		}
	}

	/*
	 * If we get here in machine mode, access is granted
	 */
	if (curr_priv == machine_mode)
		return 0;

	/*
	 * If at least one config is active and we are not in machine mode
	 * access is not granted
	 */
	if (at_least_one_active)
	{
		return -1;
	}

	/*
	 * No config seems to be active and therefore PMP is not used so access
	 * is granted
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
