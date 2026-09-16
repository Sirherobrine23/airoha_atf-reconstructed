/*
 * AN7581 dramc.c -- functional reconstruction from BL22 dramc.o.
 *
 * High confidence for MMIO constants, branch conditions, size probing and
 * tRFC selection.  C type names for external vendor APIs are ABI-inferred.
 * This is not yet claimed byte-identical to the GCC 10.3.0 oracle.
 */
#include "recovery_abi.h"


extern int printf(const char *fmt, ...);

#define SCU_BASE        0x1fa20000U
#define CHIP_BASE       0x1fb00000U
#define DRAM_BASE       0x80000000U
#define EMI_CLK_REG     (SCU_BASE + 0x1ecU)
#define RBUS_REG        (CHIP_BASE + 0x74U)
#define STRAP_REG       (CHIP_BASE + 0x8cU)
#define BOOT_REG        (CHIP_BASE + 0x9cU)
#define PACKAGE_REG     (SCU_BASE + 0x258U)
#define DDRPHY_MISC     0x00201650U
#define DDRPHY_MISC2    0x00201664U

extern U8 DramContext[];
extern void *gFreqTbl[];
extern void Init_Dram_Ctx_By_Type(void *ctx, U32 type);
extern void vSetDFSFreqSelByTable(void *ctx, void *table);
extern U32 vGet_Div_Mode(void *ctx);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void udelay(U32 usec);
extern U32 is_asic(void);
extern void Init_DRAM(U32 type, U32 a1, U32 a2, U32 a3);
extern void SET_R2C_MODE(U32 mode);

U64 dram_size;
U32 gpio_31;
U32 gpio_32;
U32 pkg_type;
U32 ddr_type = 1;

void DRAMC_Param_Init(void *ctx)
{
    pkg_type = (*mmio32(PACKAGE_REG) >> 14) & 0xf;
    gpio_31 = (*mmio32(STRAP_REG) >> 5) & 1;
    gpio_32 = (*mmio32(STRAP_REG) >> 6) & 1;

    if (pkg_type == 1) {
        ddr_type = 1;
        Init_Dram_Ctx_By_Type(ctx, 4);
    } else {
        ddr_type = gpio_31;
        Init_Dram_Ctx_By_Type(ctx, gpio_31 ? 4 : 3);
    }

    /* Oracle performs LDR [gFreqTbl] before this call. */
    vSetDFSFreqSelByTable(ctx, gFreqTbl[0]);

    if (pkg_type == 0)
        raw_set_u32(ctx, 0x44, 0x10);
    else if (pkg_type == 1)
        raw_set_u32(ctx, 0x44, gpio_31 ? 0x20 : 0x10);
}

void WRITE_RG_FLD(volatile U32 *reg, U32 width, U32 shift,
                  U32 unused, U32 value)
{
    U32 mask = 0xffffffffU >> (32U - width);
    U32 old;
    (void)unused;
    mask <<= shift;
    old = *reg;
    *reg = (old & ~mask) | (value << shift);
}

void TRFC_SET(U32 trfc_ns)
{
    U32 divisor = (vGet_Div_Mode(DramContext) == 2) ? 2000U : 4000U;
    U32 product = trfc_ns * raw_u16(DramContext, 0x54);
    U32 q = product / divisor;
    U32 rem = product % divisor;
    U32 half_step = 0;
    U32 trfc_reg;

    if (rem != 0)
        half_step = (rem < (divisor >> 1)) ? 1U : 0U;

    trfc_reg = (((rem << 1) / divisor) + (q - 11U)) << 17;
    trfc_reg &= 0x03fe0000U;
    vPhyByteIO32WriteMsk(DramContext, DDRPHY_MISC, trfc_reg, 0x03fe0000U);
    vPhyByteIO32WriteMsk(DramContext, DDRPHY_MISC2, half_step << 2, 4U);
}

void TRFC_CAL(U32 size_mb)
{
    if (pkg_type == 1) {
        if (gpio_31 == 1)
            size_mb >>= 1;
        switch (size_mb) {
        case 128:  TRFC_SET(110); break;
        case 256:  TRFC_SET(160); break;
        case 512:  TRFC_SET(260); break;
        case 1024: TRFC_SET(gpio_31 == 1 ? 550 : 350); break;
        case 2048: TRFC_SET(550); break;
        default: break;
        }
        return;
    }

    switch (size_mb) {
    case 128:  TRFC_SET(110); break;
    case 256:  TRFC_SET(160); break;
    case 512:  TRFC_SET(260); break;
    case 1024: TRFC_SET(350); break;
    default: break;
    }
}

