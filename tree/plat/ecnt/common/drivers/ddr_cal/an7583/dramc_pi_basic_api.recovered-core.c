/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Airoha AN75xx DRAMC basic-API recovery core.
 *
 * Semantics below are recovered directly from dramc_pi_basic_api.o.
 * Raw context offsets are intentional until the original PCDDR headers are
 * reconstructed.  This file is independent from the MT819x public-base file.
 */
#include "recovery_abi.h"

extern void *memset(void *, int, size_t);
extern int printf(const char *, ...);
extern void udelay(U32 usec);

extern void vSetRankNumber(void *ctx);
extern void vInitGlobalVariablesByCondition(void *ctx);
extern void DramcBroadcastOnOff(U32 on);
extern U32 GetDramcBroadcast(void);
extern U32 is_ddr3_family(void *ctx);
extern U32 is_ddr4_family(void *ctx);
extern void vReplaceDVInit(void *ctx);


void PC3_UpdateInitialSettings(void *ctx);
extern void PC4_UpdateInitialSettings(void *ctx);
extern void vResetDelayChainBeforeCalibration(void *ctx);
extern void vSetRank(void *ctx, U8 rank);
extern U8 u1GetRank(void *ctx);

extern void DQSSTBSettings(void *ctx);
extern void RODTSettings(void *ctx);
extern void vSetPHY2ChannelMapping(void *ctx, U8 channel);
extern U8 vGetPHY2ChannelMapping(void *ctx);
extern void APHY_RG_Setting_Before_K(void *ctx);
extern U16 GetVcoreDelayCellTime(void *ctx);
extern void Dramc8PhaseCal(void *ctx, U32 mode);
extern void DramcNewDutyCalibration(void *ctx);
extern void DdrUpdateACTiming(void *ctx);
extern void vBeforeCalibration(void *ctx);
extern void ANA_init(void *ctx);
extern void DIG_STATIC_SETTING(void *ctx);
extern void DIG_CONFIG_SHUF(void *ctx, U32 shu, U32 arg);
extern void PC3_DRAM_INIT(void *ctx);
extern void PC4_DRAM_INIT(void *ctx);
extern void DIG_HW_NONSHUF_SWITCH(void *ctx, U8 channel);
extern void SetClkFreeRun(void *ctx, U32 enable);
extern void vCKERankCtrl(void *ctx, U32 mode);
extern void CKE_FIX_ON(void *ctx, U32 option, U32 rank);
extern void vAutoRefreshSwitch(void *ctx, U32 enable);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vPhyByteIO32WriteMsk_All(void *ctx, U32 reg, U32 value, U32 mask);
extern void vIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vIO32WriteMsk_All(void *ctx, U32 reg, U32 value, U32 mask);
extern void vPhyByteWriteFldAlign(void *ctx, U32 reg, U32 value, U32 field,
                                 U32 all_channels);
extern 
void EnableDramcPhyDCMNonShuffle(void *ctx, U32 enable)
{
    volatile U32 *dfs = *(volatile U32 **)((U8 *)ctx + 0x90);

    vIO32WriteMsk_All(ctx, 0x00000190U, 1, 1);
    vPhyByteIO32WriteMsk_All(ctx, 0x0100066cU, 0x00000100U, 0x00540100U);
    if (dfs[1] == 2)
        vPhyByteIO32WriteMsk_All(ctx, 0x0100066cU, 0x00040000U, 0x00040000U);

    vPhyByteIO32WriteMsk_All(ctx, 0x01000674U, 0x80600b2eU, 0xffffff3fU);
    vIO32WriteMsk_All(ctx, 0x01000674U, 1, 1);
    vIO32WriteMsk_All(ctx, 0x01000674U, 0, 1);
    vIO32WriteMsk_All(ctx, 0x89000470U, 0, 0x00100000U);
    vIO32WriteMsk_All(ctx, 0x03800110U, 0x01000000U, 0x01000000U);

    if (enable) {
        vPhyByteIO32WriteMsk_All(ctx, 0x0000023cU, 0, 0x1eU);
        vPhyByteIO32WriteMsk_All(ctx, 0x01000680U, 0x00770000U, 0x00770000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x0100066cU, 0, 0x000bfe00U);
        vPhyByteIO32WriteMsk_All(ctx, 0x01000674U, 0x80U, 0xc0U);
        vIO32WriteMsk_All(ctx, 0x01000674U, 1, 1);
        vIO32WriteMsk_All(ctx, 0x01000674U, 0, 1);
        vIO32WriteMsk_All(ctx, 0x00000250U, 0, 2);
        vIO32WriteMsk_All(ctx, 0x00000168U, 0, 0x1000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x00000244U, 0, 0xc0000000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x00000190U, 0, 6);
        vPhyByteIO32WriteMsk_All(ctx, 0x00000288U, 0, 0xc0000000U);
        vIO32WriteMsk_All(ctx, 0x0000021cU, 0, 0x10U);
        vPhyByteIO32WriteMsk_All(ctx, 0x00000260U, 0, 0x08007fffU);
        vIO32WriteMsk_All(ctx, 0x010007a4U, 0, 8);
        vIO32WriteMsk_All(ctx, 0, 0, 1);
        vIO32WriteMsk_All(ctx, 0x00000208U, 0, 0x00080000U);
        vIO32WriteMsk_All(ctx, 0x00000200U, 0, 4);
        vPhyByteIO32WriteMsk_All(ctx, 0x000001fcU, 0xc0000007U, 0xc000106fU);
        vPhyByteIO32WriteMsk_All(ctx, 0x010007c4U, 0x04800000U, 0x0c800000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x000001b0U, 0, 0x00620000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x000001a0U, 0, 0x00180000U);
        vIO32WriteMsk_All(ctx, 0x010007e8U, 0, 8);
    } else {
        vPhyByteIO32WriteMsk_All(ctx, 0x0000023cU, 0x1eU, 0x1eU);
        vPhyByteIO32WriteMsk_All(ctx, 0x01000680U, 0, 0x00770000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x0100066cU, 0x000bfe00U, 0x000bfe00U);
        vPhyByteIO32WriteMsk_All(ctx, 0x01000674U, 0x40U, 0xc0U);
        vIO32WriteMsk_All(ctx, 0x01000674U, 1, 1);
        vIO32WriteMsk_All(ctx, 0x01000674U, 0, 1);
        vIO32WriteMsk_All(ctx, 0x00000250U, 2, 2);
        vIO32WriteMsk_All(ctx, 0x00000168U, 0x1000U, 0x1000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x00000244U, 0xc0000000U, 0xc0000000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x00000190U, 6, 6);
        vPhyByteIO32WriteMsk_All(ctx, 0x00000288U, 0xc0000000U, 0xc0000000U);
        vIO32WriteMsk_All(ctx, 0x0000021cU, 0x10U, 0x10U);
        vPhyByteIO32WriteMsk_All(ctx, 0x00000260U, 0x08007fffU, 0x08007fffU);
        vIO32WriteMsk_All(ctx, 0x010007a4U, 8, 8);
        vIO32WriteMsk_All(ctx, 0, 1, 1);
        vIO32WriteMsk_All(ctx, 0x00000208U, 0x00080000U, 0x00080000U);
        vIO32WriteMsk_All(ctx, 0x00000200U, 4, 4);
        vPhyByteIO32WriteMsk_All(ctx, 0x000001fcU, 0x00001068U, 0xc000106fU);
        vPhyByteIO32WriteMsk_All(ctx, 0x010007c4U, 0x0c800000U, 0x0c800000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x010007c8U, 0, 0x20U);
        vPhyByteIO32WriteMsk_All(ctx, 0x000001b0U, 0x00620000U, 0x00620000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x000001a0U, 0x00180000U, 0x00180000U);
        vIO32WriteMsk_All(ctx, 0x010007e8U, 8, 8);
    }
}

