#include <stdio.h>
#include <types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <types.h>
#include <riscv_helper.h>
#include <riscv_instr.h>
#include <hart.h>

/*
 * Implementations of the RISCV instructions
 */
void instr_NOP(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	(void)hart;
	return;
}

/*
 * Implementations of the RISCV instructions
 */
void instr_WFI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	(void)hart;
	usleep(10000);
}

/*
 * RISCV Instructions
 */
void instr_LUI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = (hart->immediate << 12);
}

void instr_AUIPC(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = (hart->pc) + (hart->immediate << 12);
}

void instr_JAL(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->pc + 4;

	if (ADDR_MISALIGNED(hart->jump_offset))
	{
		die("Addr misaligned!\n");
		prepare_sync_trap(hart, trap_cause_instr_addr_misalign, 0);
		return;
	}

	hart->next_pc = hart->pc + hart->jump_offset;
}

void instr_JALR(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen curr_pc = hart->pc + 4;
	hart->jump_offset = SIGNEX_BIT_11(hart->immediate);

	uxlen target_pc = (hart->x[hart->rs1] + hart->jump_offset);
	target_pc &= ~(1 << 0);
	if (ADDR_MISALIGNED(target_pc))
	{
		die("Addr misaligned!\n");
		prepare_sync_trap(hart, trap_cause_instr_addr_misalign, 0);
		return;
	}
	hart->next_pc = target_pc;
	hart->x[hart->rd] = curr_pc;
}

void instr_BEQ(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->x[hart->rs1] == hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->jump_offset))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->jump_offset;
	}
}

void instr_BNE(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->x[hart->rs1] != hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->jump_offset))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->jump_offset;
	}
}

void instr_BLT(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	if (signed_rs < signed_rs2)
	{
		if (ADDR_MISALIGNED(hart->jump_offset))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->jump_offset;
	}
}

void instr_BGE(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	if (signed_rs >= signed_rs2)
	{
		if (ADDR_MISALIGNED(hart->jump_offset))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->jump_offset;
	}
}

void instr_BLTU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->x[hart->rs1] < hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->jump_offset))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->jump_offset;
	}
}

void instr_BGEU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->x[hart->rs1] >= hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->jump_offset))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->jump_offset;
	}
}

void instr_ADDI(struct hart *hart)
{
	CORE_DBG("%s: %x " PRINTF_FMT "\n", __func__, hart->instruction,
			 hart->pc);
	ixlen signed_immediate = SIGNEX_BIT_11(hart->immediate);
	ixlen signed_rs_val = hart->x[hart->rs1];
	CORE_DBG("%s: " PRINTF_FMT " " PRINTF_FMT " " PRINTF_FMT " %x\n",
			 __func__, hart->x[hart->rs1], signed_rs_val,
			 signed_immediate, hart->rs1);
	hart->x[hart->rd] = (signed_immediate + signed_rs_val);
}

void instr_SLTI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_immediate = SIGNEX_BIT_11(hart->immediate);
	ixlen signed_rs_val = hart->x[hart->rs1];

	if (signed_rs_val < signed_immediate)
		hart->x[hart->rd] = 1;
	else
		hart->x[hart->rd] = 0;
}

void instr_SLTIU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen unsigned_immediate = SIGNEX_BIT_11(hart->immediate);
	uxlen unsigned_rs_val = hart->x[hart->rs1];

	if (unsigned_rs_val < unsigned_immediate)
		hart->x[hart->rd] = 1;
	else
		hart->x[hart->rd] = 0;
}

void instr_XORI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_immediate = SIGNEX_BIT_11(hart->immediate);
	hart->immediate = signed_immediate;

	if (signed_immediate == -1)
		hart->x[hart->rd] = hart->x[hart->rs1] ^ -1;
	else
		hart->x[hart->rd] = hart->x[hart->rs1] ^ hart->immediate;
}

void instr_ORI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->immediate = SIGNEX_BIT_11(hart->immediate);
	hart->x[hart->rd] = hart->x[hart->rs1] | hart->immediate;
}

void instr_ANDI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->immediate = SIGNEX_BIT_11(hart->immediate);
	hart->x[hart->rd] = hart->x[hart->rs1] & hart->immediate;
}

void instr_SLLI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] =
		(hart->x[hart->rs1] << (hart->immediate & SHIFT_OP_MASK));
}

void instr_SRAI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen rs_val = hart->x[hart->rs1];

	/*
	 * a right shift on signed ints seem to be always arithmetic
	 */
	rs_val = rs_val >> (hart->immediate & SHIFT_OP_MASK);
	hart->x[hart->rd] = rs_val;
}

void instr_SRLI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] =
		(hart->x[hart->rs1] >> (hart->immediate & SHIFT_OP_MASK));
}

void instr_ADD(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	CORE_DBG("%s: " PRINTF_FMT " %x\n", __func__, hart->x[hart->rs1],
			 hart->rs1);
	hart->x[hart->rd] = hart->x[hart->rs1] + hart->x[hart->rs2];
}

void instr_SUB(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->x[hart->rs1] - hart->x[hart->rs2];
}

void instr_SLL(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] =
		hart->x[hart->rs1] << (hart->x[hart->rs2] & SHIFT_OP_MASK);
}

void instr_SLT(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	if (signed_rs < signed_rs2)
		hart->x[hart->rd] = 1;
	else
		hart->x[hart->rd] = 0;
}

void instr_SLTU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->rs1 == 0)
	{
		if (hart->x[hart->rs2])
			hart->x[hart->rd] = 1;
		else
			hart->x[hart->rd] = 0;
	}
	else
	{
		if (hart->x[hart->rs1] < hart->x[hart->rs2])
			hart->x[hart->rd] = 1;
		else
			hart->x[hart->rd] = 0;
	}
}

