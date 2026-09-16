/*
 * AN7583 DRAMC self-refresh / calibration migration helpers.
 *
 * Functional reconstruction from the vendor BL22 GCC 10.3 Thumb object.
 * Register addresses, bit fields, tables and call ordering are taken from
 * dramc_selfrefresh_api.o.  Source spelling/variable names are reconstructed.
 */
#include "recovery_abi.h"

#include <stddef.h>
#include <string.h>

extern int printf(const char *fmt, ...);
extern void udelay(U32 usec);

#define SCU_BASE        ((uintptr_t)0x1fb00000U)
#define DRAMC_AO_BASE   ((uintptr_t)0x1fc80000U)
#define DRAMC_NAO_BASE  ((uintptr_t)0x1fc81000U)
#define DDRPHY_AO_BASE  ((uintptr_t)0x1fc8a000U)
#define DDRPHY_NAO_BASE ((uintptr_t)0x1fc8b000U)
#define MPLL_BASE       ((uintptr_t)0x1fcac000U)

#define BIT(n) (1U << (n))

U32 READ_RG_FLD(uintptr_t addr, U32 width, U32 shift)
{
    U32 mask = UINT32_MAX >> (32U - width);
    return (*mmio32(addr) & (mask << shift)) >> shift;
}

/* The vendor ABI deliberately places value in the fifth argument. */
void WRITE_RG_FLD(uintptr_t addr, U32 width, U32 shift, U32 unused, U32 value)
{
    U32 mask = UINT32_MAX >> (32U - width);
    U32 v = *mmio32(addr);

    (void)unused;
    v &= ~(mask << shift);
    v |= value << shift;
    *mmio32(addr) = v;
}

static U32 round_div_nearest(U32 numerator, U32 denominator)
{
    U32 q = numerator / denominator;
    U32 rem = numerator - q * denominator;

    if (denominator - rem <= rem)
        q++;
    return q;
}

void MPLLRGByFreq(U32 pll_mode, U32 freq, U32 fbdiv,
                  U32 *rg4, U32 *rg8, U32 *rgc)
{
    U32 scaled_freq;
    U32 post_div;
    U32 first;
    U32 second;
    U32 third;

    scaled_freq = (U16)(((200U - fbdiv) * freq) / 200U);
    post_div = pll_mode ? 6U : 4U;

    first = round_div_nearest(0x19000000U * post_div, scaled_freq);
    *rg4 = first;

    post_div = (first * 30U) / 1000U;
    scaled_freq = (fbdiv * first) / 200U;

    second = round_div_nearest(0x03600000U, post_div);
    *rg8 = second;

    third = round_div_nearest(scaled_freq, second);
    *rgc = third;
}

void SwitchSynthPLLSSC(U32 pll_mode, U32 freq_mode, U32 data_rate_mode,
                       U32 enable)
{
    volatile U32 *pll = mmio32(MPLL_BASE);
    U32 top;

    if (!enable) {
        U32 fixed;

        printf("Disable SSC before adjust frequency\n");
        top = pll[1] & 0xff000000U;

        if (pll_mode == 1U) {
            if (data_rate_mode == 1U)
                fixed = 0x00240e6cU;
            else if (freq_mode == 1U)
                fixed = 0x001807b1U;
            else
                fixed = 0x001cd42eU;
        } else {
            if (data_rate_mode == 1U)
                fixed = 0x00200a41U;
            else if (freq_mode == 1U)
                fixed = 0x0018099dU;
            else
                fixed = 0x001b77c2U;
        }

        pll[1] = top | fixed;
        pll[2] &= ~0x00003fffU;
        pll[3] &= ~0x000003ffU;
        pll[5] &= ~BIT(0);
        pll[7] &= ~BIT(0);
        pll[0] |= BIT(0);
        udelay(1000);
        return;
    }

    printf("Enable SSC after adjust frequency\n");
    {
        U32 target;
        U32 divider = (*mmio32(DRAMC_NAO_BASE + 0x410U) >> 16) & 0xfU;
        U32 rg4, rg8, rgc;

        if (pll_mode == 1U) {
            if (data_rate_mode == 1U)
                target = 0x42aU;
            else if (freq_mode == 1U)
                target = 0x640U;
            else
                target = 0x535U;
        } else {
            if (data_rate_mode == 1U)
                target = 0x320U;
            else if (freq_mode == 1U)
                target = 0x42aU;
            else
                target = 0x3a5U;
        }

        MPLLRGByFreq(pll_mode, target, divider, &rg4, &rg8, &rgc);
        pll[1] = (pll[1] & 0xff000000U) | rg4;
        pll[2] = (pll[2] & ~0x00003fffU) | rg8;
        pll[3] = (pll[3] & ~0x000003ffU) | rgc;
        pll[5] |= BIT(0);
        pll[7] |= BIT(0);
        pll[0] |= BIT(0);
        udelay(1000);
    }
}