void EnableCommonDCMShuffle(void *ctx);
extern void EnableDramcPhyDCMShuffle(void *ctx, U32 enable);
extern void DramcHWGatingDebugOnOff(void *ctx, U32 enable);

extern void EnableRxDcmDPhy(void *ctx);
extern void Enable_ClkTxRxLatchEn(void *ctx);
extern void DramcInit_DutyCalibration(void *ctx);

U8 u1PrintModeRegWrite;

void Global_Option_Init(void *ctx)
{
    vSetRankNumber(ctx);
    vInitGlobalVariablesByCondition(ctx);
}

int DramcInit(void *ctx)
{
    (void)ctx;
    return 0;
}

void SetCKE2RankIndependent(void *ctx)
{
    vCKERankCtrl(ctx, 0);
}

void DramcEnablePerBankRefresh(void *ctx, U32 enable)
{
    (void)ctx;
    (void)enable;
}

void DramcRefreshRateDeBounceEnable(void *ctx)
{
    vPhyByteIO32WriteMsk_All(ctx, 0x0000026cU, 0xffff0504U, 0xffff3fffU);
}

void S0_DCMOffWA(void *ctx)
{
    vPhyByteIO32WriteMsk_All(ctx, 0x0100066cU, 0, 0x0003fe00U);
}

void SetMck8xLowPwrOption(void *ctx)
{
    vPhyByteIO32WriteMsk(ctx, 0x010006b4U, 0x001e3800U, 0x001e3800U);
}

void DramcModifiedRefreshMode(void *ctx)
{
    vPhyByteIO32WriteMsk_All(ctx, 0x00000268U, 0x40U, 0xc0U);
    vIO32WriteMsk_All(ctx, 0x002016d4U, 4, 7);
}

void DV_InitialSettings(void *ctx)
{
    DramcBroadcastOnOff(1);
    ANA_init(ctx);
    DIG_STATIC_SETTING(ctx);
    DIG_CONFIG_SHUF(ctx, 0, 0);
    DramcBroadcastOnOff(0);
}

void DramcHWGatingOnOff(void *ctx, U32 enable)
{
    vIO32WriteMsk_All(ctx, 0x01000668U, enable << 28, 0x10000000U);
    vIO32WriteMsk_All(ctx, 0x01201094U, enable << 16, 0x00010000U);
    vIO32WriteMsk_All(ctx, 0x01201094U, enable << 17, 0x00020000U);
}

void DramcCKEDebounce(void *ctx)
{
    if (raw_u16(ctx, 0x54) > 1865U) {
        U8 rank_bak = raw_u8(ctx, 0x0c);
        U8 rank = 0;
        while ((U32)rank < raw_u32(ctx, 0x08)) {
            vSetRank(ctx, rank);
            vIO32WriteMsk_All(ctx, 0x00601260U, 0x0fU, 0xffU);
            rank++;
        }
        vSetRank(ctx, rank_bak);
    }
}

void DramcShuTrackingDcmEnBySRAM(void *ctx)
{
    EnableRxDcmDPhy(ctx);
    Enable_ClkTxRxLatchEn(ctx);
    EnableDramcPhyDCMShuffle(ctx, 0);
}

void EnableDramcPhyDCM(void *ctx, U32 enable)
{
    U32 broadcast = GetDramcBroadcast();
    DramcBroadcastOnOff(0);
    EnableDramcPhyDCMNonShuffle(ctx, enable);
    EnableDramcPhyDCMShuffle(ctx, enable);
    DramcBroadcastOnOff(broadcast);
}

void SA_InitialSettings1(void *ctx)
{
    DramcBroadcastOnOff(1);
    if (is_ddr4_family(ctx))
        PC4_UpdateInitialSettings(ctx);
    else if (is_ddr3_family(ctx))
        PC3_UpdateInitialSettings(ctx);

    vResetDelayChainBeforeCalibration(ctx);
    DramcBroadcastOnOff(0);
    memset((U8 *)ctx + 0x74, 0xff, 8);
    memset((U8 *)ctx + 0x7c, 0x00, 8);
    vSetRank(ctx, 0);
    APHY_RG_Setting_Before_K(ctx);
    *(U16 *)((U8 *)ctx + 0x72) = GetVcoreDelayCellTime(ctx);
    Dramc8PhaseCal(ctx, 0);
    DramcInit_DutyCalibration(ctx);
}