void instr_XOR(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->x[hart->rs1] ^ hart->x[hart->rs2];
}

void instr_SRL(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] =
		hart->x[hart->rs1] >> (hart->x[hart->rs2] & SHIFT_OP_MASK);
}

void instr_OR(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->x[hart->rs1] | (hart->x[hart->rs2]);
}

void instr_AND(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->x[hart->rs1] & (hart->x[hart->rs2]);
}

void instr_SRA(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	hart->x[hart->rd] = signed_rs >> (hart->x[hart->rs2] & SHIFT_OP_MASK);
}

void instr_LB(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u8 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 1) == 0)
		hart->x[hart->rd] = SIGNEX_BIT_7(tmp_load_val);
}

void instr_LH(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u16 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 2) == 0)
		hart->x[hart->rd] = SIGNEX_BIT_15(tmp_load_val);
}

void instr_LW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 4) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void instr_LBU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u8 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 1) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void instr_LHU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u16 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 2) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void instr_SB(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	u8 value_to_write = (u8)hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value_to_write, 1);
}

void instr_SH(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	u16 value_to_write = (u16)hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value_to_write, 2);
}

void instr_SW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	uxlen value_to_write = (uxlen)hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value_to_write, 4);
}

void instr_LWU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 tmp_load_val = 0;
	uxlen unsigned_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + unsigned_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 4) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void instr_LD(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	ixlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 8) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void instr_SD(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_offset = SIGNEX_BIT_11(hart->immediate);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	uxlen value_to_write = (uxlen)hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value_to_write, 8);
}

void instr_SRAIW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_rs_val = hart->x[hart->rs1];
	hart->x[hart->rd] = (signed_rs_val >> (hart->immediate & 0x1F));
}

void instr_ADDIW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_immediate = SIGNEX_BIT_11(hart->immediate);
	i32 signed_rs_val = hart->x[hart->rs1];
	hart->x[hart->rd] = (signed_rs_val + signed_immediate);
}

void instr_SLLIW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_tmp32 =
		(hart->x[hart->rs1] << (hart->immediate & 0x1F)) & 0xFFFFFFFF;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void instr_SRLIW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 unsigned_rs_val = hart->x[hart->rs1];
	i32 signed_tmp32 = (unsigned_rs_val >> (hart->immediate & 0x1F));
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void instr_SRLW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 rs1_val = hart->x[hart->rs1];
	u32 rs2_val = (hart->x[hart->rs2] & 0x1F);
	i32 signed_tmp32 = rs1_val >> rs2_val;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void instr_SRAW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 rs1_val_signed = hart->x[hart->rs1];
	u32 rs2_val = (hart->x[hart->rs2] & 0x1F);
	i32 signed_tmp32 = rs1_val_signed >> rs2_val;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void instr_SLLW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 rs1_val = hart->x[hart->rs1];
	u32 rs2_val = (hart->x[hart->rs2] & 0x1F);
	i32 signed_tmp32 = rs1_val << rs2_val;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void instr_ADDW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 rs1_val = hart->x[hart->rs1];
	u32 rs2_val = hart->x[hart->rs2];
	i32 signed_tmp32 = rs1_val + rs2_val;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void instr_SUBW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 rs1_val = hart->x[hart->rs1];
	u32 rs2_val = hart->x[hart->rs2];
	i32 signed_tmp32 = rs1_val - rs2_val;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void CSRRWx(struct hart *hart, uxlen new_val)
{
	CORE_DBG("%s: %x " PRINTF_FMT " priv level: %d\n", __func__,
			 hart->instruction, hart->pc, hart->curr_priv_mode);
	uxlen csr_val = 0;
	u16 csr_addr = hart->immediate;
	uxlen csr_mask = csr_get_mask(hart->csr_regs, csr_addr);
	uxlen not_allowed_bits = 0;
	uxlen new_csr_val = 0;

	if (hart->rd != 0)
	{
		if (csr_read_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr,
						 &csr_val))
		{
			prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
			return;
		}
	}

	not_allowed_bits = csr_val & ~csr_mask;
	new_csr_val = not_allowed_bits | (new_val & csr_mask);

	if (csr_write_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr, new_csr_val))
	{
		prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
		return;
	}

	hart->x[hart->rd] = csr_val & csr_mask;
}

void CSRRSx(struct hart *hart, uxlen new_val)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen csr_val = 0;
	u16 csr_addr = hart->immediate;
	uxlen csr_mask = csr_get_mask(hart->csr_regs, csr_addr);
	uxlen new_csr_val = 0;

	if (csr_read_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr, &csr_val))
	{
		prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
		return;
	}

	new_csr_val = (new_val & csr_mask);

	if (hart->rs1 != 0)
	{
		if (csr_write_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr,
						  csr_val | new_csr_val))
		{
			// die_msg("Error reading CSR %x "PRINTF_FMT"\n",
			// csr_addr, hart->pc);
			prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
			return;
		}
	}

	hart->x[hart->rd] = csr_val & csr_mask;
}

void CSRRCx(struct hart *hart, uxlen new_val)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen csr_val = 0;
	u16 csr_addr = hart->immediate;
	uxlen csr_mask = csr_get_mask(hart->csr_regs, csr_addr);
	uxlen new_csr_val = 0;

	if (csr_read_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr, &csr_val))
	{
		// die_msg("Error reading CSR %x "PRINTF_FMT"\n", csr_addr,
		// hart->pc);
		prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
		return;
	}

	new_csr_val = (new_val & csr_mask);

	if (hart->rs1 != 0)
	{
		if (csr_write_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr,
						  csr_val & ~new_csr_val))
		{
			// die_msg("Error reading CSR %x "PRINTF_FMT"\n",
			// csr_addr, hart->pc);
			prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
			return;
		}
	}
	hart->x[hart->rd] = csr_val & csr_mask;
}

