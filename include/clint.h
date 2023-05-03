#ifndef RISCV_CLINT_H
#define RISCV_CLINT_H

#include <types.h>

enum clint_regs
{
	clint_msip = 0,
	clint_mtimecmp,
	clint_mtime,

	clint_reg_max
};

struct clint
{
	u64 regs[clint_reg_max];
};

int clint_bus_access(struct clint *priv, privilege_level priv_level,
					 bus_access_type access_type, uxlen address,
					 void *value, u8 len);
void clint_check_interrupts(struct clint *clint, u8 *msi, u8 *mti);

#endif /* RISCV_CLINT_H */