void SA_InitialSettings2(void *ctx)
{
    DramcBroadcastOnOff(0);
    DdrUpdateACTiming(ctx);
    memset((U8 *)ctx + 0x8c, 0, 1);
    vBeforeCalibration(ctx);
}

void DFSInitForCalibration(void *ctx)
{
    U8 old_channel;
    U8 ch;

    DV_InitialSettings(ctx);
    SA_InitialSettings1(ctx);
    DramcBroadcastOnOff(1);
    SetClkFreeRun(ctx, 1);
    DramcBroadcastOnOff(0);

    u1PrintModeRegWrite = 1;
    if (is_ddr4_family(ctx))
        PC4_DRAM_INIT(ctx);
    else if (is_ddr3_family(ctx))
        PC3_DRAM_INIT(ctx);
    u1PrintModeRegWrite = 0;

    DramcBroadcastOnOff(1);
    SetClkFreeRun(ctx, 0);
    DramcBroadcastOnOff(0);

    old_channel = raw_u8(ctx, 0x04);
    for (ch = 0; (U32)ch < raw_u32(ctx, 0x00); ch++) {
        vSetPHY2ChannelMapping(ctx, ch);
        DIG_HW_NONSHUF_SWITCH(ctx, ch);
    }
    vSetPHY2ChannelMapping(ctx, old_channel);
    SA_InitialSettings2(ctx);
}

extern U32 uartDisable;
extern void MPLLRGByFreq(U32 strap, U32 frequency, U32 ssc,
                         U32 *out0, U32 *out1, U32 *out2);

void EnableRxDcmDPhy(void *ctx)
{
    vPhyByteIO32WriteMsk_All(ctx, 0x01201120U, 0x00020727U, 0x00ff0133U);
}

void Enable_ClkTxRxLatchEn(void *ctx)
{
    const U32 f2 = 0xff040102U, f15 = 0xff04010fU, f16 = 0xff040110U;
    vPhyByteWriteFldAlign(ctx, 0x51200f34U, 1, f2, 1);
    vPhyByteWriteFldAlign(ctx, 0x99200fb4U, 1, f2, 1);
    vPhyByteWriteFldAlign(ctx, 0x91200efcU, 1, f15, 1);
    vPhyByteWriteFldAlign(ctx, 0x99200f7cU, 1, f15, 1);
    vPhyByteWriteFldAlign(ctx, 0x51200ef8U, 1, f16, 1);
    vPhyByteWriteFldAlign(ctx, 0x59200f78U, 1, f16, 1);
    vPhyByteWriteFldAlign(ctx, 0x51200f00U, 1, f2, 1);
    vPhyByteWriteFldAlign(ctx, 0x59200f80U, 1, f2, 1);
    vPhyByteWriteFldAlign(ctx, 0x49200e80U, 1, f2, 1);
    vPhyByteWriteFldAlign(ctx, 0x89200e7cU, 1, f15, 1);
}

void DQSSTBSettings(void *ctx)
{
    U32 en = is_ddr4_family(ctx) ? 1U : (is_ddr3_family(ctx) ? 0U : 1U);
    vIO32WriteMsk(ctx, 0x01201094U, en << 12, 0x3000U);
    vPhyByteWriteFldAlign(ctx, 0x91200efcU, en, 0xff040308U, 0);
    vPhyByteWriteFldAlign(ctx, 0x99200f7cU, en, 0xff040308U, 0);
    vPhyByteWriteFldAlign(ctx, 0xa3a00ffcU, en, 0x00040308U, 0);
}

void DramcInit_DutyCalibration(void *ctx)
{
    U8 old_channel = vGetPHY2ChannelMapping(ctx);
    U32 broadcast = GetDramcBroadcast();
    U8 ch;

    DramcBroadcastOnOff(0);
    for (ch = 0; (U32)ch < raw_u32(ctx, 0x00); ch++) {
        vSetPHY2ChannelMapping(ctx, ch);
        DramcNewDutyCalibration(ctx);
    }
    vSetPHY2ChannelMapping(ctx, old_channel);
    DramcBroadcastOnOff(broadcast);
}

void DramcRunTimeConfig(void *ctx)
{
    DramcHWGatingDebugOnOff(ctx, 0);
    vIO32WriteMsk_All(ctx, 0x000001a4U, 0x80000000U, 0x80000000U);
    DramcShuTrackingDcmEnBySRAM(ctx);
    DramcRefreshRateDeBounceEnable(ctx);
    vAutoRefreshSwitch(ctx, 1);
    vPhyByteIO32WriteMsk_All(ctx, 0x010006bcU, 0x02000000U, 0x03000000U);
    vIO32WriteMsk(ctx, 0x0000017cU, 1, 1);
}

U32 FreqByMPLLRG(U8 dram_type)
{
    U32 div = *mmio32(0x1fcac004U);
    U32 mult = (dram_type == 3U) ? 4U : 6U;
    return (0x19000000U * mult) / div;
}

void MPLLInit_SSC(void *ctx, U32 ssc)
{
    U32 a, b, c;
    U32 reg;
    U32 strap;

    if (uartDisable == 0)
        printf("SSC Enable !!!\n");

    reg = *mmio32(0x1fc81410U);
    reg &= ~0x000f0000U;
    reg |= ssc << 16;
    *mmio32(0x1fc81410U) = reg;

    strap = (*mmio32(0x1fb0008cU) >> 5) & 1U;
    MPLLRGByFreq(strap, raw_u16(ctx, 0x54), ssc, &a, &b, &c);

    *mmio32(0x1fcac004U) = a;
    *mmio32(0x1fcac008U) = b;
    *mmio32(0x1fcac00cU) = c;
    *mmio32(0x1fcac014U) = 1;
    *mmio32(0x1fcac01cU) = 1;
    *mmio32(0x1fcac000U) = 1;
    udelay(1000);

    if (uartDisable == 0)
        printf("SSC MPLL setting done !!!\n");
}


