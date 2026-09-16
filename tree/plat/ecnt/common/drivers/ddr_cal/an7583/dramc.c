/*
 * AN7583 dramc.c -- functional reconstruction from BL22 dramc.o.
 *
 * The control flow, MMIO addresses, constants, strings and external-call
 * ordering below are recovered directly from the vendor ELF oracle.
 * External vendor ABI names/types without DWARF remain intentionally generic.
 * This is not yet claimed byte-identical to the Buildroot GCC 10.3.0 object.
 */
#include "recovery_abi.h"

#include <limits.h>
#include <stdint.h>

extern int printf(const char *fmt, ...);

#define CHIP_BASE        0x1fb00000U
#define SCU_BASE         0x1fa20000U
#define EMI_BASE         0x1fc80000U
#define DRAM_BASE        0x80000000U
#define STRAP_REG        (CHIP_BASE + 0x8cU)
#define BOOT_REG         (CHIP_BASE + 0x9cU)
#define RBUS_REG         (CHIP_BASE + 0x74U)
#define RBUS_MISC_REG    (CHIP_BASE + 0x974U)
#define EMI_CLK_REG      (SCU_BASE + 0x1ecU)
#define EMI_CONA_REG     (EMI_BASE + 0x0cU)
#define DDRPHY_TRFC_REG  0x00201650U
#define DDRPHY_TRFC2_REG 0x00201664U

extern U8 DramContext[];
extern void *gFreqTbl[];
extern U32 uartDisable;

extern void Dramc_efuse_read_parse(U32 start, U32 len, U8 *dst);
extern void Init_Dram_Ctx_By_Type(void *ctx, U32 type);
extern void vSetDFSFreqSelByTable(void *ctx, void *table);
extern U32 vGet_Div_Mode(void *ctx);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern U32 is_asic(void);
extern void udelay(U32 usec);
extern void Init_DRAM(U32 type, U32 arg);

/* Vendor register-field helpers. The six-argument write ABI is visible in BL22. */
extern U32 READ_RG_FLD(U32 reg, U32 arg1, U32 arg2);
extern void WRITE_RG_FLD(U32 reg, U32 arg1, U32 arg2,
                         U32 arg3, U32 arg4, U32 arg5);

U64 dram_size;
U8 efuse_data_width;
U32 gpio_data_rate;
U32 hw_conf;
U8 is_ddr_x8;
U32 ddr_type = 1;
const char *soc_id_dramc_version = "AN7583DRAMC V0.9";

static int ddr_reg_accessible(U32 addr)
{
    /* Three address windows recovered from the compiler's range checks. */
    if ((U32)(addr - 0x1fc80000U) < 0x1d0U)
        return 1;
    if ((U32)(addr - 0x1fc801edU) <= 0x252U)
        return 1;
    if ((U32)(addr - 0x1fc8045dU) <= 0x2bbd6U)
        return 1;
    return 0;
}

void _Register_Read_DDR(U32 addr)
{
    if (ddr_reg_accessible(addr)) {
        printf("val: 0x%x\n", *mmio32(addr));
        return;
    }

    if (addr == 0)
        printf("Invalid register address\n");
    else
        printf("RG 0x%x is inaccessible\n", addr);
}

void _Register_Write_DDR(U32 addr, U32 value)
{
    if (ddr_reg_accessible(addr)) {
        *mmio32(addr) = value;
        return;
    }

    if (addr == 0)
        printf("Invalid register address\n");
    else
        printf("RG 0x%x is inaccessible\n", addr);
}

void DRAMC_Param_Init(void *ctx)
{
    U32 strap = *mmio32(STRAP_REG);

    ddr_type = (strap >> 5) & 1U;
    gpio_data_rate = (strap >> 6) & 1U;

    Dramc_efuse_read_parse(1002U, 4U, &efuse_data_width);
    Init_Dram_Ctx_By_Type(ctx, ddr_type == 1U ? 4U : 3U);
    vSetDFSFreqSelByTable(ctx, gFreqTbl[0]);
    raw_set_u32(ctx, 0x44, 0x10U);
}

void get_config(void)
{
    U32 hw_mode = (*mmio32(STRAP_REG) >> 5) & 3U;

    DRAMC_Param_Init(DramContext);
    printf("%s\n", soc_id_dramc_version);

    switch (hw_mode) {
    case 1:
        printf("dram_type = PCDDR4, data_rate = 2666\n");
        break;
    case 2:
        printf("dram_type = PCDDR3, data_rate = 2133\n");
        break;
    case 3:
        printf("dram_type = PCDDR4, data_rate = 3200\n");
        break;
    default:
        printf("dram_type = PCDDR3, data_rate = 1866\n");
        break;
    }

    if (is_ddr_x8 == 1)
        printf("x16 by ddrx8 *2\n");
    else
        printf("x16 by ddrx16 *1\n");

    printf("DIV mode = %d\n", vGet_Div_Mode(DramContext));
}