static const U32 synth_step_3200[25] = {
    0x00240e6cU, 0x00234725U, 0x0022943bU, 0x0021e844U,
    0x002142dbU, 0x0020a3a2U, 0x00200a41U, 0x001f7667U,
    0x001ee7caU, 0x001e5e24U, 0x001dd932U, 0x001d58b9U,
    0x001cd42eU, 0x001c56e8U, 0x001be2ffU, 0x001b72bcU,
    0x001b05f2U, 0x001a9c79U, 0x001a362cU, 0x0019d2e5U,
    0x00197283U, 0x001914e7U, 0x0018b9f1U, 0x00186186U,
    0x001807b1U,
};

static const U32 synth_step_2133[13] = {
    0x00200a41U, 0x001f24c0U, 0x001e54eeU, 0x001d8fabU,
    0x001cd42eU, 0x001c21c2U, 0x001b77c2U, 0x001ad59aU,
    0x001a3ac1U, 0x0019a6bcU, 0x00191919U, 0x00189172U,
    0x0018099dU,
};

static void synth_program_step(U32 value)
{
    volatile U32 *pll = mmio32(MPLL_BASE);

    pll[1] = (pll[1] & 0xff000000U) | value;
    pll[0] |= BIT(0);
    udelay(1000);
}

static void synth_walk(const U32 *table, U32 count, int down)
{
    if (down) {
        while (count != 0U) {
            count--;
            synth_program_step(table[count]);
        }
    } else {
        U32 i;
        for (i = 0; i < count; ++i)
            synth_program_step(table[i]);
    }
}

U32 StepSynthPLL(U32 pll_mode, U32 freq_mode, U32 down)
{
    volatile U32 *pll = mmio32(MPLL_BASE);

    if ((*mmio32(SCU_BASE + 0x74U) & BIT(6)) == 0U)
        return 3U;

    pll[6] |= BIT(0);
    pll[6] &= ~BIT(0);

    if (pll_mode == 1U) {
        if (freq_mode == 1U) {
            printf(down ? "Down freq from 3200\n" : "Up freq to 3200\n");
            synth_walk(synth_step_3200, 25U, down != 0U);
        } else {
            printf(down ? "Down freq from 2666\n" : "Up freq to 2666\n");
            synth_walk(synth_step_3200, 13U, down != 0U);
        }
    } else if (pll_mode == 0U) {
        if (freq_mode == 1U) {
            printf(down ? "Down freq from 2133\n" : "Up freq to 2133\n");
            synth_walk(synth_step_2133, 13U, down != 0U);
        } else {
            printf(down ? "Down freq from 1866\n" : "Up freq to 1866\n");
            synth_walk(synth_step_2133, 7U, down != 0U);
        }
    }

    printf("StepSynthPLL setting done !!!\n");
    return 4U;
}

void DramPhyResetSelfRefresh(void)
{
    volatile U32 *dramc = mmio32(DRAMC_AO_BASE);
    volatile U32 *phy = mmio32(DDRPHY_AO_BASE);

    dramc[0x240U / 4U] |= BIT(0);
    phy[0x7bcU / 4U] |= BIT(1);
    phy[0x518U / 4U] &= ~BIT(4);
    phy[0x518U / 4U] &= ~BIT(0);
    phy[0x598U / 4U] &= ~BIT(4);
    phy[0x598U / 4U] &= ~BIT(0);
    udelay(1);
    phy[0x598U / 4U] |= BIT(4);
    phy[0x598U / 4U] |= BIT(0);
    phy[0x518U / 4U] |= BIT(4);
    phy[0x518U / 4U] |= BIT(0);
    phy[0x7bcU / 4U] &= ~BIT(1);
    dramc[0x240U / 4U] &= ~BIT(0);
}

