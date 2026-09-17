/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Airoha AN75xx DRAMC utility recovery core.
 *
 * This file contains only routines whose semantics were recovered directly
 * from the vendor dramc_utility.o oracle.  It intentionally uses raw context
 * offsets until the original PCDDR DRAMC_CTX_T headers are recovered.
 *
 * Do not confuse this with the MT8195-derived dramc_utility.c next to it.
 */
#include "recovery_abi.h"

extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern U16 FreqByMPLLRG(U8 dram_type);

extern U8 gUpdateHighestFreq;
extern const U8 *gFreqTbl;

U16 gddrphyfmeter_value;
U8 u1MaType = 2;

static __attribute__((always_inline)) inline void dsb_sy(void)
{
#if defined(__arm__) || defined(__thumb__)
    __asm__ volatile("dsb sy" ::: "memory");
#else
    __asm__ volatile("" ::: "memory");
#endif
}

void CKE_FIX_ON(void *ctx, U32 option, U32 rank)
{
    if (rank == 0)
        vPhyByteIO32WriteMsk(ctx, 0x204, (option << 6) & 0x40, 0x40);
    else if (rank == 1)
        vPhyByteIO32WriteMsk(ctx, 0x204, (option << 4) & 0x10, 0x10);
}

void CKE_FIX_OFF(void *ctx, U32 option, U32 rank)
{
    if (rank == 0)
        vPhyByteIO32WriteMsk(ctx, 0x204, (option << 7) & 0x80, 0x80);
    else if (rank == 1)
        vPhyByteIO32WriteMsk(ctx, 0x204, (option << 5) & 0x20, 0x20);
}

void SetClkFreeRun(void *ctx, U32 enable)
{
    if (enable == 1) {
        vPhyByteIO32WriteMsk(ctx, 0x1fc, 0, 0x2);
        vPhyByteIO32WriteMsk(ctx, 0x1fc, 0, 0x40000000);
        vPhyByteIO32WriteMsk(ctx, 0x1fc, 0x40, 0x40);
    } else if (enable == 0) {
        vPhyByteIO32WriteMsk(ctx, 0x1fc, 0, 0x40);
        vPhyByteIO32WriteMsk(ctx, 0x1fc, 0x40000000, 0x40000000);
        vPhyByteIO32WriteMsk(ctx, 0x1fc, 0x2, 0x2);
    }
}

U32 is_ddr3_family(void *ctx)
{
    return raw_u32(ctx, 0x18) == 3;
}

U32 is_ddr4_family(void *ctx)
{
    return raw_u32(ctx, 0x18) == 4;
}

U32 GetDramcBroadcast(void)
{
    return (*mmio32(0x1fb00074) >> 18) & 1;
}

void DramcBroadcastOnOff(U32 on)
{
    U32 v = *mmio32(0x1fb00074);
    v &= ~0x40000U;
    if (on == 1)
        v |= 0x40000U;
    *mmio32(0x1fb00074) = v;
    dsb_sy();
}

void vSetPHY2ChannelMapping(void *ctx, U8 channel)
{
    raw_set_u32(ctx, 0x04, channel);
}

U8 vGetPHY2ChannelMapping(void *ctx)
{
    return raw_u8(ctx, 0x04);
}

void vSetRank(void *ctx, U8 rank)
{
    raw_set_u32(ctx, 0x0c, rank);
}

U8 u1GetRank(void *ctx)
{
    return raw_u8(ctx, 0x0c);
}

void vSetRankNumber(void *ctx)
{
    raw_set_u32(ctx, 0x08, 1);
}

void setFreqGroup(void *ctx)
{
    U16 f = raw_u16(ctx, 0x54);
    U16 group;

    if (f <= 400) group = 400;
    else if (f <= 533) group = 533;
    else if (f <= 600) group = 600;
    else if (f < 668) group = 667;
    else if (f <= 800) group = 800;
    else if (f <= 933) group = 933;
    else if (f <= 1066) group = 1066;
    else if (f <= 1200) group = 1200;
    else if (f <= 1333) group = 1333;
    else if (f <= 1467) group = 1467;
    else group = 1600;

    raw_set_u16(ctx, 0x56, group);
}

U16 DDRPhyFMeter(void)
{
    return gddrphyfmeter_value;
}

U32 vGet_DDR_Loop_Mode(void *ctx)
{
    return raw_u32((const void *)(uintptr_t)raw_u32(ctx, 0x90), 0x14);
}

U32 vGet_Div_Mode(void *ctx)
{
    return raw_u32((const void *)(uintptr_t)raw_u32(ctx, 0x90), 0x04);
}

U32 vGet_Current_ShuLevel(void *ctx)
{
    return raw_u32((const void *)(uintptr_t)raw_u32(ctx, 0x90), 0x08);
}

U32 Get_Duty_Calibration_Mode(void *ctx)
{
    return raw_u32((const void *)(uintptr_t)raw_u32(ctx, 0x90), 0x0c);
}

