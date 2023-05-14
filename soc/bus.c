#include <types.h>
#include <soc.h>
#include <helpers.h>

int soc_bus_access(struct soc *soc, privilege_level priv_level,
				   bus_access_type access_type, uxlen address, void *value, u8 len)
{
	uxlen off = 0;
	size_t i = 0;

	for (i = 0; i < ARRAY_SIZE(soc->mappings); i++)
	{
		if (RANGE_WITHIN_RANGE(address, len, soc->mappings[i].start,
							soc->mappings[i].len))
		{
			off = address - soc->mappings[i].start;
			return soc->mappings[i].bus_access(soc->mappings[i].private, priv_level,
											   access_type, off, value, len);
		}
	}

	die("bus %s: unampped address at: %#016lx len: %d cycle: %ld  pc: %#016lx\n",
		BUS_ACCESS_STR(access_type),
		address,
		len,
		soc->hart0.csr_store.cycle,
		soc->hart0.pc);
}
