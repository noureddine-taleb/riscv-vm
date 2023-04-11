#include <stdio.h>
#include <types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <types.h>
#include <riscv_helper.h>
#include <hart.h>
#include <instruction.h>

/*
 * Implementations of the RISCV instructions
 */
void NOP(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	(void)hart;
	return;
}

void FENCE_I(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	(void)hart;
	return;
}

void FENCE(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	(void)hart;
	return;
}

void FENCE_VMA(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	(void)hart;
	return;
}

/*
 * Implementations of the RISCV instructions
 */
void WFI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	(void)hart;
	usleep(10000);
}

/*
 * RISCV Instructions
 */
void LUI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = (hart->imm << 12);
}

void AUIPC(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = (hart->pc) + (hart->imm << 12);
}

void JAL(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->pc + 4;

	if (ADDR_MISALIGNED(hart->imm))
	{
		die("Addr misaligned!\n");
		prepare_sync_trap(hart, trap_cause_instr_addr_misalign, 0);
		return;
	}

	hart->next_pc = hart->pc + hart->imm;
}

void JALR(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen curr_pc = hart->pc + 4;
	hart->imm = SIGNEX_BIT_11(hart->imm);

	uxlen target_pc = (hart->x[hart->rs1] + hart->imm);
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

void BEQ(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->x[hart->rs1] == hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->imm;
	}
}

void BNE(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->x[hart->rs1] != hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->imm;
	}
}

void BLT(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	if (signed_rs < signed_rs2)
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->imm;
	}
}

void BGE(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	if (signed_rs >= signed_rs2)
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->imm;
	}
}

void BLTU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->x[hart->rs1] < hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->imm;
	}
}

void BGEU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->x[hart->rs1] >= hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			die("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->next_pc = hart->pc + hart->imm;
	}
}

void ADDI(struct hart *hart)
{
	CORE_DBG("%s: %x " PRINTF_FMT "\n", __func__, hart->instruction,
			 hart->pc);
	ixlen signed_imm = SIGNEX_BIT_11(hart->imm);
	ixlen signed_rs_val = hart->x[hart->rs1];
	CORE_DBG("%s: " PRINTF_FMT " " PRINTF_FMT " " PRINTF_FMT " %x\n",
			 __func__, hart->x[hart->rs1], signed_rs_val,
			 signed_imm, hart->rs1);
	hart->x[hart->rd] = (signed_imm + signed_rs_val);
}

void SLTI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_imm = SIGNEX_BIT_11(hart->imm);
	ixlen signed_rs_val = hart->x[hart->rs1];

	if (signed_rs_val < signed_imm)
		hart->x[hart->rd] = 1;
	else
		hart->x[hart->rd] = 0;
}

void SLTIU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen unsigned_imm = SIGNEX_BIT_11(hart->imm);
	uxlen unsigned_rs_val = hart->x[hart->rs1];

	if (unsigned_rs_val < unsigned_imm)
		hart->x[hart->rd] = 1;
	else
		hart->x[hart->rd] = 0;
}

void XORI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_imm = SIGNEX_BIT_11(hart->imm);
	hart->imm = signed_imm;

	if (signed_imm == -1)
		hart->x[hart->rd] = hart->x[hart->rs1] ^ -1;
	else
		hart->x[hart->rd] = hart->x[hart->rs1] ^ hart->imm;
}

void ORI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->imm = SIGNEX_BIT_11(hart->imm);
	hart->x[hart->rd] = hart->x[hart->rs1] | hart->imm;
}

void ANDI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->imm = SIGNEX_BIT_11(hart->imm);
	hart->x[hart->rd] = hart->x[hart->rs1] & hart->imm;
}

void SLLI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] =
		(hart->x[hart->rs1] << (hart->imm & SHIFT_OP_MASK));
}

void SRAI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen rs_val = hart->x[hart->rs1];

	/*
	 * a right shift on signed ints seem to be always arithmetic
	 */
	rs_val = rs_val >> (hart->imm & SHIFT_OP_MASK);
	hart->x[hart->rd] = rs_val;
}

void SRLI(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] =
		(hart->x[hart->rs1] >> (hart->imm & SHIFT_OP_MASK));
}

void ADD(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	CORE_DBG("%s: " PRINTF_FMT " %x\n", __func__, hart->x[hart->rs1],
			 hart->rs1);
	hart->x[hart->rd] = hart->x[hart->rs1] + hart->x[hart->rs2];
}