void DramcHWGatingDebugOnOff(void *ctx, U32 enable)
{
    U32 value = (enable & 1U) | ((enable << 1) & 2U) |
                ((enable << 28) & 0x10000000U);
    if (enable == 1U)
        value |= 0x30U;

    vPhyByteIO32WriteMsk_All(ctx, 0x01000668U, value, 0x100003f3U);
    vIO32WriteMsk_All(ctx, 0x91000518U, 0x80U, 0x80U);
    vIO32WriteMsk_All(ctx, 0x99000598U, 0x80U, 0x80U);
    vIO32WriteMsk_All(ctx, 0xa1000618U, 0x80U, 0x80U);
}

void APHY_RG_Check(void)
{
    printf("APHY RG Check\n");
    printf("1FC8A040 = %lx\n", *mmio32(0x1fc8a040U));
    printf("1FC8A044 = %lx\n", *mmio32(0x1fc8a044U));
    printf("1FC8A4D8 = %lx\n", *mmio32(0x1fc8a4d8U));
    printf("1FC8AF0C = %lx\n", *mmio32(0x1fc8af0cU));
    printf("1FC8AF8C = %lx\n", *mmio32(0x1fc8af8cU));
    printf("1FC8AE8C = %lx\n", *mmio32(0x1fc8ae8cU));
    printf("1FC9AE8C = %lx\n", *mmio32(0x1fc9ae8cU));
    printf("1FC8AEF4 = %lx\n", *mmio32(0x1fc8aef4U));
    printf("1FC8AF74 = %lx\n", *mmio32(0x1fc8af74U));
    printf("1FC8A558 = %lx\n", *mmio32(0x1fc8a558U));
    printf("1FC8A5D8 = %lx\n", *mmio32(0x1fc8a5d8U));
    printf("1FC9A558 = %lx\n", *mmio32(0x1fc9a558U));
    printf("1FC9A5D8 = %lx\n", *mmio32(0x1fc9a5d8U));
    printf("1FC9AF34 = %lx\n", *mmio32(0x1fc9af34U));
    printf("1FC9AFB4 = %lx\n", *mmio32(0x1fc9afb4U));
    printf("1FC9A4EC = %lx\n", *mmio32(0x1fc9a4ecU));
    printf("1FC9A56C = %lx\n", *mmio32(0x1fc9a56cU));
    printf("1FC9A500 = %lx\n", *mmio32(0x1fc9a500U));
    printf("1FC9A580 = %lx\n", *mmio32(0x1fc9a580U));
    printf("1FC9A4D8 = %lx\n", *mmio32(0x1fc9a4d8U));
    printf("1FC8AF1C = %lx\n", *mmio32(0x1fc8af1cU));
    printf("1FC8AF9C = %lx\n", *mmio32(0x1fc8af9cU));
    printf("1FC8AE9C = %lx\n", *mmio32(0x1fc8ae9cU));
    printf("1FC9AE9C = %lx\n", *mmio32(0x1fc9ae9cU));
    printf("1FC8A940 = %lx\n", *mmio32(0x1fc8a940U));
    printf("1FC8A900 = %lx\n", *mmio32(0x1fc8a900U));
    printf("1FC8A920 = %lx\n", *mmio32(0x1fc8a920U));
    printf("1FC8AE94 = %lx\n", *mmio32(0x1fc8ae94U));
    printf("1FC8AF14 = %lx\n", *mmio32(0x1fc8af14U));
    printf("1FC8AF94 = %lx\n", *mmio32(0x1fc8af94U));
    printf("1FC9AE94 = %lx\n", *mmio32(0x1fc9ae94U));
    printf("1FC9AF14 = %lx\n", *mmio32(0x1fc9af14U));
    printf("1FC9AF94 = %lx\n", *mmio32(0x1fc9af94U));
    printf("1FC8AE98 = %lx\n", *mmio32(0x1fc8ae98U));
    printf("1FC8AF18 = %lx\n", *mmio32(0x1fc8af18U));
    printf("1FC8AF98 = %lx\n", *mmio32(0x1fc8af98U));
    printf("1FC9AE98 = %lx\n", *mmio32(0x1fc9ae98U));
    printf("1FC9AF18 = %lx\n", *mmio32(0x1fc9af18U));
    printf("1FC9AF98 = %lx\n", *mmio32(0x1fc9af98U));
}


void MPLLInit(void *ctx)
{
    volatile U32 *mpll = mmio32(0x1fcac000U);
    U32 type = raw_u32(ctx, 0x18);
    U32 sel = raw_u32(ctx, 0x10);

    mpll[6] = 1;
    mpll[6] = 0;

    if (type == 3U) {
        switch (sel) {
        case 15: mpll[1] = 0x00402910U; break;
        case 14: mpll[1] = 0x00301eccU; break;
        case 13: mpll[1] = 0x00267f0aU; break;
        case 12: mpll[1] = 0x00200a41U; break;
        case 11: mpll[1] = 0x001b77c2U; break;
        case 10: mpll[1] = 0x0018099dU; break;
        default: break;
        }
    } else if (type == 4U) {
        switch (sel) {
        case 6: mpll[1] = 0x00300f61U; break;
        case 5: mpll[1] = 0x002933a4U; break;
        case 4: mpll[1] = 0x00240e6cU; break;
        case 3: mpll[1] = 0x002006d5U; break;
        case 2: mpll[1] = 0x001cd42eU; break;
        case 1: mpll[1] = 0x001a362cU; break;
        case 0: mpll[1] = 0x001807b1U; break;
        default: break;
        }
    }

    if (raw_u8(ctx, 0xbd) == 0U) {
        if (type == 3U)
            mpll[1] = 0x00200a41U;
        else if (type == 4U)
            mpll[1] = 0x00240e6cU;
    }

    mpll[0] = 1;
    udelay(1000);
    if (uartDisable == 0)
        printf("MPLL setting done !!!\n");
}