U32 Get_Vref_Calibration_OnOff(void *ctx)
{
    return raw_u32((const void *)(uintptr_t)raw_u32(ctx, 0x90), 0x10);
}

U32 vGet_Dram_CBT_Mode(void *ctx)
{
    U32 ranks = raw_u32(ctx, 0x08);
    U32 rk0 = raw_u32(ctx, 0x24);

    if (ranks == 2)
        return (rk0 != 0 || raw_u32(ctx, 0x28) != 0) ? 1 : 0;
    return rk0 != 0 ? 1 : 0;
}

void vPrintCalibrationBasicInfo(void *ctx)
{
    (void)ctx;
}

U16 GetFreqBySel(void *ctx, U32 sel)
{
    (void)ctx;
    switch (sel) {
    case 0: return 1600;
    case 1: return 1467;
    case 2: return 1333;
    case 3: return 1200;
    case 4: return 1066;
    case 5: return 933;
    case 6: return 800;
    case 7: return 600;
    case 8: return 533;
    case 9: return 400;
    case 10: return 1066;
    case 11: return 933;
    case 12: return 800;
    case 13: return 667;
    case 14: return 533;
    case 15: return 400;
    default: return 0;
    }
}

U16 GetDataRateByFreq(void *ctx)
{
    switch (raw_u16(ctx, 0x54)) {
    case 400:  return 800;
    case 533:  return 1066;
    case 600:  return 1200;
    case 667:  return 1333;
    case 800:  return 1600;
    case 933:  return 1866;
    case 1066: return 2133;
    case 1200: return 2400;
    case 1333: return 2667;
    case 1467: return 2933;
    case 1600: return 3200;
    case 1866: return 3733;
    case 2133: return 4266;
    default:   return 0;
    }
}

U32 GetSelByFreq(void *ctx, U16 freq)
{
    U32 type = raw_u32(ctx, 0x18);

    if (type == 4) {
        switch (freq) {
        case 1600: return 0;
        case 1467: return 1;
        case 1333: return 2;
        case 1200: return 3;
        case 1066: return 4;
        case 933:  return 5;
        case 800:  return 6;
        case 600:  return 7;
        case 533:  return 8;
        case 400:  return 9;
        default:   return 0;
        }
    }

    if (type == 3) {
        switch (freq) {
        case 1066: return 10;
        case 933:  return 11;
        case 800:  return 12;
        case 667:  return 13;
        case 533:  return 14;
        case 400:  return 15;
        default:   return 0;
        }
    }

    return 0;
}


void DDRPhyFreqSel(void *ctx, U32 sel)
{
    U16 freq = GetFreqBySel(ctx, sel);
    U32 type;

    raw_set_u32(ctx, 0x10, sel);
    raw_set_u16(ctx, 0x54, freq);
    type = raw_u32(ctx, 0x18);

    if (type == 4)
        raw_set_u32(ctx, 0x20, freq > 1065 ? 1 : 0);
    else if (type == 3)
        raw_set_u32(ctx, 0x20, 1);

    setFreqGroup(ctx);
}

U16 DDRPhyGetRealFreq(void *ctx)
{
    U16 meter = gddrphyfmeter_value;
    U32 divmode;
    U32 mul;

    if (meter == 0)
        return raw_u16(ctx, 0x54);

    divmode = vGet_Div_Mode(ctx);
    mul = divmode == 1 ? 4 : 2;
    return (U16)((meter >> 2) * mul);
}

static U16 u2FreqMax;

U16 u2DFSGetHighestFreq(void *ctx)
{
    U32 i;
    U16 max = u2FreqMax;
    (void)ctx;

    if (max == 0 || gUpdateHighestFreq) {
        gUpdateHighestFreq = 0;
        max = 0;
        for (i = 0; i < 7; i++) {
            U32 sel = raw_u32(gFreqTbl, i * 24);
            U16 freq = GetFreqBySel(ctx, sel);
            if (max < freq)
                max = freq;
        }
        u2FreqMax = max;
    }
    return u2FreqMax;
}

U8 GetEyeScanEnable(void *ctx, U8 get_type)
{
    (void)ctx;
    (void)get_type;
    return 0;
}


/* --- v6: additional oracle-derived utility routines --- */
extern void vPhyByteIO32WriteMsk_All(void *ctx, U32 reg, U32 value, U32 mask);
extern void vIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vIO32WriteMsk_All(void *ctx, U32 reg, U32 value, U32 mask);
extern U32 u4Dram_Register_Read(void *ctx, U32 reg);
extern U32 vPhyByteReadFldAlign(void *ctx, U32 reg, U32 field);
extern void vPhyByteWriteFldAlign(void *ctx, U32 reg, U32 value, U32 field, U32 all_channels);
extern void __meta_backup_and_set(void *ctx, U8 type, U8 value);
extern void __meta_advance(void *ctx, U8 type);
extern U32 __meta_get(void *ctx, U8 type);
extern void __meta_restore(void *ctx, U8 type);
extern void udelay(U32 usec);
extern int printf(const char *fmt, ...);
extern void *memset(void *dst, int c, size_t n);
extern U32 uartDisable;

