#include <types.h>
#include <mmu.h>
#include <pmp.h>
#include <helpers.h>
#include <soc.h>

int __access_protected_memory(int skip, struct hart *hart,
							  privilege_level priv_level,
							  bus_access_type access_type, uxlen addr,
							  void *value, u8 len)
{

	static check_protection memory_protection_layers[] = {
		vm_check,
		pmp_check
	};

	int layers = ARRAY_SIZE(memory_protection_layers);
	for (int i = skip; i < layers; i++)
	{
		if (memory_protection_layers[i](hart, priv_level, access_type, &addr, value, len) < 0)
			return -1;
	}

	return soc_bus_access(hart->soc, priv_level, access_type, addr, value,
						  len);
}

int access_protected_memory(struct hart *hart,
							privilege_level priv_level,
							bus_access_type access_type, uxlen addr,
							void *value, u8 len)
{
	return __access_protected_memory(0, hart, priv_level,
									 access_type, addr, value, len);
}

int access_supervisor_physical_memory(struct hart *hart,
							  bus_access_type access_type, uxlen addr,
							  void *value, u8 len)
{
	return __access_protected_memory(1, hart, supervisor_mode,
								access_type, addr, value,
								len);
}