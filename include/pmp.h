#ifndef RISCV_PMP_H
#define RISCV_PMP_H

#include <types.h>

#define PMP_NR_CFG_REGS 2
#define PMP_NR_CFG_REGS_WARL_MAX 16

#define PMP_NR_ADDR_REGS 16
#define PMP_NR_ADDR_REGS_WARL_MAX 64

/*
 * Lock bit 
 */
#define PMP_CFG_L_BIT 7
/*
 * Adress matching bit offset 
 */
#define PMP_CFG_A_BIT_OFFS 3
/*
 * Executable bit 
 */
#define PMP_CFG_X_BIT 2
/*
 * Writable bit 
 */
#define PMP_CFG_W_BIT 1
/*
 * Readable bit 
 */
#define PMP_CFG_R_BIT 0

enum pmp_addr_matching {
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
