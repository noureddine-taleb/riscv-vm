#include <stdio.h>
#include <types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <types.h>
#include <hart.h>
#include <instruction.h>
#include <helpers.h>

/*
 * Implementations of the RISCV instructions
 */
void NOP(struct hart __maybe_unused *hart)
{
}

// fence
void FENCE_I(struct hart __maybe_unused *hart)
{
}

void FENCE(struct hart __maybe_unused *hart)
{
}

void FENCE_VMA(struct hart __maybe_unused *hart)
{
}

void WFI(struct hart __maybe_unused *hart)
{
	usleep(10000);
}

void LUI(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->imm;
}

void AUIPC(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->pc + hart->imm;
}

// relative jump
void JAL(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->pc + 4;

	if (ADDR_MISALIGNED(hart->imm))
	{
		debug("Addr misaligned!\n");
		prepare_sync_trap(hart, trap_cause_instr_addr_misalign, 0);
		return;
	}

	hart->override_pc = hart->pc + hart->imm;
}

// absolute jump
void JALR(struct hart __maybe_unused *hart)
{
	uxlen curr_pc = hart->pc + 4;
	uxlen target_pc = (hart->x[hart->rs1] + hart->imm);
	target_pc &= ~(1 << 0);
	if (ADDR_MISALIGNED(target_pc))
	{
		debug("Addr misaligned!\n");
		prepare_sync_trap(hart, trap_cause_instr_addr_misalign, 0);
		return;
	}
	hart->override_pc = target_pc;
	hart->x[hart->rd] = curr_pc;
}

// relative branches
void BEQ(struct hart __maybe_unused *hart)
{
	if (hart->x[hart->rs1] == hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			debug("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->override_pc = hart->pc + hart->imm;
	}
}

void BNE(struct hart __maybe_unused *hart)
{
	if (hart->x[hart->rs1] != hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			debug("Addr misaligned!\n");
			prepare_sync_trap(hart, trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->override_pc = hart->pc + hart->imm;
	}
}

void BLT(struct hart __maybe_unused *hart)
{
	ixlen signed_rs1 = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	if (signed_rs1 < signed_rs2)
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			debug("Addr misaligned!\n");
			prepare_sync_trap(hart, trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->override_pc = hart->pc + hart->imm;
	}
}

void BGE(struct hart __maybe_unused *hart)
{
	ixlen signed_rs1 = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	if (signed_rs1 >= signed_rs2)
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			debug("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->override_pc = hart->pc + hart->imm;
	}
}

void BLTU(struct hart __maybe_unused *hart)
{
	if (hart->x[hart->rs1] < hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			debug("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->override_pc = hart->pc + hart->imm;
	}
}

void BGEU(struct hart __maybe_unused *hart)
{
	if (hart->x[hart->rs1] >= hart->x[hart->rs2])
	{
		if (ADDR_MISALIGNED(hart->imm))
		{
			debug("Addr misaligned!\n");
			prepare_sync_trap(hart,
							  trap_cause_instr_addr_misalign, 0);
			return;
		}

		hart->override_pc = hart->pc + hart->imm;
	}
}

// arithmetics
void ADDI(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->imm + hart->x[hart->rs1];
}

void SLTI(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (ixlen)hart->x[hart->rs1] < (ixlen)hart->imm;
}

void SLTIU(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] < hart->imm;
}

void XORI(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] ^ hart->imm;
}

void ORI(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] | hart->imm;
}

void ANDI(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] & hart->imm;
}

void SLLI(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (hart->x[hart->rs1] << (hart->imm & SHAMT_MASK));
}

void SRAI(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (ixlen)hart->x[hart->rs1] >> (hart->imm & SHAMT_MASK);
}

void SRLI(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] >> (hart->imm & SHAMT_MASK);
}

void ADD(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] + hart->x[hart->rs2];
}

void SUB(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] - hart->x[hart->rs2];
}

void SLL(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] << (hart->x[hart->rs2] & SHAMT_MASK);
}

void SLT(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (ixlen)hart->x[hart->rs1] < (ixlen)hart->x[hart->rs2];
}

