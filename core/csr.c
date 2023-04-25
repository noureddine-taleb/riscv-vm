#include <stdio.h>

#include <hart.h>

#include <riscv_helper.h>

#include <soc.h>

uxlen csr_get_mask(struct csr_mapping *csr_regs, u16 address)
{
	return csr_regs[address].mask;
}

int
csr_read_reg(struct csr_mapping *csr_regs, privilege_level curr_priv_mode,
	     u16 address, uxlen *out_val)
{
	if (address > CSR_ADDR_MAX)
		return -1;

	if (csr_regs[address].valid && CSR_READABLE(address, curr_priv_mode)) {

		if (csr_regs[address].read)
			return csr_regs[address].read(address,
						      &csr_regs[address],
						      out_val);

		*out_val = *csr_regs[address].value & csr_regs[address].mask;
		return 0;
	}

	printf("-------> csr(v=%d) = %x, priv=%d(reg priv=%d), read fault\n",
	       csr_regs[address].valid, address, curr_priv_mode,
	       ((address >> 8) & 0x3));
	return -1;
}

int
csr_write_reg(struct csr_mapping *csr_regs, privilege_level curr_priv_mode,
	      u16 address, uxlen val)
{
	if (address > CSR_ADDR_MAX)
		return -1;

	if (csr_regs[address].valid && CSR_WRITABLE(address, curr_priv_mode)) {
		if (csr_regs[address].write)
			return csr_regs[address].write(address,
						       &csr_regs[address], val);

		*csr_regs[address].value =
		    *csr_regs[address].value & ~csr_regs[address].mask;
		*csr_regs[address].value |= val & csr_regs[address].mask;
		return 0;
	}

	printf("-------> csr(v=%d) = %x, priv=%d(reg priv=%d), write fault\n",
	       csr_regs[address].valid, address, curr_priv_mode,
	       ((address >> 8) & 0x3));
	return -1;
}

