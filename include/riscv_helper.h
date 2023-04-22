#ifndef RISCV_HELPER_H
#define RISCV_HELPER_H

#include <stdlib.h>
#include <stdarg.h>

#include <types.h>
#include <helpers.h>

#define PRINTF_FMT "%016lx"
#define PRINTF_FMTU "%lu"
#define XLEN_INT_MIN 0x8000000000000000

#define SHIFT_OP_MASK 0x3F

    /*
     * Generic SIGN extension 
     */
#define SIGNEX(v, sb) ((v) | (((v) & (1LL << (sb))) ? ~((1LL << (sb))-1LL) : 0))

#define UMUL umul64wide
#define MUL mul64wide
#define MULHSU mulhsu64wide

#define GEN_SIGNEX_FUNC(_bit) \
static inline ixlen SIGNEX_BIT_##_bit(ixlen input) \
{ \
    struct {ixlen x:(_bit+1);} s = { .x = input }; \
    return s.x; \
}

GEN_SIGNEX_FUNC(7)
    GEN_SIGNEX_FUNC(11)
    GEN_SIGNEX_FUNC(12)
    GEN_SIGNEX_FUNC(15)
    GEN_SIGNEX_FUNC(19)
    GEN_SIGNEX_FUNC(20)
    GEN_SIGNEX_FUNC(31)
static inline void umul64wide(u64 a, u64 b, u64 *hi, u64 *lo)
{
	u64 a_lo = (u32) a;
	u64 a_hi = a >> 32;
	u64 b_lo = (u32) b;
	u64 b_hi = b >> 32;

	u64 p0 = a_lo * b_lo;
	u64 p1 = a_lo * b_hi;
	u64 p2 = a_hi * b_lo;
	u64 p3 = a_hi * b_hi;

	u32 cy = (u32) (((p0 >> 32) + (u32) p1 + (u32) p2) >> 32);

	*lo = p0 + (p1 << 32) + (p2 << 32);
	*hi = p3 + (p1 >> 32) + (p2 >> 32) + cy;
}

static inline void mul64wide(i64 a, i64 b, i64 *hi, i64 *lo)
{
	umul64wide((u64) a, (u64) b, (u64 *) hi, (u64 *) lo);
	if (a < 0LL)
		*hi -= b;
	if (b < 0LL)
		*hi -= a;
}

static inline void mulhsu64wide(i64 a, u64 b, i64 *hi, i64 *lo)
{
	umul64wide((u64) a, (u64) b, (u64 *) hi, (u64 *) lo);
	if (a < 0LL)
		*hi -= b;
}

static inline void umul32wide(u32 a, u32 b, u32 *hi, u32 *lo)
{
	u32 a_lo = (u16) a;
	u32 a_hi = a >> 16;
	u32 b_lo = (u16) b;
	u32 b_hi = b >> 16;

	u32 p0 = a_lo * b_lo;
	u32 p1 = a_lo * b_hi;
	u32 p2 = a_hi * b_lo;
	u32 p3 = a_hi * b_hi;

	u32 cy = (u16) (((p0 >> 16) + (u16) p1 + (u16) p2) >> 16);

	*lo = p0 + (p1 << 16) + (p2 << 16);
	*hi = p3 + (p1 >> 16) + (p2 >> 16) + cy;
}

static inline void mul32wide(i32 a, i32 b, i32 *hi, i32 *lo)
{
	umul32wide((u32) a, (u32) b, (u32 *) hi, (u32 *) lo);
	if (a < 0LL)
		*hi -= b;
	if (b < 0LL)
		*hi -= a;
}

static inline void mulhsu32wide(i32 a, u32 b, i32 *hi, i32 *lo)
{
	umul32wide((u32) a, (u32) b, (u32 *) hi, (u32 *) lo);
	if (a < 0LL)
		*hi -= b;
}

#endif /* RISCV_HELPER_H */
