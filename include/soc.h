#ifndef RISCV_EXAMPLE_SOC_H
#define RISCV_EXAMPLE_SOC_H

#include <hart.h>
#include <types.h>

#include <plic.h>

#define MROM_BASE_ADDR 0x1000UL
#define MROM_SIZE_BYTES 0xf000UL

#define RAM_BASE_ADDR 0x80000000UL
#define RAM_SIZE_BYTES                                                         \
  0x8000000UL /* 128MB such as the default for the qemu virt * machine */

#define CLINT_BASE_ADDR 0x2000000UL
#define CLINT_SIZE_BYTES 0x10000UL

#define PLIC_BASE_ADDR 0x0C000000UL
#define PLIC_SIZE_BYTES 0x3FFF004UL

#define UART8250_TX_REG_ADDR 0x10000000UL
#define UART_NS8250_NR_REGS 12

#include <uart_8250.h>
#define EXTENSION_TO_MISA(extension) (1 << (extension - 'A'))
#define SUPPORTED_EXTENSIONS                                                   \
  (EXTENSION_TO_MISA('I') | EXTENSION_TO_MISA('M') | EXTENSION_TO_MISA('A') |  \
   EXTENSION_TO_MISA('S') | EXTENSION_TO_MISA('U'))

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

  struct uart_ns8250 uart8250;

  struct memory_mapping mappings[6];
};

void soc_init(struct soc *soc, char *fw_file_name, char *dtb_file_name);
void soc_run(struct soc *soc);
int soc_bus_access(struct soc *soc, privilege_level priv_level,
				   bus_access_type access_type, uxlen address, void *value,
				   u8 len);
#endif /* RISCV_EXAMPLE_SOC_H */