U32 DramcEngine2Init(void *ctx, U32 a, U32 b, U8 pattern, U8 loop_count, U8 enable_ui_shift);
void DramcEngine2SetPat(void *ctx, U8 pattern, U8 loop_count, U8 len1, U8 enable_ui_shift);
extern U32 DramcEngine2Run(void *ctx, U32 op, U8 pattern);
extern void DramcEngine2End(void *ctx);

struct airoha_reg_backup_buf {
    U32 normal[100];
    U8 normal_backup_idx;
    U8 normal_restore_idx;
    U8 _pad0[2];
    U32 mixed[64];
    U8 mixed_backup_idx;
    U8 mixed_restore_idx;
    U8 _pad1[2];
};

static struct airoha_reg_backup_buf gRegBackupBuff;

typedef char reg_backup_size_must_be_664[(sizeof(gRegBackupBuff) == 664) ? 1 : -1];

void CKEFixOnOff(void *ctx, U8 rank, U8 option, U8 all_channels)
{
    U32 on, off;

    if (option == 2) {
        on = 0;
        off = 0;
    } else {
        on = option;
        off = (U8)(1U - option);
    }

    /* This PCDDR build is single-rank.  TO_ALL_RANK (2) aliases rank 0. */
    if ((rank & (U8)~2U) != 0)
        return;

    if (all_channels == 1)
        vPhyByteIO32WriteMsk_All(ctx, 0x204, (off << 7) | (on << 6), 0xc0);
    else
        vPhyByteIO32WriteMsk(ctx, 0x204, (off << 7) | (on << 6), 0xc0);
}

void vAutoRefreshSwitch(void *ctx, U8 option)
{
    U32 rank = raw_u32(ctx, 0x0c);

    raw_set_u32(ctx, 0x0c, 0);
    if (option == 1) {
        vIO32WriteMsk(ctx, 0x40056c, 0, 1);
    } else {
        vIO32WriteMsk(ctx, 0x40056c, 1, 1);
        raw_set_u32(ctx, 0x0c, 0);
        udelay(((u4Dram_Register_Read(ctx, 0x800080) >> 24) & 0xf) << 2);
    }
    raw_set_u32(ctx, 0x0c, (U8)rank);
}

void vCKERankCtrl(void *ctx, U32 mode)
{
    U32 dependent = mode != 0;
    U32 independent = dependent ? 0 : 1;
    U32 val;

    vIO32WriteMsk_All(ctx, 0x228, dependent << 12, 0x1000);

    val = (independent << 10) |
          (independent << 24) |
          (dependent << 13) |
          (independent << 14);
    vPhyByteIO32WriteMsk_All(ctx, 0x204, val, 0x0300ef02);

    val = (independent << 29) | 0x80000000U;
    vPhyByteIO32WriteMsk_All(ctx, 0x00201610, val, 0xa0000000U);
    vIO32WriteMsk_All(ctx, 0x190, 0, 0x200);
}

void DramcSetRankEngine2(void *ctx, U8 rank)
{
    vIO32WriteMsk(ctx, 0x108, 0x20, 0x20);
    vIO32WriteMsk(ctx, 0x10c, 0, 0x70000000U);
    vIO32WriteMsk(ctx, 0x10c, ((U32)rank << 24), 0x03000000U);
}



void DramcEngine2CheckComplete(void *ctx, U8 status)
{
    U32 timeout = 100001;
    U32 shift_ui = (u4Dram_Register_Read(ctx, 0x100) >> 25) & 1U;

    while (1) {
        U32 done = vPhyByteReadFldAlign(ctx, 0x00800120U, 0);

        if ((status & ~done) == 0) {
            if (shift_ui != 0) {
                U32 matched = 1;
                if ((U16)u4Dram_Register_Read(ctx, 0x00800170U) == 2U) {
                    if (matched != 0)
                        goto check_test_mode;
                }
            } else {
check_test_mode:
                if ((u4Dram_Register_Read(ctx, 0x108) & 0x8000U) == 0)
                    return;

                {
                    U32 retry = 100001;
                    while (--retry != 0) {
                        if (vPhyByteReadFldAlign(ctx, 0x00800170U, 0) == 8U)
                            return;
                    }
                    return;
                }
            }
        } else if (shift_ui != 0) {
            U32 matched = 0;
            if ((U16)u4Dram_Register_Read(ctx, 0x00800170U) == 2U && matched != 0)
                goto check_test_mode;
        }

        udelay(1);
        if (--timeout == 0)
            goto check_test_mode;
        udelay(1);
    }
}