void TRFC_SET(U32 trfc_ns)
{
    U32 divisor = (vGet_Div_Mode(DramContext) == 2U) ? 2000U : 4000U;
    U32 product = trfc_ns * raw_u16(DramContext, 0x54);
    U32 q = product / divisor;
    U32 rem = product % divisor;
    U32 half = (rem != 0U && rem < (divisor >> 1)) ? 1U : 0U;
    U32 value = ((((rem << 1) / divisor) + (q - 11U)) << 17) & 0x03fe0000U;

    vPhyByteIO32WriteMsk(DramContext, DDRPHY_TRFC_REG,
                          value, 0x03fe0000U);
    vPhyByteIO32WriteMsk(DramContext, DDRPHY_TRFC2_REG,
                          half << 2, 4U);
}

void TRFC_CAL(U32 size_mb)
{
    if (is_ddr_x8 == 1)
        size_mb >>= 1;

    switch (size_mb) {
    case 64:   TRFC_SET(90U);  break;
    case 128:  TRFC_SET(110U); break;
    case 256:  TRFC_SET(160U); break;
    case 512:  TRFC_SET(260U); break;
    case 1024: TRFC_SET(350U); break;
    case 2048: TRFC_SET(550U); break;
    default: break;
    }
}

U32 get_2s_power(U32 value)
{
    U32 power = 0;

    while ((value >>= 1) != 0)
        ++power;
    return power;
}

U32 calculate_dram_size(void)
{
    const U32 pattern_a = 0x12345678U;
    const U32 pattern_b = 0x87654321U;
    volatile U32 *base = (volatile U32 *)(uintptr_t)(DRAM_BASE + 0x10U);
    U32 size = 32U << 20;
    U32 loops = 6;

    while (loops--) {
        volatile U32 *probe =
            (volatile U32 *)(uintptr_t)(DRAM_BASE + size + 0x10U);
        U32 pv, bv;

        *base = pattern_a;
        *probe = pattern_b;
        pv = *probe;
        bv = *base;

        if (pv != pattern_b) {
            *probe = pattern_b;
            pv = *probe;
            bv = *base;
        }

        if (bv != pattern_a) {
            if (bv == pv)
                break;
            return 0;
        }

        size <<= 1;
    }

    if (is_asic() == 1)
        TRFC_CAL(size >> 20);

    {
        U32 v = *mmio32(EMI_CONA_REG);
        U32 enc = get_2s_power(size) - 24U;
        v &= ~0xf0000000U;
        v |= enc << 28;
        *mmio32(EMI_CONA_REG) = v;
    }

    return size;
}

int DDR_Scrambler_Key_Set(U32 key)
{
    U32 i;

    /* This exactly matches the two rejected values in the Thumb prologue. */
    if (key == 0U || key == UINT32_MAX)
        return -1;

    for (i = 0; i < 8; ++i) {
        U32 shift = i * 4U;
        U32 rotated = shift == 0U ? key : (key << shift) | (key >> (32U - shift));
        *mmio32(EMI_BASE + 0x1d0U + i * 4U) = rotated;
    }

    return 0;
}

int DDR_Scrambler_Region_Set(U32 lower, U32 upper,
                             U32 region, U32 enable)
{
    U32 dram_code = (*mmio32(EMI_CONA_REG) >> 28) & 0xfU;
    int32_t dram32 = (int32_t)(1U << (dram_code + 24U));
    U64 detected = (U64)(int64_t)dram32;
    U64 max_region_addr = (detected >> 16) - 1U;
    U32 packed;
    U32 ctrl;
    U32 bit;

    if (uartDisable == 0)
        printf("dram_size = 0x%llx\n", (unsigned long long)detected);

    if (lower >= upper) {
        if (uartDisable == 0)
            printf("lower address can't larger than upper address\n");
        return -1;
    }

    if (region > 3U) {
        if (uartDisable == 0)
            printf("region number can't larger than 3\n");
        return -1;
    }

    if ((U64)upper >= max_region_addr) {
        if (uartDisable == 0)
            printf("upper address is over max dram size;\n");
        return -1;
    }

    packed = lower | (upper << 16);
    *mmio32(EMI_BASE + 0x450U - region * 4U) = packed;

    bit = 1U << (region * 4U);
    ctrl = *mmio32(EMI_BASE + 0x440U);
    ctrl &= ~bit;
    if (enable & 1U)
        ctrl |= bit;
    ctrl |= 0x01000000U;
    *mmio32(EMI_BASE + 0x440U) = ctrl;

    return 0;
}

