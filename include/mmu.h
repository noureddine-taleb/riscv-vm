#ifndef RISCV_MMU_H
#define RISCV_MMU_H

#include <types.h>
#include <types.h>

#define MMU_PAGE_VALID (1<<0)
#define MMU_PAGE_READ  (1<<1)
#define MMU_PAGE_WRITE (1<<2)
#define MMU_PAGE_EXEC  (1<<3)
#define MMU_PAGE_USER  (1<<4)
#define MMU_PAGE_GLOB  (1<<5)
#define MMU_PAGE_ACCESSED  (1<<6)
#define MMU_PAGE_DIRTY  (1<<7)
#define MMU_PAGE_PBMT  ((uxlen)0b11<<61)
#define MMU_PAGE_N  ((uxlen)1<<63)
#define MMU_PAGE_RESERVED  ((uxlen)0b1111111<<54)

#define MMU_SATP_MODE_SV39 8
#define SV39_LEVELS 3
#define SV39_PAGE_SIZE 4096
#define SV39_PAGE_TABLE_ENTRIES 512
#define SV39_PTESIZE 8

#define MMU_SATP_MODE_BIT 60
#define MMU_SATP_MODE_NR_BITS 4

#include <hart.h>
#include <types.h>

int mmu_write_csr(u16 address, struct csr_mapping *map, uxlen val);
int vm_check(struct hart *priv, privilege_level priv_level,
	     bus_access_type access_type, uxlen *addr, void *value, u8 len);
#endif /* RISCV_MMU_H */