U32 DramcEngine2Compare(void *ctx, U32 wr)
{
    U32 loop_count = u4Dram_Register_Read(ctx, 0x108) & 0xfU;
    U32 shift_ui = (u4Dram_Register_Read(ctx, 0x108) >> 15) & 1U;
    U32 test2_a0 = u4Dram_Register_Read(ctx, 0x100);
    U8 status = loop_count == 1 ? 3 : 1;

    if ((shift_ui | wr) == 0 && ((test2_a0 >> 25) & 1U) == 0) {
        DramcEngine2CheckComplete(ctx, status);
        vPhyByteIO32WriteMsk(ctx, 0x108, 0, 0xe0000000U);
        udelay(1);
        vPhyByteIO32WriteMsk(ctx, 0x108, 0x40000000U, 0xe0000000U);
    }

    DramcEngine2CheckComplete(ctx, status);
    return (vPhyByteReadFldAlign(ctx, 0x00800120, 0) >> 4) & status;
}

U32 DramcEngine2Run(void *ctx, U32 wr, U8 pattern)
{
    U32 result;

    if (wr == 1) {
        if (pattern == 1 || pattern == 2)
            vIO32WriteMsk(ctx, 0x10c, 0, 0x8000);
        vPhyByteIO32WriteMsk(ctx, 0x108, 0x40000000U, 0xe0000000U);
    } else if (wr == 0) {
        vPhyByteIO32WriteMsk(ctx, 0x108, 0x80000000U, 0xe0000000U);
    }

    DramcEngine2Compare(ctx, wr);
    udelay(1);
    result = vPhyByteReadFldAlign(ctx, 0x00800124, 0);
    vPhyByteIO32WriteMsk(ctx, 0x108, 0, 0xe0000000U);
    return result;
}

U32 TestEngineCompare(void *ctx)
{
    U32 pattern = raw_u32(ctx, 0x50);
    U32 result;

    if (pattern <= 2) {
        DramcEngine2Init(ctx, raw_u32(ctx, 0x48), raw_u32(ctx, 0x4c),
                         (U8)pattern, 0, 0);
        result = DramcEngine2Run(ctx, 0, raw_u8(ctx, 0x50));
        DramcEngine2End(ctx);
        return result;
    }

    if (pattern == 7) {
        DramcEngine2Init(ctx, raw_u32(ctx, 0x48), raw_u32(ctx, 0x4c), 1, 0, 0);
        result = DramcEngine2Run(ctx, 0, 1);
        DramcEngine2End(ctx);
        DramcEngine2Init(ctx, raw_u32(ctx, 0x48), raw_u32(ctx, 0x4c), 2, 0, 0);
        result |= DramcEngine2Run(ctx, 0, 2);
    } else {
        DramcEngine2Init(ctx, raw_u32(ctx, 0x48), raw_u32(ctx, 0x4c), 2, 0, 0);
        result = DramcEngine2Run(ctx, 0, 2);
    }

    DramcEngine2End(ctx);
    return result;
}

void TA2_Test_Run_Time_HW_Read(void *ctx, U8 enable)
{
    U32 old_channel = raw_u32(ctx, 0x04);
    U8 ch = 0;
    U32 channels = raw_u32(ctx, 0x00);

    while (ch < channels) {
        raw_set_u32(ctx, 0x04, ch++);
        vIO32WriteMsk(ctx, 0x108, (U32)enable << 30, 0x40000000U);
    }
    raw_set_u32(ctx, 0x04, old_channel);
}

void TA2_Test_Run_Time_HW_Write(void *ctx, U8 enable)
{
    U32 old_channel = raw_u32(ctx, 0x04);
    U8 ch = 0;
    U32 channels = raw_u32(ctx, 0x00);

    while (ch < channels) {
        raw_set_u32(ctx, 0x04, ch++);
        vIO32WriteMsk(ctx, 0x108, (U32)enable << 31, 0x80000000U);
    }
    raw_set_u32(ctx, 0x04, old_channel);
}

U8 u1NeedSkipDPMRG(void *ctx, U32 reg)
{
    U32 type = (reg >> 23) & 0xf;
    U8 channels_plus_one = (U8)(raw_u8(ctx, 0x00) + 1U);
    U32 channel = raw_u32(ctx, 0x04);

    if (raw_u8(ctx, 0xb6) == 0 && type == 4)
        return channel >= (channels_plus_one >> 1);
    if (type == 0)
        return channel >= (channels_plus_one >> 1);
    return 0;
}

static __attribute__((noinline)) void DramcBackupRegisters_Single(void *ctx, const U32 *regs, U32 count)
{
    U32 i;
    for (i = 0; i != count; i++) {
        U32 reg = regs[i];
        if (u1NeedSkipDPMRG(ctx, reg))
            continue;
        gRegBackupBuff.normal[gRegBackupBuff.normal_backup_idx++] =
            vPhyByteReadFldAlign(ctx, reg, 0);
    }
}

static __attribute__((noinline)) void DramcRestoreRegisters_Single(void *ctx, const U32 *regs, U32 count)
{
    U32 i;
    for (i = 0; i != count; i++) {
        U32 reg = regs[i];
        if (u1NeedSkipDPMRG(ctx, reg))
            continue;
        vPhyByteWriteFldAlign(ctx, reg,
            gRegBackupBuff.normal[gRegBackupBuff.normal_restore_idx++],
            0, 0);
    }
}