void SLTU(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] < hart->x[hart->rs2];
}

void XOR(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] ^ hart->x[hart->rs2];
}

void SRL(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] >> (hart->x[hart->rs2] & SHAMT_MASK);
}

void OR(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] | (hart->x[hart->rs2]);
}

void AND(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = hart->x[hart->rs1] & (hart->x[hart->rs2]);
}

void SRA(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (ixlen)hart->x[hart->rs1] >> (hart->x[hart->rs2] & SHAMT_MASK);
}

void SRAIW(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (i32)hart->x[hart->rs1] >> (hart->imm & 0x1F);
}

void ADDIW(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (i32)hart->x[hart->rs1] + hart->imm;
}

void SLLIW(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (i32)hart->x[hart->rs1] << (hart->imm & 0x1F);
}

void SRLIW(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (i32)((u32)hart->x[hart->rs1] >> (hart->imm & 0x1F));
}

void SRLW(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (i32)((u32)hart->x[hart->rs1] >> (hart->x[hart->rs2] & 0x1F));
}

void SRAW(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (i32)hart->x[hart->rs1] >> (hart->x[hart->rs2] & 0x1F);
}

void SLLW(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (i32)hart->x[hart->rs1] << (hart->x[hart->rs2] & 0x1F);
}

void ADDW(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (i32)hart->x[hart->rs1] + (i32)hart->x[hart->rs2];
}

void SUBW(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (i32)hart->x[hart->rs1] - (i32)hart->x[hart->rs2];
}

// absolute loads
void LB(struct hart __maybe_unused *hart)
{
	i8 tmp = 0;
	uxlen address = hart->x[hart->rs1] + hart->imm;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 1) == 0)
		hart->x[hart->rd] = tmp;
}

void LH(struct hart __maybe_unused *hart)
{
	i16 tmp = 0;
	uxlen address = hart->x[hart->rs1] + hart->imm;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 2) == 0)
		hart->x[hart->rd] = tmp;
}

void LW(struct hart __maybe_unused *hart)
{
	i32 tmp = 0;
	uxlen address = hart->x[hart->rs1] + hart->imm;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4) == 0)
		hart->x[hart->rd] = tmp;
}

void LBU(struct hart __maybe_unused *hart)
{
	u8 tmp = 0;
	uxlen address = hart->x[hart->rs1] + hart->imm;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 1) == 0)
		hart->x[hart->rd] = tmp;
}

void LHU(struct hart __maybe_unused *hart)
{
	u16 tmp = 0;
	uxlen address = hart->x[hart->rs1] + hart->imm;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 2) == 0)
		hart->x[hart->rd] = tmp;
}

// absolute stores
void SB(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1] + hart->imm;
	u8 value = hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value, 1);
}

void SH(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1] + hart->imm;
	u16 value = hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value, 2);
}

void SW(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1] + hart->imm;
	u32 value = hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value, 4);
}

void LWU(struct hart __maybe_unused *hart)
{
	u32 tmp = 0;
	uxlen address = hart->x[hart->rs1] + hart->imm;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4) == 0)
		hart->x[hart->rd] = tmp;
}

void LD(struct hart __maybe_unused *hart)
{
	i64 tmp = 0;
	ixlen address = hart->x[hart->rs1] + hart->imm;
	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8) == 0)
		hart->x[hart->rd] = tmp;
}

void SD(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1] + hart->imm;
	u64 value = hart->x[hart->rs2];
	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &value, 8);
}

// csr
void CSRRWx(struct hart *hart, uxlen new_val)
{
	uxlen csr_val = 0;
	u16 csr_addr = (u16)(hart->imm & 0xfff);
	uxlen csr_mask = hart->csr_regs[csr_addr].mask;
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
	uxlen csr_val = 0;
	u16 csr_addr = (u16)(hart->imm & 0xfff);

	if (csr_read_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr, &csr_val))
	{
		prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
		return;
	}

	if (hart->rs1 != 0)
	{
		if (csr_write_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr,
						  csr_val | new_val))
		{
			prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
			return;
		}
	}

	hart->x[hart->rd] = csr_val;
}

