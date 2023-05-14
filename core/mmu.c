#include <stdio.h>
#include <string.h>

#include <mmu.h>
#include <trap.h>
#include <types.h>
#include <helpers.h>

#define ACCESS_TYPE_TO_MMU(access_type) (1 << access_type)

int mmu_write_csr(__maybe_unused u16 address, struct csr_mapping *map, uxlen val)
{
	int mode = SATP_MODE(val);
	if (mode != MMU_SATP_MODE_OFF && mode != MMU_SATP_MODE_SV39) {
		debug("rv64 unsupported mode = %d", mode);
		return 0;
	}

	*map->value = val;
	return 0;
}
/**
 * check *virt_addr permissions using page tables,
 * then translate it into a physical address
 * and store it into the same pointer *virt_addr
 * 
 * on successful translation it returns 0, otherwise != 0
*/
int mmu_virt_to_phys(struct hart __maybe_unused *hart,
					 privilege_level curr_priv,
					 uxlen *virt_addr, bus_access_type access_type, u8 mxr, u8 sum)
{
	int i;
	pte_t pte;

	/*
	 * address translation works only with supervisor and user mode
	 */
	if ((curr_priv > supervisor_mode) || SATP_MODE(hart->csr_store.satp) == MMU_SATP_MODE_OFF)
		return 0;

	uxlen vpn[SV39_LEVELS] = {
		(*virt_addr >> 30) & 0x1ff,
		(*virt_addr >> 21) & 0x1ff,
		(*virt_addr >> 12) & 0x1ff,
	};

	/*
	 * this is not a real pointer (guest pointer), we just made it so
	 * to benefit from pointer arithmetics
	 */
	pte_t *page_table = SATP_PPN(hart->csr_store.satp) << PAGE_SHIFT;

	for (i = 0; i < SV39_LEVELS; i++)
	{
		/*
		 * this is another fake pointer
		 * this doesn't perform any memory access
		 * it just computes the offset of the pte
		 */
		pte_t *pte_addr = &page_table[vpn[i]];

		access_supervisor_physical_memory(hart, supervisor_mode,
								  bus_read_access, pte_addr, &pte,
								  SV39_PTESIZE);

		/*
		 * check invalid combinations
		 */
		if (!pte.valid || (!pte.read && pte.write) || pte.pbmt || pte.n || pte.__reserved)
			return -1;

		/*
		 * if any RWX flag is set then we are in a leaf
		 */
		if (pte.read || pte.write || pte.exec)
			break;

		/*
		 * check invalid combinations
		 * D, A and U are reserved in non leaf pte
		 */
		if (pte.dirty || pte.accessed || pte.user)
			return -1;

		page_table = PTE_PPN(pte) << PAGE_SHIFT;
	}
	/**
	 * no leaf pte found
	*/
	if (i == SV39_LEVELS) {
		debug("page fault: no leaf found!\n");
		return -1;
	}

	/*
	 * User has only access to user pages
	 */
	if (curr_priv == user_mode && !pte.user)
	{
		debug("page fault: user access to higher priv page!\n");
		return -1;
	}

	/*
	 * Supervisor only has access to user pages if SUM = 1
	 * but only for loads and stores, exec is never allowed
	 * permit Supervisor User Memory access
	 */
	if (curr_priv == supervisor_mode && pte.user && (!sum || access_type == bus_exec_access)) {
		debug("page fault: supervisor access user pages!\n");
		return -1;
	}

	/*
	 * Check if MXR
	 * Make Executable Readable
	 */
	if (mxr && pte.exec)
		pte.read = 1;

	if (!(ACCESS_TYPE_TO_MMU(access_type) & PTE_ACCESS_FLAGS(pte))) {
		debug("page fault: insufficiant permissions! access=%#x pte=%#x\n", ACCESS_TYPE_TO_MMU(access_type), PTE_ACCESS_FLAGS(pte));
		return -1;
	}

	/**
	 * now let's check the alignement of the physical page
	*/
	uxlen ppn[SV39_LEVELS] = {
		pte.ppn2,
		pte.ppn1,
		pte.ppn0,
	};

	/**
	 * if we found leaf pte after i iteration
	 * all other remaining ppn should be zero
	*/
	for (int j = i + 1; j < SV39_LEVELS; j++) {
		if (ppn[j]) { // ppn not aligned
			debug("page fault: non aligned physical address!\n");
			return -1;
		}
	}

	/**
	 * if A is not set trigger page fault
	 * on write access D bit should also be set
	*/
	if (!pte.accessed || ((access_type == bus_write_access) && !pte.dirty)) {
		debug("page fault: dirty or accessed bit are not set!\n");
		return -1;
	}

	u64 offsets[SV39_LEVELS] = {
		(*virt_addr & 0x3fffffff),
		(*virt_addr & 0x1fffff),
		(*virt_addr & 0xfff),
	};
	*virt_addr = (PTE_PPN(pte) << PAGE_SHIFT) | offsets[i];
	return 0;
}

/**
 * M mode could masquerade as another priv level (in MPP)
 * to use mmu to perform loads and stores but no exec
 * when MPRV (Modify PRiVlege) is set
*/
privilege_level effective_priv_level(struct hart *hart, bus_access_type access_type)
{
	if (access_type == bus_exec_access)
		return hart->curr_priv_mode;

	int mprv = GET_BIT(hart->csr_store.status, TRAP_XSTATUS_MPRV_BIT);
	privilege_level ret_val = GET_BIT_RANGE(hart->csr_store.status, TRAP_XSTATUS_MPP_BITS, 2);
	return mprv ? ret_val : hart->curr_priv_mode;
}

int vm_check(struct hart *hart, privilege_level __maybe_unused priv_level,
			 bus_access_type access_type, uxlen *addr,
			 __maybe_unused void *value, __maybe_unused u8 len)
{
	privilege_level internal_priv_level = effective_priv_level(hart, access_type);
	uxlen trap_cause;
	switch (access_type)
	{
	case bus_exec_access:
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

	u8 mxr = GET_BIT(hart->csr_store.status, TRAP_XSTATUS_MXR_BIT);
	u8 sum = GET_BIT(hart->csr_store.status, TRAP_XSTATUS_SUM_BIT);

	if (mmu_virt_to_phys(hart, internal_priv_level, addr,
						 access_type, mxr, sum))
	{
		prepare_sync_trap(hart, trap_cause, *addr);
		return -1;
	}
	return 0;
}
