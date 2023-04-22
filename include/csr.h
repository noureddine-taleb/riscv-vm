#ifndef RISCV_CSR_H
#define RISCV_CSR_H

#include <types.h>
#include <pmp.h>

#define __CSR_PRIV_LEVEL(addr) ((addr >> 8) & 0x3)
#define __CSR_ACCESSIBLE(addr, priv_level) (priv_level >= __CSR_PRIV_LEVEL(addr))
#define __CSR_WRITABLE(addr) (((addr >> 10) & 0x3) != 0x3)
#define CSR_READABLE(addr, priv_level) (__CSR_ACCESSIBLE(addr, priv_level))
#define CSR_WRITABLE(addr, priv_level) (__CSR_ACCESSIBLE(addr, priv_level) && __CSR_WRITABLE(addr))

#define CSR_ADDR_MVENDORID 0xF11
#define CSR_ADDR_MARCHID   0xF12
#define CSR_ADDR_MIMPID    0xF13
#define CSR_ADDR_MHARTID   0xF14

#define CSR_ADDR_MSTATUS      0x300
#define CSR_ADDR_MISA         0x301
#define CSR_ADDR_MEDELEG      0x302
#define CSR_ADDR_MIDELEG      0x303
#define CSR_ADDR_MIE          0x304
#define CSR_ADDR_MTVEC        0x305
#define CSR_ADDR_MCOUNTEREN   0x306

#define CSR_ADDR_MSCRATCH     0x340
#define CSR_ADDR_MEPC         0x341
#define CSR_ADDR_MCAUSE       0x342
#define CSR_ADDR_MTVAL        0x343
#define CSR_ADDR_MIP          0x344

#define CSR_PMPCFG0           0x3A0
#define CSR_PMPCFG1           0x3A1
#define CSR_PMPCFG2           0x3A2
#define CSR_PMPCFG3           0x3A3
#define CSR_PMPADDR0          0x3B0
#define CSR_PMPADDR1          0x3B1
#define CSR_PMPADDR2          0x3B2
#define CSR_PMPADDR3          0x3B3
#define CSR_PMPADDR4          0x3B4
#define CSR_PMPADDR5          0x3B5
#define CSR_PMPADDR6          0x3B6
#define CSR_PMPADDR7          0x3B7
#define CSR_PMPADDR8          0x3B8
#define CSR_PMPADDR9          0x3B9
#define CSR_PMPADDR10         0x3BA
#define CSR_PMPADDR11         0x3BB
#define CSR_PMPADDR12         0x3BC
#define CSR_PMPADDR13         0x3BD
#define CSR_PMPADDR14         0x3BE
#define CSR_PMPADDR15         0x3BF

/*
 * Supervisor CSRs 
 */
#define CSR_ADDR_SSTATUS      0x100
#define CSR_ADDR_SEDELEG      0x102
#define CSR_ADDR_SIDELEG      0x103
#define CSR_ADDR_SIE          0x104
#define CSR_ADDR_STVEC        0x105
#define CSR_ADDR_SCOUNTEREN   0x106

#define CSR_ADDR_SSCRATCH     0x140
#define CSR_ADDR_SEPC         0x141
#define CSR_ADDR_SCAUSE       0x142
#define CSR_ADDR_STVAL        0x143
#define CSR_ADDR_SIP          0x144

#define CSR_ADDR_SATP         0x180

#define CSR_ADDR_MCYCLE       0xB00
#define CSR_ADDR_MINSTRET     0xB02
#define CSR_ADDR_MCYCLEH      0xB80
#define CSR_ADDR_MINSTRETH    0xB82

#define CSR_ADDR_CYCLE        0xC00
#define CSR_ADDR_TIME         0xC01
#define CSR_ADDR_CYCLEH       0xC80
#define CSR_ADDR_TIMEH        0xC81

#define CSR_HPMCOUNTER_WARL_MAX 32

#define CSR_ADDR_MAX          0xFFF

/*
 * CSR WRITE MASKS 
 */
#define CSR_MASK_WR_ALL 0xFFFFFFFFFFFFFFFF
#define CSR_MSTATUS_MASK 0x8000000F007FF9BB
#define CSR_MTVEC_MASK 0xFFFFFFFFFFFFFFFC

#define CSR_SSTATUS_MASK 0x80000003000DE133
#define CSR_SATP_MASK 0xF0000FFFFFFFFFFF

#define CSR_MASK_ZERO 0
#define CSR_MIP_MIE_MASK 0xBBB
#define CSR_MIDELEG_MASK CSR_MIP_MIE_MASK
/*
 * In particular, medeleg[11] are hardwired to zero. 
 */
#define CSR_MEDELEG_MASK 0xF3FF

#define CSR_STVEC_MASK CSR_MTVEC_MASK
#define CSR_SIP_SIE_MASK 0x333
#define CSR_SIDELEG_MASK CSR_SIP_SIE_MASK
/*
 * In particular, sedeleg[11:9] are all hardwired to zero. 
 */
#define CSR_SEDELEG_MASK 0xF1FF

#define INIT_CSR_REG_SPECIAL_COOKIE(csr_mapping, _i, _mask, _address, _read, _write, _cookie) { \
    csr_mapping[_i].valid = 1; \
    csr_mapping[_i].value = _address; \
    csr_mapping[_i].mask = _mask; \
    csr_mapping[_i].read = _read; \
    csr_mapping[_i].write = _write; \
    csr_mapping[_i].cookie = (unsigned long)_cookie; }

#define INIT_CSR_REG_SPECIAL(csr_mapping, _i, _mask, _address, _read, _write) { \
    csr_mapping[_i].valid = 1; \
    csr_mapping[_i].value = _address; \
    csr_mapping[_i].mask = _mask; \
    csr_mapping[_i].read = _read; \
    csr_mapping[_i].write = _write; }

#define INIT_CSR_REG_DEFAULT(csr_mapping, i, mask, address) \
	INIT_CSR_REG_SPECIAL(csr_mapping, i, mask, address, NULL, NULL)

struct csr_backing_store {
	/*
	 * shared 
	 */
	uxlen status;
	uxlen ie;
	uxlen ip;
	uxlen isa;
	uxlen vendorid;
	uxlen archid;
	uxlen impid;
	uxlen hartid;
	uxlen cycle;
	uxlen instret;
	uxlen time;

	/*
	 * machine regs 
	 */
	uxlen medeleg;
	uxlen mideleg;
	uxlen mtvec;
	uxlen mcounteren;
	uxlen mscratch;
	uxlen mepc;
	uxlen mcause;
	uxlen mtval;

	/*
	 * supervisor regs 
	 */
	uxlen stvec;
	uxlen scounteren;
	uxlen sscratch;
	uxlen sepc;
	uxlen scause;
	uxlen stval;
	uxlen satp;

	/*
	 * pmp 
	 */
	uxlen pmpcfg[PMP_NR_CFG_REGS];
	uxlen pmpaddr[PMP_NR_ADDR_REGS];

};

struct csr_mapping {
	int valid;
	uxlen *value;
	uxlen mask;
	unsigned long cookie;

	int (*read)(u16 address, struct csr_mapping *map, uxlen *val);
	int (*write)(u16 address, struct csr_mapping *map, uxlen val);
};

int csr_read_reg(struct csr_mapping *csr_regs,
		 privilege_level curr_priv_mode, u16 address, uxlen *out_val);
int csr_write_reg(struct csr_mapping *csr_regs,
		  privilege_level curr_priv_mode, u16 address, uxlen val);

#endif /* RISCV_CSR_H */