void CSRRCx(struct hart *hart, uxlen new_val)
{
	uxlen csr_val = 0;
	u16 csr_addr = (u16)(hart->imm & 0xfff);

	if (csr_read_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr, &csr_val))
	{
		prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
		return;
	}

	if (hart->rs1 != 0)
	{
		if (csr_write_reg(hart->csr_regs, hart->curr_priv_mode, csr_addr,
						  csr_val & ~new_val))
		{
			prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
			return;
		}
	}
	hart->x[hart->rd] = csr_val;
}

void CSRRW(struct hart __maybe_unused *hart)
{
	CSRRWx(hart, hart->x[hart->rs1]);
}

void CSRRS(struct hart __maybe_unused *hart)
{
	CSRRSx(hart, hart->x[hart->rs1]);
}

void CSRRC(struct hart __maybe_unused *hart)
{
	CSRRCx(hart, hart->x[hart->rs1]);
}

void CSRRWI(struct hart __maybe_unused *hart)
{
	CSRRWx(hart, hart->rs1);
}

void CSRRSI(struct hart __maybe_unused *hart)
{
	CSRRSx(hart, hart->rs1);
}

void CSRRCI(struct hart __maybe_unused *hart)
{
	CSRRCx(hart, hart->rs1);
}

// ecalls
void ECALL(struct hart __maybe_unused *hart)
{
	prepare_sync_trap(hart, trap_cause_user_ecall + hart->curr_priv_mode, 0);
}

void EBREAK(struct hart __maybe_unused *hart)
{
	die("EBREAK");
}

void MRET(struct hart __maybe_unused *hart)
{
	return_from_exception(hart, hart->curr_priv_mode);
}

void SRET(struct hart __maybe_unused *hart)
{
	return_from_exception(hart, hart->curr_priv_mode);
}

// LR / SC
void LR_W(struct hart __maybe_unused *hart)
{
	hart->lr_address = hart->x[hart->rs1];
	hart->lr_valid = 1;
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, hart->x[hart->rs1],
							&tmp, 4))
		return;
	hart->x[hart->rd] = tmp;
}

void SC_W(struct hart __maybe_unused *hart)
{
	if (hart->lr_valid && (hart->lr_address == hart->x[hart->rs1]))
	{
		if (hart->access_memory(hart, hart->curr_priv_mode,
								bus_write_access, hart->x[hart->rs1], &hart->x[hart->rs2], 4))
			return;
		hart->x[hart->rd] = 0;
	}
	else
	{
		hart->x[hart->rd] = 1;
	}

	hart->lr_valid = 0;
	hart->lr_address = 0;
}

void LR_D(struct hart __maybe_unused *hart)
{
	hart->lr_valid = 1;
	hart->lr_address = hart->x[hart->rs1];
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, hart->x[hart->rs1],
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;
}

void SC_D(struct hart __maybe_unused *hart)
{
	if (hart->lr_valid && (hart->lr_address == hart->x[hart->rs1]))
	{
		if (hart->access_memory(hart, hart->curr_priv_mode,
								bus_write_access, hart->x[hart->rs1], &hart->x[hart->rs2], 8))
			return;
		hart->x[hart->rd] = 0;
	}
	else
	{
		hart->x[hart->rd] = 1;
	}

	hart->lr_valid = 0;
	hart->lr_address = 0;
}

// atomic
void AMOSWAP_W(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	u32 rs2_val = hart->x[hart->rs2];
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4))
		return;
	hart->x[hart->rd] = tmp;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &rs2_val, 4);
}

