#ifndef RISCV_HELPER_H
#define RISCV_HELPER_H

#include <stdlib.h>
#include <stdarg.h>

#include <types.h>
#include <helpers.h>

#define XLEN_INT_MIN 0x8000000000000000

#define SHAMT_MASK 0x3F

#define SIGNEXTEND(input, _bit)                        \
	({                                                 \
		struct                                         \
		{                                              \
			ixlen x : (_bit + 1);                      \
		} s = {.x = input};                            \
		s.x;                                    \
	})

#endif /* RISCV_HELPER_H */