static void aphy_common_after_drive(void *ctx)
{
    vPhyByteIO32WriteMsk(ctx, 0x11000558U, 0, 0x00020000U);
    vPhyByteIO32WriteMsk(ctx, 0x190005d8U, 0, 0x00020000U);
    vPhyByteIO32WriteMsk(ctx, 0x51200f34U, 1, 1);
    vPhyByteIO32WriteMsk(ctx, 0x99200fb4U, 1, 1);
    vPhyByteIO32WriteMsk(ctx, 0x910004ecU, 0, 8);
    vPhyByteIO32WriteMsk(ctx, 0x9900056cU, 0, 8);
    vPhyByteWriteFldAlign(ctx, 0x91000500U, 0, 0, 0);
    vPhyByteWriteFldAlign(ctx, 0x99000580U, 0, 0, 0);
    vPhyByteIO32WriteMsk(ctx, 0x99000580U, 0, 4);
}

static void aphy_finish_by_ddr_type(void *ctx, U32 type)
{
    if (type == 0U) {
        vPhyByteIO32WriteMsk(ctx, 0x91200ef4U, 0x300U, 0x3f00U);
        vPhyByteIO32WriteMsk(ctx, 0x99200f74U, 0x300U, 0x3f00U);
        vPhyByteWriteFldAlign(ctx, 0x49200e94U, 0x02009000U, 0, 0);
        vPhyByteWriteFldAlign(ctx, 0x49200e98U, 0x01011d1cU, 0, 0);
    } else if (type == 1U) {
        vPhyByteIO32WriteMsk(ctx, 0x91200ef4U, 0x100U, 0x3f00U);
        vPhyByteIO32WriteMsk(ctx, 0x99200f74U, 0x100U, 0x3f00U);
    }
}

extern U32 ddr_type;

void APHY_RG_Setting_Before_K(void *ctx)
{
    U8 old_channel;

    if (uartDisable == 0)
        printf("APHY RG Setting\n");
    old_channel = vGetPHY2ChannelMapping(ctx);
    vSetPHY2ChannelMapping(ctx, 0);
    vPhyByteIO32WriteMsk(ctx, 0x01000044U, 0x8000U, 0xe000U);
    vPhyByteWriteFldAlign(ctx, 0x090004d8U, 0x180U, 0, 0);
    vPhyByteWriteFldAlign(ctx, 0x11200f0cU, 0x48U, 0, 0);
    vPhyByteWriteFldAlign(ctx, 0x19200f8cU, 0x48U, 0, 0);
    vPhyByteWriteFldAlign(ctx, 0x09200e8cU, 0x48U, 0, 0);
    vSetPHY2ChannelMapping(ctx, 1);
    vPhyByteWriteFldAlign(ctx, 0x09200e8cU, 0x48U, 0, 0);
    aphy_common_after_drive(ctx);
    vSetPHY2ChannelMapping(ctx, 0);
    aphy_finish_by_ddr_type(ctx, ddr_type);
    vSetPHY2ChannelMapping(ctx, old_channel);
}