void SetMR6_SREF(U32 mr6)
{
    volatile U32 *dramc = mmio32(DRAMC_AO_BASE);
    U32 value = mr6 & 0xffU;
    U32 range = (mr6 >> 8) & 0x3fU;
    U32 timeout = 100U;

    dramc[0x130U / 4U] &= ~0x000003ffU;
    dramc[0x130U / 4U] &= ~0x00000c00U;
    dramc[0x460U / 4U] &= ~0x00700000U;
    dramc[0x130U / 4U] = (dramc[0x130U / 4U] & ~0x0ff00000U) |
                           (value << 20);
    dramc[0x4ecU / 4U] = (dramc[0x4ecU / 4U] & ~0x3f000000U) |
                           (range << 24);
    dramc[0x4ecU / 4U] = (dramc[0x4ecU / 4U] & ~0xfU) | 6U;
    dramc[0x174U / 4U] |= BIT(7);
    dramc[0x2d4U / 4U] = 0x30U;
    dramc[0x124U / 4U] = (dramc[0x124U / 4U] & ~0xfc000000U) |
                           0x10000000U;
    dramc[0x124U / 4U] |= 0x02000000U;

    while (timeout != 0U) {
        U32 done = *mmio32(DRAMC_AO_BASE + 0x4050U) & BIT(0);
        timeout--;
        udelay(1);
        if (done)
            break;
    }

    if (timeout == 0U)
        printf("RTSWCMD timeout, response = 0x%x\n", timeout);

    dramc[0x124U / 4U] &= ~0x02000000U;
}

static void SetTXVref_SREF_inner(U32 range, U32 vref, U32 value)
{
    U32 mr6 = (range == 0U ? 0x0c00U : 0x1000U) |
              (vref << 6) | 0x80U;

    SetMR6_SREF(mr6);
    mr6 |= value;
    SetMR6_SREF(mr6);
    SetMR6_SREF(mr6 & ~0x80U);
}

void SetTXVref_SREF(U32 enable, U32 range, U32 vref, U32 value)
{
    if (enable == 1U)
        SetTXVref_SREF_inner(range, vref, value);
}

void HW_AUTO_SREF_IDLE_TIME(U32 idle)
{
    U32 v = *mmio32(SCU_BASE + 0x44U);
    *mmio32(SCU_BASE + 0x44U) = (v & 0xff000000U) | idle;
    printf("HW_AUTO_SREF_IDLE_TIME = %d\n", idle);
}

void HW_AUTO_SREF_SETTING(void)
{
    *mmio32(DDRPHY_AO_BASE + 0x6bcU) &= ~0x02000000U;
    *mmio32(DDRPHY_AO_BASE + 0x460U) |= BIT(0);
    *mmio32(DDRPHY_AO_BASE + 0x460U) |= BIT(4);
    *mmio32(DDRPHY_AO_BASE + 0x4b4U) |= BIT(18);
    *mmio32(DDRPHY_AO_BASE + 0x4b4U) |= BIT(19);
}

U32 HW_AUTO_SREF_TIMES(void)
{
    return *mmio32(SCU_BASE + 0x44U) >> 24;
}

void HW_AUTO_SREF_ONOFF(U32 on)
{
    U32 v = *mmio32(SCU_BASE + 0x74U);
    v = (v & ~BIT(4)) | (on << 4);
    *mmio32(SCU_BASE + 0x74U) = v;
}

void DVFS_MEM_CK_MUX(U32 mux)
{
    U32 v;

    printf("SWITCH DRAMC CLK\n");
    v = *mmio32(DDRPHY_AO_BASE + 0x6a0U);
    v = (v & ~BIT(0)) | mux;
    *mmio32(DDRPHY_AO_BASE + 0x6a0U) = v;
    v = *mmio32(DDRPHY_AO_BASE + 0x6a0U);
    v = (v & ~BIT(1)) | (mux << 1);
    *mmio32(DDRPHY_AO_BASE + 0x6a0U) = v;
    v = *mmio32(DDRPHY_AO_BASE + 0x6a0U);
    v = (v & ~BIT(2)) | (mux << 2);
    *mmio32(DDRPHY_AO_BASE + 0x6a0U) = v;
}