void SUB(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->x[hart->rs1] - hart->x[hart->rs2];
}

void SLL(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] =
		hart->x[hart->rs1] << (hart->x[hart->rs2] & SHIFT_OP_MASK);
}

void SLT(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	if (signed_rs < signed_rs2)
		hart->x[hart->rd] = 1;
	else
		hart->x[hart->rd] = 0;
}

void SLTU(struct hart *hart)
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

void XOR(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->x[hart->rs1] ^ hart->x[hart->rs2];
}

void SRL(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] =
		hart->x[hart->rs1] >> (hart->x[hart->rs2] & SHIFT_OP_MASK);
}

void OR(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->x[hart->rs1] | (hart->x[hart->rs2]);
}

void AND(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->x[hart->rd] = hart->x[hart->rs1] & (hart->x[hart->rs2]);
}

void SRA(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	hart->x[hart->rd] = signed_rs >> (hart->x[hart->rs2] & SHIFT_OP_MASK);
}

void LB(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u8 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 1) == 0)
		hart->x[hart->rd] = SIGNEX_BIT_7(tmp_load_val);
}

void LH(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u16 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 2) == 0)
		hart->x[hart->rd] = SIGNEX_BIT_15(tmp_load_val);
}

void LW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 4) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void LBU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u8 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 1) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void LHU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u16 tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 2) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void SB(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	u8 value_to_write = (u8)hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value_to_write, 1);
}

void SH(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	u16 value_to_write = (u16)hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value_to_write, 2);
}

void SW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	uxlen value_to_write = (uxlen)hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value_to_write, 4);
}

void LWU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 tmp_load_val = 0;
	uxlen unsigned_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + unsigned_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 4) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void LD(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen tmp_load_val = 0;
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	ixlen address = hart->x[hart->rs1] + signed_offset;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp_load_val, 8) == 0)
		hart->x[hart->rd] = tmp_load_val;
}

void SD(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_offset = SIGNEX_BIT_11(hart->imm);
	uxlen address = hart->x[hart->rs1] + signed_offset;
	uxlen value_to_write = (uxlen)hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value_to_write, 8);
}

void SRAIW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_rs_val = hart->x[hart->rs1];
	hart->x[hart->rd] = (signed_rs_val >> (hart->imm & 0x1F));
}

void ADDIW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_imm = SIGNEX_BIT_11(hart->imm);
	i32 signed_rs_val = hart->x[hart->rs1];
	hart->x[hart->rd] = (signed_rs_val + signed_imm);
}

void SLLIW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_tmp32 =
		(hart->x[hart->rs1] << (hart->imm & 0x1F)) & 0xFFFFFFFF;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void SRLIW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 unsigned_rs_val = hart->x[hart->rs1];
	i32 signed_tmp32 = (unsigned_rs_val >> (hart->imm & 0x1F));
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void SRLW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 rs1_val = hart->x[hart->rs1];
	u32 rs2_val = (hart->x[hart->rs2] & 0x1F);
	i32 signed_tmp32 = rs1_val >> rs2_val;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void SRAW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 rs1_val_signed = hart->x[hart->rs1];
	u32 rs2_val = (hart->x[hart->rs2] & 0x1F);
	i32 signed_tmp32 = rs1_val_signed >> rs2_val;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void SLLW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 rs1_val = hart->x[hart->rs1];
	u32 rs2_val = (hart->x[hart->rs2] & 0x1F);
	i32 signed_tmp32 = rs1_val << rs2_val;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void ADDW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	u32 rs1_val = hart->x[hart->rs1];
	u32 rs2_val = hart->x[hart->rs2];
	i32 signed_tmp32 = rs1_val + rs2_val;
	hart->x[hart->rd] = (ixlen)signed_tmp32;
}

void SUBW(struct hart *hart)
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
	u16 csr_addr = hart->imm;
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
	u16 csr_addr = hart->imm;
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
	u16 csr_addr = hart->imm;
	uxlen csr_mask = csr_get_mask(hart->csr_regs, csr_addr);
	uxlen new_csr_val = 0;

	if (csr_read_reg
	    (hart->csr_regs, hart->curr_priv_mode, csr_addr, &csr_val)) {
		prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
		return;
	}

	new_csr_val = (new_val & csr_mask);

	if (hart->rs1 != 0) {
		if (csr_write_reg
		    (hart->csr_regs, hart->curr_priv_mode, csr_addr,
		     csr_val & ~new_csr_val)) {
			prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
			return;
		}
	}
	hart->x[hart->rd] = csr_val & csr_mask;
}