void vReplaceDVInit(void *ctx)
{
    U8 rank_bak = raw_u8(ctx, 0x0c);
    U32 broadcast_bak;
    U16 freq;
    U32 cap_sel;
    U32 value;
    U8 rank;

    vPhyByteIO32WriteMsk(ctx, 0x110004e4U, 0, 0x90000000U);
    vPhyByteIO32WriteMsk(ctx, 0x19000564U, 0, 0x90000000U);

    for (rank = 0; (U32)rank < raw_u32(ctx, 0x08); rank++) {
        vSetRank(ctx, rank);
        vPhyByteIO32WriteMsk(ctx, 0x114000e8U, 0, 0xd0800000U);
        vPhyByteIO32WriteMsk(ctx, 0x19400168U, 0, 0xd0800000U);
    }
    vSetRank(ctx, rank_bak);

    vIO32WriteMsk(ctx, 0x03800150U, 0, 0x7800U);
    vIO32WriteMsk(ctx, 0x0000012cU, 0, 0x80000000U);

    broadcast_bak = GetDramcBroadcast();
    DramcBroadcastOnOff(0);

    freq = raw_u16(ctx, 0x54);
    if (is_ddr3_family(ctx))
        cap_sel = (freq <= 1065U) ? 3U : 0U;
    else if (is_ddr4_family(ctx))
        cap_sel = 0U;
    else
        cap_sel = 0x18U;

    value = (cap_sel << 12) | 0x00400000U;
    vPhyByteIO32WriteMsk_All(ctx, 0x91200ef0U, value, 0x00c7f000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x99200f70U, value, 0x00c7f000U);
    vPhyByteIO32WriteMsk_All(ctx, 0xa3a00ff0U, value, 0x00c7f000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x89200e70U, value, 0x00c7f000U);

    value = (freq > 2132U) ? 0x0c00U : 0;
    vPhyByteIO32WriteMsk_All(ctx, 0x51200ef8U, value, 0x0f00U);
    vPhyByteIO32WriteMsk_All(ctx, 0x59200f78U, value, 0x0f00U);
    if (freq > 2132U)
        value = 0x0c00U;
    else if (freq <= 300U)
        value = 0x0300U;
    else
        value = 0;
    vPhyByteIO32WriteMsk_All(ctx, 0x49200e78U, value, 0x0f00U);

    vPhyByteWriteFldAlign(ctx, 0x01000808U, 0, 0, 1);
    vPhyByteIO32WriteMsk_All(ctx, 0x01201120U, 0x00020000U, 0x00ff0003U);
    vIO32WriteMsk_All(ctx, 0x00000220U, 0, 0x40U);
    vIO32WriteMsk_All(ctx, 0x00000268U, 0, 0x40U);
    vIO32WriteMsk_All(ctx, 0x000002d8U, 0, 0xffffU);
    vPhyByteIO32WriteMsk_All(ctx, 0x000002e0U, 0, 0xff000001U);
    vIO32WriteMsk_All(ctx, 0x00601264U, 0, 1);
    vIO32WriteMsk_All(ctx, 0x090004acU, 0x80000000U, 0x80000000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x91200efcU, 0x12U, 0x2U);
    vPhyByteIO32WriteMsk_All(ctx, 0x99200f7cU, 0x12U, 0x2U);
    vPhyByteIO32WriteMsk_All(ctx, 0x11200f08U, 0xfdd80000U, 0xfdd80000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x19200f88U, 0xfdd80000U, 0xfdd80000U);
    vIO32WriteMsk_All(ctx, 0x03800110U, 0, 0x00400000U);
    vIO32WriteMsk_All(ctx, 0x00000130U, 0x20U, 0x3ffU);
    vIO32WriteMsk_All(ctx, 0x0000013cU, 0x00100000U, 0x01ff8000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x00000170U, 0, 0x82U);
    vPhyByteIO32WriteMsk_All(ctx, 0x00000174U, 0x80U, 0xc0U);
    vPhyByteIO32WriteMsk_All(ctx, 0x00000190U, 0x41U, 0x41U);
    vIO32WriteMsk_All(ctx, 0x00000204U, 0x08000000U, 0x08000000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x00000208U, 0x820U, 0x820U);
    vIO32WriteMsk_All(ctx, 0x00000210U, 1, 0xffU);
    vPhyByteIO32WriteMsk_All(ctx, 0x00000220U, 0x80U, 0x286U);
    vIO32WriteMsk_All(ctx, 0x00000240U, 0, 0x40U);
    vPhyByteIO32WriteMsk_All(ctx, 0x0000024cU, 0x80000400U, 0x80000400U);
    vPhyByteIO32WriteMsk_All(ctx, 0x002016d4U, 0x14U, 0x37U);
    vIO32WriteMsk_All(ctx, 0x00000254U, 0, 1);
    vPhyByteIO32WriteMsk_All(ctx, 0x00000264U, 0x0aU, 0x0fU);
    vPhyByteIO32WriteMsk_All(ctx, 0x00000268U, 0x00800500U, 0x02801508U);
    vIO32WriteMsk_All(ctx, 0x0000026cU, 0x500U, 0x1f00U);
    vIO32WriteMsk_All(ctx, 0x00000284U, 2, 2);
    vIO32WriteMsk_All(ctx, 0x000002d4U, 0x30U, 0xffffffffU);
    vIO32WriteMsk_All(ctx, 0x000002e0U, 0, 0xff000000U);
    vIO32WriteMsk_All(ctx, 0x00201610U, 0x80U, 0x80U);
    vPhyByteIO32WriteMsk_All(ctx, 0x00201614U, 0x01ff0000U, 0x0fffff00U);
    vIO32WriteMsk_All(ctx, 0x03a01640U, 0x0b000000U, 0xff000000U);
    vIO32WriteMsk_All(ctx, 0x002016a4U, 2, 0x0fU);
    vIO32WriteMsk_All(ctx, 0x110004e8U, 0, 0x2000U);
    vIO32WriteMsk_All(ctx, 0x19000568U, 0, 0x2000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x510004f4U, 0x300U, 0x300U);
    vPhyByteIO32WriteMsk_All(ctx, 0x59000574U, 0x300U, 0x300U);
    vPhyByteIO32WriteMsk_All(ctx, 0x49000474U, 0x300U, 0x300U);
    vPhyByteIO32WriteMsk_All(ctx, 0x0b8004a0U, 0x0a0aU, 0x1f1fU);

    vPhyByteIO32WriteMsk_All(ctx, 0x01000674U, 0x80600000U, 0x83e00040U);
    vIO32WriteMsk_All(ctx, 0x010006a0U, 4, 4);
    vPhyByteIO32WriteMsk_All(ctx, 0x01000688U, 0, 0x000aaa10U);
    vPhyByteIO32WriteMsk_All(ctx, 0x01000698U, 0x890U, 0x0fc00f90U);
    vIO32WriteMsk_All(ctx, 0x010006bcU, 0x0fU, 0xffU);
    vPhyByteIO32WriteMsk_All(ctx, 0x038006c4U, 0x000a0020U, 0x000a002cU);
    vPhyByteIO32WriteMsk_All(ctx, 0x010007a4U, 0x10U, 0x30U);
    vIO32WriteMsk_All(ctx, 0x038007acU, 0, 0x01000000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x010007b8U, 0x000c0000U, 0x000c0000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x010007c8U, 0, 0x20U);
    vPhyByteIO32WriteMsk_All(ctx, 0x010007ccU, 0x01402000U, 0x05400400U);
    vPhyByteIO32WriteMsk_All(ctx, 0x010007d0U, 0x203U, 0x2U);
    vIO32WriteMsk_All(ctx, 0x01000838U, 0x0e000000U, 0x1f000000U);
    vIO32WriteMsk_All(ctx, 0x01000848U, 0, 0xf800U);
    vPhyByteIO32WriteMsk_All(ctx, 0x010007b8U, 0, 0x0a00U);
    vIO32WriteMsk_All(ctx, 0x0100084cU, 0x06000000U, 0xffff0000U);
    vIO32WriteMsk_All(ctx, 0x01000850U, 0x8004U, 0xffffU);
    vIO32WriteMsk_All(ctx, 0x01000854U, 0, 0x40U);
    vPhyByteIO32WriteMsk_All(ctx, 0x91200ef4U, 0x8c000000U, 0x04000000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x99200f74U, 0x8c000000U, 0x8c000000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x91200efcU, 0x00040012U, 0x2U);
    vPhyByteIO32WriteMsk_All(ctx, 0x99200f7cU, 0x00040012U, 0x2U);
    vPhyByteWriteFldAlign(ctx, 0x51200f00U, 1, 0xff040212U, 1);
    vPhyByteWriteFldAlign(ctx, 0x59200f80U, 1, 0xff040212U, 1);
    vIO32WriteMsk_All(ctx, 0x01200900U, 0, 0xffffU);
    vIO32WriteMsk_All(ctx, 0x01200920U, 0, 0xffffU);
    vPhyByteWriteFldAlign(ctx, 0x49200e80U, 1, 0xff040212U, 1);
    vPhyByteIO32WriteMsk_All(ctx, 0x09200e88U, 0xfc200000U, 0xfc300000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x03a0108cU, 0x00077000U, 0x00077000U);

    vIO32WriteMsk_All(ctx, 0x012010dcU, is_ddr3_family(ctx) ? 9U : 8U, 0x3fU);
    vIO32WriteMsk_All(ctx, 0x03a010c4U, 6, 7);
    vPhyByteIO32WriteMsk_All(ctx, 0x01201120U, 0x00020000U, 0x00ff0003U);
    vIO32WriteMsk_All(ctx, 0x03800870U, 0x00020000U, 0x00ff0000U);

    DramcBroadcastOnOff(broadcast_bak);
}

