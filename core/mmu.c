#include <stdio.h>
#include <string.h>

#include <mmu.h>
#include <trap.h>
#include <types.h>
#include <riscv_helper.h>

// #define MMU_DEBUG_ENABLE
#ifdef MMU_DEBUG_ENABLE
#define MMU_DEBUG(...)       \
	do                       \
	{                        \
		printf(__VA_ARGS__); \
	} while (0)
#else
#define MMU_DEBUG(...) \
	do                 \
	{                  \
	} while (0)
#endif

/*
 * PTE Flags are arranged as follows, the XWR (bus_access_type) bits are
 * shifted by on in contrast to the PMP, so we simply shift by 1 so we can
 * reuse it
 */
/*
 * 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
 */
/*
 * D | A | G | U | X | W | R | V
 */
/*
 * simple macro to tranlsate this to the mmu arrangement
 */
#define ACCESS_TYPE_TO_MMU(access_type) ((1 << access_type) << 1)

int mmu_write_csr(__unused u16 address, struct csr_mapping *map, uxlen val)
{
	int mode = (val >> 60);
	if (mode != 0 && mode != MMU_SATP_MODE_SV39)
		die("rv64 unsupported mode = %d", mode); // todo: remove

	*map->value = val;
	return 0;
}

int mmu_virt_to_phys(struct hart *hart,
					 privilege_level curr_priv,
					 uxlen *virt_addr, bus_access_type access_type, u8 mxr, u8 sum)
{
	/*
	 * We only have these here for debug purposes
	 */
	(void)hart;

	int i, j = 0;
	uxlen a = 0;
	uxlen pte = 0;
	uxlen pte_addr = 0;
	u8 pte_flags = 0;
	u8 user_page = 0;
	u8 mode = extractxlen(hart->csr_store.satp, MMU_SATP_MODE_BIT,
						  MMU_SATP_MODE_NR_BITS);

	/*
	 * in machine mode we don't have address translation
	 */
	if ((curr_priv == machine_mode) || !mode)
	{
		return 0;
	}
	// sv39
	uxlen vpn[SV39_LEVELS] = {
		(*virt_addr >> 12) & 0x1ff,
		(*virt_addr >> 21) & 0x1ff,
		(*virt_addr >> 30) & 0x1ff,
	};
	// MMU_DEBUG("vpn[1] "PRINTF_FMT"\n", vpn[1]);
	// MMU_DEBUG("vpn[0] "PRINTF_FMT"\n", vpn[0]);

	/*
	 * 1. Let a be satp.ppn × PAGESIZE, and let i = LEVELS − 1. (For Sv32, PAGESIZE=2^12 and LEVELS=2.)
	 */
	a = (hart->csr_store.satp & 0x3FFFFF) * SV39_PAGE_SIZE;
	MMU_DEBUG("satp: %x\n", hart->csr_store.satp);

	for (i = (SV39_LEVELS - 1), j = 0; i >= 0; i--, j++)
	{
		/*
		 * 2. Let pte be the value of the PTE at address a+va.vpn[i]×PTESIZE. (For Sv32, PTESIZE=4.)
		 * If accessing pte violates a PMA or PMP check, raise an access exception.
		 */
		pte_addr = a + (vpn[i] * SV39_PTESIZE);
		MMU_DEBUG("address a: " PRINTF_FMT " pte_addr: " PRINTF_FMT
				  "\n",
				  a, pte_addr);

		/*
		 * Here we should raise an exception if PMP violation occurs,
		 * will be done automatically if read_mem is set to the
		 * "checked_read_mem()" function.
		 */
		__access_protected_memory(1, hart, supervisor_mode,
								  bus_read_access, pte_addr, &pte,
								  sizeof(uxlen));
		MMU_DEBUG("pte[%d] " PRINTF_FMT "\n", i, pte);
		pte_flags = pte;

		/*
		 * 3. If pte.v = 0, or if pte.r = 0 and pte.w = 1, stop and raise a page-fault exception.
		 */
		if ((!(pte_flags & MMU_PAGE_VALID)) || ((!(pte_flags & MMU_PAGE_READ)) && (pte_flags & MMU_PAGE_WRITE)) || (pte_flags & MMU_PAGE_PBMT) || (pte_flags & MMU_PAGE_N) || (pte_flags & MMU_PAGE_RESERVED))
		{
			MMU_DEBUG("page fault: pte.v = 0, or if pte.r = 0 and pte.w = 1 access_type: %d a: " PRINTF_FMT " virt_addr: " PRINTF_FMT " pte: " PRINTF_FMT " pte_addr: " PRINTF_FMT
					  " flags: %x curr_priv: %d level: %d pc: " PRINTF_FMT " value: " PRINTF_FMT "\n",
					  access_type,
					  a, *virt_addr, pte, pte_addr, pte_flags, curr_priv,
					  i, hart->pc, value);
			return -1;
		}

		/*
		 * 4. Otherwise, the PTE is valid. If pte.r = 1 or pte.x = 1, go to step 5. Otherwise, this PTE is a
		 * pointer to the next level of the page table. Let i = i − 1. If i < 0, stop and raise a page-fault
		 * exception. Otherwise, let a = pte.ppn × PAGESIZE and go to step 2.
		 */
		/*
		 * check if any RWX flag is set
		 */
		if (pte_flags & 0xA)
		{
			MMU_DEBUG("Leaf pte %d\n", i);
			break;
		}

		a = ((pte >> 10) & (((uxlen)1 << 44) - 1)) * SV39_PAGE_SIZE;
	}

	if (i < 0)
	{
		MMU_DEBUG("page fault: i < 0\n");
		return -1;
	}

	/*
	 * 5. A leaf PTE has been found. Determine if the requested memory access is allowed by the
	 * pte.r, pte.w, pte.x, and pte.u bits, given the current privilege mode and the value of the SUM
	 * and MXR fields of the mstatus register. If not, stop and raise a page-fault exception.
	 */
	user_page = pte_flags & MMU_PAGE_USER;

	/*
	 * User has only access to user pages
	 */
	if ((curr_priv == user_mode) && !user_page)
	{
		printf("page fault: user access to higher priv page!\n");
		return -1;
	}

	/*
	 * Supervisor only has access to user pages if SUM = 1
	 */
	if ((curr_priv == supervisor_mode) && user_page && !sum)
	{
		MMU_DEBUG("page fault: supervisor access to user page!\n");
		return -1;
	}

	/*
	 * Check if MXR
	 */
	if ((access_type == bus_read_access) && (ACCESS_TYPE_TO_MMU(bus_instr_access) & pte_flags) && mxr)
		pte_flags |= MMU_PAGE_READ;

	if (!(ACCESS_TYPE_TO_MMU(access_type) & pte_flags))
	{
		MMU_DEBUG("page fault: invalid RWX flags!\n");
		return -1;
	}

	/*
	 * physical addresses are 64 Bit wide!!! even on RV39 systems
	 */
	uxlen ppn[SV39_LEVELS] = {
		(pte >> 10) & 0x1ff,
		(pte >> 19) & 0x1ff,
		(pte >> 28) & 0x3ffffff,
	};
	// MMU_DEBUG("ppn[1]: %x\n", ppn[1]);
	// MMU_DEBUG("ppn[0]: %x\n", ppn[0]);

	/*
	 * physical addresses are at least 34 Bits wide, so we need u64 here
	 */
	u64 phys_addr_translation[SV39_LEVELS] = {
		(ppn[2] << 30) | (ppn[1] << 21) | (ppn[0] << 12) | (*virt_addr & 0xfff),
		(ppn[2] << 30) | (ppn[1] << 21) | (*virt_addr & 0x1fffff),
		(ppn[2] << 30) | (*virt_addr & 0x3fffffff),
	};

	/*
	 * 6. If i > 0 and pa.ppn[i − 1 : 0] != 0, this is a misaligned superpage; stop and raise a page-fault exception.
	 */
	if (i > 0)
	{
		for (int k = 0; k < i; k++)
		{
			if (ppn[k] != 0)
			{
				MMU_DEBUG("misaligned superpage!\n");
				return -1;
			}
		}
	}

	/*
	 * 7. If pte.a = 0, or if the memory access is a store and pte.d = 0, either raise a page-fault exception or:
	 *  - Set pte.a to 1 and, if the memory access is a store, also set pte.d to 1.
	 *  - If this access violates a PMA or PMP check, raise an access exception.
	 *  - This update and the loading of pte in step 2 must be atomic; in particular, no intervening store to the PTE may be perceived to have occurred in-between.
	 */
	if ((!(pte_flags & MMU_PAGE_ACCESSED)) || ((access_type == bus_write_access) && !(pte_flags & MMU_PAGE_DIRTY)))
	{
		// printf("pta.a or pte.d page fault!\n");
		return -1;
	}

	/*
	 * 8. The translation is successful. The translated physical address is given as follows:
	 * - pa.pgoff = va.pgoff.
	 * - If i > 0, then this is a superpage translation and pa.ppn[i − 1 : 0] = va.vpn[i − 1 : 0].
	 * - pa.ppn[LEVELS − 1 : i] = pte.ppn[LEVELS − 1 : i].
	 */
	*virt_addr = phys_addr_translation[i];
	return 0;
}