void CSRRW(struct hart *hart)
{
	CSRRWx(hart, hart->x[hart->rs1]);
}

void CSRRS(struct hart *hart)
{
	CSRRSx(hart, hart->x[hart->rs1]);
}

void CSRRC(struct hart *hart)
{
	CSRRCx(hart, hart->x[hart->rs1]);
}

void CSRRWI(struct hart *hart)
{
	CSRRWx(hart, hart->rs1);
}

void CSRRSI(struct hart *hart)
{
	CSRRSx(hart, hart->rs1);
}

void CSRRCI(struct hart *hart)
{
	CSRRCx(hart, hart->rs1);
}

void ECALL(struct hart *hart)
{
	prepare_sync_trap(hart,
					  trap_cause_user_ecall + hart->curr_priv_mode, 0);
}

void EBREAK(struct hart *hart)
{
	/*
	 * not implemented
	 */
	(void)hart;
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
}

void MRET(struct hart *hart)
{
	CORE_DBG("%s: " PRINTF_FMT "\n", __func__,
			 hart->csr_store.ip);
	return_from_exception(hart, hart->curr_priv_mode);
}

void SRET(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	return_from_exception(hart, hart->curr_priv_mode);
}

void URET(struct hart *hart)
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

void LR_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->lr_address = hart->x[hart->rs1];
	hart->lr_valid = 1;
	LW(hart);
}

void SC_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->lr_valid && (hart->lr_address == hart->x[hart->rs1]))
	{
		SW(hart);
		hart->x[hart->rd] = 0;
	}
	else
	{
		hart->x[hart->rd] = 1;
	}

	hart->lr_valid = 0;
	hart->lr_address = 0;
}

