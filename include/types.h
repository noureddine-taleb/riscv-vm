#ifndef RISCV_TYPES_H
#define RISCV_TYPES_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define __PACKED __attribute__((__packed__))
#define __maybe_unused __attribute__((unused))
#define __aligned(align) __attribute__((aligned(align)))
#define BUS_ACCESS_STR(access) (access == bus_read_access ? "read" : access == bus_write_access ? "write" \
																								: "execute")

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef __uint128_t u128;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef __int128_t i128;

#define XLEN 64
typedef u64 uxlen;
typedef i64 ixlen;

typedef enum
{
	priv_level_unknown = -1, /* This just ensures that the enum is
							  * signed, which might be needed in
							  * down counting for loops */
	user_mode = 0,
	supervisor_mode = 1,
	reserved_mode = 2, /* Hypervisor ?? */
	machine_mode = 3,
	priv_level_max = 4
} privilege_level;

typedef enum
{
	bus_read_access = 0,
	bus_write_access,
	bus_instr_access,

	bus_access_type_max
} bus_access_type;

struct hart;
struct csr_mapping;
struct soc;
typedef int (*bus_access_func)(struct hart *priv,
							   privilege_level priv_level,
							   bus_access_type access_type, uxlen addr,
							   void *value, u8 len);
typedef int (*check_protection)(struct hart *priv,
								privilege_level priv_level,
								bus_access_type access_type, uxlen *addr,
								void *value, u8 len);

#endif /* RISCV_TYPES_H */