U8 phase8_result_rg_rk0[2];
U8 phase8_result_rg_rk1[2];
U32 DCC_K_result_rg_rk0[3];
U32 DCC_K_result_rg_rk1[3];
U32 dqs_gating_K_result_rg_rk0[2];
U32 dqs_gating_K_result_rg_rk1[2];
U32 Tx_win_K_result_rg_rk0[12];
U32 Tx_win_K_result_rg_rk1[12];
U8 Tx_vref_K_result_rg_rk0;
U8 Tx_vref_K_result_rg_rk1;
U32 Rx_datlat_K_result_rg_rk0[2];
U32 Rx_datlat_K_result_rg_rk1[2];
U32 Rx_win_K_result_rg_rk0[10];
U32 Rx_win_K_result_rg_rk1[10];
U32 RX_dly_ADD[16];
U32 TX_dly_ADD[16];

void BACKUP_DRAM_K_RESULT_RK0(void)
{
    U32 dramc_misc = *mmio32(DRAMC_AO_BASE + 0x0cU);

    phase8_result_rg_rk0[0] = (U8)READ_RG_FLD(DDRPHY_AO_BASE + 0xf0cU, 3, 5);
    phase8_result_rg_rk0[1] = (U8)READ_RG_FLD(DDRPHY_AO_BASE + 0xef4U, 6, 8);

    DCC_K_result_rg_rk0[0] = *mmio32(DDRPHY_AO_BASE + 0x970U);
    DCC_K_result_rg_rk0[1] = *mmio32(DDRPHY_AO_BASE + 0x9e8U);
    DCC_K_result_rg_rk0[2] = *mmio32(DDRPHY_AO_BASE + 0xa68U);
    dqs_gating_K_result_rg_rk0[0] = *mmio32(DDRPHY_AO_BASE + 0xa2cU);
    dqs_gating_K_result_rg_rk0[1] = *mmio32(DDRPHY_AO_BASE + 0xaacU);

    Tx_win_K_result_rg_rk0[0] = *mmio32(DRAMC_NAO_BASE + 0x200U);
    Tx_win_K_result_rg_rk0[1] = *mmio32(DRAMC_NAO_BASE + 0x204U);
    Tx_win_K_result_rg_rk0[2] = *mmio32(DRAMC_NAO_BASE + 0x208U);
    Tx_win_K_result_rg_rk0[3] = *mmio32(DRAMC_NAO_BASE + 0x20cU);
    Tx_win_K_result_rg_rk0[4] = *mmio32(DDRPHY_AO_BASE + 0xa20U);
    Tx_win_K_result_rg_rk0[5] = *mmio32(DDRPHY_AO_BASE + 0xaa0U);
    Tx_win_K_result_rg_rk0[6] = *mmio32(DDRPHY_AO_BASE + 0x9e0U);
    Tx_win_K_result_rg_rk0[7] = *mmio32(DDRPHY_AO_BASE + 0x9e4U);
    Tx_win_K_result_rg_rk0[8] = *mmio32(DDRPHY_AO_BASE + 0xa60U);
    Tx_win_K_result_rg_rk0[9] = *mmio32(DDRPHY_AO_BASE + 0xa64U);
    Tx_win_K_result_rg_rk0[10] = *mmio32(DDRPHY_AO_BASE + 0x9ecU);
    Tx_win_K_result_rg_rk0[11] = *mmio32(DDRPHY_AO_BASE + 0xa6cU);

    Tx_vref_K_result_rg_rk0 = (U8)((dramc_misc >> 16) & 0x3fU);
    Rx_datlat_K_result_rg_rk0[0] = *mmio32(DDRPHY_NAO_BASE + 0xb8U);
    Rx_datlat_K_result_rg_rk0[1] = *mmio32(DRAMC_NAO_BASE + 0x68cU);

    Rx_win_K_result_rg_rk0[0] = *mmio32(DDRPHY_AO_BASE + 0x9f8U);
    Rx_win_K_result_rg_rk0[1] = *mmio32(DDRPHY_AO_BASE + 0x9fcU);
    Rx_win_K_result_rg_rk0[2] = *mmio32(DDRPHY_AO_BASE + 0xa00U);
    Rx_win_K_result_rg_rk0[3] = *mmio32(DDRPHY_AO_BASE + 0xa04U);
    Rx_win_K_result_rg_rk0[4] = *mmio32(DDRPHY_AO_BASE + 0xa78U);
    Rx_win_K_result_rg_rk0[5] = *mmio32(DDRPHY_AO_BASE + 0xa7cU);
    Rx_win_K_result_rg_rk0[6] = *mmio32(DDRPHY_AO_BASE + 0xa80U);
    Rx_win_K_result_rg_rk0[7] = *mmio32(DDRPHY_AO_BASE + 0xa84U);
    Rx_win_K_result_rg_rk0[8] = *mmio32(DDRPHY_AO_BASE + 0xeecU);
    Rx_win_K_result_rg_rk0[9] = *mmio32(DDRPHY_AO_BASE + 0xf6cU);
}

