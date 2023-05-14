#include <stdlib.h>
#include <stdio.h>
#include <hart.h>
#include <soc.h>
#include <helpers.h>

char *csr_names[CSR_ADDR_MAX] = {
	[0x001] = "fflags",
	[0x002] = "frm",
	[0x003] = "fcsr",
	[0xC00] = "cycle",
	[0xC01] = "time",
	[0xC02] = "instret",
	[0xC03] = "hpmcounter3",
	[0xC04] = "hpmcounter4",
	[0xC05] = "hpmcounter5",
	[0xC06] = "hpmcounter6",
	[0xC07] = "hpmcounter7",
	[0xC08] = "hpmcounter8",
	[0xC09] = "hpmcounter9",
	[0xC0A] = "hpmcounter10",
	[0xC0B] = "hpmcounter11",
	[0xC0C] = "hpmcounter12",
	[0xC0D] = "hpmcounter13",
	[0xC0E] = "hpmcounter14",
	[0xC0F] = "hpmcounter15",
	[0xC10] = "hpmcounter16",
	[0xC11] = "hpmcounter17",
	[0xC12] = "hpmcounter18",
	[0xC13] = "hpmcounter19",
	[0xC14] = "hpmcounter20",
	[0xC15] = "hpmcounter21",
	[0xC16] = "hpmcounter22",
	[0xC17] = "hpmcounter23",
	[0xC18] = "hpmcounter24",
	[0xC19] = "hpmcounter25",
	[0xC1A] = "hpmcounter26",
	[0xC1B] = "hpmcounter27",
	[0xC1C] = "hpmcounter28",
	[0xC1D] = "hpmcounter29",
	[0xC1E] = "hpmcounter30",
	[0xC1F] = "hpmcounter31",

	[0x100] = "sstatus",
	[0x104] = "sie",
	[0x105] = "stvec",
	[0x106] = "scounteren",
	[0x10A] = "senvcfg",
	[0x140] = "sscratch",
	[0x141] = "sepc",
	[0x142] = "scause",
	[0x143] = "stval",
	[0x144] = "sip",
	[0x180] = "satp",
	[0x5A8] = "scontext",

	[0xF11] = "mvendorid",
	[0xF12] = "marchid",
	[0xF13] = "mimpid",
	[0xF14] = "mhartid",
	[0xF15] = "mconfigptr",

	[0x300] = "mstatus",
	[0x301] = "misa",
	[0x302] = "medeleg",
	[0x303] = "mideleg",
	[0x304] = "mie",
	[0x305] = "mtvec",
	[0x306] = "mcounteren",

	[0x340] = "mscratch",
	[0x341] = "mepc",
	[0x342] = "mcause",
	[0x343] = "mtval",
	[0x344] = "mip",
	[0x34A] = "mtinst",
	[0x34B] = "mtval2",

	[0x30A] = "menvcfg",
	[0x747] = "mseccfg",

	[0x3A0] = "pmpcfg0",
	[0x3A1] = "pmpcfg1",
	[0x3A2] = "pmpcfg2",
	[0x3A3] = "pmpcfg3",
	[0x3A4] = "pmpcfg4",
	[0x3A5] = "pmpcfg5",
	[0x3A6] = "pmpcfg6",
	[0x3A7] = "pmpcfg7",
	[0x3A8] = "pmpcfg8",
	[0x3A9] = "pmpcfg9",
	[0x3AA] = "pmpcfg10",
	[0x3AB] = "pmpcfg11",
	[0x3AC] = "pmpcfg12",
	[0x3AD] = "pmpcfg13",
	[0x3AE] = "pmpcfg14",
	[0x3AF] = "pmpcfg15",

	[0x3B0] = "pmpaddr0",
	[0x3B1] = "pmpaddr1",
	[0x3B2] = "pmpaddr2",
	[0x3B3] = "pmpaddr3",
	[0x3B4] = "pmpaddr4",
	[0x3B5] = "pmpaddr5",
	[0x3B6] = "pmpaddr6",
	[0x3B7] = "pmpaddr7",
	[0x3B8] = "pmpaddr8",
	[0x3B9] = "pmpaddr9",
	[0x3BA] = "pmpaddr10",
	[0x3BB] = "pmpaddr11",
	[0x3BC] = "pmpaddr12",
	[0x3BD] = "pmpaddr13",
	[0x3BE] = "pmpaddr14",
	[0x3BF] = "pmpaddr15",
	[0x3C0] = "pmpaddr16",
	[0x3C1] = "pmpaddr17",
	[0x3C2] = "pmpaddr18",
	[0x3C3] = "pmpaddr19",
	[0x3C4] = "pmpaddr20",
	[0x3C5] = "pmpaddr21",
	[0x3C6] = "pmpaddr22",
	[0x3C7] = "pmpaddr23",
	[0x3C8] = "pmpaddr24",
	[0x3C9] = "pmpaddr25",
	[0x3CA] = "pmpaddr26",
	[0x3CB] = "pmpaddr27",
	[0x3CC] = "pmpaddr28",
	[0x3CD] = "pmpaddr29",
	[0x3CE] = "pmpaddr30",
	[0x3CF] = "pmpaddr31",
	[0x3D0] = "pmpaddr32",
	[0x3D1] = "pmpaddr33",
	[0x3D2] = "pmpaddr34",
	[0x3D3] = "pmpaddr35",
	[0x3D4] = "pmpaddr36",
	[0x3D5] = "pmpaddr37",
	[0x3D6] = "pmpaddr38",
	[0x3D7] = "pmpaddr39",
	[0x3D8] = "pmpaddr40",
	[0x3D9] = "pmpaddr41",
	[0x3DA] = "pmpaddr42",
	[0x3DB] = "pmpaddr43",
	[0x3DC] = "pmpaddr44",
	[0x3DD] = "pmpaddr45",
	[0x3DE] = "pmpaddr46",
	[0x3DF] = "pmpaddr47",
	[0x3E0] = "pmpaddr48",
	[0x3E1] = "pmpaddr49",
	[0x3E2] = "pmpaddr50",
	[0x3E3] = "pmpaddr51",
	[0x3E4] = "pmpaddr52",
	[0x3E5] = "pmpaddr53",
	[0x3E6] = "pmpaddr54",
	[0x3E7] = "pmpaddr55",
	[0x3E8] = "pmpaddr56",
	[0x3E9] = "pmpaddr57",
	[0x3EA] = "pmpaddr58",
	[0x3EB] = "pmpaddr59",
	[0x3EC] = "pmpaddr60",
	[0x3ED] = "pmpaddr61",
	[0x3EE] = "pmpaddr62",
	[0x3EF] = "pmpaddr63",

	[0xB00] = "mcycle",
	/* no mtime */
	[0xB02] = "minstret",
	[0xB03] = "mhpmcounter3",
	[0xB04] = "mhpmcounter4",
	[0xB05] = "mhpmcounter5",
	[0xB06] = "mhpmcounter6",
	[0xB07] = "mhpmcounter7",
	[0xB08] = "mhpmcounter8",
	[0xB09] = "mhpmcounter9",
	[0xB0A] = "mhpmcounter10",
	[0xB0B] = "mhpmcounter11",
	[0xB0C] = "mhpmcounter12",
	[0xB0D] = "mhpmcounter13",
	[0xB0E] = "mhpmcounter14",
	[0xB0F] = "mhpmcounter15",
	[0xB10] = "mhpmcounter16",
	[0xB11] = "mhpmcounter17",
	[0xB12] = "mhpmcounter18",
	[0xB13] = "mhpmcounter19",
	[0xB14] = "mhpmcounter20",
	[0xB15] = "mhpmcounter21",
	[0xB16] = "mhpmcounter22",
	[0xB17] = "mhpmcounter23",
	[0xB18] = "mhpmcounter24",
	[0xB19] = "mhpmcounter25",
	[0xB1A] = "mhpmcounter26",
	[0xB1B] = "mhpmcounter27",
	[0xB1C] = "mhpmcounter28",
	[0xB1D] = "mhpmcounter29",
	[0xB1E] = "mhpmcounter30",
	[0xB1F] = "mhpmcounter31",

	[0x320] = "mcountinhibit",
	[0x323] = "mhpmevent3",
	[0x324] = "mhpmevent4",
	[0x325] = "mhpmevent5",
	[0x326] = "mhpmevent6",
	[0x327] = "mhpmevent7",
	[0x328] = "mhpmevent8",
	[0x329] = "mhpmevent9",
	[0x32A] = "mhpmevent10",
	[0x32B] = "mhpmevent11",
	[0x32C] = "mhpmevent12",
	[0x32D] = "mhpmevent13",
	[0x32E] = "mhpmevent14",
	[0x32F] = "mhpmevent15",
	[0x330] = "mhpmevent16",
	[0x331] = "mhpmevent17",
	[0x332] = "mhpmevent18",
	[0x333] = "mhpmevent19",
	[0x334] = "mhpmevent20",
	[0x335] = "mhpmevent21",
	[0x336] = "mhpmevent22",
	[0x337] = "mhpmevent23",
	[0x338] = "mhpmevent24",
	[0x339] = "mhpmevent25",
	[0x33A] = "mhpmevent26",
	[0x33B] = "mhpmevent27",
	[0x33C] = "mhpmevent28",
	[0x33D] = "mhpmevent29",
	[0x33E] = "mhpmevent30",
	[0x33F] = "mhpmevent31",

	[0x7A0] = "tselect",
	[0x7A1] = "tdata1",
	[0x7A2] = "tdata2",
	[0x7A3] = "tdata3",
	[0x7A8] = "mcontext",

	[0x7B0] = "dcsr",
	[0x7B1] = "dpc",
	[0x7B2] = "dscratch0",
	[0x7B3] = "dscratch1",
};