static U32 mixed_mask(U32 field)
{
    U32 shift = field & 0xffU;
    U32 width = (field >> 8) & 0xffU;
    if (width == 0)
        return 0;
    return (0xffffffffU >> (32U - width)) << shift;
}

static __attribute__((noinline)) void DramcBackupMixedRG_Single(void *ctx, const U32 *desc, U32 count)
{
    U32 i;
    for (i = 0; i < count; i++) {
        U32 reg = desc[i * 2];
        U32 field = desc[i * 2 + 1];
        U32 value;

        if (((field >> 18) & 3U) != 1U) {
            U32 shift = field & 0xffU;
            U32 mask = mixed_mask(field);
            value = (u4Dram_Register_Read(ctx, reg) & mask) >> shift;
        } else {
            value = vPhyByteReadFldAlign(ctx, reg, field);
        }
        gRegBackupBuff.mixed[gRegBackupBuff.mixed_backup_idx++] = value;
    }
}

static __attribute__((noinline)) void DramcRestoreMixedRG_Single(void *ctx, const U32 *desc, U32 count)
{
    U32 i;
    for (i = 0; i < count; i++) {
        U32 reg = desc[i * 2];
        U32 field = desc[i * 2 + 1];
        U32 value = gRegBackupBuff.mixed[gRegBackupBuff.mixed_restore_idx++];

        if (((field >> 18) & 3U) != 1U) {
            U32 shift = field & 0xffU;
            U32 mask = mixed_mask(field);
            vIO32WriteMsk(ctx, reg, value << shift, mask);
        } else {
            vPhyByteWriteFldAlign(ctx, reg, value, field, 0);
        }
    }
}

void DramcModeRegRead(void *ctx, U8 mr, U16 *value)
{
    vIO32WriteMsk(ctx, 0x128, (U32)mr << 8, 0x001fff00U);
    vIO32WriteMsk(ctx, 0x124, 0x1000, 0x1000);
    while ((u4Dram_Register_Read(ctx, 0x00800088) & 2U) == 0)
        udelay(1);
    *value = (U16)u4Dram_Register_Read(ctx, 0x0080008c);
    vIO32WriteMsk(ctx, 0x124, 0, 0x1000);
}

void DramcModeRegReadByRank(void *ctx, U8 rank, U8 mr, U16 *value)
{
    U32 old = u4Dram_Register_Read(ctx, 0x128);
    U16 tmp = 0;
    vIO32WriteMsk(ctx, 0x128, (U32)rank << 24, 0x03000000U);
    DramcModeRegRead(ctx, mr, &tmp);
    *value = tmp;
    vIO32WriteMsk(ctx, 0x128, old & 0x03000000U, 0x03000000U);
}



struct airoha_rtswcmd {
    U32 command;
    U32 rank;
    U8 arg8;
    U8 _pad9;
    U16 arg_a;
    U16 result_c;
    U8 result_ext;
    U8 _pad_f;
    U32 result_10;
};

typedef char rtswcmd_size_must_be_20[(sizeof(struct airoha_rtswcmd) == 20) ? 1 : -1];

void DramcTriggerRTSWCMD(void *ctx, void *opaque)
{
    struct airoha_rtswcmd *cmd = (struct airoha_rtswcmd *)opaque;
    U32 response = 0;
    U32 timeout = 100;

    vPhyByteIO32WriteMsk(ctx, 0x130, (cmd->rank << 10) & 0xc00, 0xfff);
    vIO32WriteMsk(ctx, 0x460, 0, 0x700000);

    if (cmd->command == 4) {
        vIO32WriteMsk(ctx, 0x130, ((U32)cmd->arg_a << 20) & 0x0ff00000U,
                      0x0ff00000U);
        vPhyByteIO32WriteMsk(ctx, 0x4ec,
            ((U32)(cmd->arg8 & 7U)) | (((U32)cmd->arg_a << 16) & 0x3f000000U),
            0x3f00000fU);
    } else if (cmd->command == 8) {
        vIO32WriteMsk(ctx, 0x130, (U32)cmd->arg8 << 12, 0x000ff000U);
    } else if (cmd->command == 14) {
        vIO32WriteMsk(ctx, 0x4ec, 0, 0xf);
        vIO32WriteMsk(ctx, 0x130, 0, 0x0ff00000U);
    }

    vIO32WriteMsk(ctx, 0x174, 0x80, 0x80);
    vIO32WriteMsk(ctx, 0x2d4, 0x30, 0xffffffffU);
    vIO32WriteMsk(ctx, 0x124, cmd->command << 26, 0xfc000000U);
    vIO32WriteMsk(ctx, 0x124, 0x02000000U, 0x02000000U);

    do {
        response = u4Dram_Register_Read(ctx, 0x00800050) & 1U;
        timeout--;
        udelay(1);
        if (response)
            break;
    } while (timeout != 0);

    if (!response || timeout == 0) {
        vIO32WriteMsk(ctx, 0x124, 0, 0x02000000U);
        return;
    }

    if (cmd->command == 8) {
        cmd->result_c = (U16)u4Dram_Register_Read(ctx, 0x00800020);
    } else if (cmd->command == 9 || cmd->command == 10 || cmd->command == 14) {
        U32 v = vPhyByteReadFldAlign(ctx, 0x00800014, 0);
        cmd->result_10 = (U16)v;
        if (cmd->result_ext != 0) {
            U32 a = u4Dram_Register_Read(ctx, 0x00800018);
            U32 b = u4Dram_Register_Read(ctx, 0x00800018);
            U32 ext = ((a | (b >> 2)) << 16) & 0x00030000U;
            cmd->result_10 |= ext;
        }
    }

    vIO32WriteMsk(ctx, 0x124, 0, 0x02000000U);
}

