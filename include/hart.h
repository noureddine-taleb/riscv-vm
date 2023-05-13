#ifndef RISCV_CORE_H
#define RISCV_CORE_H

#include <types.h>

#include <csr.h>
#include <pmp.h>
#include <trap.h>
#include <clint.h>

#define NR_RVI_REGS 32

#define ADDR_MISALIGNED(addr) (addr & 0x3)

#include <mmu.h>
#include <pmp.h>
#include <types.h>

struct hart
{
	struct soc *soc;
	privilege_level curr_priv_mode;

	/*
	 * Registers
	 */
	uxlen x[NR_RVI_REGS];
	uxlen pc;
	uxlen override_pc;

	/*
	 * CSR
	 */
	struct csr_backing_store csr_store;
	struct csr_mapping csr_regs[CSR_ADDR_MAX];

	u32 instruction;
	u8 opcode;
	u8 rd;
	u8 rs1;
	u8 rs2;
	u8 func3;
	u8 func7;
	uxlen imm;

	u8 sync_trap_pending;
	uxlen sync_trap_cause;
	uxlen sync_trap_tval;

	/*
	 * points to the next instruction
	 */
	void (*execute)(struct hart *hart);

	/*
	 * externally hooked
	 */
	bus_access_func access_memory;

	int lr_valid;
	uxlen lr_address;
};

void hart_run(struct hart *hart);
void hart_update_ip(struct hart *hart, u8 ext_int, u8 tim_int, u8 sw_int);
u8 hart_handle_pending_interrupts(struct hart *hart);

void hart_init(struct hart *hart, struct soc *soc, u64 _start);
void prepare_sync_trap(struct hart *hart, uxlen cause, uxlen tval);
int access_protected_memory(struct hart *hart,
							privilege_level priv_level,
							bus_access_type access_type, uxlen addr,
							void *value, u8 len);
int __access_protected_memory(int skip, struct hart *hart,
							  privilege_level priv_level,
							  bus_access_type access_type,
							  uxlen addr, void *value, u8 len);
void hart_init_csr_regs(struct hart *hart);

#endif /* RISCV_CORE_H */
