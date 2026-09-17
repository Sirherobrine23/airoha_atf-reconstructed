/*
 * AN7583 DRAM test helpers.
 *
 * Functional reconstruction from the vendor BL22 GCC 10.3 Thumb object.
 * The older EcoNet/Airoha bootrom dramtest.c was used only as a lineage aid;
 * argument ABI, little-endian packing, MMIO addresses and test sequencing here
 * follow the AN7583 object oracle.
 */
#include "recovery_abi.h"

#include <stddef.h>

extern int printf(const char *fmt, ...);
extern void udelay(U32 usec);
extern void *memset(void *dst, int c, size_t n);

extern int DDR_Scrambler_Key_Set(U32 key);
extern int DDR_Scrambler_Region_Set(U32 lower, U32 upper,
                                    U32 region, U32 enable);
extern U32 READ_RG_FLD(uintptr_t addr, U32 width, U32 shift);
extern void WRITE_RG_FLD(uintptr_t addr, U32 width, U32 shift,
                         U32 unused, U32 value);

#ifndef BIT
#define BIT(n) (1U << (n))
#endif

#define ORI_PAT       0U
#define INCR_PAT      1U
#define ANTI_INCR_PAT 2U

#define GDMA_BASE       ((uintptr_t)0x1fb30000U)
#define DRAMC_BIST_BASE ((uintptr_t)0x1fca8000U)
#define SCRAMBLE_BASE   ((uintptr_t)0x1fc80000U)
#define RX_DELAY_REG    ((uintptr_t)0x1fc8a9f8U)

static __attribute__((always_inline)) inline U32 *dram_addr32(U32 addr)
{
    return (U32 *)(uintptr_t)addr;
}

static __attribute__((always_inline)) inline U16 *dram_addr16(U32 addr)
{
    return (U16 *)(uintptr_t)addr;
}

static __attribute__((always_inline)) inline U8 *dram_addr8(U32 addr)
{
    return (U8 *)(uintptr_t)addr;
}

int dram_pat_set(U32 *start_addr_ref, U32 size, U32 pattern,
                 U32 pat_type, U8 w_byte)
{
    U32 *addr4 = NULL;
    U16 *addr2 = NULL;
    U8 *addr1 = NULL;
    U32 pat4 = pattern;
    U16 pat2 = (U16)pattern;
    U8 pat1 = (U8)pattern;

    if (start_addr_ref == NULL) {
        printf("dram_pat_set: input data ERROR!\n");
        return -1;
    }

    printf("dram_pat_set\n");
    printf("start addr:0x%x\n", *start_addr_ref);
    printf("test size:0x%lx\n", (unsigned long)size);
    printf("pattern:0x%x\n", pattern);

    switch (w_byte) {
    case 4:
        addr4 = dram_addr32(*start_addr_ref);
        size >>= 2;
        break;
    case 2:
        addr2 = dram_addr16(*start_addr_ref);
        size >>= 1;
        break;
    case 1:
        addr1 = dram_addr8(*start_addr_ref);
        break;
    default:
        return -1;
    }

    while (size != 0U) {
        switch (w_byte) {
        case 4:
            /* The vendor object intentionally truncates antiPat4 to U8. */
            *addr4++ = pat_type == ANTI_INCR_PAT ? (U8)~pat4 : pat4;
            if (pat_type == INCR_PAT || pat_type == ANTI_INCR_PAT)
                pat4++;
            break;
        case 2:
            /* Same historical truncation quirk is present for 16-bit mode. */
            *addr2++ = pat_type == ANTI_INCR_PAT ? (U8)~pat2 : pat2;
            if (pat_type == INCR_PAT || pat_type == ANTI_INCR_PAT)
                pat2++;
            break;
        case 1:
            *addr1++ = pat_type == ANTI_INCR_PAT ? (U8)~pat1 : pat1;
            if (pat_type == INCR_PAT || pat_type == ANTI_INCR_PAT)
                pat1++;
            break;
        default:
            return -1;
        }
        size--;
    }

    return 0;
}

