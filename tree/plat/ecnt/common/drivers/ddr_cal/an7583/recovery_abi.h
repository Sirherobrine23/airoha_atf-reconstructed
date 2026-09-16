/*
 * Reverse-engineering support declarations for AN7581/AN7583 DRAMC recovery.
 *
 * These declarations model the ABI observed in the vendor ELF objects.  They
 * deliberately avoid claiming the original C type names/layouts where DWARF
 * information is not available.
 */
#ifndef AN75XX_DRAMC_RECOVERY_ABI_H
#define AN75XX_DRAMC_RECOVERY_ABI_H

#include <stdint.h>
#include <stddef.h>

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

static inline volatile U32 *mmio32(uintptr_t addr)
{
    return (volatile U32 *)addr;
}

static inline U32 raw_u32(const void *p, size_t off)
{
    return *(const U32 *)((const U8 *)p + off);
}

static inline U16 raw_u16(const void *p, size_t off)
{
    return *(const U16 *)((const U8 *)p + off);
}

static inline U8 raw_u8(const void *p, size_t off)
{
    return *((const U8 *)p + off);
}

static inline void raw_set_u32(void *p, size_t off, U32 value)
{
    *(U32 *)((U8 *)p + off) = value;
}

#endif