void BACKUP_DRAM_K_RESULT_RK1(void)
{
    U32 dramc_misc = *mmio32(DRAMC_AO_BASE + 0x0cU);

    phase8_result_rg_rk1[0] = (U8)((dramc_misc >> 3) & 0x7U);
    phase8_result_rg_rk1[1] = (U8)(dramc_misc & 0x7U);

    DCC_K_result_rg_rk1[0] = *mmio32(DDRPHY_AO_BASE + 0xbf0U);
    DCC_K_result_rg_rk1[1] = *mmio32(DDRPHY_AO_BASE + 0xc68U);
    DCC_K_result_rg_rk1[2] = *mmio32(DDRPHY_AO_BASE + 0xce8U);
    dqs_gating_K_result_rg_rk1[0] = *mmio32(DDRPHY_AO_BASE + 0xcacU);
    dqs_gating_K_result_rg_rk1[1] = *mmio32(DDRPHY_AO_BASE + 0xd2cU);

    Tx_win_K_result_rg_rk1[0] = *mmio32(DRAMC_NAO_BASE + 0x400U);
    Tx_win_K_result_rg_rk1[1] = *mmio32(DRAMC_NAO_BASE + 0x404U);
    Tx_win_K_result_rg_rk1[2] = *mmio32(DRAMC_NAO_BASE + 0x408U);
    Tx_win_K_result_rg_rk1[3] = *mmio32(DRAMC_NAO_BASE + 0x40cU);
    Tx_win_K_result_rg_rk1[4] = *mmio32(DDRPHY_AO_BASE + 0xca0U);
    Tx_win_K_result_rg_rk1[5] = *mmio32(DDRPHY_AO_BASE + 0xd20U);
    Tx_win_K_result_rg_rk1[6] = *mmio32(DDRPHY_AO_BASE + 0xc60U);
    Tx_win_K_result_rg_rk1[7] = *mmio32(DDRPHY_AO_BASE + 0xc64U);
    Tx_win_K_result_rg_rk1[8] = *mmio32(DDRPHY_AO_BASE + 0xce0U);
    Tx_win_K_result_rg_rk1[9] = *mmio32(DDRPHY_AO_BASE + 0xce4U);
    Tx_win_K_result_rg_rk1[10] = *mmio32(DDRPHY_AO_BASE + 0xc6cU);
    Tx_win_K_result_rg_rk1[11] = *mmio32(DDRPHY_AO_BASE + 0xcecU);

    Tx_vref_K_result_rg_rk1 = (U8)((dramc_misc >> 22) & 0x3fU);
    Rx_datlat_K_result_rg_rk1[0] = *mmio32(DDRPHY_AO_BASE + 0x510U);
    Rx_datlat_K_result_rg_rk1[1] = *mmio32(DDRPHY_AO_BASE + 0x490U);

    Rx_win_K_result_rg_rk1[0] = *mmio32(DDRPHY_AO_BASE + 0xc78U);
    Rx_win_K_result_rg_rk1[1] = *mmio32(DDRPHY_AO_BASE + 0xc7cU);
    Rx_win_K_result_rg_rk1[2] = *mmio32(DDRPHY_AO_BASE + 0xc80U);
    Rx_win_K_result_rg_rk1[3] = *mmio32(DDRPHY_AO_BASE + 0xc84U);
    Rx_win_K_result_rg_rk1[4] = *mmio32(DDRPHY_AO_BASE + 0xcf8U);
    Rx_win_K_result_rg_rk1[5] = *mmio32(DDRPHY_AO_BASE + 0xcfcU);
    Rx_win_K_result_rg_rk1[6] = *mmio32(DDRPHY_AO_BASE + 0xd00U);
    Rx_win_K_result_rg_rk1[7] = *mmio32(DDRPHY_AO_BASE + 0xd04U);
    Rx_win_K_result_rg_rk1[8] = (dramc_misc >> 6) & 0x1fU;
    Rx_win_K_result_rg_rk1[9] = (dramc_misc >> 11) & 0x1fU;
}