void AMOADD_W(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4))
		return;

	hart->x[hart->rd] = tmp;

	result = hart->x[hart->rd] + rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOXOR_W(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = rd_val ^ rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOAND_W(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = rd_val & rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOOR_W(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	u32 result = 0;
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = rd_val | rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOMIN_W(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	i32 rd_val = 0;
	i32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOMAX_W(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	i32 rd_val = 0;
	i32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOMINU_W(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOMAXU_W(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	u32 rd_val = 0;
	u32 rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i32 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 4))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 4);
}

void AMOSWAP_D(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;
	result = rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOADD_D(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;
	rd_val = hart->x[hart->rd];
	result = rd_val + rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOXOR_D(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;
	rd_val = hart->x[hart->rd];
	result = rd_val ^ rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOAND_D(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;
	rd_val = hart->x[hart->rd];
	result = rd_val & rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOOR_D(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;
	rd_val = hart->x[hart->rd];
	result = rd_val | rs2_val;

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOMIN_D(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	ixlen rd_val = 0;
	ixlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOMAX_D(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	ixlen rd_val = 0;
	ixlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOMINU_D(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = MIN(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

void AMOMAXU_D(struct hart __maybe_unused *hart)
{
	uxlen address = hart->x[hart->rs1];
	uxlen rd_val = 0;
	uxlen rs2_val = hart->x[hart->rs2];
	uxlen result = 0;
	i64 tmp;

	if (hart->access_memory(hart, hart->curr_priv_mode, bus_read_access, address,
							&tmp, 8))
		return;
	hart->x[hart->rd] = tmp;

	rd_val = hart->x[hart->rd];
	result = MAX(rd_val, rs2_val);

	hart->access_memory(hart, hart->curr_priv_mode,
						bus_write_access, address, &result, 8);
}

// multiplications
void DIV(struct hart __maybe_unused *hart)
{
	ixlen signed_rs1 = hart->x[hart->rs1];
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
	if (((uxlen)signed_rs1 == XLEN_INT_MIN) && (signed_rs2 == -1))
	{
		hart->x[hart->rd] = signed_rs1;
		return;
	}

	hart->x[hart->rd] = (signed_rs1 / signed_rs2);
}

void DIVU(struct hart __maybe_unused *hart)
{
	uxlen unsigned_rs1 = hart->x[hart->rs1];
	uxlen unsigned_rs2 = hart->x[hart->rs2];

	/*
	 * division by zero
	 */
	if (unsigned_rs2 == 0)
	{
		hart->x[hart->rd] = -1;
		return;
	}

	hart->x[hart->rd] = (unsigned_rs1 / unsigned_rs2);
}

void REM(struct hart __maybe_unused *hart)
{
	ixlen signed_rs1 = hart->x[hart->rs1];
	ixlen signed_rs2 = hart->x[hart->rs2];

	/*
	 * division by zero
	 */
	if (signed_rs2 == 0)
	{
		hart->x[hart->rd] = signed_rs1;
		return;
	}

	/*
	 * overflow
	 */
	if (((uxlen)signed_rs1 == XLEN_INT_MIN) && (signed_rs2 == -1))
	{
		hart->x[hart->rd] = 0;
		return;
	}

	hart->x[hart->rd] = (signed_rs1 % signed_rs2);
}

void REMU(struct hart __maybe_unused *hart)
{
	uxlen unsigned_rs1 = hart->x[hart->rs1];
	uxlen unsigned_rs2 = hart->x[hart->rs2];

	/*
	 * division by zero
	 */
	if (unsigned_rs2 == 0)
	{
		hart->x[hart->rd] = unsigned_rs1;
		return;
	}

	hart->x[hart->rd] = (unsigned_rs1 % unsigned_rs2);
}

void MUL(struct hart __maybe_unused *hart)
{
	hart->x[hart->rd] = (ixlen)hart->x[hart->rs1] * (ixlen)hart->x[hart->rs2];
}

void MULH(struct hart __maybe_unused *hart)
{
	i128 result = (i128)hart->x[hart->rs1] * (i128)hart->x[hart->rs2];
	hart->x[hart->rd] = (uxlen)(result >> 64);
}

void MULHU(struct hart __maybe_unused *hart)
{
	u128 result = (u128)hart->x[hart->rs1] * (u128)hart->x[hart->rs2];
	hart->x[hart->rd] = (uxlen)(result >> 64);
}

void MULHSU(struct hart __maybe_unused *hart)
{
	u128 result = (i128)hart->x[hart->rs1] * (u128)hart->x[hart->rs2];
	hart->x[hart->rd] = (uxlen)(result >> 64);
}

void MULW(struct hart __maybe_unused *hart)
{
	i32 signed_rs1 = hart->x[hart->rs1];
	i32 signed_rs2 = hart->x[hart->rs2];
	hart->x[hart->rd] = (i32)(signed_rs1 * signed_rs2);
}

void DIVW(struct hart __maybe_unused *hart)
{
	i32 signed_rs1 = hart->x[hart->rs1];
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
	if ((signed_rs1 == INT32_MIN) && (signed_rs2 == -1))
	{
		hart->x[hart->rd] = signed_rs1;
		return;
	}

	result = (signed_rs1 / signed_rs2);

	hart->x[hart->rd] = result;
}

void DIVUW(struct hart __maybe_unused *hart)
{
	u32 unsigned_rs1 = hart->x[hart->rs1];
	u32 unsigned_rs2 = hart->x[hart->rs2];
	i32 result = 0;

	/*
	 * division by zero
	 */
	if (unsigned_rs2 == 0)
	{
		hart->x[hart->rd] = -1;
		return;
	}

	result = (unsigned_rs1 / unsigned_rs2);

	hart->x[hart->rd] = result;
}

void REMW(struct hart __maybe_unused *hart)
{
	i32 signed_rs1 = hart->x[hart->rs1];
	i32 signed_rs2 = hart->x[hart->rs2];
	i32 result = 0;

	/*
	 * division by zero
	 */
	if (signed_rs2 == 0)
	{
		hart->x[hart->rd] = (ixlen)signed_rs1;
		return;
	}

	/*
	 * overflow
	 */
	if ((signed_rs1 == INT32_MIN) && (signed_rs2 == -1))
	{
		hart->x[hart->rd] = 0;
		return;
	}

	result = (signed_rs1 % signed_rs2);

	hart->x[hart->rd] = (ixlen)result;
}

void REMUW(struct hart __maybe_unused *hart)
{
	u32 unsigned_rs1 = hart->x[hart->rs1];
	u32 unsigned_rs2 = hart->x[hart->rs2];
	u32 result = 0;

	/*
	 * division by zero
	 */
	if (unsigned_rs2 == 0)
	{
		hart->x[hart->rd] = (i32)unsigned_rs1;
		return;
	}

	result = (unsigned_rs1 % unsigned_rs2);

	hart->x[hart->rd] = result;
}

int hart_decode(struct hart *hart)
{
	hart->opcode = (hart->instruction & 0x7F);
	switch (instruction_map[hart->opcode])
	{
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
		hart->imm = SIGN_EXTEND(hart->imm, 11);
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
		hart->imm = SIGN_EXTEND(hart->imm, 11);

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
		hart->func3 = (hart->instruction >> 12) & 0x7;
		hart->rs1 = (hart->instruction >> 15) & 0x1F;
		hart->rs2 = (hart->instruction >> 20) & 0x1F;
		hart->imm = (GET_BIT_RANGE(hart->instruction, 8, 4) << 1) |
					(GET_BIT_RANGE(hart->instruction, 25, 6) << 5) |
					(GET_BIT(hart->instruction, 7) << 11) |
					(GET_BIT(hart->instruction, 31) << 12);
		hart->imm = SIGN_EXTEND(hart->imm, 12);

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
		hart->imm = SIGN_EXTEND(hart->imm, 19) << 12;

		if (hart->opcode == 0b0010111)
			hart->execute = AUIPC;
		else if (hart->opcode == 0b0110111)
			hart->execute = LUI;
		else
			goto illegal_inst;

		break;
	case J:
		hart->rd = (hart->instruction >> 7) & 0x1F;
		hart->imm = (GET_BIT_RANGE(hart->instruction, 21, 10) << 1) |
					(GET_BIT(hart->instruction, 20) << 11) |
					(GET_BIT_RANGE(hart->instruction, 12, 8) << 12) |
					(GET_BIT(hart->instruction, 31) << 20);
		hart->imm = SIGN_EXTEND(hart->imm, 20);

		if (hart->opcode == 0b1101111)
			hart->execute = JAL;
		else
			goto illegal_inst;

		break;

	default:
	illegal_inst:
		prepare_sync_trap(hart, trap_cause_illegal_instr, 0);
		return -1;
	}

	return 0;
}