privilege_level check_mpoverride(struct hart *hart, bus_access_type access_type)
{
	if (access_type == bus_instr_access)
		return hart->curr_priv_mode;

	int mprv = extractxlen(hart->csr_store.status,
						   TRAP_XSTATUS_MPBIT, 1);
	privilege_level ret_val = extractxlen(hart->csr_store.status,
										  TRAP_XSTATUS_MPP_BIT, 2);
	return mprv ? ret_val : hart->curr_priv_mode;
}

int vm_check(struct hart *hart, privilege_level priv_level,
			 bus_access_type access_type, uxlen *addr,
			 __unused void *value, __unused u8 len)
{
	(void)priv_level;
	privilege_level internal_priv_level =
		check_mpoverride(hart, access_type);
	uxlen trap_cause;
	switch (access_type)
	{
	case bus_instr_access:
		trap_cause = trap_cause_instr_page_fault;
		break;
	case bus_read_access:
		trap_cause = trap_cause_load_page_fault;
		break;
	case bus_write_access:
		trap_cause = trap_cause_store_amo_page_fault;
		break;
	default:
		die("unreachable");
	}

	u8 mxr = CHECK_BIT(hart->csr_store.status,
					   TRAP_XSTATUS_MXR_BIT)
				 ? 1
				 : 0;
	u8 sum = CHECK_BIT(hart->csr_store.status,
					   TRAP_XSTATUS_SUM_BIT)
				 ? 1
				 : 0;

	if (mmu_virt_to_phys(hart, internal_priv_level, addr,
						 access_type, mxr, sum))
	{
		prepare_sync_trap(hart, trap_cause, *addr);
		return -1;
	}
	return 0;
}