static void write_full(uintptr_t addr, U32 value)
{
    /* The vendor object emits a load before most stores; the load has no
     * semantic effect and is omitted here. */
    *mmio32(addr) = value;
}

void APPLY_DRAM_K_RESULT_RK1_TO_RK0(void)
{
    U32 v;
    U32 i;
    static const U16 dcc_off[3] = { 0x970, 0x9e8, 0xa68 };
    static const U16 gate_off[2] = { 0xa2c, 0xaac };
    static const U16 phy_tx_off[8] = {
        0xa20, 0xaa0, 0x9e0, 0x9e4, 0xa60, 0xa64, 0x9ec, 0xa6c
    };
    static const U16 rx_win_off[10] = {
        0x9f8, 0x9fc, 0xa00, 0xa04, 0xa78, 0xa7c, 0xa80, 0xa84, 0xeec, 0xf6c
    };

    for (i = 0; i < 3; ++i) {
        uintptr_t a = DDRPHY_AO_BASE + (i == 0 ? 0xf0cU : i == 1 ? 0xf8cU : 0xe8cU);
        v = *mmio32(a);
        *mmio32(a) = (v & ~0xe0U) | ((U32)phase8_result_rg_rk1[0] << 5);
    }
    for (i = 0; i < 3; ++i) {
        uintptr_t a = DDRPHY_AO_BASE + (i == 0 ? 0xef4U : i == 1 ? 0xf74U : 0xe74U);
        v = *mmio32(a);
        *mmio32(a) = (v & ~0x3f00U) | ((U32)phase8_result_rg_rk1[1] << 8);
    }

    for (i = 0; i < 3; ++i)
        write_full(DDRPHY_AO_BASE + dcc_off[i], DCC_K_result_rg_rk1[i]);
    for (i = 0; i < 2; ++i)
        write_full(DDRPHY_AO_BASE + gate_off[i], dqs_gating_K_result_rg_rk1[i]);
    for (i = 0; i < 4; ++i)
        write_full(DRAMC_NAO_BASE + 0x200U + i * 4U, Tx_win_K_result_rg_rk1[i]);
    for (i = 0; i < 8; ++i)
        write_full(DDRPHY_AO_BASE + phy_tx_off[i], Tx_win_K_result_rg_rk1[i + 4U]);

    write_full(DDRPHY_NAO_BASE + 0xb8U, Rx_datlat_K_result_rg_rk1[0]);
    write_full(DRAMC_NAO_BASE + 0x68cU, Rx_datlat_K_result_rg_rk1[1]);
    for (i = 0; i < 10; ++i)
        write_full(DDRPHY_AO_BASE + rx_win_off[i], Rx_win_K_result_rg_rk1[i]);
}