int dram_pat_cmp(U32 *start_addr_ref, U32 size, U32 pattern,
                 U8 w_byte, U8 r_byte)
{
    U32 r_pat[4];
    U32 w_pat;
    U32 *addr4 = NULL;
    U16 *addr2 = NULL;
    U8 *addr1 = NULL;
    U32 p_num;
    U32 i = 0;

    memset(r_pat, 0, sizeof(r_pat));
    printf("pat_cmp\n");
    printf("pattern:0x%x\n", pattern);

    if (start_addr_ref == NULL) {
        printf("dram_pat_cmp: input data ERROR!\n");
        return -1;
    }

    switch (w_byte) {
    case 4:
        w_pat = pattern;
        break;
    case 2:
        w_pat = (U16)pattern;
        break;
    case 1:
        w_pat = (U8)pattern;
        break;
    default:
        return -1;
    }

    switch (r_byte) {
    case 4:
        addr4 = dram_addr32(*start_addr_ref);
        size >>= 2;
        break;
    case 2:
        addr2 = dram_addr16(*start_addr_ref);
        size >>= 1;
        break;
    case 1:
        addr1 = dram_addr8(*start_addr_ref);
        break;
    default:
        return -1;
    }

    if (w_byte > r_byte) {
        U32 n;

        p_num = w_byte / r_byte;
        for (n = 0; n < p_num; n++) {
            if (r_byte == 2U) {
                r_pat[n] = (U16)w_pat;
                w_pat >>= 16;
            } else if (r_byte == 1U) {
                r_pat[n] = (U8)w_pat;
                w_pat >>= 8;
            } else {
                return -1;
            }
        }
    } else if (w_byte < r_byte) {
        U32 n;

        p_num = 1;
        n = r_byte / w_byte;
        while (--n != 0U) {
            if (w_byte == 2U)
                w_pat |= w_pat << 16;
            else if (w_byte == 1U)
                w_pat |= w_pat << 8;
            else
                return -1;
        }
        r_pat[0] = w_pat;
    } else {
        p_num = w_byte / r_byte;
        r_pat[0] = w_pat;
    }

    while (size != 0U) {
        U32 expected = r_pat[i];
        U32 actual;

        if (r_byte == 4U) {
            actual = *addr4;
            if (actual != expected) {
                printf("error!\n");
                printf("pat:%x\n", expected);
                printf("data:%x\n", actual);
                return -1;
            }
            addr4++;
        } else if (r_byte == 2U) {
            expected = (U16)expected;
            actual = *addr2;
            if (actual != expected) {
                printf("error!\n");
                printf("pat:%x\n", expected);
                printf("data:%x\n", actual);
                return -1;
            }
            addr2++;
        } else if (r_byte == 1U) {
            expected = (U8)expected;
            actual = *addr1;
            if (actual != expected) {
                printf("error!\n");
                printf("pat:%x\n", expected);
                printf("data:%x\n", actual);
                return -1;
            }
            addr1++;
        } else {
            return -1;
        }

        i++;
        if (i >= p_num)
            i = 0;
        size--;
    }

    printf("pass\n\n");
    return 0;
}