char *get_csr_name(u16 addr)
{
	if (addr <= CSR_ADDR_MAX && csr_names[addr])
		return csr_names[addr];
	char *buffer = malloc(20);
	sprintf(buffer, "%#x", addr);
	return buffer;
}

int csr_read_reg(struct csr_reg *csr_regs, privilege_level curr_priv_mode,
				 u16 address, uxlen *out_val)
{
#ifdef CSR_TRACE
	debug("csr(%s) read (valid=%d, value=%#lx) addr=%#x, curr-priv=%d(reg priv=%d)\n",
			get_csr_name(address),
			csr_regs[address].valid, *csr_regs[address].value, address, curr_priv_mode,
			((address >> 8) & 0x3));
#endif

	if (csr_regs[address].valid && CSR_READABLE(address, curr_priv_mode))
	{
		if (csr_regs[address].read)
			return csr_regs[address].read(address,
										  &csr_regs[address],
										  out_val);

		*out_val = *csr_regs[address].value & csr_regs[address].mask;
		return 0;
	}

	debug("csr read fault(valid=%d) addr=%#x, curr-priv=%d(reg priv=%d)\n",
		   csr_regs[address].valid, address, curr_priv_mode,
		   ((address >> 8) & 0x3));
	return -1;
}

int csr_write_reg(struct csr_reg *csr_regs, privilege_level curr_priv_mode,
				  u16 address, uxlen val)
{
#ifdef CSR_TRACE
	debug("csr(%s) write (valid=%d, value=%x) addr=%#x, curr-priv=%d(reg priv=%d)\n",
			get_csr_name(address),
			csr_regs[address].valid, val, address, curr_priv_mode,
			((address >> 8) & 0x3));
#endif
	if (csr_regs[address].valid && CSR_WRITABLE(address, curr_priv_mode))
	{
		if (csr_regs[address].write)
			return csr_regs[address].write(address,
										   &csr_regs[address], val);
		// clear allowed fields
		*csr_regs[address].value =
			*csr_regs[address].value & ~csr_regs[address].mask;
		// fill them with new value
		*csr_regs[address].value |= val & csr_regs[address].mask;
		return 0;
	}

	debug("csr write fault(valid=%d) addr=%#x, curr-priv=%d(reg priv=%d)\n",
		   csr_regs[address].valid, address, curr_priv_mode,
		   ((address >> 8) & 0x3));
	return -1;
}