void PC3_UpdateInitialSettings(void *ctx)
{
    vIO32WriteMsk(ctx, 0x010007c4U, 0, 0);
    vIO32WriteMsk(ctx, 0x010007c8U, 0, 0);
    vReplaceDVInit(ctx);
    vPhyByteWriteFldAlign(ctx, 0x49200eb8U, 0x50U, 0xff050810U, 0);
    vPhyByteIO32WriteMsk(ctx, 0x91200f04U, 0, 0x3f00U);
    vPhyByteIO32WriteMsk(ctx, 0x99200f84U, 0, 0x3f00U);
    DQSSTBSettings(ctx);
    RODTSettings(ctx);
    vIO32WriteMsk(ctx, 0x03a01688U, 0, 0x1000U);
    vIO32WriteMsk(ctx, 0x00000268U, 0x2000U, 0x2000U);
    vIO32WriteMsk(ctx, 0x00000250U, 1, 1);
    vIO32WriteMsk(ctx, 0x0000017cU, 0, 0);
    vIO32WriteMsk(ctx, 0x03a01688U, raw_u32(ctx, 0x1c) << 15, 0x8000U);
    vIO32WriteMsk(ctx, 0x012010f8U, 0, 1);
    vPhyByteIO32WriteMsk(ctx, 0x03800110U, 0x08000000U, 0x08200000U);
    vPhyByteIO32WriteMsk(ctx, 0x0000024cU, 0x200U, 0x300U);
    vIO32WriteMsk(ctx, 0x01000668U, 0x400U, 0x400U);
    vPhyByteIO32WriteMsk(ctx, 0x01000684U, 0x10000U, 0x10800U);
    vPhyByteIO32WriteMsk(ctx, 0x012010a4U, 0x02118000U, 0x0777c000U);
    vIO32WriteMsk(ctx, 0x01201128U, 1, 1);
    SetMck8xLowPwrOption(ctx);
}

void PC4_UpdateInitialSettings(void *ctx)
{
    vPhyByteWriteFldAlign(ctx, 0x49200eb8U, 0, 0xff050810U, 0);
    vReplaceDVInit(ctx);
    vPhyByteIO32WriteMsk(ctx, 0x91200f04U, 0, 0x3f00U);
    vPhyByteIO32WriteMsk(ctx, 0x99200f84U, 0, 0x3f00U);
    vPhyByteIO32WriteMsk(ctx, 0xa3a01004U, 0, 0x3f00U);
    DQSSTBSettings(ctx);
    RODTSettings(ctx);
    vIO32WriteMsk(ctx, 0x00000268U, 0x2000U, 0x2000U);
    vIO32WriteMsk(ctx, 0x00000250U, 1, 1);
    vIO32WriteMsk(ctx, 0x0000017cU, 0, 0);
    vIO32WriteMsk(ctx, 0x012010f8U, 0, 1);
    vIO32WriteMsk(ctx, 0x01000668U, 0x400U, 0x400U);
    vPhyByteIO32WriteMsk(ctx, 0x03800110U, 0x08000000U, 0x08200000U);
    vPhyByteIO32WriteMsk(ctx, 0x0000024cU, 0x200U, 0x300U);
    vIO32WriteMsk(ctx, 0x01000668U, 0x400U, 0x400U);
    vPhyByteIO32WriteMsk(ctx, 0x01000684U, 0x10000U, 0x10800U);
    vPhyByteIO32WriteMsk(ctx, 0x012010a4U, 0x02118000U, 0x0777c000U);
    vIO32WriteMsk(ctx, 0x01201128U, 1, 1);
    SetMck8xLowPwrOption(ctx);
}


void RODTSettings(void *ctx)
{
    U32 odt = raw_u32(ctx, 0x20);
    U32 unterm = !odt;
    U32 vref;
    U8 rank_bak;
    U8 rank;

    vPhyByteWriteFldAlign(ctx, 0x53a00f30U, unterm, 0x00040116U, 0);
    vPhyByteWriteFldAlign(ctx, 0x5ba00fb0U, unterm, 0x00040116U, 0);
    vIO32WriteMsk(ctx, 0x11000508U, 0, 0x10000U);
    vIO32WriteMsk(ctx, 0x19000588U, 0x10000U, 0x10000U);

    if (is_ddr4_family(ctx))
        vref = (odt == 1U) ? 0x16U : 0x0eU;
    else
        vref = is_ddr3_family(ctx);

    rank_bak = u1GetRank(ctx);
    for (rank = 0; (U32)rank < raw_u32(ctx, 0x08); rank++) {
        vSetRank(ctx, rank);
        vIO32WriteMsk(ctx, 0x91200eecU, vref, 0x3fU);
        vIO32WriteMsk(ctx, 0x99200f6cU, vref, 0x3fU);
    }
    vSetRank(ctx, rank_bak);

    vIO32WriteMsk(ctx, 0x012010a8U, odt, 1);
    vIO32WriteMsk(ctx, 0x91200f04U, odt << 15, 0x8000U);
    vIO32WriteMsk(ctx, 0x99200f84U, odt << 15, 0x8000U);
    vIO32WriteMsk(ctx, 0xa3a01004U, odt << 15, 0x8000U);

    vPhyByteWriteFldAlign(ctx, 0x51200f34U, unterm, 0x00040100U, 0);
    vPhyByteWriteFldAlign(ctx, 0x99200fb4U, unterm, 0x00040100U, 0);
    vPhyByteWriteFldAlign(ctx, 0x89200eb4U, unterm, 0xff040100U, 0);
    vPhyByteWriteFldAlign(ctx, 0x51200f38U, 0, 0xff04010bU, 0);
    vPhyByteWriteFldAlign(ctx, 0x99200fb8U, 0, 0xff04010bU, 0);
}


