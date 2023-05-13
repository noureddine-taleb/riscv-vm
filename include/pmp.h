#ifndef RISCV_PMP_H
#define RISCV_PMP_H

#include <types.h>

#define PMP_NR_CFG_REGS 2
#define PMP_NR_CFG_REGS_WARL_MAX 16

#define PMP_NR_ADDR_REGS 16
#define PMP_NR_ADDR_REGS_WARL_MAX 64

#define PMP_CFG_L_BIT 7
#define PMP_CFG_A_BITS 3
#define PMP_CFG_X_BIT 2
#define PMP_CFG_W_BIT 1
#define PMP_CFG_R_BIT 0

#define PMP_CFG_ADDRESS_MATCHING(pmpcfg) GET_BIT_RANGE(pmpcfg, PMP_CFG_A_BITS, 2)
#define PMP_CFG_LOCKED(pmpcfg) GET_BIT(pmpcfg, PMP_CFG_L_BIT)
#define PMP_CFG_ACCESS(pmpcfg) (pmpcfg & 0x7)

enum pmp_addr_matching
{
	pmp_a_off = 0,
	pmp_a_tor,
	pmp_a_na4,
	pmp_a_napot
};

int pmp_write_csr_cfg(u16 address, struct csr_mapping *map, uxlen val);
int pmp_write_csr_addr(u16 address, struct csr_mapping *map, uxlen val);
int pmp_check(struct hart *priv, privilege_level priv_level,
			  bus_access_type access_type, uxlen *addr, void *value, u8 len);

#endif /* RISCV_PMP_H */