void APPLY_DRAM_K_RESULT_RK0_TO_RK1(void)
{
    U32 v;
    U32 i;
    static const U16 dcc_off[3] = { 0xbf0, 0xc68, 0xce8 };
    static const U16 gate_off[2] = { 0xcac, 0xd2c };
    static const U16 phy_tx_off[8] = {
        0xca0, 0xd20, 0xc60, 0xc64, 0xce0, 0xce4, 0xc6c, 0xcec
    };
    static const U16 rx_win_off[8] = {
        0xc78, 0xc7c, 0xc80, 0xc84, 0xcf8, 0xcfc, 0xd00, 0xd04
    };

    v = *mmio32(DRAMC_AO_BASE + 0x0cU);
    v &= ~0x3fU;
    v |= (U32)phase8_result_rg_rk0[1] |
         ((U32)phase8_result_rg_rk0[0] << 3);
    *mmio32(DRAMC_AO_BASE + 0x0cU) = v;

    for (i = 0; i < 3; ++i)
        write_full(DDRPHY_AO_BASE + dcc_off[i], DCC_K_result_rg_rk0[i]);
    for (i = 0; i < 2; ++i)
        write_full(DDRPHY_AO_BASE + gate_off[i], dqs_gating_K_result_rg_rk0[i]);
    for (i = 0; i < 4; ++i)
        write_full(DRAMC_NAO_BASE + 0x400U + i * 4U, Tx_win_K_result_rg_rk0[i]);
    for (i = 0; i < 8; ++i)
        write_full(DDRPHY_AO_BASE + phy_tx_off[i], Tx_win_K_result_rg_rk0[i + 4U]);

    write_full(DDRPHY_AO_BASE + 0x510U, Rx_datlat_K_result_rg_rk0[0]);
    write_full(DDRPHY_AO_BASE + 0x490U, Rx_datlat_K_result_rg_rk0[1]);
    for (i = 0; i < 8; ++i)
        write_full(DDRPHY_AO_BASE + rx_win_off[i], Rx_win_K_result_rg_rk0[i]);

    write_full(DDRPHY_NAO_BASE + 0x16cU, Rx_win_K_result_rg_rk0[8]);
    write_full(DDRPHY_NAO_BASE + 0x1ecU, Rx_win_K_result_rg_rk0[9]);

    v = *mmio32(DRAMC_AO_BASE + 0x0cU);
    v &= ~0x0000ffc0U;
    v |= (Rx_win_K_result_rg_rk0[8] |
          (Rx_win_K_result_rg_rk0[9] << 5)) << 6;
    *mmio32(DRAMC_AO_BASE + 0x0cU) = v;
}

void APPLY_DRAM_K_RESULT(void)
{
    BACKUP_DRAM_K_RESULT_RK0();
    BACKUP_DRAM_K_RESULT_RK1();
    APPLY_DRAM_K_RESULT_RK0_TO_RK1();
    APPLY_DRAM_K_RESULT_RK1_TO_RK0();
}

U32 Enter_SelfRefresh_api(U32 freq_sel)
{
    U32 before, after;
    U32 scu8c;
    U32 pll_ssc;
    U32 pll_mode;
    U32 freq_mode;
    U32 current;
    U32 step_ret;

    *mmio32(SCU_BASE + 0x74U) &= ~BIT(2);
    printf("Enter self-refresh freq_sel = %d\n", freq_sel);

    scu8c = *mmio32(SCU_BASE + 0x8cU);
    pll_ssc = *mmio32(MPLL_BASE + 0x14U) & BIT(0);
    current = (*mmio32(DRAMC_NAO_BASE + 0x410U) >> 20) & 1U;

    if (freq_sel == current) {
        *mmio32(SCU_BASE + 0x74U) |= BIT(2);
        if (freq_sel == 1U)
            printf("PLL is low frequency now. Adjusting PLL is unnecessary\n");
        else
            printf("PLL is normal frequency now. Adjusting PLL is unnecessary\n");
        return 2U;
    }

    {
        U32 v = *mmio32(DRAMC_NAO_BASE + 0x410U);
        v = (v & ~BIT(20)) | (freq_sel << 20);
        *mmio32(DRAMC_NAO_BASE + 0x410U) = v;
    }

    HW_AUTO_SREF_IDLE_TIME(1);
    HW_AUTO_SREF_SETTING();
    DVFS_MEM_CK_MUX(0);
    udelay(1000);
    *mmio32(DRAMC_NAO_BASE + 0x684U) &= ~BIT(3);

    before = HW_AUTO_SREF_TIMES();
    HW_AUTO_SREF_ONOFF(1);
    udelay(1000);
    after = HW_AUTO_SREF_TIMES();

    if (before != 0xffU && after - 1U != before) {
        printf("Self-refresh times is fail !!!\n");
        printf("before = %d, after = %d\n", before, after);
        HW_AUTO_SREF_ONOFF(0);
        *mmio32(DRAMC_NAO_BASE + 0x684U) |= BIT(3);
        *mmio32(SCU_BASE + 0x74U) |= BIT(2);
        DVFS_MEM_CK_MUX(1);
        return 1U;
    }

    pll_mode = (scu8c >> 5) & 1U;
    freq_mode = (scu8c >> 6) & 1U;

    if (pll_ssc)
        SwitchSynthPLLSSC(pll_mode, freq_mode, current, 0);

    step_ret = StepSynthPLL(pll_mode, freq_mode, freq_sel);

    if (pll_ssc)
        SwitchSynthPLLSSC(pll_mode, freq_mode, freq_sel, 1);

    if (step_ret != 4U) {
        printf("Self-refresh flow fail on PLL!!!\n");
        HW_AUTO_SREF_ONOFF(0);
        *mmio32(DRAMC_NAO_BASE + 0x684U) |= BIT(3);
        *mmio32(SCU_BASE + 0x74U) |= BIT(2);
        DVFS_MEM_CK_MUX(1);
        return 1U;
    }

    APPLY_DRAM_K_RESULT();
    HW_AUTO_SREF_ONOFF(0);
    udelay(2);
    *mmio32(DRAMC_NAO_BASE + 0x684U) |= BIT(3);

    if (pll_mode != 0U) {
        U32 txvref = freq_sel == 1U ? Tx_vref_K_result_rg_rk1
                                    : Tx_vref_K_result_rg_rk0;
        SetTXVref_SREF_inner(freq_mode, 0, txvref);
    }

    DVFS_MEM_CK_MUX(1);
    *mmio32(SCU_BASE + 0x74U) |= BIT(2);
    printf("Self-refresh flow done !!!\n");
    return 0U;
}