void hart_init_csr_regs(struct hart *hart)
{
	u16 i = 0;
	hart->csr_store.isa = SUPPORTED_EXTENSIONS | (2UL << (XLEN - 2));

	/*
	 * Machine Information Registers 
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MVENDORID, CSR_MASK_ZERO,
			     &hart->csr_store.vendorid);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MARCHID, CSR_MASK_ZERO,
			     &hart->csr_store.archid);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MIMPID, CSR_MASK_ZERO,
			     &hart->csr_store.impid);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MHARTID, CSR_MASK_ZERO,
			     &hart->csr_store.hartid);

	/*
	 * Machine Trap Setup 
	 */
	// todo: change default masks
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MSTATUS, CSR_MSTATUS_MASK,
			     &hart->csr_store.status);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MISA, CSR_MASK_WR_ALL,
			     &hart->csr_store.isa);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MEDELEG, CSR_MEDELEG_MASK,
			     &hart->csr_store.medeleg);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MIDELEG, CSR_MIDELEG_MASK,
			     &hart->csr_store.mideleg);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MIE, CSR_MIP_MIE_MASK,
			     &hart->csr_store.ie);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MTVEC, CSR_MTVEC_MASK,
			     &hart->csr_store.mtvec);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MCOUNTEREN, CSR_MASK_ZERO,
			     &hart->csr_store.mcounteren);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MCOUNTERINHIBIT, CSR_MASK_ZERO,
			     &hart->csr_store.mcounterinhibit);

	/*
	 * Machine Trap Handling 
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MSCRATCH, CSR_MASK_WR_ALL,
			     &hart->csr_store.mscratch);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MEPC, CSR_MASK_WR_ALL,
			     &hart->csr_store.mepc);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MCAUSE, CSR_MASK_WR_ALL,
			     &hart->csr_store.mcause);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MTVAL, CSR_MASK_WR_ALL,
			     &hart->csr_store.mtval);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MIP, CSR_MIP_MIE_MASK,
			     &hart->csr_store.ip);

	/*
	 * Machine Protection and Translation 
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MENVCFG, CSR_MASK_ZERO,
				&hart->csr_store.menvcfg);

	for (i = 0; i < PMP_NR_CFG_REGS; i++) {
		INIT_CSR_REG_SPECIAL(hart->csr_regs, (CSR_PMPCFG0 + i),
				     CSR_MASK_WR_ALL,
				     &hart->csr_store.pmpcfg[i], NULL,
				     pmp_write_csr_cfg);
	}

	/*
	 * All others are WARL 
	 */
	static uxlen dummy_pmp;
	for (i = PMP_NR_CFG_REGS; i < PMP_NR_CFG_REGS_WARL_MAX; i++)
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_PMPCFG0 + i),
				     CSR_MASK_ZERO, &dummy_pmp);

	for (i = 0; i < PMP_NR_ADDR_REGS; i++) {
		INIT_CSR_REG_SPECIAL_COOKIE(hart->csr_regs, (CSR_PMPADDR0 + i),
					    CSR_MASK_WR_ALL,
					    &hart->csr_store.pmpaddr[i], NULL,
					    pmp_write_csr_addr,
					    &hart->csr_store.pmpcfg);
	}

	/*
	 * All others are WARL 
	 */
	for (i = PMP_NR_ADDR_REGS; i < PMP_NR_ADDR_REGS_WARL_MAX; i++)
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_PMPADDR0 + i),
				     CSR_MASK_ZERO, &dummy_pmp);

	/*
	 * Supervisor Trap Setup 
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SSTATUS, CSR_SSTATUS_MASK,
			     &hart->csr_store.status);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SIE, CSR_SIP_SIE_MASK,
			     &hart->csr_store.ie);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_STVEC, CSR_STVEC_MASK,
			     &hart->csr_store.stvec);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SCOUNTEREN, CSR_MASK_ZERO,
			     &hart->csr_store.scounteren);

	/*
	 * Supervisor Trap Setup 
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SSCRATCH, CSR_MASK_WR_ALL,
			     &hart->csr_store.sscratch);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SEPC, CSR_MASK_WR_ALL,
			     &hart->csr_store.sepc);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SCAUSE, CSR_MASK_WR_ALL,
			     &hart->csr_store.scause);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_STVAL, CSR_MASK_WR_ALL,
			     &hart->csr_store.stval);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SIP, CSR_SIP_SIE_MASK,
			     &hart->csr_store.ip);

	/*
	 * Supervisor Address Translation and Protection 
	 */
	INIT_CSR_REG_SPECIAL(hart->csr_regs, CSR_ADDR_SATP, CSR_SATP_MASK,
			     &hart->csr_store.satp, NULL, mmu_write_csr);

	// todo: remove csrs ends with H
	/*
	 * Performance Counters 
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MCYCLE, CSR_MASK_WR_ALL,
			     &hart->csr_store.cycle);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MCYCLEH, CSR_MASK_WR_ALL,
			     &hart->csr_store.cycle);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MINSTRET, CSR_MASK_WR_ALL,
			     &hart->csr_store.instret);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MINSTRETH,
			     CSR_MASK_WR_ALL, &hart->csr_store.cycle);

	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_CYCLE, CSR_MASK_WR_ALL,
			     &hart->csr_store.cycle);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_CYCLEH, CSR_MASK_WR_ALL,
			     &hart->csr_store.cycle);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_TIME, CSR_MASK_WR_ALL,
			     &hart->csr_store.time);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_TIMEH, CSR_MASK_WR_ALL,
			     &hart->csr_store.time);

	/*
	 * All others are WARL, they start at 3 
	 */
	static uxlen dummy_hpm;
	for (i = 3; i < CSR_HPMCOUNTER_WARL_MAX; i++) {
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_ADDR_MCYCLE + i),
				     CSR_MASK_ZERO, &dummy_hpm);
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_ADDR_CYCLE + i),
				     CSR_MASK_ZERO, &dummy_hpm);
	}

	for (i = 3; i < CSR_HPMCOUNTER_WARL_MAX; i++) {
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_ADDR_MCYCLEH + i),
				     CSR_MASK_ZERO, &dummy_hpm);
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_ADDR_CYCLEH + i),
				     CSR_MASK_ZERO, &dummy_hpm);
	}
}