U32 calculate_dram_size(void)
{
    const U32 a = 0x12345678U;
    const U32 b = 0x87654321U;
    volatile U32 *base = (volatile U32 *)(uintptr_t)(DRAM_BASE + 0x10U);
    U32 size = 32U << 20;
    unsigned n = 6;

    while (n--) {
        volatile U32 *probe = (volatile U32 *)(uintptr_t)(DRAM_BASE + size + 0x10U);
        U32 bv, av;

        *base = a;
        *probe = b;
        bv = *probe;
        av = *base;

        /* The vendor code rewrites the probe if its first read was unstable. */
        if (bv != b) {
            *probe = b;
            bv = *probe;
            av = *base;
        }

        if (av != a) {
            if (av == bv) {
                TRFC_CAL(size >> 20);
                return size;
            }
            printf("dram r/w error!\r\n");
            return 0;
        }
        size <<= 1;
    }

    TRFC_CAL(size >> 20);
    return size;
}

void EMI_CLK_Switch(U32 enable)
{
    U32 v;
    if (enable > 1)
        return;
    v = *mmio32(EMI_CLK_REG);
    if (enable)
        v |= 0x200U;
    else
        v &= ~0x200U;
    *mmio32(EMI_CLK_REG) = v;
}

void DDR4_BG_Num_Check(void)
{
    U32 v;

    v = *mmio32(EMI_CLK_REG);
    *mmio32(EMI_CLK_REG) = v & ~0x200U;
    v = *mmio32(RBUS_REG);
    *mmio32(RBUS_REG) = v | 0x800U;
    udelay(1);
    EMI_CLK_Switch(1);

    *mmio32(DRAM_BASE) = 0x12345678U;
    udelay(1000);
    *mmio32(DRAM_BASE + 0x40U) = 0x87654321U;
    udelay(1000);

    if (*mmio32(DRAM_BASE) == 0x12345678U) {
        printf("BG_num = 4\n");
        return;
    }

    v = *mmio32(EMI_CLK_REG);
    *mmio32(EMI_CLK_REG) = v & ~0x200U;
    v = *mmio32(RBUS_REG);
    *mmio32(RBUS_REG) = v & ~0x800U;
    udelay(1);
    EMI_CLK_Switch(1);
    printf("BG_num = 2\n");
}

U64 dramc_main(void)
{
    U32 v;

    printf("\r\nAN7581DRAMC V1.0\r\n");
    *mmio32(CHIP_BASE + 0x40) = 1;
    *mmio32(CHIP_BASE + 0x40) = 0;

    if (!is_asic()) {
#if defined(__arm__) || defined(__aarch64__)
        __asm__ volatile("dsb sy" ::: "memory");
#else
        __asm__ volatile("" ::: "memory");
#endif
        udelay(100000);
    }

    if (*mmio32(BOOT_REG) & 1U) {
        pkg_type = (*mmio32(PACKAGE_REG) >> 14) & 0xf;
        gpio_31 = (*mmio32(STRAP_REG) >> 5) & 1U;
        gpio_32 = (*mmio32(STRAP_REG) >> 6) & 1U;

        if (pkg_type == 1) {
            printf("pkg_type is BGA1\n");
            ddr_type = 1;
            Init_DRAM(4, 0, 0, 1);

            v = *mmio32(EMI_CLK_REG);
            *mmio32(EMI_CLK_REG) = v & ~0x200U;
            v = *mmio32(RBUS_REG);
            v &= ~0x3807U;
            if (gpio_31 == 1)
                v |= 0x2005U;
            else
                v |= 0x3005U;
            *mmio32(RBUS_REG) = v;
            SET_R2C_MODE(gpio_31 == 1 ? 3 : 1);
            udelay(1);
            *mmio32(EMI_CLK_REG) |= 0x200U;
        } else {
            printf("pkg_type is BGA2\n");
            ddr_type = gpio_31 ? 1U : 0U;
            if (gpio_31) {
                Init_DRAM(4, 0, 0, 1);
                v = *mmio32(EMI_CLK_REG);
                *mmio32(EMI_CLK_REG) = v & ~0x200U;
                v = *mmio32(RBUS_REG);
                v = (v & ~0x3807U) | 0x3005U;
                *mmio32(RBUS_REG) = v;
                SET_R2C_MODE(1);
                udelay(1);
                *mmio32(EMI_CLK_REG) |= 0x200U;
            } else {
                Init_DRAM(3, 0, 0, 1);
                *mmio32(EMI_CLK_REG) &= ~0x200U;
                v = *mmio32(RBUS_REG);
                v = (v & ~0x1807U) | 0x1805U;
                *mmio32(RBUS_REG) = v;
                SET_R2C_MODE(0);
                udelay(1);
                *mmio32(EMI_CLK_REG) |= 0x200U;
            }
        }
    }

    if (pkg_type == 1 && gpio_31 == 1)
        DDR4_BG_Num_Check();

    dram_size = calculate_dram_size();
    printf("DRAM FLOW DONE!!!\n");
    return dram_size;
}
