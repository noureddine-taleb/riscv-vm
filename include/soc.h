#ifndef RISCV_EXAMPLE_SOC_H
#define RISCV_EXAMPLE_SOC_H

#include <types.h>
#include <hart.h>

#include <plic.h>
#include <uart_8250.h>
#include <simple_uart.h>

#define MROM_BASE_ADDR 0x1000UL
#define MROM_SIZE_BYTES 0xf000UL

#define RAM_BASE_ADDR 0x80000000UL
#define RAM_SIZE_BYTES                                                         \
  0x8000000UL			/* 128MB such as the default for the qemu virt * machine */

#define CLINT_BASE_ADDR 0x2000000UL
#define CLINT_SIZE_BYTES 0x10000UL

#define SIMPLE_UART_TX_REG_ADDR 0x3000000UL
#define SIMPLE_UART_SIZE_BYTES 0x2

#define PLIC_BASE_ADDR 0x0C000000UL
#define PLIC_SIZE_BYTES 0x3FFF004UL

#define UART8250_TX_REG_ADDR 0x10000000UL

#define EXTENSION_TO_MISA(extension) (1 << (extension - 'A'))
#define SUPPORTED_EXTENSIONS                                                \
  (EXTENSION_TO_MISA('I') | EXTENSION_TO_MISA('M') |                     \
   EXTENSION_TO_MISA('A') | EXTENSION_TO_MISA('S') |                     \
   EXTENSION_TO_MISA('U'))

struct memory_mapping {
	bus_access_func bus_access;
	void *priv;
	uxlen addr_start;
	uxlen mem_size;
};

struct soc {
	struct hart hart0;
	u8 *mrom;
	u8 *ram;

	struct clint clint;
	struct plic plic;

#ifdef USE_SIMPLE_UART
	struct simple_uart uart;
#else
	struct uart_ns8250 uart8250;
#endif

	struct memory_mapping mappings[6];
};

void soc_init(struct soc *soc, char *fw_file_name, char *dtb_file_name);
void soc_run(struct soc *soc);
int soc_bus_access(struct soc *soc, privilege_level priv_level,
		   bus_access_type access_type,
		   uxlen address, void *value, u8 len);
#endif /* RISCV_EXAMPLE_SOC_H */