int dram_incrPat_cmp(U32 *start_addr_ref, U32 size, U32 pattern,
                     U8 w_byte, U8 r_byte)
{
    U32 r_pat[4];
    U32 *addr4 = NULL;
    U16 *addr2 = NULL;
    U8 *addr1 = NULL;
    U32 pat4 = pattern;
    U16 pat2 = (U16)pattern;
    U8 pat1 = (U8)pattern;
    int pat_idx = -1;

    memset(r_pat, 0, sizeof(r_pat));
    printf("incrpat_cmp\n");

    if (start_addr_ref == NULL) {
        printf("dram_incrPat_cmp: input data ERROR!\n");
        return -1;
    }

    switch (w_byte) {
    case 4:
        break;
    case 2:
        pat2 = (U16)pattern;
        break;
    case 1:
        pat1 = (U8)pattern;
        break;
    default:
        return -1;
    }

    switch (r_byte) {
    case 4:
        addr4 = dram_addr32(*start_addr_ref);
        size >>= 2;
        break;
    case 2:
        addr2 = dram_addr16(*start_addr_ref);
        size >>= 1;
        break;
    case 1:
        addr1 = dram_addr8(*start_addr_ref);
        break;
    default:
        return -1;
    }

    while (size != 0U) {
        U32 p_num;

        if (w_byte > r_byte) {
            U32 tmp;
            U32 n;

            p_num = w_byte / r_byte;
            if (pat_idx >= (int)p_num - 1 || pat_idx == -1) {
                if (w_byte == 4U) {
                    tmp = pat4++;
                } else if (w_byte == 2U) {
                    tmp = pat2++;
                } else if (w_byte == 1U) {
                    tmp = pat1++;
                } else {
                    return -1;
                }

                for (n = 0; n < p_num; n++) {
                    if (r_byte == 2U) {
                        r_pat[n] = (U16)tmp;
                        tmp >>= 16;
                    } else if (r_byte == 1U) {
                        r_pat[n] = (U8)tmp;
                        tmp >>= 8;
                    } else {
                        return -1;
                    }
                }
                pat_idx = 0;
            } else {
                pat_idx++;
            }
        } else if (w_byte < r_byte) {
            U32 n;
            U32 accum = 0;

            pat_idx = 0;
            n = r_byte / (w_byte * 2U);
            r_pat[0] = 0;

            for (U32 j = 0; j < n; j++) {
                if (w_byte == 2U) {
                    U32 lo = pat2;
                    U32 hi = (U16)(pat2 + 1U);
                    pat2 = (U16)(pat2 + 2U);
                    accum = (accum << 16) | lo | (hi << 16);
                } else if (w_byte == 1U) {
                    U32 lo = pat1;
                    U32 hi = (U8)(pat1 + 1U);
                    U32 pair = lo | (hi << 8);
                    pat1 = (U8)(pat1 + 2U);
                    accum |= pair << (j * 16U);
                } else {
                    return -1;
                }
            }
            r_pat[0] = accum;
        } else {
            pat_idx = 0;
            if (w_byte == 4U)
                r_pat[0] = pat4++;
            else if (w_byte == 2U)
                r_pat[0] = pat2++;
            else if (w_byte == 1U)
                r_pat[0] = pat1++;
            else
                return -1;
        }

        {
            U32 expected = r_pat[pat_idx];
            U32 actual;

            if (r_byte == 4U) {
                actual = *addr4;
                if (actual != expected) {
                    printf("error!\n");
                    printf("pat: %x\n", expected);
                    printf("data: %x\n", actual);
                    return -1;
                }
                addr4++;
            } else if (r_byte == 2U) {
                expected = (U16)expected;
                actual = *addr2;
                if (actual != expected) {
                    printf("error!\n");
                    printf("pat: %x\n", expected);
                    printf("data: %x\n", actual);
                    return -1;
                }
                addr2++;
            } else if (r_byte == 1U) {
                expected = (U8)expected;
                actual = *addr1;
                if (actual != expected) {
                    printf("error!\n");
                    printf("pat: %x\n", expected);
                    printf("data: %x\n", actual);
                    return -1;
                }
                addr1++;
            } else {
                return -1;
            }
        }

        size--;
    }

    printf("pass\n\n");
    return 0;
}