void DramcSetRWOFOEN(void *ctx, U8 enable)
{
    U32 timeout = 100001;
    for (;;) {
        U32 status = u4Dram_Register_Read(ctx, 0x00800080);
        if ((status >> 2) & 1U)
            break;
        udelay(1);
        if (--timeout == 0)
            break;
    }
    vIO32WriteMsk(ctx, 0x17c, enable, 1);
}

static U32 uiReg0D0h;
void DramcEngine2End(void *ctx)
{
    vPhyByteIO32WriteMsk(ctx, 0x10c, 0, 0x20000);
    DramcSetRWOFOEN(ctx, 1);
    vPhyByteWriteFldAlign(ctx, 0x110, uiReg0D0h, 0, 0);
}

struct rtswcmd_mrw_v6 {
    U32 command;
    U32 rank;
    U8 mr;
    U8 _pad0;
    U16 value;
    U8 _pad1[8];
};

__attribute__((noinline)) void DramcModeRegWriteByRank_RTSWCMD_MRW(void *ctx, U8 rank, U8 mr, U16 value)
{
    struct rtswcmd_mrw_v6 cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = 4;
    cmd.rank = rank;
    cmd.mr = mr;
    cmd.value = value;
    DramcTriggerRTSWCMD(ctx, &cmd);
}

void DramcModeRegWriteByRank(void *ctx, U8 rank, U8 mr, U16 value)
{
    printf("MRW RK%d MR#%d = 0x%x\n", rank, mr, value);
    DramcModeRegWriteByRank_RTSWCMD_MRW(ctx, rank, mr, value);
}

void DramcBackupRegisters(void *ctx, const U32 *regs, U32 count, U8 all_channels)
{
    U8 channels = raw_u8(ctx, 0x00);
    if (raw_u32(ctx, 0x44) == 0x20)
        channels++;
    if (all_channels != 1) {
        DramcBackupRegisters_Single(ctx, regs, count);
        return;
    }
    __meta_backup_and_set(ctx, 0, 0);
    do {
        DramcBackupRegisters_Single(ctx, regs, count);
        __meta_advance(ctx, 0);
    } while (__meta_get(ctx, 0) < channels);
    __meta_restore(ctx, 0);
}

void DramcRestoreRegisters(void *ctx, const U32 *regs, U32 count, U8 all_channels)
{
    U8 channels = raw_u8(ctx, 0x00);
    if (raw_u32(ctx, 0x44) == 0x20)
        channels++;
    if (all_channels != 1) {
        DramcRestoreRegisters_Single(ctx, regs, count);
        gRegBackupBuff.normal_backup_idx = 0;
        gRegBackupBuff.normal_restore_idx = 0;
        return;
    }
    __meta_backup_and_set(ctx, 0, 0);
    do {
        DramcRestoreRegisters_Single(ctx, regs, count);
        __meta_advance(ctx, 0);
    } while (__meta_get(ctx, 0) < channels);
    __meta_restore(ctx, 0);
    gRegBackupBuff.normal_backup_idx = 0;
    gRegBackupBuff.normal_restore_idx = 0;
}

void DramcBackupMixedRG(void *ctx, const U32 *desc, U32 count, U8 all_channels)
{
    U8 channels = raw_u8(ctx, 0x00);
    if (raw_u32(ctx, 0x44) == 0x20)
        channels++;
    if (all_channels != 1) {
        DramcBackupMixedRG_Single(ctx, desc, count);
        return;
    }
    __meta_backup_and_set(ctx, 0, 0);
    do {
        DramcBackupMixedRG_Single(ctx, desc, count);
        __meta_advance(ctx, 0);
    } while (__meta_get(ctx, 0) < channels);
    __meta_restore(ctx, 0);
}

void DramcRestoreMixedRG(void *ctx, const U32 *desc, U32 count, U8 all_channels)
{
    U8 channels = raw_u8(ctx, 0x00);
    if (raw_u32(ctx, 0x44) == 0x20)
        channels++;
    if (all_channels != 1) {
        DramcRestoreMixedRG_Single(ctx, desc, count);
        gRegBackupBuff.mixed_backup_idx = 0;
        gRegBackupBuff.mixed_restore_idx = 0;
        return;
    }
    __meta_backup_and_set(ctx, 0, 0);
    do {
        DramcRestoreMixedRG_Single(ctx, desc, count);
        __meta_advance(ctx, 0);
    } while (__meta_get(ctx, 0) < channels);
    __meta_restore(ctx, 0);
    gRegBackupBuff.mixed_backup_idx = 0;
    gRegBackupBuff.mixed_restore_idx = 0;
}