void instr_CSRRW(struct hart *hart)
{
	CSRRWx(hart, hart->x[hart->rs1]);
}

void instr_CSRRS(struct hart *hart)
{
	CSRRSx(hart, hart->x[hart->rs1]);
}

void instr_CSRRC(struct hart *hart)
{
	CSRRCx(hart, hart->x[hart->rs1]);
}

void instr_CSRRWI(struct hart *hart)
{
	CSRRWx(hart, hart->rs1);
}

void instr_CSRRSI(struct hart *hart)
{
	CSRRSx(hart, hart->rs1);
}

void instr_CSRRCI(struct hart *hart)
{
	CSRRCx(hart, hart->rs1);
}

void instr_ECALL(struct hart *hart)
{
	prepare_sync_trap(hart,
					  trap_cause_user_ecall + hart->curr_priv_mode, 0);
}

void instr_EBREAK(struct hart *hart)
{
	/*
	 * not implemented
	 */
	(void)hart;
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
}

void instr_MRET(struct hart *hart)
{
	CORE_DBG("%s: " PRINTF_FMT "\n", __func__,
			 hart->csr_store.ip);
	return_from_exception(hart, hart->curr_priv_mode);
}

void instr_SRET(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	return_from_exception(hart, hart->curr_priv_mode);
}

void instr_URET(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	printf("URET!\n");
	while (1)
		;
	/*
	 * not implemented
	 */
	(void)hart;
}

void instr_LR_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->lr_address = hart->x[hart->rs1];
	hart->lr_valid = 1;
	instr_LW(hart);
}

void instr_SC_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->lr_valid && (hart->lr_address == hart->x[hart->rs1]))
	{
		instr_SW(hart);
		hart->x[hart->rd] = 0;
	}
	else
	{
		hart->x[hart->rd] = 1;
	}

	hart->lr_valid = 0;
	hart->lr_address = 0;
}

