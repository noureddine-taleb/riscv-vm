#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <soc.h>
#include <riscv_helper.h>

#include <file.h>

#define INIT_MEM_ACCESS_STRUCT(_ref_soc, _entry, _bus_access_func, _priv,   \
							   _addr_start, _mem_size)                         \
  {                                                                            \
	size_t _tmp_count = _entry;                                                \
	if (_tmp_count >= (sizeof(_ref_soc->mappings) /                   \
					   sizeof(_ref_soc->mappings[0])))                \
	  die("No mem access pointer available for entry nr %d, please increase "  \
		  "mappings!\n",                                                 \
		  _entry);                                                             \
	_ref_soc->mappings[_tmp_count].bus_access = (bus_access_func)_bus_access_func;     \
	_ref_soc->mappings[_tmp_count].priv = _priv;                      \
	_ref_soc->mappings[_tmp_count].addr_start = _addr_start;          \
	_ref_soc->mappings[_tmp_count].mem_size = _mem_size;              \
  }

#define MiB 0x100000
#define FDT_ALIGN (2 * MiB)

static int
memory_bus_access(u8 *mem_ptr, privilege_level priv_level,
		  bus_access_type access_type,
		  uxlen address, void *value, u8 len)
{
	(void)priv_level;

	if (access_type == bus_write_access)
		memcpy(&mem_ptr[address], value, len);
	else
		memcpy(value, &mem_ptr[address], len);

	return 0;
}

static void soc_init_mappings(struct soc *soc)
{
	int count = 0;
	INIT_MEM_ACCESS_STRUCT(soc, count++, memory_bus_access, soc->ram,
			       RAM_BASE_ADDR, RAM_SIZE_BYTES);
	INIT_MEM_ACCESS_STRUCT(soc, count++, clint_bus_access, &soc->clint,
			       CLINT_BASE_ADDR, CLINT_SIZE_BYTES);
	INIT_MEM_ACCESS_STRUCT(soc, count++, plic_bus_access, &soc->plic,
			       PLIC_BASE_ADDR, PLIC_SIZE_BYTES);
	INIT_MEM_ACCESS_STRUCT(soc, count++, uart_bus_access, &soc->ns16550,
			       UARTNS16550_TX_REG_ADDR, UART_NS16550_NR_REGS);
	INIT_MEM_ACCESS_STRUCT(soc, count++, memory_bus_access, soc->mrom,
			       MROM_BASE_ADDR, MROM_SIZE_BYTES);
}

int
soc_bus_access(struct soc *soc, privilege_level priv_level,
	       bus_access_type access_type, uxlen address, void *value, u8 len)
{
	uxlen tmp_addr = 0;
	size_t i = 0;

	for (i = 0; i < (sizeof(soc->mappings) / sizeof(soc->mappings[0])); i++) {
		if (ADDR_WITHIN_LEN(address, len, soc->mappings[i].addr_start,
				    soc->mappings[i].mem_size)) {
			tmp_addr = address - soc->mappings[i].addr_start;
			return soc->mappings[i].bus_access(soc->mappings[i].
							   priv, priv_level,
							   access_type,
							   tmp_addr, value,
							   len);
		}
	}

	die("%s unampped address at!: "
	    "Addr: " PRINTF_FMT " Len: %d Cycle: %ld  PC: " PRINTF_FMT "\n",
	    access_type == bus_read_access ? "read from" : access_type ==
	    bus_write_access ? "write to" : "execute", address, len,
	    soc->hart0.csr_store.cycle, soc->hart0.pc);
}

void soc_init(struct soc *soc, char *fdt, char *kernel)
{
	memset(soc, 0, sizeof(struct soc));

	static u8 __attribute__((aligned(4))) soc_mrom[MROM_SIZE_BYTES];
	static u8 __attribute__((aligned(4))) soc_ram[RAM_SIZE_BYTES];

	soc->mrom = soc_mrom;
	soc->ram = soc_ram;

	/*
	 * Copy dtb and firmware 
	 */
	u64 fdt_size = get_file_size(fdt);
	u64 kernel_size = get_file_size(kernel);

	if (fdt_size + FDT_ALIGN + kernel_size > RAM_SIZE_BYTES)
		die("memory to small for kernel + fdt");
	/*
	 * This is a little annoying: qemu keeps changing this stuff
	 * from time to time and I need to do it the same, otherwise the
	 * tests would fail as they won't match with qemu's results anymore
	 * Be Aware: 2 * MiB was taken from qemu 5.2 but it seems
	 * they already changed this again in later versions to 16 * MiB
	 */
	u64 fdt_addr =
	    ADDR_ALIGN_DOWN(RAM_BASE_ADDR + RAM_SIZE_BYTES - fdt_size,
			    FDT_ALIGN);
	u64 fdt_off = fdt_addr - RAM_BASE_ADDR;
	load_file(kernel, &soc_ram[0]);
	load_file(fdt, &soc_ram[fdt_off]);

	/*
	 * this is the reset vector, taken from qemu v5.2 
	 * abi = firmware(hartid, fdt)
	 */
	u32 reset_vec[] = {
		0x00000297,	/* 1: auipc t0, %pcrel_hi(fw_dyn) = get current 
				 * pc */
		0x02828613,	/* addi a2, t0, %pcrel_lo(1b) = load the end of 
				 * rvec */
		0xf1402573,	/* csrr a0, mhartid */
		0x0202b583,	/* ld a1, 32(t0) = fdt */
		0x0182b283,	/* ld t0, 24(t0) = ram base */
		0x00028067,	/* jr t0 */
		(u32) RAM_BASE_ADDR,	/* start: .dword */
		(u32) (RAM_BASE_ADDR >> 32),	/* start: .dword */
		fdt_addr,	/* fdt_laddr: .dword */
		0x00000000,
		/*
		 * fw_dyn: 
		 */
	};

	if (sizeof(reset_vec) > MROM_SIZE_BYTES)
		die("rom too small for reset verctor");
	memcpy(soc_mrom, reset_vec, sizeof(reset_vec));

	/*
	 * initialize one hart with a csr table 
	 */
	hart_init(&soc->hart0, soc, MROM_BASE_ADDR);

	uart_init(&soc->ns16550);

	/*
	 * initialize ram and peripheral read write access pointers 
	 */
	soc_init_mappings(soc);

	DEBUG_PRINT("rv SOC initialized!\n");
}

void soc_run(struct soc *soc)
{
	u8 mei = 0, msi = 0, mti = 0;
	u8 uart_irq_pending = 0;

	while (1) {
		hart_run(&soc->hart0);

		uart_irq_pending = uart_check_interrupts(&soc->ns16550);
		plic_set_pending_interrupt(&soc->plic, 10, uart_irq_pending);

		mei = plic_check_interrupts(&soc->plic);
		clint_check_interrupts(&soc->clint, &msi, &mti);

		hart_update_ip(&soc->hart0, mei, mti, msi);
		hart_handle_pending_interrupts(&soc->hart0);
	}
}