int dram_antiIncrPat_cmp(U32 *start_addr_ref, U32 size, U32 pattern,
                         U8 w_byte)
{
    U32 *addr4 = NULL;
    U16 *addr2 = NULL;
    U8 *addr1 = NULL;
    U32 pat4 = pattern;
    U16 pat2 = (U16)pattern;
    U8 pat1 = (U8)pattern;

    printf("antiIncrpat_cmp\n");

    if (start_addr_ref == NULL) {
        printf("dram_antiIncrPat_cmp: input data ERROR!\n");
        return -1;
    }

    switch (w_byte) {
    case 4:
        addr4 = dram_addr32(*start_addr_ref);
        size >>= 2;
        break;
    case 2:
        addr2 = dram_addr16(*start_addr_ref);
        size >>= 1;
        break;
    case 1:
        addr1 = dram_addr8(*start_addr_ref);
        break;
    default:
        return -1;
    }

    while (size != 0U) {
        U32 expected;
        U32 actual;

        if (w_byte == 4U) {
            expected = (U8)~pat4;
            actual = *addr4;
            if (actual != expected) {
                printf("error!\n");
                printf("pat: %x\n", expected);
                printf("data: %x\n", actual);
                return -1;
            }
            addr4++;
            pat4++;
        } else if (w_byte == 2U) {
            expected = (U8)~pat2;
            actual = *addr2;
            if (actual != expected) {
                printf("error!\n");
                printf("pat: %x\n", expected);
                printf("data: %x\n", actual);
                return -1;
            }
            addr2++;
            pat2++;
        } else if (w_byte == 1U) {
            expected = (U8)~pat1;
            actual = *addr1;
            if (actual != expected) {
                printf("error!\n");
                printf("pat: %x\n", expected);
                printf("data: %x\n", actual);
                return -1;
            }
            addr1++;
            pat1++;
        } else {
            return -1;
        }
        size--;
    }

    printf("pass\n\n");
    return 0;
}

void set_gdma_config(U32 channel, U32 src, U32 dst, U32 cfg, U32 len)
{
    uintptr_t off = (uintptr_t)channel << 4;

    *mmio32(GDMA_BASE + off + 0x00U) = src;
    *mmio32(GDMA_BASE + off + 0x04U) = dst;
    *mmio32(GDMA_BASE + off + 0x0cU) = len;

    if ((cfg & BIT(1)) != 0U)
        __asm__ volatile("dsb sy" ::: "memory");

    *mmio32(GDMA_BASE + off + 0x08U) = cfg;
}

void trigger_gdma(U32 src, U32 dst, U32 transfer_count,
                  U32 channel, U32 burst_size)
{
    U32 cfg = (burst_size << 3) | (transfer_count << 16) | 3U;
    set_gdma_config(channel, src, dst, cfg, 0U);
}

void wait_gdma_done(U32 channel)
{
    U32 bit = 1U << channel;

    while ((*mmio32(GDMA_BASE + 0x204U) & bit) == 0U)
        ;
    *mmio32(GDMA_BASE + 0x204U) = bit;
}

int dramTest(U32 size)
{
    U8 def_w_byte[4] = { 1, 2, 4, 0 };
    U8 def_r_byte[4] = { 1, 2, 4, 0 };
    U8 def_pat[4] = { 0x55, 0xaa, 0x00, 0xff };
    U32 start_addr[3] = { 0x80000000U, 0U, 0U };
    int ret = 0;

    for (U32 region = 0; start_addr[region] != 0U; region++) {
        U32 physical = start_addr[region] & ~0xe0000000U;
        U32 region_size = size - physical;

        printf("startAddr = %lx, size = %lx\n",
               (unsigned long)start_addr[region],
               (unsigned long)region_size);

        for (U32 wi = 0; def_w_byte[wi] != 0U; wi++) {
            U8 w_byte = def_w_byte[wi];

            printf("incremental pattern, WByte: %d\n", w_byte);
            dram_pat_set(&start_addr[region], region_size, 0U,
                         INCR_PAT, w_byte);

            for (U32 ri = 0; def_r_byte[ri] != 0U; ri++)
                ret += dram_incrPat_cmp(&start_addr[region], region_size,
                                        0U, w_byte, def_r_byte[ri]);
        }

        printf("anti-incremental patten, WByte:1\n");
        dram_pat_set(&start_addr[region], region_size, 0U,
                     ANTI_INCR_PAT, 1U);
        ret += dram_antiIncrPat_cmp(&start_addr[region], region_size,
                                    0U, 1U);

        for (U32 i = 0; i < 4U; i++) {
            dram_pat_set(&start_addr[region], region_size, def_pat[i],
                         ORI_PAT, 1U);
            ret += dram_pat_cmp(&start_addr[region], region_size,
                                def_pat[i], 1U, 1U);
        }
    }

    if (ret != 0) {
        printf("dramTest FAIL!\n\n");
        for (;;)
            ;
    }

    printf("dramTest SUCCESS!\n\n");
    return ret;
}