void instr_AMOSWAP_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	instr_LW(hart);
	result = rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void instr_AMOADD_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	instr_LW(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val + rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void instr_AMOXOR_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	instr_LW(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val ^ rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void instr_AMOAND_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	instr_LW(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val & rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void instr_AMOOR_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	instr_LW(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val | rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void instr_AMOMIN_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	i32 rd_val = 0;
	i32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LW(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void instr_AMOMAX_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	i32 rd_val = 0;
	i32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LW(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void instr_AMOMINU_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LW(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void instr_AMOMAXU_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LW(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void instr_LR_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->lr_valid = 1;
	hart->lr_address = hart->x[hart->rs1];
	instr_LD(hart);
}

void instr_SC_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->lr_valid && (hart->lr_address == hart->x[hart->rs1]))
	{
		instr_SD(hart);
		hart->x[hart->rd] = 0;
	}
	else
	{
		hart->x[hart->rd] = 1;
	}

	hart->lr_valid = 0;
	hart->lr_address = 0;
}

void instr_AMOSWAP_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LD(hart);
	result = rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void instr_AMOADD_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LD(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val + rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void instr_AMOXOR_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LD(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val ^ rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void instr_AMOAND_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LD(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val & rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void instr_AMOOR_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LD(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val | rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void instr_AMOMIN_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	ixlen rd_val = 0;
	ixlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LD(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void instr_AMOMAX_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	ixlen rd_val = 0;
	ixlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LD(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void instr_AMOMINU_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LD(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void instr_AMOMAXU_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	instr_LD(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void instr_DIV(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	/*
	 * division by zero
	 */
	if (signed_rs2 == 0)
	{
		hart->x[hart->rd] = -1;
		return;
	}

	/*
	 * overflow
	 */
	if (((uxlen)signed_rs == XLEN_INT_MIN) && (signed_rs2 == -1))
	{
		hart->x[hart->rd] = XLEN_INT_MIN;
		return;
	}

	hart->x[hart->rd] = (signed_rs / signed_rs2);
}

void instr_DIVU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen unsigned_rs = hart->x[hart->rs1];
	uxlen unsigned_rs2 = hart->x[hart->rs2];

	/*
	 * division by zero
	 */
	if (unsigned_rs2 == 0)
	{
		hart->x[hart->rd] = -1;
		return;
	}

	hart->x[hart->rd] = (unsigned_rs / unsigned_rs2);
}

void instr_REM(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	/*
	 * division by zero
	 */
	if (signed_rs2 == 0)
	{
		hart->x[hart->rd] = signed_rs;
		return;
	}

	/*
	 * overflow
	 */
	if (((uxlen)signed_rs == XLEN_INT_MIN) && (signed_rs2 == -1))
	{
		hart->x[hart->rd] = 0;
		return;
	}

	hart->x[hart->rd] = (signed_rs % signed_rs2);
}

void instr_REMU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen unsigned_rs = hart->x[hart->rs1];
	uxlen unsigned_rs2 = hart->x[hart->rs2];

	/*
	 * division by zero
	 */
	if (unsigned_rs2 == 0)
	{
		hart->x[hart->rd] = unsigned_rs;
		return;
	}

	hart->x[hart->rd] = (unsigned_rs % unsigned_rs2);
}

void instr_MUL(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];
	hart->x[hart->rd] = signed_rs * signed_rs2;
}

void instr_MULH(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen result_hi = 0;
	ixlen result_lo = 0;
	MUL(hart->x[hart->rs1], hart->x[hart->rs2], &result_hi, &result_lo);
	hart->x[hart->rd] = result_hi;
}

void instr_MULHU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen result_hi = 0;
	uxlen result_lo = 0;
	UMUL(hart->x[hart->rs1], hart->x[hart->rs2], &result_hi, &result_lo);
	hart->x[hart->rd] = result_hi;
}

void instr_MULHSU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen result_hi = 0;
	ixlen result_lo = 0;
	MULHSU(hart->x[hart->rs1], hart->x[hart->rs2], &result_hi, &result_lo);
	hart->x[hart->rd] = result_hi;
}

void instr_MULW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_rs = hart->x[hart->rs1];
	i32 signed_rs2 = hart->x[hart->rs2];
	hart->x[hart->rd] = (ixlen)(signed_rs * signed_rs2);
}

void instr_DIVW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_rs = hart->x[hart->rs1];
	i32 signed_rs2 = hart->x[hart->rs2];
	i32 result = 0;

	/*
	 * division by zero
	 */
	if (signed_rs2 == 0)
	{
		hart->x[hart->rd] = -1;
		return;
	}

	/*
	 * overflow
	 */
	if ((signed_rs == INT32_MIN) && (signed_rs2 == -1))
	{
		hart->x[hart->rd] = INT32_MIN;
		return;
	}

	result = (signed_rs / signed_rs2);

	hart->x[hart->rd] = (ixlen)result;
}

void instr_DIVUW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 unsigned_rs = hart->x[hart->rs1];
	u32 unsigned_rs2 = hart->x[hart->rs2];
	u32 result = 0;

	/*
	 * division by zero
	 */
	if (unsigned_rs2 == 0)
	{
		hart->x[hart->rd] = -1;
		return;
	}

	result = (unsigned_rs / unsigned_rs2);

	hart->x[hart->rd] = SIGNEX_BIT_31(result);
}

void instr_REMW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_rs = hart->x[hart->rs1];
	i32 signed_rs2 = hart->x[hart->rs2];
	i32 result = 0;

	/*
	 * division by zero
	 */
	if (signed_rs2 == 0)
	{
		hart->x[hart->rd] = (ixlen)signed_rs;
		return;
	}

	/*
	 * overflow
	 */
	if ((signed_rs == INT32_MIN) && (signed_rs2 == -1))
	{
		hart->x[hart->rd] = 0;
		return;
	}

	result = (signed_rs % signed_rs2);

	hart->x[hart->rd] = (ixlen)result;
}

void instr_REMUW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 unsigned_rs = hart->x[hart->rs1];
	u32 unsigned_rs2 = hart->x[hart->rs2];
	u32 result = 0;

	/*
	 * division by zero
	 */
	if (unsigned_rs2 == 0)
	{
		hart->x[hart->rd] = SIGNEX_BIT_31(unsigned_rs);
		return;
	}

	result = (unsigned_rs % unsigned_rs2);

	hart->x[hart->rd] = SIGNEX_BIT_31(result);
}

void preparation_func5(struct hart *hart, i32 *next_subcode)
{
	hart->func5 = ((hart->instruction >> 27) & 0x1F);
	*next_subcode = hart->func5;
}

void preparation_func6(struct hart *hart, i32 *next_subcode)
{
	hart->func6 = ((hart->instruction >> 26) & 0x3F);
	*next_subcode = hart->func6;
}

void preparation_func7(struct hart *hart, i32 *next_subcode)
{
	hart->func7 = ((hart->instruction >> 25) & 0x7F);
	*next_subcode = hart->func7;
}

void preparation_func7_func12_sub5_extended(struct hart *hart,
											i32 *next_subcode)
{
	hart->func5 = ((hart->instruction >> 20) & 0x1F);
	*next_subcode = hart->func5;
}

void R_type_preparation(struct hart *hart, i32 *next_subcode)
{
	hart->rd = ((hart->instruction >> 7) & 0x1F);
	hart->func3 = ((hart->instruction >> 12) & 0x7);
	hart->rs1 = ((hart->instruction >> 15) & 0x1F);
	hart->rs2 = ((hart->instruction >> 20) & 0x1F);
	*next_subcode = hart->func3;
}

void I_type_preparation(struct hart *hart, i32 *next_subcode)
{
	hart->rd = ((hart->instruction >> 7) & 0x1F);
	hart->func3 = ((hart->instruction >> 12) & 0x7);
	hart->rs1 = ((hart->instruction >> 15) & 0x1F);
	hart->immediate = ((hart->instruction >> 20) & 0xFFF);
	*next_subcode = hart->func3;
}

void S_type_preparation(struct hart *hart, i32 *next_subcode)
{
	hart->func3 = ((hart->instruction >> 12) & 0x7);
	hart->rs1 = ((hart->instruction >> 15) & 0x1F);
	hart->rs2 = ((hart->instruction >> 20) & 0x1F);
	hart->immediate =
		(((hart->instruction >> 25) << 5) |
		 ((hart->instruction >> 7) & 0x1F));
	*next_subcode = hart->func3;
}

void B_type_preparation(struct hart *hart, i32 *next_subcode)
{
	hart->rd = ((hart->instruction >> 7) & 0x1F);
	hart->func3 = ((hart->instruction >> 12) & 0x7);
	hart->rs1 = ((hart->instruction >> 15) & 0x1F);
	hart->rs2 = ((hart->instruction >> 20) & 0x1F);
	hart->jump_offset = ((extract32(hart->instruction, 8, 4) << 1) |
						 (extract32(hart->instruction, 25, 6) << 5) |
						 (extract32(hart->instruction, 7, 1) << 11) |
						 (extract32(hart->instruction, 31, 1) << 12));
	hart->jump_offset = SIGNEX_BIT_12(hart->jump_offset);
	*next_subcode = hart->func3;
}

void U_type_preparation(struct hart *hart, i32 *next_subcode)
{
	hart->rd = ((hart->instruction >> 7) & 0x1F);
	hart->immediate = ((hart->instruction >> 12) & 0xFFFFF);
	hart->immediate = SIGNEX_BIT_19(hart->immediate);
	*next_subcode = -1;
}

void J_type_preparation(struct hart *hart, i32 *next_subcode)
{
	hart->rd = ((hart->instruction >> 7) & 0x1F);
	hart->jump_offset = ((extract32(hart->instruction, 21, 10) << 1) |
						 (extract32(hart->instruction, 20, 1) << 11) |
						 (extract32(hart->instruction, 12, 8) << 12) |
						 (extract32(hart->instruction, 31, 1) << 20));
	/*
	 * sign extend the 20 bit number
	 */
	hart->jump_offset = SIGNEX_BIT_20(hart->jump_offset);
	*next_subcode = -1;
}

instruction_hook_t JALR_func3_subcode_list[] = {
	[FUNC3_INSTR_JALR] = {NULL, instr_JALR, NULL},
};

INIT_INSTRUCTION_LIST_DESC(JALR_func3_subcode_list);

instruction_hook_t BEQ_BNE_BLT_BGE_BLTU_BGEU_func3_subcode_list[] = {
	[FUNC3_INSTR_BEQ] = {NULL, instr_BEQ, NULL},
	[FUNC3_INSTR_BNE] = {NULL, instr_BNE, NULL},
	[FUNC3_INSTR_BLT] = {NULL, instr_BLT, NULL},
	[FUNC3_INSTR_BGE] = {NULL, instr_BGE, NULL},
	[FUNC3_INSTR_BLTU] = {NULL, instr_BLTU, NULL},
	[FUNC3_INSTR_BGEU] = {NULL, instr_BGEU, NULL},
};

INIT_INSTRUCTION_LIST_DESC(BEQ_BNE_BLT_BGE_BLTU_BGEU_func3_subcode_list);

instruction_hook_t LB_LH_LW_LBU_LHU_LWU_LD_func3_subcode_list[] = {
	[FUNC3_INSTR_LB] = {NULL, instr_LB, NULL},
	[FUNC3_INSTR_LH] = {NULL, instr_LH, NULL},
	[FUNC3_INSTR_LW] = {NULL, instr_LW, NULL},
	[FUNC3_INSTR_LBU] = {NULL, instr_LBU, NULL},
	[FUNC3_INSTR_LHU] = {NULL, instr_LHU, NULL},
	[FUNC3_INSTR_LWU] = {NULL, instr_LWU, NULL},
	[FUNC3_INSTR_LD] = {NULL, instr_LD, NULL},
};

INIT_INSTRUCTION_LIST_DESC(LB_LH_LW_LBU_LHU_LWU_LD_func3_subcode_list);

instruction_hook_t SB_SH_SW_SD_func3_subcode_list[] = {
	[FUNC3_INSTR_SB] = {NULL, instr_SB, NULL},
	[FUNC3_INSTR_SH] = {NULL, instr_SH, NULL},
	[FUNC3_INSTR_SW] = {NULL, instr_SW, NULL},
	[FUNC3_INSTR_SD] = {NULL, instr_SD, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SB_SH_SW_SD_func3_subcode_list);

instruction_hook_t SRLI_SRAI_func6_subcode_list[] = {
	[FUNC6_INSTR_SRLI] = {NULL, instr_SRLI, NULL},
	[FUNC6_INSTR_SRAI] = {NULL, instr_SRAI, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SRLI_SRAI_func6_subcode_list);

instruction_hook_t
	ADDI_SLTI_SLTIU_XORI_ORI_ANDI_SLLI_SRLI_SRAI_func3_subcode_list[] = {
		[FUNC3_INSTR_ADDI] = {NULL, instr_ADDI, NULL},
		[FUNC3_INSTR_SLTI] = {NULL, instr_SLTI, NULL},
		[FUNC3_INSTR_SLTIU] = {NULL, instr_SLTIU, NULL},
		[FUNC3_INSTR_XORI] = {NULL, instr_XORI, NULL},
		[FUNC3_INSTR_ORI] = {NULL, instr_ORI, NULL},
		[FUNC3_INSTR_ANDI] = {NULL, instr_ANDI, NULL},
		[FUNC3_INSTR_SLLI] = {NULL, instr_SLLI, NULL},
		[FUNC3_INSTR_SRLI_SRAI] =
			{preparation_func6, NULL, &SRLI_SRAI_func6_subcode_list_desc},
};

INIT_INSTRUCTION_LIST_DESC(ADDI_SLTI_SLTIU_XORI_ORI_ANDI_SLLI_SRLI_SRAI_func3_subcode_list);

instruction_hook_t ADD_SUB_MUL_func7_subcode_list[] = {
	[FUNC7_INSTR_ADD] = {NULL, instr_ADD, NULL},
	[FUNC7_INSTR_SUB] = {NULL, instr_SUB, NULL},
	[FUNC7_INSTR_MUL] = {NULL, instr_MUL, NULL},
};

INIT_INSTRUCTION_LIST_DESC(ADD_SUB_MUL_func7_subcode_list);

instruction_hook_t SLL_MULH_func7_subcode_list[] = {
	[FUNC7_INSTR_SLL] = {NULL, instr_SLL, NULL},
	[FUNC7_INSTR_MUL] = {NULL, instr_MULH, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SLL_MULH_func7_subcode_list);

instruction_hook_t SLT_MULHSU_func7_subcode_list[] = {
	[FUNC7_INSTR_SLT] = {NULL, instr_SLT, NULL},
	[FUNC7_INSTR_MULHSU] = {NULL, instr_MULHSU, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SLT_MULHSU_func7_subcode_list);

instruction_hook_t SLTU_MULHU_func7_subcode_list[] = {
	[FUNC7_INSTR_SLTU] = {NULL, instr_SLTU, NULL},
	[FUNC7_INSTR_MULHU] = {NULL, instr_MULHU, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SLTU_MULHU_func7_subcode_list);

instruction_hook_t XOR_DIV_func7_subcode_list[] = {
	[FUNC7_INSTR_XOR] = {NULL, instr_XOR, NULL},
	[FUNC7_INSTR_DIV] = {NULL, instr_DIV, NULL},
};

INIT_INSTRUCTION_LIST_DESC(XOR_DIV_func7_subcode_list);

instruction_hook_t SRL_SRA_DIVU_func7_subcode_list[] = {
	[FUNC7_INSTR_SRL] = {NULL, instr_SRL, NULL},
	[FUNC7_INSTR_SRA] = {NULL, instr_SRA, NULL},
	[FUNC7_INSTR_DIVU] = {NULL, instr_DIVU, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SRL_SRA_DIVU_func7_subcode_list);

instruction_hook_t OR_REM_func7_subcode_list[] = {
	[FUNC7_INSTR_OR] = {NULL, instr_OR, NULL},
	[FUNC7_INSTR_REM] = {NULL, instr_REM, NULL},
};

INIT_INSTRUCTION_LIST_DESC(OR_REM_func7_subcode_list);

instruction_hook_t AND_REMU_func7_subcode_list[] = {
	[FUNC7_INSTR_AND] = {NULL, instr_AND, NULL},
	[FUNC7_INSTR_REMU] = {NULL, instr_REMU, NULL},
};

INIT_INSTRUCTION_LIST_DESC(AND_REMU_func7_subcode_list);

instruction_hook_t
	ADD_SUB_SLL_SLT_SLTU_XOR_SRL_SRA_OR_AND_func3_subcode_list[] = {
		[FUNC3_INSTR_ADD_SUB_MUL] =
			{preparation_func7, NULL, &ADD_SUB_MUL_func7_subcode_list_desc},
		[FUNC3_INSTR_SLL_MULH] =
			{preparation_func7, NULL, &SLL_MULH_func7_subcode_list_desc},
		[FUNC3_INSTR_SLT_MULHSU] =
			{preparation_func7, NULL, &SLT_MULHSU_func7_subcode_list_desc},
		[FUNC3_INSTR_SLTU_MULHU] =
			{preparation_func7, NULL, &SLTU_MULHU_func7_subcode_list_desc},
		[FUNC3_INSTR_XOR_DIV] =
			{preparation_func7, NULL, &XOR_DIV_func7_subcode_list_desc},
		[FUNC3_INSTR_SRL_SRA_DIVU] =
			{preparation_func7, NULL, &SRL_SRA_DIVU_func7_subcode_list_desc},
		[FUNC3_INSTR_OR_REM] =
			{preparation_func7, NULL, &OR_REM_func7_subcode_list_desc},
		[FUNC3_INSTR_AND_REMU] =
			{preparation_func7, NULL, &AND_REMU_func7_subcode_list_desc},
};

INIT_INSTRUCTION_LIST_DESC(ADD_SUB_SLL_SLT_SLTU_XOR_SRL_SRA_OR_AND_func3_subcode_list);

instruction_hook_t SRLIW_SRAIW_func7_subcode_list[] = {
	[FUNC7_INSTR_SRLIW] = {NULL, instr_SRLIW, NULL},
	[FUNC7_INSTR_SRAIW] = {NULL, instr_SRAIW, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SRLIW_SRAIW_func7_subcode_list);

instruction_hook_t SLLIW_SRLIW_SRAIW_ADDIW_func3_subcode_list[] = {
	[FUNC3_INSTR_SLLIW] = {NULL, instr_SLLIW, NULL},
	[FUNC3_INSTR_SRLIW_SRAIW] =
		{preparation_func7, NULL, &SRLIW_SRAIW_func7_subcode_list_desc},
	[FUNC3_INSTR_ADDIW] = {NULL, instr_ADDIW, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SLLIW_SRLIW_SRAIW_ADDIW_func3_subcode_list);

instruction_hook_t SRLW_SRAW_DIVUW_func7_subcode_list[] = {
	[FUNC7_INSTR_SRLW] = {NULL, instr_SRLW, NULL},
	[FUNC7_INSTR_SRAW] = {NULL, instr_SRAW, NULL},
	[FUNC7_INSTR_DIVUW] = {NULL, instr_DIVUW, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SRLW_SRAW_DIVUW_func7_subcode_list);

instruction_hook_t ADDW_SUBW_MULW_func7_subcode_list[] = {
	[FUNC7_INSTR_ADDW] = {NULL, instr_ADDW, NULL},
	[FUNC7_INSTR_SUBW] = {NULL, instr_SUBW, NULL},
	[FUNC7_INSTR_MULW] = {NULL, instr_MULW, NULL},
};

INIT_INSTRUCTION_LIST_DESC(ADDW_SUBW_MULW_func7_subcode_list);

instruction_hook_t
	ADDW_SUBW_SLLW_SRLW_SRAW_MULW_DIVW_DIVUW_REMW_REMUW_func3_subcode_list[] = {
		[FUNC3_INSTR_ADDW_SUBW_MULW] = {preparation_func7, NULL,
										&ADDW_SUBW_MULW_func7_subcode_list_desc},
		[FUNC3_INSTR_SLLW] = {NULL, instr_SLLW, NULL},
		[FUNC3_INSTR_SRLW_SRAW_DIVUW] = {preparation_func7, NULL,
										 &SRLW_SRAW_DIVUW_func7_subcode_list_desc},
		[FUNC3_INSTR_DIVW] = {NULL, instr_DIVW, NULL},
		[FUNC3_INSTR_REMW] = {NULL, instr_REMW, NULL},
		[FUNC3_INSTR_REMUW] = {NULL, instr_REMUW, NULL},
};

INIT_INSTRUCTION_LIST_DESC(ADDW_SUBW_SLLW_SRLW_SRAW_MULW_DIVW_DIVUW_REMW_REMUW_func3_subcode_list);

instruction_hook_t ECALL_EBREAK_URET_func12_sub5_subcode_list[] = {
	[FUNC5_INSTR_ECALL] = {NULL, instr_ECALL, NULL},
	[FUNC5_INSTR_EBREAK] = {NULL, instr_EBREAK, NULL},
	[FUNC5_INSTR_URET] = {NULL, instr_URET, NULL},
};

INIT_INSTRUCTION_LIST_DESC(ECALL_EBREAK_URET_func12_sub5_subcode_list);

instruction_hook_t SRET_WFI_func12_sub5_subcode_list[] = {
	[FUNC5_INSTR_SRET] = {NULL, instr_SRET, NULL},
	[FUNC5_INSTR_WFI] = {NULL, instr_WFI, NULL},
};

INIT_INSTRUCTION_LIST_DESC(SRET_WFI_func12_sub5_subcode_list);

instruction_hook_t
	ECALL_EBREAK_URET_SRET_MRET_WFI_SFENCEVMA_func7_subcode_list[] = {
		[FUNC7_INSTR_ECALL_EBREAK_URET] =
			{preparation_func7_func12_sub5_extended, NULL,
			 &ECALL_EBREAK_URET_func12_sub5_subcode_list_desc},
		[FUNC7_INSTR_SRET_WFI] = {preparation_func7_func12_sub5_extended, NULL,
								  &SRET_WFI_func12_sub5_subcode_list_desc},
		[FUNC7_INSTR_MRET] = {NULL, instr_MRET, NULL},
		[FUNC7_INSTR_SFENCEVMA] = {NULL, instr_NOP, NULL},
};

INIT_INSTRUCTION_LIST_DESC(ECALL_EBREAK_URET_SRET_MRET_WFI_SFENCEVMA_func7_subcode_list);

instruction_hook_t
	CSRRW_CSRRS_CSRRC_CSRRWI_CSRRSI_CSRRCI_ECALL_EBREAK_URET_SRET_MRET_WFI_SFENCEVMA_func3_subcode_list
		[] = {
			[FUNC3_INSTR_CSRRW] = {NULL, instr_CSRRW, NULL},
			[FUNC3_INSTR_CSRRS] = {NULL, instr_CSRRS, NULL},
			[FUNC3_INSTR_CSRRC] = {NULL, instr_CSRRC, NULL},
			[FUNC3_INSTR_CSRRWI] = {NULL, instr_CSRRWI, NULL},
			[FUNC3_INSTR_CSRRSI] = {NULL, instr_CSRRSI, NULL},
			[FUNC3_INSTR_CSRRCI] = {NULL, instr_CSRRCI, NULL},
			[FUNC3_INSTR_ECALL_EBREAK_MRET_SRET_URET_WFI_SFENCEVMA] =
				{preparation_func7, NULL,
				 &ECALL_EBREAK_URET_SRET_MRET_WFI_SFENCEVMA_func7_subcode_list_desc}};

INIT_INSTRUCTION_LIST_DESC(CSRRW_CSRRS_CSRRC_CSRRWI_CSRRSI_CSRRCI_ECALL_EBREAK_URET_SRET_MRET_WFI_SFENCEVMA_func3_subcode_list);

instruction_hook_t
	W_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func5_subcode_list[] = {
		[FUNC5_INSTR_AMO_LR] = {NULL, instr_LR_W, NULL},
		[FUNC5_INSTR_AMO_SC] = {NULL, instr_SC_W, NULL},
		[FUNC5_INSTR_AMO_SWAP] = {NULL, instr_AMOSWAP_W, NULL},
		[FUNC5_INSTR_AMO_ADD] = {NULL, instr_AMOADD_W, NULL},
		[FUNC5_INSTR_AMO_XOR] = {NULL, instr_AMOXOR_W, NULL},
		[FUNC5_INSTR_AMO_AND] = {NULL, instr_AMOAND_W, NULL},
		[FUNC5_INSTR_AMO_OR] = {NULL, instr_AMOOR_W, NULL},
		[FUNC5_INSTR_AMO_MIN] = {NULL, instr_AMOMIN_W, NULL},
		[FUNC5_INSTR_AMO_MAX] = {NULL, instr_AMOMAX_W, NULL},
		[FUNC5_INSTR_AMO_MINU] = {NULL, instr_AMOMINU_W, NULL},
		[FUNC5_INSTR_AMO_MAXU] = {NULL, instr_AMOMAXU_W, NULL},
};

INIT_INSTRUCTION_LIST_DESC(W_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func5_subcode_list);

instruction_hook_t
	D_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func5_subcode_list[] = {
		[FUNC5_INSTR_AMO_LR] = {NULL, instr_LR_D, NULL},
		[FUNC5_INSTR_AMO_SC] = {NULL, instr_SC_D, NULL},
		[FUNC5_INSTR_AMO_SWAP] = {NULL, instr_AMOSWAP_D, NULL},
		[FUNC5_INSTR_AMO_ADD] = {NULL, instr_AMOADD_D, NULL},
		[FUNC5_INSTR_AMO_XOR] = {NULL, instr_AMOXOR_D, NULL},
		[FUNC5_INSTR_AMO_AND] = {NULL, instr_AMOAND_D, NULL},
		[FUNC5_INSTR_AMO_OR] = {NULL, instr_AMOOR_D, NULL},
		[FUNC5_INSTR_AMO_MIN] = {NULL, instr_AMOMIN_D, NULL},
		[FUNC5_INSTR_AMO_MAX] = {NULL, instr_AMOMAX_D, NULL},
		[FUNC5_INSTR_AMO_MINU] = {NULL, instr_AMOMINU_D, NULL},
		[FUNC5_INSTR_AMO_MAXU] = {NULL, instr_AMOMAXU_D, NULL},
};

INIT_INSTRUCTION_LIST_DESC(D_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func5_subcode_list);

instruction_hook_t
	W_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func3_subcode_list[] = {
		[FUNC3_INSTR_W_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU] =
			{preparation_func5, NULL,
			 &W_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func5_subcode_list_desc},
		[FUNC3_INSTR_D_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU] =
			{preparation_func5, NULL,
			 &D_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func5_subcode_list_desc},
};

INIT_INSTRUCTION_LIST_DESC(W_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func3_subcode_list);

instruction_hook_t opcode_list[] = {
	[INSTR_LUI] = {U_type_preparation, instr_LUI, NULL},
	[INSTR_AUIPC] = {U_type_preparation, instr_AUIPC, NULL},
	[INSTR_JAL] = {J_type_preparation, instr_JAL, NULL},
	[INSTR_JALR] =
		{I_type_preparation, NULL, &JALR_func3_subcode_list_desc},
	[INSTR_BEQ_BNE_BLT_BGE_BLTU_BGEU] = {B_type_preparation, NULL,
										 &BEQ_BNE_BLT_BGE_BLTU_BGEU_func3_subcode_list_desc},
	[INSTR_LB_LH_LW_LBU_LHU_LWU_LD] = {I_type_preparation, NULL,
									   &LB_LH_LW_LBU_LHU_LWU_LD_func3_subcode_list_desc},
	[INSTR_SB_SH_SW_SD] =
		{S_type_preparation, NULL, &SB_SH_SW_SD_func3_subcode_list_desc},
	[INSTR_ADDI_SLTI_SLTIU_XORI_ORI_ANDI_SLLI_SRLI_SRAI] =
		{I_type_preparation, NULL,
		 &ADDI_SLTI_SLTIU_XORI_ORI_ANDI_SLLI_SRLI_SRAI_func3_subcode_list_desc},
	[INSTR_ADD_SUB_SLL_SLT_SLTU_XOR_SRL_SRA_OR_AND_MUL_MULH_MULHSU_MULHU_DIV_DIVU_REM_REMU] = {R_type_preparation, NULL, &ADD_SUB_SLL_SLT_SLTU_XOR_SRL_SRA_OR_AND_func3_subcode_list_desc},
	[INSTR_FENCE_FENCE_I] = {NULL, instr_NOP, NULL}, /* Not
													  * implemented */

	[INSTR_ADDIW_SLLIW_SRLIW_SRAIW] = {I_type_preparation, NULL,
									   &SLLIW_SRLIW_SRAIW_ADDIW_func3_subcode_list_desc},
	[INSTR_ADDW_SUBW_SLLW_SRLW_SRAW_MULW_DIVW_DIVUW_REMW_REMUW] =
		{R_type_preparation, NULL,
		 &ADDW_SUBW_SLLW_SRLW_SRAW_MULW_DIVW_DIVUW_REMW_REMUW_func3_subcode_list_desc},

	[INSTR_ECALL_EBREAK_MRET_SRET_URET_WFI_CSRRW_CSRRS_CSRRC_CSRRWI_CSRRSI_CSRRCI_SFENCEVMA] = {I_type_preparation, NULL, &CSRRW_CSRRS_CSRRC_CSRRWI_CSRRSI_CSRRCI_ECALL_EBREAK_URET_SRET_MRET_WFI_SFENCEVMA_func3_subcode_list_desc},

	[INSTR_AMO_W_D_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU] =
		{R_type_preparation, NULL,
		 &W_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func3_subcode_list_desc},
};

INIT_INSTRUCTION_LIST_DESC(opcode_list);

void call_from_opcode_list(struct hart *hart,
						   instruction_desc_t *opcode_list_desc, u32 opcode)
{
	i32 next_subcode = -1;

	unsigned int list_size = opcode_list_desc->instruction_hook_list_size;
	instruction_hook_t *opcode_list =
		opcode_list_desc->instruction_hook_list;

	if ((opcode_list[opcode].preparation_cb == NULL) &&
		(opcode_list[opcode].execution_cb == NULL) &&
		(opcode_list[opcode].next == NULL))
		die("Unknown instruction: %08x PC: " PRINTF_FMT
			" Cycle: %016ld\n",
			hart->instruction, hart->pc,
			hart->csr_store.cycle);

	if (opcode >= list_size)
		die("Unknown instruction: %08x PC: " PRINTF_FMT
			" Cycle: %016ld\n",
			hart->instruction, hart->pc,
			hart->csr_store.cycle);

	if (opcode_list[opcode].preparation_cb != NULL)
		opcode_list[opcode].preparation_cb(hart, &next_subcode);

	if (opcode_list[opcode].execution_cb != NULL)
		hart->execute_cb = opcode_list[opcode].execution_cb;

	if ((next_subcode != -1) && (opcode_list[opcode].next != NULL))
		call_from_opcode_list(hart, opcode_list[opcode].next,
							  next_subcode);
}