void hart_init_csr_regs(struct hart *hart)
{
	u16 i = 0;
	// 2UL << (XLEN - 2) = means xlen = 64
	hart->csr_store.isa = SUPPORTED_EXTENSIONS | (2UL << (XLEN - 2));

	/*
	 * Machine Information Registers
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MVENDORID, CSR_MASK_NONE,
						 &hart->csr_store.vendorid);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MARCHID, CSR_MASK_NONE,
						 &hart->csr_store.archid);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MIMPID, CSR_MASK_NONE,
						 &hart->csr_store.impid);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MHARTID, CSR_MASK_NONE,
						 &hart->csr_store.hartid);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MISA, CSR_MASK_ALL,
						 &hart->csr_store.isa); // CSR_MASK_ALL is necessary or else the software can't read the value

	/*
	 * Machine Trap Registers
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MSTATUS, CSR_MSTATUS_MASK,
						 &hart->csr_store.status);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MEDELEG, CSR_MEDELEG_MASK,
						 &hart->csr_store.medeleg);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MIDELEG, CSR_MIDELEG_MASK,
						 &hart->csr_store.mideleg);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MIE, CSR_MIP_MIE_MASK,
						 &hart->csr_store.ie);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MTVEC, CSR_MTVEC_MASK,
						 &hart->csr_store.mtvec);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MCOUNTEREN, CSR_MASK_NONE,
						 &hart->csr_store.mcounteren);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MCOUNTERINHIBIT, CSR_MASK_NONE,
						 &hart->csr_store.mcounterinhibit);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MSCRATCH, CSR_MASK_ALL,
						 &hart->csr_store.mscratch);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MEPC, CSR_MASK_ALL,
						 &hart->csr_store.mepc);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MCAUSE, CSR_MASK_ALL,
						 &hart->csr_store.mcause);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MTVAL, CSR_MASK_ALL,
						 &hart->csr_store.mtval);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MIP, CSR_MIP_MIE_MASK,
						 &hart->csr_store.ip);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MENVCFG, CSR_MASK_NONE,
						 &hart->csr_store.menvcfg);

	/*
	 * Physical Memory Protection
	 */

	/*
	 * PMP Config Registers
	 */
	for (i = 0; i < PMP_NR_CFG_REGS; i++)
	{
		INIT_CSR_REG_SPECIAL(hart->csr_regs, (CSR_PMPCFG0 + i),
							 CSR_MASK_ALL,
							 &hart->csr_store.pmpcfg[i], NULL,
							 pmp_write_csr_cfg);
	}
	/*
	 * All others are WARL
	 */
	static uxlen dummy_pmp;
	for (i = PMP_NR_CFG_REGS; i < PMP_NR_CFG_REGS_WARL_MAX; i++)
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_PMPCFG0 + i),
							 CSR_MASK_NONE, &dummy_pmp);

	/*
	 * PMP Address Registers
	 */
	for (i = 0; i < PMP_NR_ADDR_REGS; i++)
	{
		INIT_CSR_REG_SPECIAL_COOKIE(hart->csr_regs, (CSR_PMPADDR0 + i),
									CSR_MASK_ALL,
									&hart->csr_store.pmpaddr[i], NULL,
									pmp_write_csr_addr,
									&hart->csr_store.pmpcfg);
	}
	/*
	 * All others are WARL
	 */
	for (i = PMP_NR_ADDR_REGS; i < PMP_NR_ADDR_REGS_WARL_MAX; i++)
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_PMPADDR0 + i),
							 CSR_MASK_NONE, &dummy_pmp);

	/*
	 * Supervisor Trap Registers
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SSTATUS, CSR_SSTATUS_MASK,
						 &hart->csr_store.status);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SIE, CSR_SIP_SIE_MASK,
						 &hart->csr_store.ie);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SIP, CSR_SIP_SIE_MASK,
						 &hart->csr_store.ip);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_STVEC, CSR_STVEC_MASK,
						 &hart->csr_store.stvec);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SCOUNTEREN, CSR_MASK_NONE,
						 &hart->csr_store.scounteren);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SSCRATCH, CSR_MASK_ALL,
						 &hart->csr_store.sscratch);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SEPC, CSR_MASK_ALL,
						 &hart->csr_store.sepc);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_SCAUSE, CSR_MASK_ALL,
						 &hart->csr_store.scause);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_STVAL, CSR_MASK_ALL,
						 &hart->csr_store.stval);

	/*
	 * Supervisor Address Translation and Protection
	 */
	INIT_CSR_REG_SPECIAL(hart->csr_regs, CSR_ADDR_SATP, CSR_SATP_MASK,
						 &hart->csr_store.satp, NULL, mmu_write_csr);

	/*
	 * cycle, time, instret
	 */
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MCYCLE, CSR_MASK_ALL,
						 &hart->csr_store.cycle);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_MINSTRET, CSR_MASK_ALL,
						 &hart->csr_store.instret);

	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_CYCLE, CSR_MASK_ALL,
						 &hart->csr_store.cycle);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_TIME, CSR_MASK_ALL,
						 &hart->soc->clint.regs[clint_mtime]);
	INIT_CSR_REG_DEFAULT(hart->csr_regs, CSR_ADDR_INSTRET, CSR_MASK_ALL,
						 &hart->csr_store.instret);

	/*
	 * Performance Counters
	 */
	static uxlen dummy_hpm;
	for (i = 3; i < CSR_HPMCOUNTER_WARL_MAX; i++)
	{
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_ADDR_MCYCLE + i),
							 CSR_MASK_NONE, &dummy_hpm);
		INIT_CSR_REG_DEFAULT(hart->csr_regs, (CSR_ADDR_CYCLE + i),
							 CSR_MASK_NONE, &dummy_hpm);
	}
}
