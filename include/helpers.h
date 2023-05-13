#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <types.h>

#define debug(fmt, ...) fprintf(stderr, "--------> " fmt "\n", ##__VA_ARGS__)

#define die(fmt, ...)                                       \
	{                                                           \
		debug(fmt, ##__VA_ARGS__); 										\
		exit(1);                                                \
	}

#define ADDR_WITHIN_RANGE(_addr, _start, _size) ((_addr >= _start) && (_addr < (_start + _size)))
#define RANGE_WITHIN_RANGE(_addr, _len, _start, _size) ((_addr >= _start) && ((_addr + _len) <= (_start + _size)))
#define ADDR_MISALIGNED(addr) (addr & 0x3)
#define ADDR_ALIGN_DOWN(addr, align) ((addr) & ~(align - 1))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define XLEN_INT_MIN 0x8000000000000000

#define SHAMT_MASK 0x3F

/*
 * Bit Operators
 */
#define SET_BIT(_out_var, _nbit) ((_out_var) |= (1 << (_nbit)))
#define CLEAR_BIT(_out_var, _nbit) ((_out_var) &= ~(1 << (_nbit)))
#define FLIP_BIT(_out_var, _nbit) ((_out_var) ^= (1 << (_nbit)))
#define GET_BIT(_out_var, _nbit) (!!((_out_var) & (1 << (_nbit))))
#define GET_BIT_RANGE(_out_var, _start, _len) (((_out_var) >> _start) & ((1 << _len) - 1))
#define UPDATE_BIT(val, bit, bit_value) ((val) = ((val) & ~(1UL << (bit))) | (((bit_value)&1) << (bit)))
/*
 * Returns one plus the index of the least significant bit set of x, or if x is zero, returns zero. 
*/
#define FIND_FIRST_BIT_SET(x) (__builtin_ffsl(x))
#define SIGN_EXTEND(input, _bit)   \
	({                            \
		struct                    \
		{                         \
			ixlen x : (_bit + 1); \
		} s = {.x = input};       \
		s.x;                      \
	})

#endif /* HELPERS_H */
