#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <soc.h>
#include <helpers.h>

void soc_init(struct soc *soc, char *fdt, char *kernel)
{
	memset(soc, 0, sizeof(struct soc));

	/*
	 * Copy dtb and firmware
	 */
	u64 fdt_size = get_file_size(fdt);
	u64 kernel_size = get_file_size(kernel);

	if (fdt_size + FDT_ALIGN + kernel_size > RAM_SIZE_BYTES)
		die("memory to small for kernel + fdt");

	u64 fdt_addr =
		ADDR_ALIGN_DOWN(RAM_BASE_ADDR + RAM_SIZE_BYTES - fdt_size,
						FDT_ALIGN);
	u64 fdt_off = fdt_addr - RAM_BASE_ADDR;
	load_file(kernel, &soc->ram[0]);
	load_file(fdt, &soc->ram[fdt_off]);

	/*
	 * reset vector
	 * abi = firmware(hartid, fdt)
	 */
	u32 reset_vec[] = {
		0x00000297,					/* 1: auipc t0, %pcrel_hi(fw_dyn) = get current
									 * pc */
		0x02828613,					/* addi a2, t0, %pcrel_lo(1b) = load the end of
									 * rvec */
		0xf1402573,					/* csrr a0, mhartid */
		0x0202b583,					/* ld a1, 32(t0) = fdt */
		0x0182b283,					/* ld t0, 24(t0) = ram base */
		0x00028067,					/* jr t0 */
		(u32)RAM_BASE_ADDR,			/* start: .dword */
		(u32)(RAM_BASE_ADDR >> 32), /* start: .dword */
		fdt_addr,					/* fdt_laddr: .dword */
		0x00000000,
		/*
		 * fw_dyn:
		 */
	};

	if (sizeof(reset_vec) > MROM_SIZE_BYTES)
		die("rom too small for reset vector");
	memcpy(soc->rom, reset_vec, sizeof(reset_vec));

	/*
	 * initialize one hart with a csr table
	 */
	hart_init(&soc->hart0, soc, MROM_BASE_ADDR);

	uart_init(&soc->ns16550);

	/*
	 * initialize ram and peripheral read write access pointers
	 */
	soc_init_mappings(soc);
}

void soc_run(struct soc *soc)
{
	u8 mei = 0, msi = 0, mti = 0;
	u8 uart_irq_pending = 0;

	while (1)
	{
		hart_run(&soc->hart0);

		uart_irq_pending = uart_check_interrupts(&soc->ns16550);
		plic_set_pending_interrupt(&soc->plic, 10, uart_irq_pending);

		mei = plic_check_interrupts(&soc->plic);
		clint_check_interrupts(&soc->clint, &msi, &mti);

		hart_update_ip(&soc->hart0, mei, mti, msi);
		hart_handle_pending_interrupts(&soc->hart0);
	}
}
