#include <types.h>
#include <soc.h>
#include <helpers.h>
#include <string.h>

#define INIT_MEM_ACCESS_STRUCT(_ref_soc, _entry, _bus_access_func, _priv,              \
							   _addr_start, _mem_size)                                 \
	{                                                                                  \
		size_t _tmp_count = _entry;                                                    \
		if (_tmp_count >= ARRAY_SIZE(_ref_soc->mappings))                             \
			die("No mem access pointer available for entry nr %d, please increase "    \
				"mappings!\n",                                                         \
				_entry);                                                               \
		_ref_soc->mappings[_tmp_count].bus_access = (bus_access_func)_bus_access_func; \
		_ref_soc->mappings[_tmp_count].private = _priv;                                   \
		_ref_soc->mappings[_tmp_count].start = _addr_start;                       \
		_ref_soc->mappings[_tmp_count].len = _mem_size;                           \
	}


static int memory_bus_access(u8 *mem_ptr, privilege_level __maybe_unused priv_level,
				  bus_access_type access_type,
				  uxlen address, void *value, u8 len)
{
	if (access_type == bus_write_access)
		memcpy(&mem_ptr[address], value, len);
	else
		memcpy(value, &mem_ptr[address], len);

	return 0;
}

void soc_init_mappings(struct soc *soc)
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
	INIT_MEM_ACCESS_STRUCT(soc, count++, memory_bus_access, soc->rom,
						   MROM_BASE_ADDR, MROM_SIZE_BYTES);
}