void DramcEngine2SetPat(void *ctx, U8 pattern, U8 loop_count, U8 len1, U8 enable_ui_shift)
{
    U32 loop = (U32)loop_count & 0xfU;

    if (pattern == 2 || pattern == 9) {
        if (len1 != 0) {
            vPhyByteIO32WriteMsk(ctx, 0x10c, 0x20000, 0x20000);
            DramcSetRWOFOEN(ctx, 0);
        } else {
            vPhyByteIO32WriteMsk(ctx, 0x10c, 0, 0x20000);
            DramcSetRWOFOEN(ctx, 1);
        }

        vPhyByteIO32WriteMsk(ctx, 0x108, loop, 0x0007008fU);
        vPhyByteIO32WriteMsk(ctx, 0x10c, 0x10000, 0x0001c000U);
        vPhyByteIO32WriteMsk(ctx, 0x10c, pattern == 9 ? 0x40 : 0, 0xc0);
    } else if (pattern == 1) {
        vPhyByteIO32WriteMsk(ctx, 0x10c, 0, 0x20000);
        vPhyByteIO32WriteMsk(ctx, 0x10c, 0x510d, 0x0001df1fU);
        vPhyByteIO32WriteMsk(ctx, 0x108, loop | 0x80, 0x0007008fU);
    } else if (pattern == 3) {
        vPhyByteIO32WriteMsk(ctx, 0x10c,
                             ((U32)len1 << 17) & 0x20000,
                             0x00031f5fU);
        vPhyByteIO32WriteMsk(ctx, 0x108, loop | 0x70000, 0x0007008fU);
        vIO32WriteMsk(ctx, 0x104, 0x560, 0xfffffff0U);
    } else {
        vPhyByteIO32WriteMsk(ctx, 0x10c, 0, 0x20000);
        vPhyByteIO32WriteMsk(ctx, 0x108, loop, 0x0007008fU);
        vPhyByteIO32WriteMsk(ctx, 0x10c, 0, 0x10000);
    }

    if (enable_ui_shift == 1) {
        vPhyByteIO32WriteMsk(ctx, 0x100, 0x340000, 0x00f40000U);
        vPhyByteIO32WriteMsk(ctx, 0x108, 0x8000, 0x8800);
    } else {
        vPhyByteIO32WriteMsk(ctx, 0x100, 0, 0x00f40000U);
        vPhyByteIO32WriteMsk(ctx, 0x108, 0, 0x8000);
    }
}


static U8 u1Pat = 2;
static U32 err_count;
static U32 pass_count;

void TA2_Test_Run_Time_Pat_Setting(void *ctx, U8 pat_switch)
{
    U32 old_channel = raw_u32(ctx, 0x04);
    U8 ch = 0;
    U32 zero = 0;

    while (ch < raw_u32(ctx, 0x00)) {
        raw_set_u32(ctx, 0x04, ch++);
        DramcEngine2SetPat(ctx, u1Pat, (U8)(raw_u32(ctx, 0x08) - 1U), 0, (U8)zero);
    }

    raw_set_u32(ctx, 0x04, old_channel);
    {
        U32 worst = (u1Pat == 3) ? 1U : 0U;
        vIO32WriteMsk_All(ctx, 0x100, worst << 18, 0x40000);
        vIO32WriteMsk_All(ctx, 0x100, worst << 16, 0x10000);
        vIO32WriteMsk_All(ctx, 0x100, worst << 17, 0x20000);
        vIO32WriteMsk_All(ctx, 0x10c, worst << 17, 0x20000);
    }

    if (pat_switch)
        u1Pat = (U8)((u1Pat + 1U) & 3U);
}

void TA2_Show_Cnt(void *ctx, U32 error_value)
{
    U32 err = err_count;
    U32 pass = pass_count;
    U32 rk = 0;
    U8 err_changed = 0;
    U8 pass_changed = 0;

    while ((U8)rk < raw_u32(ctx, 0x08)) {
        if (error_value & (1U << (U8)rk)) {
            err++;
            err_changed = 1;
        } else {
            pass++;
            pass_changed = 1;
        }
        rk++;
    }

    if (err_changed)
        err_count = err;
    if (pass_changed)
        pass_count = pass;
}


