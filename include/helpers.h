#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdarg.h>

#include <types.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ printf( __VA_ARGS__ ); } while( 0 )
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

#define die(fmt, ...) { fprintf(stderr, "--------> " fmt "\n", ##__VA_ARGS__); exit(1); }

#define ADDR_WITHIN(_addr, _start, _size) ( (_addr >= _start) && (_addr < (_start + _size)) )
#define ADDR_WITHIN_LEN(_addr, _len, _start, _size) ( (_addr >= _start) && ((_addr + _len) <= (_start + _size)) )

#define ADDR_ALIGN_DOWN(n, m) ((n) / (m) * (m))

#define ASSIGN_MIN(a,b) (((a)<(b))?(a):(b))
#define ASSIGN_MAX(a,b) (((a)>(b))?(a):(b))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

/*
 * Bit Operators 
 */
#define SET_BIT(_out_var,_nbit)   ((_out_var) |=  (1<<(_nbit)))
#define CLEAR_BIT(_out_var,_nbit) ((_out_var) &= ~(1<<(_nbit)))
#define FLIP_BIT(_out_var,_nbit)  ((_out_var) ^=  (1<<(_nbit)))
#define CHECK_BIT(_out_var,_nbit) ((_out_var) &   (1<<(_nbit)))
#define FIND_FIRST_BIT_SET(_var) (__builtin_ffsl(_var))

static inline void assign_u8_bit(u8 *out_var, u8 nbit, u8 bit_value)
{
	*out_var = (*out_var & ~(1UL << nbit)) | ((bit_value & 1) << nbit);
}

static inline void assign_u32_bit(u32 *out_var, u32 nbit, u32 bit_value)
{
	*out_var = (*out_var & ~(1UL << nbit)) | ((bit_value & 1) << nbit);
}

static inline void assign_xlen_bit(uxlen *out_var, uxlen nbit, uxlen bit_value)
{
	*out_var = (*out_var & ~(1UL << nbit)) | ((bit_value & 1) << nbit);
}

static inline void
assign_xlen_value_within_reg(uxlen *out_var,
			     uxlen nbit, uxlen value, uxlen mask)
{
	*out_var = (*out_var & ~(mask << (nbit))) | ((value & mask) << (nbit));
}

static inline u8 extract8(u8 value, int start, int length)
{
	return (value >> start) & (0xFF >> (8 - length));
}

static inline u32 extract32(u32 value, int start, int length)
{
	return (value >> start) & (0xFFFFFFFF >> (32 - length));
}

static inline uxlen extractxlen(uxlen value, int start, int length)
{
	return (value >> start) & (((uxlen) - 1) >>
				   ((sizeof(uxlen) * 8) - length));
}

#endif /* HELPERS_H */
