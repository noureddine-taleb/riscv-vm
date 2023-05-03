#include <stdio.h>
#include <string.h>

#include <riscv_helper.h>

#include <clint.h>

#define CLINT_MSIP_OFFS       0x0000
#define CLINT_MTIMECMP_OFFS   0x4000
#define CLINT_MTIME_OFFS      0xBFF8
#define CLINT_REG_SIZE_BYTES  8

static ixlen get_u8_arr_index_offs(uxlen address)
{
	if (address < (CLINT_MSIP_OFFS + CLINT_REG_SIZE_BYTES)) {
		return (0 * CLINT_REG_SIZE_BYTES);
	} else
	    if (ADDR_WITHIN(address, CLINT_MTIMECMP_OFFS, CLINT_REG_SIZE_BYTES))
	{
		return (1 * CLINT_REG_SIZE_BYTES);
	} else if (ADDR_WITHIN(address, CLINT_MTIME_OFFS, CLINT_REG_SIZE_BYTES)) {
		// printf("NOOOOO!\n");
		// while(1);
		return (2 * CLINT_REG_SIZE_BYTES);
	}

	return -1;
}

int
clint_bus_access(struct clint *clint, privilege_level priv_level,
		 bus_access_type access_type, uxlen address,
		 void *value, u8 len)
{
	(void)priv_level;
	uxlen tmp_addr = 0;
	u8 *tmp_u8 = (u8 *) clint->regs;
	ixlen arr_index_offs = -1;

	arr_index_offs = get_u8_arr_index_offs(address);

	if (arr_index_offs >= 0) {
		tmp_addr = (address & 0x7) + arr_index_offs;
		if (access_type == bus_write_access) {
			memcpy(&tmp_u8[tmp_addr], value, len);
			// uxlen tmp_xlen = 0;
			// memcpy(&tmp_xlen, value, len);
			// printf("addr: %x CMP %d cmp reg: %ld time reg: %ld
			// len %d arr index offs %d tmp_addr: %d\n", address,
			// tmp_u32, clint->regs[clint_mtimecmp],
			// clint->regs[clint_mtime], len, arr_index_offs,
			// tmp_addr);
			// if(address == 0xBFF8)
			// printf("addr: "PRINTF_FMT" CMP %ld cmp reg: %lu time 
			// 
			// 
			// reg: %lu tmp_addr "PRINTF_FMT"\n", address,
			// tmp_xlen, clint->regs[clint_mtimecmp],
			// clint->regs[clint_mtime], tmp_addr);
		} else {
			memcpy(value, &tmp_u8[tmp_addr], len);
			// if(address == 0xBFF8)
			// {
			// printf("0xBFF8!!!!!!!!!!!!!!\n");
			// while(1);
			// }
		}
	}

	return 0;
}

void clint_check_interrupts(struct clint *clint, u8 *msi, u8 *mti)
{
	static int i = 1;
	// todo: remove clint_mtime
	if (i % 1 == 0)
		clint->regs[clint_mtime] += 1;

	i++;

	*mti = (clint->regs[clint_mtime] >= clint->regs[clint_mtimecmp]);
	*msi = (clint->regs[clint_msip] & 0x1);
}
