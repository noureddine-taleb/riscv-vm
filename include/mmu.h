#ifndef RISCV_MMU_H
#define RISCV_MMU_H

#include <types.h>
#include <hart.h>

#define MMU_PAGE_VALID (1 << 0)
#define MMU_PAGE_READ (1 << 1)
#define MMU_PAGE_WRITE (1 << 2)
#define MMU_PAGE_EXEC (1 << 3)
#define MMU_PAGE_USER (1 << 4)
#define MMU_PAGE_GLOB (1 << 5)
#define MMU_PAGE_ACCESSED (1 << 6)
#define MMU_PAGE_DIRTY (1 << 7)
#define MMU_PAGE_PBMT ((uxlen)0b11 << 61)
#define MMU_PAGE_N ((uxlen)1 << 63)
#define MMU_PAGE_RESERVED ((uxlen)0b1111111 << 54)

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT) // 4096

#define MMU_SATP_MODE_SV39 8
#define MMU_SATP_MODE_OFF 0

#define SV39_LEVELS 3
#define SV39_PTESIZE 8

#define SATP_MODE(satp) GET_BIT_RANGE(satp, 60, 4)
#define SATP_PPN(satp) (satp & 0x3FFFFF)

typedef struct pte {
	u8 valid: 1;
	u8 read: 1;
	u8 write: 1;
	u8 exec: 1;
	u8 user: 1;
	u8 global: 1;
	u8 accessed: 1;
	u8 dirty: 1;
	u8 rsw: 2; // The RSW field is reserved for use by supervisor software
	u64 ppn0: 9;
	u64 ppn1: 9;
	u64 ppn2: 26;
	u8 __reserved: 7;
	u8 pbmt: 2;
	u8 n: 1; // Svnapot extension
} __packed pte_t;
_Static_assert(sizeof(pte_t) == SV39_PTESIZE, "pte_t size is not correct");

#define PTE_PPN(pte) ((u64)((pte.ppn2 << 18) | (pte.ppn1 << 9) | pte.ppn0))
#define PTE_ACCESS_FLAGS(pte) ((pte.exec << 2) | (pte.write << 1) | pte.read)

int mmu_write_csr(u16 address, struct csr_reg *map, uxlen val);
int vm_check(struct hart *priv, privilege_level priv_level,
			 bus_access_type access_type, uxlen *addr, void *value, u8 len);
#endif /* RISCV_MMU_H */