U32 DramcEngine2Init(void *ctx, U32 test2_1, U32 test2_2, U8 pattern,
                     U8 loop_count, U8 enable_ui_shift)
{
    U8 len1;
    if (ctx == NULL || loop_count > 15)
        return 1;

    DramcSetRankEngine2(ctx, raw_u8(ctx, 0x0c));
    uiReg0D0h = vPhyByteReadFldAlign(ctx, 0x110, 0);
    vPhyByteIO32WriteMsk(ctx, 0x110, 0, 0x00f000c0U);
    vPhyByteIO32WriteMsk(ctx, 0x108, 0, 0xe0000000U);
    vPhyByteIO32WriteMsk(ctx, 0x100,
                         ((test2_1 >> 24) << 8) | (test2_2 >> 24),
                         0xffff);
    vIO32WriteMsk(ctx, 0x00400500U, (test2_1 << 3) & 0x07fffff8U,
                  0xfffffff8U);
    vIO32WriteMsk(ctx, 0x104, (test2_2 << 4) & 0x0ffffff0U,
                  0xfffffff0U);

    len1 = (U8)(pattern >> 7);
    DramcEngine2SetPat(ctx, (U8)(pattern & 0x7f), loop_count, len1,
                       enable_ui_shift);
    return 0;
}

void TA2_Test_Run_Time_SW_Presetting(void *ctx, U32 test2_1, U32 test2_2,
                                     U8 pattern, U8 loop_count)
{
    (void)pattern;
    (void)loop_count;
    DramcSetRankEngine2(ctx, raw_u8(ctx, 0x0c));
    uiReg0D0h = vPhyByteReadFldAlign(ctx, 0x110, 0);
    vPhyByteIO32WriteMsk(ctx, 0x110, 0, 0x00f000c0U);
    vPhyByteIO32WriteMsk(ctx, 0x108, 0, 0xe0000000U);
    vPhyByteIO32WriteMsk(ctx, 0x100,
                         ((test2_1 >> 24) << 8) | (test2_2 >> 24),
                         0xffff);
    vIO32WriteMsk(ctx, 0x00400500U, 0, 0xfffffff8U);
    vIO32WriteMsk(ctx, 0x104, 0x20, 0xfffffff0U);
}

void TA2_Test_Run_Time_HW_Presetting(void *ctx, U32 len, U32 rksel_mode)
{
    U32 old_channel = raw_u32(ctx, 0x04);
    U32 old_rank = raw_u32(ctx, 0x0c);
    U32 offset = (rksel_mode == 3) ? (len >> 4) : (len >> 5);
    U32 mode_value = rksel_mode << 28;
    U8 rwofoen = (rksel_mode == 3) ? 0 : 1;
    U8 ch = 0;

    if (offset < 1)
        offset = 1;
    if (u4Dram_Register_Read(ctx, 0x10c) & 0x20000U)
        offset -= 2;

    while ((U8)ch < raw_u32(ctx, 0x00)) {
        U8 rk = 0;
        raw_set_u32(ctx, 0x04, ch);
        while ((U8)rk < raw_u32(ctx, 0x08)) {
            raw_set_u32(ctx, 0x0c, rk++);
            vIO32WriteMsk(ctx, 0x00400500U, 0x100000, 0xfffffff8U);
        }
        if (raw_u32(ctx, 0x18) == 4 && offset >= 0x200)
            offset = 0x200;
        vIO32WriteMsk(ctx, 0x104, offset << 4, 0xfffffff0U);
        vIO32WriteMsk(ctx, 0x10c, mode_value, 0x70000000U);
        DramcSetRWOFOEN(ctx, rwofoen);
        ch++;
    }
    raw_set_u32(ctx, 0x04, old_channel);
    raw_set_u32(ctx, 0x0c, old_rank);
}

U32 TA2_Test_Run_Time_HW_Status(void *ctx)
{
    U32 old_channel = raw_u32(ctx, 0x04);
    U32 bit_error = 0;
    U8 ch = 0;

    while ((U8)ch < raw_u32(ctx, 0x00)) {
        U32 error_value;
        raw_set_u32(ctx, 0x04, ch++);
        if (u4Dram_Register_Read(ctx, 0x100) & 0x40000U) {
            U32 status = ((u4Dram_Register_Read(ctx, 0x108) & 0xfU) == 1U) ? 3U : 0U;
            vIO32WriteMsk(ctx, 0x100, 0, 0x10000);
            DramcEngine2CheckComplete(ctx, (U8)status);
            error_value = (vPhyByteReadFldAlign(ctx, 0x00800120U, 0) >> 4) & 3U;
        } else {
            error_value = DramcEngine2Compare(ctx, 0);
        }
        TA2_Show_Cnt(ctx, error_value);
        bit_error |= vPhyByteReadFldAlign(ctx, 0x00800124U, 0);
        vPhyByteIO32WriteMsk(ctx, 0x108, 0, 0xe0000000U);
    }
    raw_set_u32(ctx, 0x04, (U8)old_channel);
    return bit_error;
}

void TA2_Test_Run_Time_HW(void *ctx)
{
    U32 old_channel = raw_u32(ctx, 0x04);
    U32 old_rank = raw_u32(ctx, 0x0c);
    TA2_Test_Run_Time_HW_Presetting(ctx, 0x10000, 4);
    TA2_Test_Run_Time_Pat_Setting(ctx, 0);
    TA2_Test_Run_Time_HW_Write(ctx, 1);
    (void)TA2_Test_Run_Time_HW_Status(ctx);
    raw_set_u32(ctx, 0x04, old_channel);
    raw_set_u32(ctx, 0x0c, old_rank);
}