void EnableCommonDCMShuffle(void *ctx)
{
    vPhyByteWriteFldAlign(ctx, 0x89200eb4U, 0, 0xff040111U, 1);
    vPhyByteIO32WriteMsk_All(ctx, 0x01201120U, 0x300U, 0x100U);
    vPhyByteIO32WriteMsk_All(ctx, 0x11200f08U, 0x01200000U, 0x01200000U);
    vIO32WriteMsk_All(ctx, 0x01201124U, 0x33403000U, 0xffffffffU);
    vPhyByteIO32WriteMsk_All(ctx, 0x19200f88U, 0x01200000U, 0x01200000U);
}


void EnableDramcPhyDCMShuffle(void *ctx, U32 enable)
{
    EnableCommonDCMShuffle(ctx);
    if (enable) {
        vPhyByteIO32WriteMsk_All(ctx, 0x0020168cU, 0, 0x0000000fU);
        vIO32WriteMsk_All(ctx, 0x002016acU, 0x80000000U, 0x80000000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x09200e88U, 0, 0xfc000000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x11200f08U, 0, 0xfdd80000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x19200f88U, 0, 0xfde80000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x23a01008U, 0, 0xfcd80000U);
        vIO32WriteMsk_All(ctx, 0x012010a8U, 0, 0x2U);
        vIO32WriteMsk_All(ctx, 0x01201120U, 0x20U, 0x20U);
        vIO32WriteMsk_All(ctx, 0x01201124U, 0x334f3000U, 0xffffffffU);
    } else {
        vPhyByteIO32WriteMsk_All(ctx, 0x11200f08U, 0xfcd80000U, 0xfcd80000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x09200e88U, 0xfc300000U, 0xfc300000U);
        vIO32WriteMsk_All(ctx, 0x002016acU, enable, 0x80000000U);
        vPhyByteIO32WriteMsk_All(ctx, 0x19200f88U, 0xfcd80000U, 0xfcd80000U);
        vIO32WriteMsk_All(ctx, 0x012010a8U, enable, 0x2U);
    }
}


/* Recovered from AN7583 dramc_pi_basic_api.o .text.DramcDRVinitSetting. */
void DramcDRVinitSetting(void *ctx)
{
    if (is_ddr3_family(ctx)) {
        vPhyByteIO32WriteMsk(ctx, 0x090004bcU, 0x1e1e1616U, 0x3f3f3f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x090004c0U, 0x16161616U, 0x3f3f3f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x090004c4U, 0x00001e1eU, 0x00003f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x1100053cU, 0x1e1e1e1eU, 0x3f3f3f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x11000544U, 0x00001e1eU, 0x00003f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x190005bcU, 0x1e1e1e1eU, 0x3f3f3f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x012010d0U, 0x18c6318cU, 0x3fffffffU);
        vPhyByteIO32WriteMsk(ctx, 0x012010d4U, 0x18c42108U, 0x3fffffffU);
        vPhyByteIO32WriteMsk(ctx, 0x190005c4U, 0x00001e1eU, 0x00003f3fU);
        return;
    }

#define W(_reg, _val, _mask) vIO32WriteMsk_All(ctx, (_reg), (_val), (_mask))
    W(0x090004bcU, 0x00001e00U, 0x00003f00U);
    W(0x090004bcU, 0x0000001eU, 0x0000003fU);
    W(0x090004bcU, 0x26000000U, 0x3f000000U);
    W(0x090004bcU, 0x00260000U, 0x003f0000U);

    W(0x090004c0U, 0x00002600U, 0x00003f00U);
    W(0x090004c0U, 0x00000026U, 0x0000003fU);
    W(0x090004c0U, 0x26000000U, 0x3f000000U);
    W(0x090004c0U, 0x00260000U, 0x003f0000U);

    W(0x090004c4U, 0x00002600U, 0x00003f00U);
    W(0x090004c4U, 0x00000026U, 0x0000003fU);

    W(0x1100053cU, 0x00001e00U, 0x00003f00U);
    W(0x1100053cU, 0x00000026U, 0x0000003fU);
    W(0x1100053cU, 0x1e000000U, 0x3f000000U);
    W(0x1100053cU, 0x00260000U, 0x003f0000U);

    W(0x11000544U, 0x00001e00U, 0x00003f00U);
    W(0x11000544U, 0x00000026U, 0x0000003fU);

    W(0x190005bcU, 0x00001e00U, 0x00003f00U);
    W(0x190005bcU, 0x00000026U, 0x0000003fU);
    W(0x190005bcU, 0x1e000000U, 0x3f000000U);
    W(0x190005bcU, 0x00260000U, 0x003f0000U);

    W(0x190005c4U, 0x00001e00U, 0x00003f00U);
    W(0x190005c4U, 0x00000026U, 0x0000003fU);

    W(0x012010d0U, 0x0000000dU, 0x0000001fU);
    W(0x012010d0U, 0x000001a0U, 0x000003e0U);
    W(0x012010d0U, 0x00003400U, 0x00007c00U);
    W(0x012010d0U, 0x00068000U, 0x000f8000U);
    W(0x012010d0U, 0x00d00000U, 0x01f00000U);
    W(0x012010d0U, 0x1a000000U, 0x3e000000U);

    W(0x012010d4U, 0x0000000dU, 0x0000001fU);
    W(0x012010d4U, 0x000001a0U, 0x000003e0U);
    W(0x012010d4U, 0x00003400U, 0x00007c00U);
    W(0x012010d4U, 0x00068000U, 0x000f8000U);
    W(0x012010d4U, 0x00d00000U, 0x01f00000U);
    W(0x012010d4U, 0x1a000000U, 0x3e000000U);
#undef W
}
