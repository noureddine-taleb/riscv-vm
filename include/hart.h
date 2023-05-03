#ifndef RISCV_CORE_H
#define RISCV_CORE_H

#include <types.h>

#include <csr.h>
#include <pmp.h>
#include <trap.h>
#include <clint.h>

#define NR_RVI_REGS 32

#ifdef CORE_DEBUG
#define CORE_DBG(...)        \
	do                       \
	{                        \
		printf(__VA_ARGS__); \
	} while (0)
#else
#define CORE_DBG(...) \
	do                \
	{                 \
	} while (0)
#endif

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
	uxlen next_pc;

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
	u8 func6;
	u8 func5;
	u16 func12;
	uxlen immediate;
	uxlen jump_offset;

	u8 sync_trap_pending;
	uxlen sync_trap_cause;
	uxlen sync_trap_tval;

	/*
	 * points to the next instruction
	 */
	void (*execute_cb)(struct hart *hart);

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
uxlen csr_get_mask(struct csr_mapping *csr_regs, u16 address);
int access_protected_memory(struct hart *hart,
							privilege_level priv_level,
							bus_access_type access_type, uxlen addr,
							void *value, u8 len);
int __access_protected_memory(int skip, struct hart *hart,
							  privilege_level priv_level,
							  bus_access_type access_type,
							  uxlen addr, void *value, u8 len);
void hart_init_csr_regs(struct hart *hart);

typedef struct instruction_hook_struct
{
	void (*preparation_cb)(struct hart *hart, i32 *next_subcode);
	void (*execution_cb)(struct hart *hart_data);
	struct instruction_desc_struct *next;

} instruction_hook_t;

typedef struct instruction_desc_struct
{
	unsigned int instruction_hook_list_size;
	instruction_hook_t *instruction_hook_list;

} instruction_desc_t;
#define INIT_INSTRUCTION_LIST_DESC(_instruction_list) \
	instruction_desc_t _instruction_list##_desc =     \
		{sizeof(_instruction_list) / sizeof(_instruction_list[0]), _instruction_list}

#define DECL_INSTRUCTION_LIST_DESC(_instruction_list) \
	extern instruction_desc_t _instruction_list##_desc;

DECL_INSTRUCTION_LIST_DESC(JALR_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(BEQ_BNE_BLT_BGE_BLTU_BGEU_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(LB_LH_LW_LBU_LHU_LWU_LD_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SB_SH_SW_SD_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SRLI_SRAI_func6_subcode_list);
DECL_INSTRUCTION_LIST_DESC(ADDI_SLTI_SLTIU_XORI_ORI_ANDI_SLLI_SRLI_SRAI_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(ADD_SUB_MUL_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SLL_MULH_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SLT_MULHSU_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SLTU_MULHU_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(XOR_DIV_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SRL_SRA_DIVU_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(OR_REM_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(AND_REMU_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(ADD_SUB_SLL_SLT_SLTU_XOR_SRL_SRA_OR_AND_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SRLIW_SRAIW_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SLLIW_SRLIW_SRAIW_ADDIW_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SRLW_SRAW_DIVUW_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(ADDW_SUBW_MULW_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(ADDW_SUBW_SLLW_SRLW_SRAW_MULW_DIVW_DIVUW_REMW_REMUW_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(ECALL_EBREAK_URET_func12_sub5_subcode_list);
DECL_INSTRUCTION_LIST_DESC(SRET_WFI_func12_sub5_subcode_list);
DECL_INSTRUCTION_LIST_DESC(ECALL_EBREAK_URET_SRET_MRET_WFI_SFENCEVMA_func7_subcode_list);
DECL_INSTRUCTION_LIST_DESC(CSRRW_CSRRS_CSRRC_CSRRWI_CSRRSI_CSRRCI_ECALL_EBREAK_URET_SRET_MRET_WFI_SFENCEVMA_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(W_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func5_subcode_list);
DECL_INSTRUCTION_LIST_DESC(D_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func5_subcode_list);
DECL_INSTRUCTION_LIST_DESC(W_LR_SC_SWAP_ADD_XOR_AND_OR_MIN_MAX_MINU_MAXU_func3_subcode_list);
DECL_INSTRUCTION_LIST_DESC(opcode_list);

void call_from_opcode_list(struct hart *hart,
						   instruction_desc_t *opcode_list_desc, u32 opcode);
#endif /* RISCV_CORE_H */
