/*
 * AN7583 dramtest.c -- recovered low-level test/BIST helpers.
 *
 * Functional reconstruction from the vendor BL22 Thumb object.  The pattern
 * walkers and the full scrambler/BIST orchestration remain to be consolidated.
 */
#include "recovery_abi.h"

extern int printf(const char *fmt, ...);
extern void udelay(U32 usec);

#ifndef BIT
#define BIT(n) (1U << (n))
#endif

#define GDMA_BASE       ((uintptr_t)0x1fb30000U)
#define DRAMC_BIST_BASE ((uintptr_t)0x1fca8000U)
#define SCRAMBLE_BASE   ((uintptr_t)0x1fc80000U)

void set_gdma_config(U32 channel, U32 src, U32 dst, U32 cfg, U32 len)
{
    uintptr_t off = (uintptr_t)channel << 4;

    *mmio32(GDMA_BASE + off + 0x00) = src;
    *mmio32(GDMA_BASE + off + 0x04) = dst;
    *mmio32(GDMA_BASE + off + 0x0c) = len;

    if ((cfg & BIT(31)) != 0U)
        __asm__ volatile("dsb sy" ::: "memory");

    *mmio32(GDMA_BASE + off + 0x08) = cfg;
}

void trigger_gdma(U32 a0, U32 a1, U32 a2, U32 a3, U32 a4)
{
    /* Preserve the vendor ABI literally until the original parameter names
     * are recovered: set_gdma_config(a3, a0, a1, (a4 << 3) |
     * (a2 << 16) | 3, 0). */
    set_gdma_config(a3, a0, a1, (a4 << 3) | (a2 << 16) | 3U, 0U);
}

void wait_gdma_done(U32 channel)
{
    U32 bit = 1U << channel;

    while ((*mmio32(GDMA_BASE + 0x204) & bit) == 0U)
        ;
    *mmio32(GDMA_BASE + 0x204) = bit;
}

void Check_DRAMC_BIST_Done(void)
{
    U32 timeout = 0x000fffffU;

    while (((*mmio32(DRAMC_BIST_BASE + 0x0c) & BIT(0)) == 0U) && timeout) {
        udelay(1);
        timeout--;
    }

    if (timeout == 0U)
        printf("Wait BIST done timeout\n");
}

int DDR_Scrambler_Region_EN(U32 region, U32 enable)
{
    U32 shift, mask, v;

    if (region > 3U)
        return -1;

    shift = region * 4U;
    mask = 1U << shift;
    v = *mmio32(SCRAMBLE_BASE + 0x440);
    v = (v & ~mask) | ((enable & 1U) << shift);
    v |= BIT(24);
    *mmio32(SCRAMBLE_BASE + 0x440) = v;
    return 0;
}

int DDR_Scrambler_Region_Type(U32 region, U32 type)
{
    U32 shift, mask, v;

    if (region > 3U)
        return -1;

    shift = region * 4U + 2U;
    mask = 1U << shift;
    v = *mmio32(SCRAMBLE_BASE + 0x440);
    v = (v & ~mask) | ((type & 1U) << shift);
    *mmio32(SCRAMBLE_BASE + 0x440) = v;
    return 0;
}

int DRAMC_BIST(void)
{
    volatile U32 *b = mmio32(DRAMC_BIST_BASE);

    printf("DRAMC_BIST start\n");
    b[1] = 0x000f3a00U;
    b[2] = 0x0000103fU;
    b[0] = 0x00008327U;
    Check_DRAMC_BIST_Done();

    if (b[3] != 1U) {
        printf("DRAMC_BIST Fail\n");
        return -1;
    }

    printf("DRAMC_BIST Pass\n");
    b[0] = 0x00008324U;
    return 0;
}

int DRAMC_BIST_set(U32 bist_set, U32 test_num, U32 test_loop_num,
                   U32 rand_delay, U32 cto_len_setting)
{
    volatile U32 *b = mmio32(DRAMC_BIST_BASE);
    U32 common;

    if ((bist_set - 8U) <= 1U) {
        printf("Bist mode: %d\n", bist_set);
        b[1] = 0x000f3a51U;
    } else {
        printf("set bit0 rx delay = 0\n");
        b[1] = 0;
    }
    b[2] = 0x0000103fU;

    printf("BIST_set = %d, test_num = %d, test_loop_num = %d, rand_delay = %d, cto_len_setting = %d\n",
           bist_set, test_num, test_loop_num, rand_delay, cto_len_setting);

    common = (cto_len_setting << 16) |
             (bist_set << 2) |
             (rand_delay << 12) |
             (test_loop_num << 9) |
             (test_num << 6) |
             0x8000U;
    b[0] = common | 2U;
    b[0] = common | 3U;
    Check_DRAMC_BIST_Done();

    if (b[3] != 1U) {
        printf("DRAMC_BIST Fail\n");
        return -1;
    }

    printf("DRAMC_BIST Pass\n");
    return 0;
}

/* Still to consolidate from this object:
 *   dram_pat_set(), dram_pat_cmp(), dram_incrPat_cmp(),
 *   dram_antiIncrPat_cmp(), dramTest(), DDR_Scrmabler_test(),
 *   DRAMC_BIST_test().
 */