void AMOSWAP_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	LW(hart);
	result = rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOADD_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	LW(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val + rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOXOR_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	LW(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val ^ rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOAND_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	LW(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val & rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOOR_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;

	LW(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val | rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOMIN_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	i32 rd_val = 0;
	i32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LW(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOMAX_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	i32 rd_val = 0;
	i32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LW(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOMINU_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LW(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOMAXU_W(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LW(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void LR_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	hart->lr_valid = 1;
	hart->lr_address = hart->x[hart->rs1];
	LD(hart);
}

void SC_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	if (hart->lr_valid && (hart->lr_address == hart->x[hart->rs1]))
	{
		SD(hart);
		hart->x[hart->rd] = 0;
	}
	else
	{
		hart->x[hart->rd] = 1;
	}

	hart->lr_valid = 0;
	hart->lr_address = 0;
}

void AMOSWAP_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LD(hart);
	result = rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOADD_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LD(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val + rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOXOR_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LD(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val ^ rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOAND_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LD(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val & rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOOR_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LD(hart);
	rd_val = hart->x[hart->rd];
	result = rd_val | rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOMIN_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	ixlen rd_val = 0;
	ixlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LD(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOMAX_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	ixlen rd_val = 0;
	ixlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LD(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOMINU_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LD(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOMAXU_D(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;

	LD(hart);

	rd_val = hart->x[hart->rd];
	result = ASSIGN_MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void DIV(struct hart *hart)
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

void DIVU(struct hart *hart)
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

void REM(struct hart *hart)
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

void REMU(struct hart *hart)
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

void MUL(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen signed_rs = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];
	hart->x[hart->rd] = signed_rs * signed_rs2;
}

void MULH(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen result_hi = 0;
	ixlen result_lo = 0;
	__MUL(hart->x[hart->rs1], hart->x[hart->rs2], &result_hi, &result_lo);
	hart->x[hart->rd] = result_hi;
}

void MULHU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	uxlen result_hi = 0;
	uxlen result_lo = 0;
	__UMUL(hart->x[hart->rs1], hart->x[hart->rs2], &result_hi, &result_lo);
	hart->x[hart->rd] = result_hi;
}

void MULHSU(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	ixlen result_hi = 0;
	ixlen result_lo = 0;
	__MULHSU(hart->x[hart->rs1], hart->x[hart->rs2], &result_hi, &result_lo);
	hart->x[hart->rd] = result_hi;
}

void MULW(struct hart *hart)
{
	CORE_DBG("%s: %x\n", __func__, hart->instruction);
	i32 signed_rs = hart->x[hart->rs1];
	i32 signed_rs2 = hart->x[hart->rs2];
	hart->x[hart->rd] = (ixlen)(signed_rs * signed_rs2);
}

void DIVW(struct hart *hart)
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

void DIVUW(struct hart *hart)
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

void REMW(struct hart *hart)
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

void REMUW(struct hart *hart)
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

uxlen hart_decode(struct hart *hart)
{
	hart->opcode = (hart->instruction & 0x7F);
	hart->rd = 0;
	hart->rs1 = 0;
	hart->rs2 = 0;
	hart->func3 = 0;
	hart->func7 = 0;
	hart->imm = 0;
	switch(instruction_map[hart->opcode]) {
		case R:
			hart->rd = (hart->instruction >> 7) & 0x1f;
			hart->func3 = (hart->instruction >> 12) & 0x7;
			hart->rs1 = (hart->instruction >> 15) & 0x1f;
			hart->rs2 = (hart->instruction >> 20) & 0x1f;
			hart->func7 = (hart->instruction >> 25) & 0x7f;

			if (hart->opcode == 0b0110011 && hart->func3 == 0b000 && hart->func7 == 0b0000000)
				hart->execute = ADD;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b000 && hart->func7 == 0b0100000)
				hart->execute = SUB;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b001 && hart->func7 == 0b0000000)
				hart->execute = SLL;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b010 && hart->func7 == 0b0000000)
				hart->execute = SLT;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b011 && hart->func7 == 0b0000000)
				hart->execute = SLTU;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b100 && hart->func7 == 0b0000000)
				hart->execute = XOR;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b101 && hart->func7 == 0b0000000)
				hart->execute = SRL;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b101 && hart->func7 == 0b0100000)
				hart->execute = SRA;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b110 && hart->func7 == 0b0000000)
				hart->execute = OR;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b111 && hart->func7 == 0b0000000)
				hart->execute = AND;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b000 && hart->func7 == 1)
				hart->execute = MUL;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b001 && hart->func7 == 1)
				hart->execute = MULH;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b010 && hart->func7 == 1)
				hart->execute = MULHSU;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b011 && hart->func7 == 1)
				hart->execute = MULHU;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b100 && hart->func7 == 1)
				hart->execute = DIV;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b101 && hart->func7 == 1)
				hart->execute = DIVU;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b110 && hart->func7 == 1)
				hart->execute = REM;
			else if (hart->opcode == 0b0110011 && hart->func3 == 0b111 && hart->func7 == 1)
				hart->execute = REMU;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b000 && hart->func7 == 1)
				hart->execute = MULW;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b100 && hart->func7 == 1)
				hart->execute = DIVW;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b101 && hart->func7 == 1)
				hart->execute = DIVUW;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b110 && hart->func7 == 1)
				hart->execute = REMW;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b111 && hart->func7 == 1)
				hart->execute = REMUW;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b000 && hart->func7 == 0)
				hart->execute = ADDW;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b000 && hart->func7 == 0b0100000)
				hart->execute = SUBW;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b001 && hart->func7 == 0)
				hart->execute = SLLW;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b101 && hart->func7 == 0)
				hart->execute = SRLW;
			else if (hart->opcode == 0b0111011 && hart->func3 == 0b101 && hart->func7 == 0b0100000)
				hart->execute = SRAW;

			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b00010 && hart->rs2 == 0)
				hart->execute = LR_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b00011)
				hart->execute = SC_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b00001)
				hart->execute = AMOSWAP_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b00000)
				hart->execute = AMOADD_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b00100)
				hart->execute = AMOXOR_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b01100)
				hart->execute = AMOAND_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b01000)
				hart->execute = AMOOR_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b10000)
				hart->execute = AMOMIN_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b10100)
				hart->execute = AMOMAX_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b11000)
				hart->execute = AMOMINU_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b010 && hart->func7 >> 2 == 0b11100)
				hart->execute = AMOMAXU_W;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b00010 && hart->rs2 == 0)
				hart->execute = LR_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b00011)
				hart->execute = SC_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b00001)
				hart->execute = AMOSWAP_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b00000)
				hart->execute = AMOADD_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b00100)
				hart->execute = AMOXOR_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b01100)
				hart->execute = AMOAND_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b01000)
				hart->execute = AMOOR_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b10000)
				hart->execute = AMOMIN_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b10100)
				hart->execute = AMOMAX_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b11000)
				hart->execute = AMOMINU_D;
			else if (hart->opcode == 0b0101111 && hart->func3 == 0b011 && hart->func7 >> 2 == 0b11100)
				hart->execute = AMOMAXU_D;
			else
				goto illegal_inst;

		break;
		case I:
			hart->rd = (hart->instruction >> 7) & 0x1f;
			hart->func3 = (hart->instruction >> 12) & 0x7;
			hart->rs1 = (hart->instruction >> 15) & 0x1f;
			hart->imm = (hart->instruction >> 20) & 0xfff;
			// used by fence vma
			hart->rs2 = (hart->instruction >> 20) & 0x1f;

			if (hart->opcode == 0b0000011 && hart->func3 == 0b000)
				hart->execute = LB;
			else if (hart->opcode == 0b0000011 && hart->func3 == 0b001)
				hart->execute = LH;
			else if (hart->opcode == 0b0000011 && hart->func3 == 0b010)
				hart->execute = LW;
			else if (hart->opcode == 0b0000011 && hart->func3 == 0b011)
				hart->execute = LD;
			else if (hart->opcode == 0b0000011 && hart->func3 == 0b100)
				hart->execute = LBU;
			else if (hart->opcode == 0b0000011 && hart->func3 == 0b101)
				hart->execute = LHU;
			else if (hart->opcode == 0b0000011 && hart->func3 == 0b110)
				hart->execute = LWU;
			else if (hart->opcode == 0b0010011 && hart->func3 == 0b000)
				hart->execute = ADDI;
			else if (hart->opcode == 0b0010011 && hart->func3 == 0b010)
				hart->execute = SLTI;
			else if (hart->opcode == 0b0010011 && hart->func3 == 0b011)
				hart->execute = SLTIU;
			else if (hart->opcode == 0b0010011 && hart->func3 == 0b100)
				hart->execute = XORI;
			else if (hart->opcode == 0b0010011 && hart->func3 == 0b110)
				hart->execute = ORI;
			else if (hart->opcode == 0b0010011 && hart->func3 == 0b111)
				hart->execute = ANDI;
        	else if (hart->opcode == 0b0010011 && hart->func3 == 0b001 && ((hart->imm >> 6) & 0x3f) == 0b000000)
				hart->execute = SLLI;
        	else if (hart->opcode == 0b0010011 && hart->func3 == 0b101 && ((hart->imm >> 6) & 0x3f) == 0b000000)
				hart->execute = SRLI;
        	else if (hart->opcode == 0b0010011 && hart->func3 == 0b101 && ((hart->imm >> 6) & 0x3f) == 0b010000)
				hart->execute = SRAI;
			else if (hart->opcode == 0b0011011 && hart->func3 == 0b001 && ((hart->imm >> 5) & 0x7f) == 0b0000000)
				hart->execute = SLLIW;
			else if (hart->opcode == 0b0011011 && hart->func3 == 0b101 && ((hart->imm >> 5) & 0x7f) == 0b0000000)
				hart->execute = SRLIW;
			else if (hart->opcode == 0b0011011 && hart->func3 == 0b101 && ((hart->imm >> 5) & 0x7f) == 0b0100000)
				hart->execute = SRAIW;
			else if (hart->opcode == 0b0011011 && hart->func3 == 0b000)
				hart->execute = ADDIW;
			else if (hart->opcode == 0b1100111 && hart->func3 == 0b000)
				hart->execute = JALR;
			else if (hart->opcode == 0b1110011 && hart->rd == 0 && hart->func3 == 0 && hart->rs1 == 0 && hart->imm == 0)
				hart->execute = ECALL;
			else if (hart->opcode == 0b1110011 && hart->rd == 0 && hart->func3 == 0 && hart->rs1 == 0 && hart->imm == 1)
				hart->execute = EBREAK;
			else if (hart->opcode == 0b1110011 && hart->rd == 0 && hart->func3 == 0 && hart->rs1 == 0 && hart->imm == 0b001100000010)
				hart->execute = MRET;
			else if (hart->opcode == 0b1110011 && hart->rd == 0 && hart->func3 == 0 && hart->rs1 == 0 && hart->imm == 0b000100000010)
				hart->execute = SRET;
			else if (hart->opcode == 0b1110011 && hart->rd == 0 && hart->func3 == 0 && hart->rs1 == 0 && hart->imm == 0b000100000101)
				hart->execute = WFI;

			else if (hart->opcode == 0b1110011 && hart->rd == 0 && hart->func3 == 0 && (hart->imm >> 5) == 0b0001001) 
				hart->execute = FENCE_VMA;

			else if (hart->opcode == 0b1110011 && hart->func3 == 0b001)
				hart->execute = CSRRW;
			else if (hart->opcode == 0b1110011 && hart->func3 == 0b010)
				hart->execute = CSRRS;
			else if (hart->opcode == 0b1110011 && hart->func3 == 0b011)
				hart->execute = CSRRC;
			else if (hart->opcode == 0b1110011 && hart->func3 == 0b101)
				hart->execute = CSRRWI;
			else if (hart->opcode == 0b1110011 && hart->func3 == 0b110)
				hart->execute = CSRRSI;
			else if (hart->opcode == 0b1110011 && hart->func3 == 0b111)
				hart->execute = CSRRCI;
			else if (hart->opcode == 0b0001111 && hart->func3 == 0)
				hart->execute = FENCE;
			else if (hart->opcode == 0b0001111 && hart->func3 == 1)
				hart->execute = FENCE_I;

			else
				goto illegal_inst;

		break;
		case S:
			hart->func3 = (hart->instruction >> 12) & 0x7;
			hart->rs1 = (hart->instruction >> 15) & 0x1F;
			hart->rs2 = (hart->instruction >> 20) & 0x1F;
			hart->imm = ((hart->instruction >> 25) << 5) |
				((hart->instruction >> 7) & 0x1F);

			if (hart->opcode == 0b0100011 && hart->func3 == 0b000)
				hart->execute = SB;
			else if (hart->opcode == 0b0100011 && hart->func3 == 0b001)
				hart->execute = SH;
			else if (hart->opcode == 0b0100011 && hart->func3 == 0b011)
				hart->execute = SD;
			else if (hart->opcode == 0b0100011 && hart->func3 == 0b010)
				hart->execute = SW;
			else
				goto illegal_inst;

		break;
		case B:
			hart->rd = (hart->instruction >> 7) & 0x1F;
			hart->func3 = (hart->instruction >> 12) & 0x7;
			hart->rs1 = (hart->instruction >> 15) & 0x1F;
			hart->rs2 = (hart->instruction >> 20) & 0x1F;
			hart->imm = (extract32(hart->instruction, 8, 4) << 1) |
						(extract32(hart->instruction, 25, 6) << 5) |
						(extract32(hart->instruction, 7, 1) << 11) |
						(extract32(hart->instruction, 31, 1) << 12);
			hart->imm = SIGNEX_BIT_12(hart->imm);

			if (hart->opcode == 0b1100011 && hart->func3 == 0b000)
				hart->execute = BEQ;
			else if (hart->opcode == 0b1100011 && hart->func3 == 0b001)
				hart->execute = BNE;
			else if (hart->opcode == 0b1100011 && hart->func3 == 0b100)
				hart->execute = BLT;
			else if (hart->opcode == 0b1100011 && hart->func3 == 0b101)
				hart->execute = BGE;
			else if (hart->opcode == 0b1100011 && hart->func3 == 0b110)
				hart->execute = BLTU;
			else if (hart->opcode == 0b1100011 && hart->func3 == 0b111)
				hart->execute = BGEU;
			else
				goto illegal_inst;

		break;
		case U:
			hart->rd = (hart->instruction >> 7) & 0x1F;
			hart->imm = (hart->instruction >> 12) & 0xFFFFF;
			hart->imm = SIGNEX_BIT_19(hart->imm);

			if (hart->opcode == 0b0010111)
				hart->execute = AUIPC;
			else if (hart->opcode == 0b0110111)
				hart->execute = LUI;
			else
				goto illegal_inst;

		break;
		case J:
			hart->rd = (hart->instruction >> 7) & 0x1F;
			hart->imm = (extract32(hart->instruction, 21, 10) << 1) |
						(extract32(hart->instruction, 20, 1) << 11) |
						(extract32(hart->instruction, 12, 8) << 12) |
						(extract32(hart->instruction, 31, 1) << 20);
			hart->imm = SIGNEX_BIT_20(hart->imm);

			if (hart->opcode == 0b1101111)
				hart->execute = JAL;
			else
				goto illegal_inst;

		break;

		default:
illegal_inst:
			die("unknowen pc=%lx instruction=%#x opcode=%#x func3=%#x rd=%#x rs1=%#x imm=%#lx", hart->pc, hart->instruction, hart->opcode, hart->func3, hart->rd, hart->rs1, hart->imm);
	}

	return 0;
}