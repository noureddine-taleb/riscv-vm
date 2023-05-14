#ifndef RISCV_EXAMPLE_SOC_H
#define RISCV_EXAMPLE_SOC_H

#include <hart.h>
#include <types.h>

#include <plic.h>

#define MROM_BASE_ADDR 0x1000UL
#define MROM_SIZE_BYTES 0xf000UL

#define RAM_BASE_ADDR 0x80000000UL
#define RAM_SIZE_BYTES 0x8000000UL /* 128MB */

#define CLINT_BASE_ADDR 0x2000000UL
#define CLINT_SIZE_BYTES 0x10000UL

#define PLIC_BASE_ADDR 0x0C000000UL
#define PLIC_SIZE_BYTES 0x3FFF004UL

#define UARTNS16550_TX_REG_ADDR 0x10000000UL
#define UART_NS16550_NR_REGS 12

#include <ns16550.h>
#define EXTENSION_TO_MISA(extension) (1 << (extension - 'A'))
#define SUPPORTED_EXTENSIONS                                                    \
	(EXTENSION_TO_MISA('I') | EXTENSION_TO_MISA('M') | EXTENSION_TO_MISA('A') | \
	 EXTENSION_TO_MISA('S') | EXTENSION_TO_MISA('U'))

#define MiB 0x100000
#define FDT_ALIGN (2 * MiB)

struct memory_mapping
{
	bus_access_func bus_access;
	void *private;
	uxlen start;
	uxlen len;
};

struct soc
{
	struct hart hart0;
	u8 __aligned(4) rom[MROM_SIZE_BYTES];
	u8 __aligned(4) ram[RAM_SIZE_BYTES];

	struct clint clint;
	struct plic plic;

	struct ns16550 ns16550;

	struct memory_mapping mappings[6];
};

void soc_init(struct soc *soc, char *fw_file_name, char *dtb_file_name);
void soc_run(struct soc *soc);
int soc_bus_access(struct soc *soc, privilege_level priv_level,
				   bus_access_type access_type, uxlen address, void *value,
				   u8 len);
void soc_init_mappings(struct soc *soc);
size_t get_file_size(char *file);
void load_file(char *file, u8 *memory);
#endif /* RISCV_EXAMPLE_SOC_H */
