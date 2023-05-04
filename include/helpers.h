#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdarg.h>

#include <types.h>

#ifdef DEBUG
#define DEBUG_PRINT(...)     \
	do                       \
	{                        \
		printf(__VA_ARGS__); \
	} while (0)
#else
#define DEBUG_PRINT(...) \
	do                   \
	{                    \
	} while (0)
#endif

#define die(fmt, ...)                                          \
	{                                                          \
		fprintf(stderr, "--------> " fmt "\n", ##__VA_ARGS__); \
		exit(1);                                               \
	}

#define ADDR_WITHIN(_addr, _start, _size) ((_addr >= _start) && (_addr < (_start + _size)))
#define ADDR_WITHIN_LEN(_addr, _len, _start, _size) ((_addr >= _start) && ((_addr + _len) <= (_start + _size)))

#define ADDR_ALIGN_DOWN(n, m) ((n) / (m) * (m))

#define ASSIGN_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define ASSIGN_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

/*
 * Bit Operators
 */
#define SET_BIT(_out_var, _nbit) ((_out_var) |= (1 << (_nbit)))
#define CLEAR_BIT(_out_var, _nbit) ((_out_var) &= ~(1 << (_nbit)))
#define FLIP_BIT(_out_var, _nbit) ((_out_var) ^= (1 << (_nbit)))
#define GET_BIT(_out_var, _nbit) (!!((_out_var) & (1 << (_nbit))))
#define GET_RANGE(_out_var, _start, _len) (((_out_var) >> _start) & ((1 << _len) - 1))
#define UPDATE_BIT(val, bit, bit_value) ((val) = ((val) & ~(1UL << (bit))) | (((bit_value) & 1) << (bit)))
#define FIND_FIRST_BIT_SET(_var) (__builtin_ffsl(_var))

#endif /* HELPERS_H */