void TX_DQ_delay_shift(U32 bit_num, U32 dly)
{
    uintptr_t reg;
    U32 shift;
    U32 old;
    U32 delta;
    U32 value;

    printf("TX bit_num = %d, dly = %d\n", bit_num, dly);

    if (bit_num < 4U) {
        reg = DDRPHY_AO_BASE + 0x9e0U;
        shift = bit_num * 8U;
    } else if (bit_num < 8U) {
        reg = DDRPHY_AO_BASE + 0x9e4U;
        shift = (bit_num - 4U) * 8U;
    } else if (bit_num < 12U) {
        reg = DDRPHY_AO_BASE + 0xa60U;
        shift = (bit_num - 8U) * 8U;
    } else if (bit_num < 16U) {
        reg = DDRPHY_AO_BASE + 0xa64U;
        shift = (bit_num - 12U) * 8U;
    } else {
        return;
    }

    old = (*mmio32(reg) >> shift) & 0xffU;
    delta = old - TX_dly_ADD[bit_num];
    value = dly + delta;
    if (value > 0xffU) {
        printf("input delay underflow or overflow\n");
        printf("bit%d default dly value = %d\n", bit_num, delta);
        return;
    }

    TX_dly_ADD[bit_num] = dly;
    WRITE_RG_FLD(reg, 8, shift, 0, value);
}

void RX_DQ_delay_shift(U32 bit_num, U32 dly)
{
    uintptr_t reg;
    U32 shift;
    U32 old;
    U32 delta;
    U32 value;

    printf("RX bit_num = %d, dly = %d\n", bit_num, dly);

    if (bit_num < 8U) {
        reg = DDRPHY_AO_BASE + 0x9f8U + (bit_num >> 1) * 4U;
        shift = (bit_num & 1U) * 16U;
    } else if (bit_num < 16U) {
        U32 b = bit_num - 8U;
        reg = DDRPHY_AO_BASE + 0xa78U + (b >> 1) * 4U;
        shift = (b & 1U) * 16U;
    } else {
        return;
    }

    old = (*mmio32(reg) >> shift) & 0xffU;
    delta = old - RX_dly_ADD[bit_num];
    value = dly + delta;
    if (value > 0xffU) {
        printf("input delay underflow or overflow\n");
        printf("bit%d default dly value = %d\n", bit_num, delta);
        return;
    }

    RX_dly_ADD[bit_num] = dly;
    value |= value << 8;
    WRITE_RG_FLD(reg, 16, shift, 0, value);
}