void EMI_CLK_Switch(U32 enable)
{
    U32 v;

    if (enable > 1U)
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

    v = *mmio32(RBUS_REG);
    v &= ~0x800U;
    v |= 0x1000U;
    *mmio32(RBUS_REG) = v;
    udelay(1U);

    *mmio32(DRAM_BASE) = 0x12345678U;
    udelay(1000U);
    *mmio32(DRAM_BASE + 0x2000U) = 0x87654321U;
    udelay(1000U);

    if (*mmio32(DRAM_BASE) == 0x12345678U) {
        if (uartDisable == 0)
            printf("BG_num = 4\n");
        return;
    }

    v = *mmio32(RBUS_REG);
    v &= ~0x1000U;
    v |= 0x800U;
    *mmio32(RBUS_REG) = v;
    udelay(1U);

    if (uartDisable == 0)
        printf("BG_num = 2\n");
}

void RBUS_Setting(void)
{
    U32 saved = *mmio32(RBUS_REG) & 0x7ffd43f8U;
    U32 top;

    if (uartDisable == 0)
        printf("RBUS out of order setting.\n");

    if (ddr_type == 0U) {
        *mmio32(RBUS_REG) = saved | 0x80000405U;
        *mmio32(RBUS_MISC_REG) &= ~1U;
        return;
    }

    *mmio32(RBUS_REG) = saved | 0x80000805U;
    *mmio32(RBUS_MISC_REG) &= ~1U;

    top = (*mmio32(EMI_CONA_REG) >> 28) & 0xfU;
    if (top == 0U) {
        DDR4_BG_Num_Check();
        return;
    }

    if (top != 8U)
        return;

    {
        U32 v = *mmio32(RBUS_REG);
        v &= ~0x800U;
        v |= 0x1000U;
        *mmio32(RBUS_REG) = v;
    }

    if (uartDisable == 0)
        printf("BG num = 4\n");
}

U64 dramc_main(void)
{
    U32 boot;

    if (uartDisable == 0)
        printf("\r\n%s\r\n", soc_id_dramc_version);

    hw_conf = *mmio32(STRAP_REG);
    boot = *mmio32(BOOT_REG);

    if ((boot & 1U) == 0U) {
        *mmio32(CHIP_BASE + 0x40U) = 1U;
        *mmio32(CHIP_BASE + 0x40U) = 0U;

        if (is_asic() == 0U) {
#if defined(__arm__) || defined(__aarch64__)
            __asm__ volatile("dsb sy" ::: "memory");
#else
            __asm__ volatile("" ::: "memory");
#endif
            udelay(100000U);
            if (uartDisable == 0)
                printf("FPGA delay\n");
        }

        dram_size = calculate_dram_size();
        if (uartDisable == 0)
            printf("DRAM FLOW DONE!!!\n");
        return dram_size;
    }

    ddr_type = (hw_conf >> 5) & 1U;
    gpio_data_rate = (hw_conf >> 6) & 1U;
    Dramc_efuse_read_parse(1002U, 4U, &efuse_data_width);

    if (READ_RG_FLD(CHIP_BASE + 0x240U, 2U, 0U) == 3U &&
        ((*mmio32(CHIP_BASE + 0x240U) & 0x8U) == 0U)) {
        U32 size_code;

        if (uartDisable == 0)
            printf("SW reboot DRAM init.\r\n");

        WRITE_RG_FLD(CHIP_BASE + 0x240U, 2U, 0U, 0U, 0U, 0U);
        size_code = (*mmio32(EMI_CONA_REG) >> 28) & 0xfU;

        if (size_code >= 1U && size_code <= 8U) {
            if (size_code == 8U) {
                dram_size = 0x80000000ULL;
            } else {
                /* Preserve the vendor's signed 32-bit shift/sign-extension. */
                int32_t sz = (int32_t)(1U << (size_code + 24U));
                dram_size = (U64)(int64_t)sz;
            }
            RBUS_Setting();
            return dram_size;
        }
    }

    Init_DRAM(ddr_type ? 4U : 3U, 0U);
    RBUS_Setting();

    WRITE_RG_FLD(EMI_BASE + 0x1410U, 1U, 0x14U, 0U, 0U, 0U);
    dram_size = calculate_dram_size();

    if (uartDisable == 0)
        printf("DRAM FLOW DONE!!!\n");

    /* Vendor debug/trap strap: intentionally spins forever. */
    if (((hw_conf >> 8) & 0x1fU) == 0x17U)
        for (;;)
            ;

    return dram_size;
}