void Check_DRAMC_BIST_Done(void)
{
    U32 timeout = 0x000fffffU;

    while (((*mmio32(DRAMC_BIST_BASE + 0x0cU) & BIT(0)) == 0U) &&
           timeout != 0U) {
        udelay(1U);
        timeout--;
    }

    if (timeout == 0U)
        printf("Wait BIST done timeout\n");
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

int DDR_Scrambler_Region_Type(U32 region, U32 type)
{
    U32 shift;
    U32 mask;
    U32 v;

    if (region > 3U)
        return -1;

    shift = region * 4U + 2U;
    mask = 1U << shift;
    v = *mmio32(SCRAMBLE_BASE + 0x440U);
    v &= ~mask;
    v |= (type & 1U) << shift;
    *mmio32(SCRAMBLE_BASE + 0x440U) = v;
    return 0;
}

int DDR_Scrambler_Region_EN(U32 region, U32 enable)
{
    U32 shift;
    U32 mask;
    U32 v;

    if (region > 3U)
        return -1;

    shift = region * 4U;
    mask = 1U << shift;
    v = *mmio32(SCRAMBLE_BASE + 0x440U);
    v &= ~mask;
    v |= (enable & 1U) << shift;
    v |= BIT(24);
    *mmio32(SCRAMBLE_BASE + 0x440U) = v;
    return 0;
}

void DDR_Scrmabler_test(void)
{
    U32 start_addr[4] = {
        0x80100000U,
        0x8c000000U,
        0x98000000U,
        0x9fe00000U,
    };
    const U32 test_size = 0x00140000U;

    printf("Scrmable test start\n");
    DDR_Scrambler_Key_Set(0xf0f0f0ffU);

    printf("--------- Set upper_addr & lower_addr of region\n");
    DDR_Scrambler_Region_Set(0x0010U, 0x001fU, 0U, 0U);
    DDR_Scrambler_Region_Set(0x0c00U, 0x0c0fU, 1U, 0U);
    DDR_Scrambler_Region_Set(0x1800U, 0x180fU, 2U, 0U);
    DDR_Scrambler_Region_Set(0x1fe0U, 0x1fefU, 3U, 0U);

    printf("/***********************************************************/\n");
    printf("---------- Set region 0 outside mode\n");
    DDR_Scrambler_Region_Type(0U, 1U);
    DDR_Scrambler_Region_EN(0U, 1U);

    printf("---------- Test region 0 outside mode\n");
    dram_pat_set(&start_addr[0], test_size, 0U, INCR_PAT, 4U);
    dram_incrPat_cmp(&start_addr[0], test_size, 0U, 4U, 4U);
    DDR_Scrambler_Region_EN(0U, 0U);
    dram_incrPat_cmp(&start_addr[0], test_size, 0U, 4U, 4U);

    printf("/***********************************************************/\n");
    printf("\n");
    printf("/***********************************************************/\n");
    printf("---------- Test region 0~3 inside mode\n");

    for (U32 region = 0; region < 4U; region++) {
        printf("--------- Set region - %d\n", region);
        DDR_Scrambler_Region_Type(region, 0U);
        DDR_Scrambler_Region_EN(region, 1U);
        dram_pat_set(&start_addr[region], test_size, 0U, INCR_PAT, 4U);
        dram_incrPat_cmp(&start_addr[region], test_size, 0U, 4U, 4U);
        DDR_Scrambler_Region_EN(region, 0U);
        dram_incrPat_cmp(&start_addr[region], test_size, 0U, 4U, 4U);
    }

    printf("/***********************************************************/\n");
    printf("\n");
    printf("/***********************************************************/\n");
    printf("---------- Test all region inside mode\n");

    for (U32 region = 0; region < 4U; region++) {
        dram_pat_set(&start_addr[region], test_size, 0U, INCR_PAT, 4U);
        dram_incrPat_cmp(&start_addr[region], test_size, 0U, 4U, 4U);
    }

    for (U32 region = 0; region < 4U; region++)
        DDR_Scrambler_Region_EN(region, 1U);

    for (U32 region = 0; region < 4U; region++)
        dram_incrPat_cmp(&start_addr[region], test_size, 0U, 4U, 4U);

    printf("Scrmable test end!!!\n");
    printf("/***********************************************************/\n");
}

int DRAMC_BIST_set(U32 bist_set, U32 test_num, U32 test_loop_num,
                   U32 rand_delay, U8 cto_len_setting)
{
    volatile U32 *b = mmio32(DRAMC_BIST_BASE);
    U32 common;

    if ((bist_set - 8U) <= 1U) {
        printf("Sequence mode \n");
        b[1] = 0x000f3a51U;
    } else {
        printf("Linear mode \n");
        b[1] = 0U;
    }

    b[2] = 0x0000103fU;
    printf("BIST_set = %d, test_num = %d, test_loop_num = %d, rand_delay = %d, cto_len_setting = %d\n",
           bist_set, test_num, test_loop_num, rand_delay, cto_len_setting);

    common = ((U32)cto_len_setting << 16) |
             (bist_set << 2) |
             (rand_delay << 12) |
             (test_loop_num << 9) |
             (test_num << 6) |
             0x00008000U;

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

void DRAMC_BIST_test(void)
{
    U32 modes[6] = { 9U, 8U, 3U, 2U, 1U, 0U };
    U32 rx_delay_bit0;
    U32 rx_delay_bit8;

    printf("DRAMC_BIST start\n");

    rx_delay_bit0 = READ_RG_FLD(RX_DELAY_REG, 8U, 0U);
    rx_delay_bit8 = READ_RG_FLD(RX_DELAY_REG, 8U, 8U);

    for (U32 i = 0; i < 6U; i++) {
        U32 mode = modes[i];

        printf("Bist mode: %d\n", mode);
        DRAMC_BIST_set(mode, 4U, 3U, 0U, 0U);
        printf("/*****************************/\n");

        WRITE_RG_FLD(RX_DELAY_REG, 8U, 0U, 0U, 0U);
        WRITE_RG_FLD(RX_DELAY_REG, 8U, 8U, 0U, 0U);
        DRAMC_BIST_set(mode, 4U, 3U, 0U, 0U);
        WRITE_RG_FLD(RX_DELAY_REG, 8U, 0U, 0U, rx_delay_bit0);
        WRITE_RG_FLD(RX_DELAY_REG, 8U, 8U, 0U, rx_delay_bit8);
        printf("/*****************************/\n");
    }

    printf("Bist mode: 0 and rand delay\n");
    DRAMC_BIST_set(0U, 4U, 3U, 0U, 0U);
    printf("/*****************************/\n");
    WRITE_RG_FLD(RX_DELAY_REG, 8U, 0U, 0U, 0U);
    WRITE_RG_FLD(RX_DELAY_REG, 8U, 8U, 0U, 0U);
    DRAMC_BIST_set(0U, 4U, 3U, 0U, 0U);
    WRITE_RG_FLD(RX_DELAY_REG, 8U, 0U, 0U, rx_delay_bit0);
    WRITE_RG_FLD(RX_DELAY_REG, 8U, 8U, 0U, rx_delay_bit8);
    printf("\n");
    printf("DRAMC_BIST end!!!\n");
}
