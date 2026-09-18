/* SPDX-License-Identifier: BSD-3-Clause */
/* Oracle-derived Airoha PCDDR calibration API recovery core. */
#include "recovery_abi.h"

typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef struct {
    U32 reg;
    U32 field;
} REG_TRANSFER_T;

extern void *memset(void *, int, size_t);
extern void *memcpy(void *, const void *, size_t);
extern U8 ef_read_byte(U32 index);
extern U32 u4Dram_Register_Read(void *ctx, U32 reg);
extern U32 vGet_Div_Mode(void *ctx);
extern U32 vPhyByteReadFldAlign(void *ctx, U32 reg, U32 field);
extern void vIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void __meta_backup_and_set(void *ctx, U8 type, U8 value);
extern void __meta_advance(void *ctx, U8 type);
extern U32 __meta_process_complete(void *ctx, U8 type);
extern void __meta_restore(void *ctx, U8 type);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vPhyByteIO32WriteMsk_All(void *ctx, U32 reg, U32 value, U32 mask);
extern void vPhyByteWriteFldAlign(void *ctx, U32 reg, U32 value,
                                  U32 field, U32 channel_mask);
extern void vIO32WriteMsk_All(void *ctx, U32 reg, U32 value, U32 mask);
extern U32 is_ddr4_family(void *ctx);
extern U32 is_ddr3_family(void *ctx);
extern U32 pkg_type;
extern int printf(const char *fmt, ...);
extern void DramcTriggerRTSWCMD(void *ctx, void *opaque);
extern void vSetCalibrationResult(void *ctx, U8 cal_type, U8 result);
extern void DramcModeRegWriteByRank(void *ctx, U8 rank, U8 mr, U16 value);
extern U16 gMRVal[];
extern void vPrintCalibrationBasicInfo(void *ctx);
extern void vAutoRefreshSwitch(void *ctx, U32 enable);
extern U32 DramcEngine2Init(void *ctx, U32 test2_1, U32 test2_2, U8 pattern,
                             U8 loop_count, U8 enable_ui_shift);
extern U32 DramcEngine2Run(void *ctx, U32 wr, U8 pattern);
extern void DramcEngine2End(void *ctx);
extern void DramPhyReset(void *ctx);
void dle_factor_handler(void *ctx, U8 value);
extern void udelay(U32 usec);
static void _LoopAryToDelay(void *ctx, REG_TRANSFER_T *ui_reg,
                             REG_TRANSFER_T *mck_reg, U8 count,
                             S8 shift_ui, U8 byte_idx);
void DramcImpedanceSetValue(void *ctx, U32 code, U32 bit5, U32 type);
extern U32 vGet_DDR_Loop_Mode(void *ctx);
extern void CKEFixOnOff(void *ctx, U8 rank, U8 option, U8 all_channels);
extern void DramcBackupRegisters(void *ctx, const U32 *regs, U32 count,
                                  U8 all_channels);
extern void DramcRestoreRegisters(void *ctx, const U32 *regs, U32 count,
                                   U8 all_channels);
extern void DramcBackupMixedRG(void *ctx, const U32 *desc, U32 count,
                                U8 all_channels);
extern void DramcRestoreMixedRG(void *ctx, const U32 *desc, U32 count,
                                 U8 all_channels);
extern U32 GetDramcBroadcast(void);
extern void DramcBroadcastOnOff(U32 on);
extern U8 u1GetRank(void *ctx);
extern void vSetRank(void *ctx, U8 rank);
extern U8 r_filter_count[];
/*
 * The vendor object indexes this as wrlevel_dqs_final_delay[lane + rank*4]
 * (a flat word array), which does not obviously match the "static S32
 * wrlevel_dqs_final_delay[RANK_MAX][DQS_BYTE_NUMBER]" shape declared in
 * the still-PUBLIC_BASE dramc_pi_calibration_api.c -- kept flat here to
 * match the object rather than assume that public-lineage shape is right.
 */
extern S32 wrlevel_dqs_final_delay[];

void PCDDR_ShiftDQSUI(void *ctx, S8 shift_ui, U8 byte_idx)
{
    REG_TRANSFER_T ui[] = {
        {0x00601284U, 0x00000400U},
        {0x00601284U, 0x00000404U},
        {0x00601284U, 0x00000408U},
        {0x00601284U, 0x0000040cU},
    };
    REG_TRANSFER_T mck[] = {
        {0x00601280U, 0x00000400U},
        {0x00601280U, 0x00000404U},
        {0x00601280U, 0x00000408U},
        {0x00601280U, 0x0000040cU},
    };

    _LoopAryToDelay(ctx, ui, mck, 4, shift_ui, byte_idx);
}

void PCDDR_ShiftDQS_OENUI(void *ctx, S8 shift_ui, U8 byte_idx)
{
    REG_TRANSFER_T ui[] = {
        {0x00601284U, 0x00000410U},
        {0x00601284U, 0x00000414U},
        {0x00601284U, 0x00000418U},
        {0x00601284U, 0x0000041cU},
    };
    REG_TRANSFER_T mck[] = {
        {0x00601280U, 0x00000410U},
        {0x00601280U, 0x00000414U},
        {0x00601280U, 0x00000418U},
        {0x00601280U, 0x0000041cU},
    };

    _LoopAryToDelay(ctx, ui, mck, 4, shift_ui, byte_idx);
}

U8 get_gating_start_pos(void *ctx)
{
    U8 a = (U8)u4Dram_Register_Read(ctx, 0x11600a2cU);
    U8 b = (U8)u4Dram_Register_Read(ctx, 0x19600aacU);
    U8 v = (a > b) ? a : b;

    if (v > 2)
        v -= 3;
    return v;
}

void Dramc_efuse_read_parse(U32 start_bit, U32 bit_len, U8 *dst)
{
    U32 i;
    U32 bytes = (bit_len >> 3) + ((bit_len & 7U) != 0U);

    memset(dst, 0, bytes);
    for (i = 0; i < bit_len; i++) {
        U32 src_bit = start_bit + i;
        U8 b = ef_read_byte(src_bit >> 3);
        U8 bit = (U8)((b >> (src_bit & 7U)) & 1U);
        dst[i >> 3] |= (U8)(bit << (i & 7U));
    }
}

void DramcImpedanceEfuseValue(void *ctx, U32 efuse_value, U32 type)
{
    U32 code;
    U32 bit5;

    if (!(efuse_value & (1U << 6)))
        return;

    code = efuse_value & 0x1fU;
    bit5 = (efuse_value >> 5) & 1U;

    __meta_backup_and_set(ctx, 0, 0);
    do {
        DramcImpedanceSetValue(ctx, code, bit5, type);
        __meta_advance(ctx, 0);
    } while (!__meta_process_complete(ctx, 0));
    __meta_restore(ctx, 0);
}

S8 DutyScan_Offset_Convert(U32 index)
{
    U8 table[15] = {
        0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    };
    U8 v = table[index];

    if (v > 8)
        v = (U8)(-(S8)(v & 7U));
    return (S8)v;
}

/*
 * on_off != 1 asserts PHY register 0x21c bits [21:20]; on_off == 1 clears
 * them. rank == 1 selects the *_All (broadcast) write, any other value the
 * single-target write. Byte-identical AN7581/AN7583.
 */
void CmdOEOnOff(void *ctx, U32 on_off, U32 rank)
{
    U32 value = (on_off != 1U) ? 0x300000U : 0U;
    U32 mask = 0x300000U;
    U32 reg = 0x21cU;

    if (rank == 1U)
        vPhyByteIO32WriteMsk_All(ctx, reg, value, mask);
    else
        vPhyByteIO32WriteMsk(ctx, reg, value, mask);
}

/* Broadcasts delay into all four byte lanes and programs both RX DQ delay
 * cell registers for the given byte lane. Byte-identical AN7581/AN7583. */
void SetRxDqDelay(void *ctx, U32 byte_idx, U8 delay)
{
    U32 val = (U32)delay | ((U32)delay << 8) | ((U32)delay << 16) |
              ((U32)delay << 24);

    vPhyByteWriteFldAlign(ctx, 0x116009f8U + 4U * byte_idx, val, 0U, 1U);
    vPhyByteWriteFldAlign(ctx, 0x19600a78U + 4U * byte_idx, val, 0U, 1U);
}

/* Vendor object reduces this export to a bare `bx lr` (no relocations, no
 * observable effect). Confirmed no-op on both SoCs. */
void Get_RX_DelayCell(void)
{
}

/* Returns the number of UI subdivisions per MCK cycle: 4 (shift 2) or
 * 8 (shift 3), selected by the current MCK/UI clock-divider mode. */
U8 u1MCK2UI_DivShift(void *ctx)
{
    return (U8)((vGet_Div_Mode(ctx) != 2U) ? 3U : 2U);
}

/*
 * Applies shift_ui delay steps (positive or negative) to the packed
 * UI/MCK delay-cell field pair described by ui_reg[lane]/mck_reg[lane],
 * for lane values starting at byte_idx and advancing by "stride" while
 * lane < count. Every current caller passes count == 4 and stride
 * resolves to 4, so the loop always runs exactly once per call, over
 * the single entry addressed by byte_idx.
 *
 * Each packed field word encodes:
 *   bits[19:18] : 1 selects the PHY-space accessors (vPhyByteReadFldAlign/
 *                 vPhyByteWriteFldAlign); otherwise the generic MMIO
 *                 accessors (u4Dram_Register_Read/vIO32WriteMsk) are used
 *   bits[15:8]  : field width in bits
 *   bits[7:0]   : field bit position
 *
 * AN7581 has no further gating on the MMIO path (this differs from
 * AN7583, which additionally treats bits[31:24] == 0xff as "field
 * absent"; see the AN7583 recovered-core for that variant).
 */
static void _LoopAryToDelay(void *ctx, REG_TRANSFER_T *ui_reg,
                      REG_TRANSFER_T *mck_reg, U8 count,
                      S8 shift_ui, U8 byte_idx)
{
    U32 lane;
    U32 stride;

    switch (byte_idx) {
    case 0:
    case 1:
    case 2:
        lane = byte_idx;
        stride = 4;
        break;
    case 3:
        lane = 3;
        stride = 4;
        break;
    default:
        lane = 0;
        stride = 1;
        break;
    }

    while (count > lane) {
        U32 ui_reg_addr = ui_reg[lane].reg;
        U32 ui_field = ui_reg[lane].field;
        U32 mck_reg_addr = mck_reg[lane].reg;
        U32 mck_field = mck_reg[lane].field;
        U32 ui_width = (ui_field >> 8) & 0xffU;
        U32 ui_pos = ui_field & 0xffU;
        U32 mck_width = (mck_field >> 8) & 0xffU;
        U32 mck_pos = mck_field & 0xffU;
        U8 div = u1MCK2UI_DivShift(ctx);
        U32 ui_val;
        U32 mck_val;
        S32 total;
        U32 new_ui;
        U32 new_mck;

        if (((ui_field >> 18) & 3U) == 1U)
            ui_val = vPhyByteReadFldAlign(ctx, ui_reg_addr, ui_field);
        else {
            U32 raw = u4Dram_Register_Read(ctx, ui_reg_addr);
            U32 mask = (0xffffffffU >> (32U - ui_width)) << ui_pos;

            ui_val = (raw & mask) >> ui_pos;
        }

        if (((mck_field >> 18) & 3U) == 1U)
            mck_val = vPhyByteReadFldAlign(ctx, mck_reg_addr, mck_field);
        else {
            U32 raw = u4Dram_Register_Read(ctx, mck_reg_addr);
            U32 mask = (0xffffffffU >> (32U - mck_width)) << mck_pos;

            mck_val = (raw & mask) >> mck_pos;
        }

        total = (S32)((mck_val << div) + ui_val) + (S32)shift_ui;
        if (total < 0) {
            new_mck = 0;
            new_ui = 0;
        } else {
            new_mck = (U32)total >> div;
            new_ui = (U32)total - (new_mck << div);
        }

        if (((ui_field >> 18) & 3U) == 1U)
            vPhyByteWriteFldAlign(ctx, ui_reg_addr, new_ui, ui_field, 0);
        else {
            U32 mask = (0xffffffffU >> (32U - ui_width)) << ui_pos;

            vIO32WriteMsk(ctx, ui_reg_addr, new_ui << ui_pos, mask);
        }

        if (((mck_field >> 18) & 3U) == 1U)
            vPhyByteWriteFldAlign(ctx, mck_reg_addr, new_mck, mck_field, 0);
        else {
            U32 mask = (0xffffffffU >> (32U - mck_width)) << mck_pos;

            vIO32WriteMsk(ctx, mck_reg_addr, new_mck << mck_pos, mask);
        }

        lane += stride;
    }
}

/*
 * Reads the current field value (via the generic MMIO accessors, or via
 * vPhyByteReadFldAlign when field's bits[19:18] select PHY space),
 * converts it to a resistance-like magnitude using a weighted-bit sum
 * (base 10000 + 2500/5000/10000/20000/40000 per set bit 0-4, plus 80000
 * for bit 5 when type is neither 2 nor 3, i.e. for the 6-bit-wide DRVN/
 * DRVP/ODTN/ODTP fields rather than the 5-bit-wide ones), multiplies by
 * "code" and divides by 1175 to get a grade 0-8 via a fixed threshold
 * table, then either adds that grade to the original field value
 * (bit5 == 1) or subtracts it (otherwise), and writes the clamped byte
 * result back to the same field.
 *
 * AN7581 has no "field absent" sentinel on the MMIO path (see
 * _LoopAryToDelay for the AN7583 variant that adds one).
 */
void DramcImpedanceDrvSetRG(void *ctx, U32 reg, U32 field, U32 code,
                             U8 bit5, U8 type)
{
    U32 width = (field >> 8) & 0xffU;
    U32 pos = field & 0xffU;
    U32 space = (field >> 18) & 3U;
    U32 raw;
    S32 sum;
    S32 grade;
    U32 result;

    if (space == 1U)
        raw = vPhyByteReadFldAlign(ctx, reg, field);
    else {
        U32 v = u4Dram_Register_Read(ctx, reg);
        U32 mask = (0xffffffffU >> (32U - width)) << pos;

        raw = (v & mask) >> pos;
    }
    raw &= 0xffU;

    sum = 10000
        + 2500  * (S32)((raw >> 0) & 1U)
        + 5000  * (S32)((raw >> 1) & 1U)
        + 10000 * (S32)((raw >> 2) & 1U)
        + 20000 * (S32)((raw >> 3) & 1U)
        + 40000 * (S32)((raw >> 4) & 1U);
    if (type != 2U && type != 3U)
        sum += 80000 * (S32)((raw >> 5) & 1U);

    grade = (S32)((code * (U32)sum) / 1175U);
    if (grade <= 49)
        grade = 0;
    else if (grade <= 149)
        grade = 1;
    else if (grade <= 249)
        grade = 2;
    else if (grade < 350)
        grade = 3;
    else if (grade < 450)
        grade = 4;
    else if (grade <= 549)
        grade = 5;
    else if (grade <= 649)
        grade = 6;
    else if (grade <= 749)
        grade = 7;
    else
        grade = 8;

    if (bit5 == 1U)
        result = (U8)((S32)raw + grade);
    else
        result = (U8)((S32)raw - grade);

    if (space == 1U)
        vPhyByteWriteFldAlign(ctx, reg, result, field, 1U);
    else {
        U32 mask = (0xffffffffU >> (32U - width)) << pos;

        vIO32WriteMsk_All(ctx, reg, result << pos, mask);
    }
}

/*
 * type == 2: 6-lane, 5-bit ODT-code fields at reg 0x012010d0/0x012010d4,
 * bit positions {5,15,25} on each register.
 * type == 3: same two registers, bit positions {0,10,20}; the vendor
 * object calls DramcImpedanceDrvSetRG(reg=0x012010d4, pos=20) twice
 * (once directly, once through a shared tail with type 0/1/2's last
 * call) -- preserved here exactly as compiled, not simplified away.
 * type == 0 / type == 1: 4 registers x 6-bit DRVN/DRVP/ODTN/ODTP fields
 * at bit positions {8,24} (type 0) or {0,16} (type 1), except the last
 * 3 registers which only get one of the two positions.
 */
void DramcImpedanceSetValue(void *ctx, U32 code, U32 bit5, U32 type)
{
    if (type == 2U) {
        DramcImpedanceDrvSetRG(ctx, 0x012010d0U, 0x505U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d0U, 0x50fU, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d0U, 0x519U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d4U, 0x505U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d4U, 0x50fU, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d4U, 0x519U, code, bit5, type);
    } else if (type == 3U) {
        DramcImpedanceDrvSetRG(ctx, 0x012010d0U, 0x500U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d0U, 0x50aU, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d4U, 0x514U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d4U, 0x500U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d4U, 0x50aU, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x012010d4U, 0x514U, code, bit5, type);
    } else if (type == 0U) {
        DramcImpedanceDrvSetRG(ctx, 0x1100053cU, 0x608U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x190005bcU, 0x608U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x1100053cU, 0x618U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x190005bcU, 0x618U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004bcU, 0x608U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004bcU, 0x618U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004c0U, 0x608U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004c0U, 0x618U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x11000544U, 0x608U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x190005c4U, 0x608U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004c4U, 0x608U, code, bit5, type);
    } else if (type == 1U) {
        DramcImpedanceDrvSetRG(ctx, 0x1100053cU, 0x600U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x190005bcU, 0x600U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x1100053cU, 0x610U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x190005bcU, 0x610U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004bcU, 0x600U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004bcU, 0x610U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004c0U, 0x600U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004c0U, 0x610U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x11000544U, 0x600U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x190005c4U, 0x600U, code, bit5, type);
        DramcImpedanceDrvSetRG(ctx, 0x090004c4U, 0x600U, code, bit5, type);
    } else {
        printf("Drv type error \n");
    }
}

/*
 * DRV/ODT impedance trim from efuse, gated by DDR type. AN7581 always
 * prints the read fuse bytes; AN7583 additionally gates those prints on
 * the global uartDisable flag (see the AN7583 recovered-core).
 */
void DramcImpedanceByEfuse(void *ctx)
{
    U8 fuse6 = 0;
    U8 fuse7 = 0;

    if (is_ddr4_family(ctx)) {
        Dramc_efuse_read_parse(0x29dU, 7, &fuse6);
        Dramc_efuse_read_parse(0x296U, 7, &fuse7);
    } else {
        if (!is_ddr3_family(ctx))
            return;
        Dramc_efuse_read_parse(0x28fU, 7, &fuse6);
        Dramc_efuse_read_parse(0x288U, 7, &fuse7);
    }

    if ((fuse6 >> 6) & 1U) {
        printf("DRVP driving setting info: %x\n", fuse6);
        printf("ODTP driving setting info: %x\n", fuse6);
        DramcImpedanceEfuseValue(ctx, fuse6, 0);
        DramcImpedanceEfuseValue(ctx, fuse6, 2);
    }

    if ((fuse7 >> 6) & 1U) {
        printf("DRVN driving setting info: %x\n", fuse7);
        printf("ODTN driving setting info: %x\n", fuse7);
        DramcImpedanceEfuseValue(ctx, fuse7, 1);
        DramcImpedanceEfuseValue(ctx, fuse7, 3);
    }
}

/*
 * AN7581-only in this object (AN7583's DramcDRVinitSetting instead lives
 * in dramc_pi_basic_api.o and has no pkg_type branch -- see that file's
 * simpler variant, which independently matches this function's DDR4/
 * pkg_type==0 constants exactly, cross-validating both recoveries).
 *
 * DDR3: 9 one-shot vPhyByteIO32WriteMsk writes, no pkg_type dependency.
 * DDR4 (or anything else is_ddr3_family() rejects): the 6 DRVN/DRVP/
 * ODTN/ODTP byte-lane registers get pkg_type-dependent default values
 * (0x1e vs 0x26 per lane), then both package types converge on the same
 * default ODT code (13) written to all twelve 5-bit lanes of
 * 0x012010d0/0x012010d4.
 */
void DramcDRVinitSetting(void *ctx)
{
    if (is_ddr3_family(ctx)) {
        vPhyByteIO32WriteMsk(ctx, 0x090004bcU, 0x1e1e1616U, 0x3f3f3f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x090004c0U, 0x16161616U, 0x3f3f3f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x090004c4U, 0x00001e1eU, 0x00003f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x1100053cU, 0x24242626U, 0x3f3f3f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x11000544U, 0x00002424U, 0x00003f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x190005bcU, 0x24242626U, 0x3f3f3f3fU);
        vPhyByteIO32WriteMsk(ctx, 0x012010d0U, 0x318c6318U, 0x3fffffffU);
        vPhyByteIO32WriteMsk(ctx, 0x012010d4U, 0x31842108U, 0x3fffffffU);
        vPhyByteIO32WriteMsk(ctx, 0x190005c4U, 0x00002424U, 0x00003f3fU);
        return;
    }

#define W(_reg, _val, _mask) vIO32WriteMsk_All(ctx, (_reg), (_val), (_mask))
    if (pkg_type == 0U) {
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
    } else {
        W(0x090004bcU, 0x00001e00U, 0x00003f00U);
        W(0x090004bcU, 0x0000001eU, 0x0000003fU);
        W(0x090004bcU, 0x26000000U, 0x3f000000U);
        W(0x090004bcU, 0x00260000U, 0x003f0000U);

        W(0x090004c0U, 0x00002600U, 0x00003f00U);
        W(0x090004c0U, 0x00000026U, 0x0000003fU);
        W(0x090004c0U, 0x1e000000U, 0x3f000000U);
        W(0x090004c0U, 0x001e0000U, 0x003f0000U);

        W(0x090004c4U, 0x00002600U, 0x00003f00U);
        W(0x090004c4U, 0x00000026U, 0x0000003fU);

        W(0x1100053cU, 0x00001e00U, 0x00003f00U);
        W(0x1100053cU, 0x0000001eU, 0x0000003fU);
        W(0x1100053cU, 0x1e000000U, 0x3f000000U);
        W(0x1100053cU, 0x001e0000U, 0x003f0000U);

        W(0x11000544U, 0x00001e00U, 0x00003f00U);
        W(0x11000544U, 0x0000001eU, 0x0000003fU);

        W(0x190005bcU, 0x00001e00U, 0x00003f00U);
        W(0x190005bcU, 0x0000001eU, 0x0000003fU);
        W(0x190005bcU, 0x1e000000U, 0x3f000000U);
        W(0x190005bcU, 0x001e0000U, 0x003f0000U);

        W(0x190005c4U, 0x00001e00U, 0x00003f00U);
        W(0x190005c4U, 0x0000001eU, 0x0000003fU);
    }

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

/*
 * Issues RTSWCMD opcode 12 for the given rank (see DramcTriggerRTSWCMD in
 * dramc_utility.c for the command dispatch; opcode 12 is not one of the
 * commands that function special-cases, so it just triggers the generic
 * wait-for-response path) and unconditionally reports calibration type 2
 * / result 0, with no check of the RTSWCMD response at all -- this is a
 * fire-and-forget trigger, not an actual pass/fail calibration loop.
 * The EN7523 GPL lineage header (reference/en7523/gpl-ddr-cal/
 * dramc_pi_api.h) would name these DRAM_CALIBRATION_CA_TRAIN / DRAM_OK,
 * but that enum ordering is not confirmed for this SoC's actual
 * dramc_common.h, so the raw values are kept instead of asserting a
 * possibly-wrong symbolic name. Byte-identical AN7581/AN7583.
 */
void DramcZQCalibration(void *ctx, U32 rank)
{
    struct airoha_rtswcmd cmd;

    memset(&cmd, 0, sizeof(cmd));
    cmd.command = 12;
    cmd.rank = rank;
    DramcTriggerRTSWCMD(ctx, &cmd);

    vSetCalibrationResult(ctx, 2, 0);
}

/*
 * DDR4-only (a no-op on DDR3): sequences a JEDEC-style MR6 VrefDQ
 * training write for the DRAM at ctx's current channel (offset 0x4) and
 * rank (offset 0xc): "range" (bit 6) and "vref_code" (bits 5:0) are
 * written with the training-enable bit (0x80) set, then again with it
 * held while vref_code is added, then a final write with the enable bit
 * cleared to latch the value. The low byte of the cached MR6 shadow
 * (gMRVal[], indexed the same way as DDR3/DDR4_dram_init.c) is replaced
 * with the final value; the upper byte is preserved. Note the shadow is
 * read unconditionally before the is_ddr4_family() check but only
 * written back (and only the MRWs issued) on DDR4. Byte-identical
 * AN7581/AN7583.
 */
void DramcTXSetVref(void *ctx, U32 range, U32 vref_code)
{
    U32 channel = raw_u32(ctx, 0x4);
    U32 rank = raw_u32(ctx, 0xc);
    U16 *shadow = &gMRVal[channel * 14U + rank * 7U + 6U];
    U16 base = (U16)(*shadow & 0xff00U);
    U16 value;

    if (!is_ddr4_family(ctx))
        return;

    value = (U16)(base | (range << 6) | 0x80U);
    DramcModeRegWriteByRank(ctx, (U8)rank, 6, value);

    value = (U16)(value | vref_code);
    DramcModeRegWriteByRank(ctx, (U8)rank, 6, value);

    value = (U16)(value & ~0x80U);
    DramcModeRegWriteByRank(ctx, (U8)rank, 6, value);

    *shadow = value;
}

/*
 * Programs a DATLAT delay code into three packed 5-bit lanes of
 * 0x012010b8 (adjusted by -1 unless status register 0x012010ec bit 0 is
 * set or value == 0, in which case the raw value is used unmodified),
 * then programs a 3-level threshold-crossing range select (value > 7,
 * > 13, > 18) into 0x0020168c, and finally tail-calls DramPhyReset().
 * Byte-identical AN7581/AN7583.
 */
void dle_factor_handler(void *ctx, U8 value)
{
    U32 status = u4Dram_Register_Read(ctx, 0x012010ecU);
    U32 v = ((status & 1U) != 0U || value == 0U) ? value : (U32)(value - 1U);
    U32 packed = ((v & 0x1fU) << 16) | ((v & 0x1fU) << 8) | (value & 0x1fU);
    U32 gt7 = (value > 7U) ? 1U : 0U;
    U32 gt13 = (value > 13U) ? 1U : 0U;
    U32 gt18 = (value > 18U) ? 1U : 0U;
    U32 range = gt18 | (gt13 << 3) | (gt13 << 2) | (gt7 << 1) | (gt7 << 4);

    vPhyByteIO32WriteMsk_All(ctx, 0x012010b8U, packed, 0x1f1f1fU);
    vPhyByteIO32WriteMsk_All(ctx, 0x0020168cU, range, 0x3fU);
    DramPhyReset(ctx);
}

/*
 * Scans the 32 UI positions of the DATLAT delay line for the first
 * contiguous run of passing DramcEngine2Run() comparisons (capped at a
 * run length of 5; once a run ends, later successes are NOT counted as
 * a new run -- this finds the first pass/fail/pass boundary, not the
 * widest window overall), then centers dle_factor_handler() on that run
 * (offset by run_len/2 for runs of <=3..4, or a flat +1 for longer
 * runs), or restores the pre-scan baseline and reports failure if
 * nothing ever passed.
 *
 * Returns 1 only for a NULL ctx; every other path returns 0 regardless
 * of whether calibration actually found a passing window (success/
 * failure is only reported through vSetCalibrationResult).
 *
 * Byte-identical control flow with AN7583 up to DramcEngine2End();
 * AN7583 additionally truncates the compare result to 8 bits when
 * ctx+0x44 == 8, and adds a rank-1 path that mirrors rank-0's saved
 * result into fixed hardware registers instead of re-measuring (see
 * the AN7583 recovered-core).
 */
U32 DramcRxdatlatCal(void *ctx)
{
    U32 phase = 0;
    U32 run_len = 0;
    U32 state = 0;
    U32 best_phase = 0xffU;
    U32 baseline;

    if (ctx == 0)
        return 1;

    vPrintCalibrationBasicInfo(ctx);
    vAutoRefreshSwitch(ctx, 1);
    baseline = vPhyByteReadFldAlign(ctx, 0x012010b8U, 0);
    vSetCalibrationResult(ctx, 0xc, 1);
    (void)u4Dram_Register_Read(ctx, 0x012010b8U);

    (void)DramcEngine2Init(ctx, raw_u32(ctx, 72), raw_u32(ctx, 76),
                            raw_u8(ctx, 0x50), 0, 1);

    for (;;) {
        dle_factor_handler(ctx, (U8)phase);

        if (DramcEngine2Run(ctx, 0, raw_u8(ctx, 0x50)) != 0) {
            if (state == 1U)
                state = 0xffU;
        } else if (state != 0xffU) {
            if (state == 0U)
                best_phase = phase;
            run_len = (U8)(run_len + 1U);
            if (run_len > 4U)
                break;
            state = 1U;
        }

        if (phase == 0x1fU)
            break;
        phase++;
    }

    DramcEngine2End(ctx);

    if (run_len == 0U) {
        vPhyByteWriteFldAlign(ctx, 0x012010b8U, baseline, 0, 1U);
        vSetCalibrationResult(ctx, 0xc, 1);
    } else {
        U32 center = best_phase;

        if (run_len <= 3U)
            center += run_len >> 1;
        else
            center += 1U;
        dle_factor_handler(ctx, (U8)center);
        vSetCalibrationResult(ctx, 0xc, 0);
    }

    vAutoRefreshSwitch(ctx, 0);
    return 0;
}

/*
 * Same _LoopAryToDelay wrapper pattern as PCDDR_ShiftDQSUI/OENUI, but
 * over all 8 DQ byte lanes (count 8) across two registers instead of
 * one (0x60120c/0x601208 for the UI table, 0x601204/0x601200 for MCK,
 * 4 lanes at field positions 0/4/8/12 each). Table content confirmed
 * against .rodata; byte-identical AN7581/AN7583.
 */
void ShiftDQUI(void *ctx, S8 shift_ui, U8 byte_idx)
{
    REG_TRANSFER_T ui[] = {
        {0x0060120cU, 0x00000400U},
        {0x0060120cU, 0x00000404U},
        {0x0060120cU, 0x00000408U},
        {0x0060120cU, 0x0000040cU},
        {0x00601208U, 0x00000400U},
        {0x00601208U, 0x00000404U},
        {0x00601208U, 0x00000408U},
        {0x00601208U, 0x0000040cU},
    };
    REG_TRANSFER_T mck[] = {
        {0x00601204U, 0x00000400U},
        {0x00601204U, 0x00000404U},
        {0x00601204U, 0x00000408U},
        {0x00601204U, 0x0000040cU},
        {0x00601200U, 0x00000400U},
        {0x00601200U, 0x00000404U},
        {0x00601200U, 0x00000408U},
        {0x00601200U, 0x0000040cU},
    };

    _LoopAryToDelay(ctx, ui, mck, 8, shift_ui, byte_idx);
}

/* Tail-jumps straight to ShiftDQUI in the vendor object (4-byte alias). */
void ShiftDQUI_AllRK(void *ctx, S8 shift_ui, U8 byte_idx)
{
    ShiftDQUI(ctx, shift_ui, byte_idx);
}

/* Same as ShiftDQUI but for the OE_N tables (field positions 0x10/0x14/
 * 0x18/0x1c on the same four registers). Byte-identical AN7581/AN7583. */
void ShiftDQ_OENUI(void *ctx, S8 shift_ui, U8 byte_idx)
{
    REG_TRANSFER_T ui[] = {
        {0x0060120cU, 0x00000410U},
        {0x0060120cU, 0x00000414U},
        {0x0060120cU, 0x00000418U},
        {0x0060120cU, 0x0000041cU},
        {0x00601208U, 0x00000410U},
        {0x00601208U, 0x00000414U},
        {0x00601208U, 0x00000418U},
        {0x00601208U, 0x0000041cU},
    };
    REG_TRANSFER_T mck[] = {
        {0x00601204U, 0x00000410U},
        {0x00601204U, 0x00000414U},
        {0x00601204U, 0x00000418U},
        {0x00601204U, 0x0000041cU},
        {0x00601200U, 0x00000410U},
        {0x00601200U, 0x00000414U},
        {0x00601200U, 0x00000418U},
        {0x00601200U, 0x0000041cU},
    };

    _LoopAryToDelay(ctx, ui, mck, 8, shift_ui, byte_idx);
}

/* Tail-jumps straight to ShiftDQ_OENUI in the vendor object (4-byte alias). */
void ShiftDQ_OENUI_AllRK(void *ctx, S8 shift_ui, U8 byte_idx)
{
    ShiftDQ_OENUI(ctx, shift_ui, byte_idx);
}

/* Applies the same UI shift to both the DQS and DQS_OEN delay chains. */
void ShiftDQSWCK_UI(void *ctx, S8 shift_ui, U8 byte_idx)
{
    PCDDR_ShiftDQSUI(ctx, shift_ui, byte_idx);
    PCDDR_ShiftDQS_OENUI(ctx, shift_ui, byte_idx);
}

/*
 * Turns the O1 (1x-frequency) datapath on/off: when enable == 1, first
 * asserts a block of registers that are otherwise left alone (their
 * mask equals "enable" itself, so writing 0 through this same path
 * later is a harmless no-op for those specific fields); the remaining
 * writes apply unconditionally with enable used directly as the field
 * value. Ends with a fixed 1us delay. Byte-identical AN7581/AN7583.
 */
void O1PathOnOff(void *ctx, U32 enable)
{
    U32 bit1 = (enable << 1) & 2U;
    U32 combo;

    if (enable == 1U) {
        vPhyByteWriteFldAlign(ctx, 0x51200f30U, enable, 0x00040116U, enable);
        vPhyByteWriteFldAlign(ctx, 0x59200fb0U, enable, 0x00040116U, enable);
        vIO32WriteMsk_All(ctx, 0x010007b0U, enable, enable);
        vIO32WriteMsk_All(ctx, 0x010007b4U, enable, enable);
        vIO32WriteMsk_All(ctx, 0x91200eecU, 0xeU, 0x3fU);
        vIO32WriteMsk_All(ctx, 0x99200f6cU, 0xeU, 0x3fU);
    }

    vPhyByteWriteFldAlign(ctx, 0x9100050cU, enable, 0x00040109U, 1U);
    vIO32WriteMsk_All(ctx, 0x91000500U, enable, 3U);
    vPhyByteWriteFldAlign(ctx, 0x9900058cU, enable, 0x00040109U, 1U);
    vIO32WriteMsk_All(ctx, 0x99000580U, enable, 3U);

    combo = bit1 | (U32)(U8)(enable << 7) | ((enable << 5) & 0x20U);
    vPhyByteIO32WriteMsk_All(ctx, 0x9100050cU, combo, 0xa2U);
    vPhyByteIO32WriteMsk_All(ctx, 0x9900058cU, combo, 0xa2U);

    vPhyByteWriteFldAlign(ctx, 0x91000528U, enable, 0x0004011dU, 1U);
    vPhyByteWriteFldAlign(ctx, 0x990005a8U, enable, 0x0004011dU, 1U);

    vIO32WriteMsk_All(ctx, 0x010007b0U, ((enable << 1) | enable) << 8, 0xf00U);

    udelay(1);
}

/*
 * Sets or clears the MR1 write-leveling-enable bit (0x80) in the cached
 * shadow and writes it out, plus a family-specific MR2 tweak on rank 1
 * (clears bits [10:9] on DDR3, bits [11:9] on DDR4) when enabling; on
 * disable, restores MR2's cached shadow value unconditionally after the
 * MR1 write. LPDDR/other families (neither DDR3 nor DDR4) are a no-op.
 * Byte-identical AN7581/AN7583.
 */
void vSetDramMRWriteLevelingOnOff(void *ctx, U32 enable)
{
    U32 channel = raw_u32(ctx, 0x4);
    U32 rank = raw_u32(ctx, 0xc);
    U16 *mr1_shadow = &gMRVal[channel * 14U + rank * 7U + 1U];
    U16 *mr2_shadow = &gMRVal[channel * 14U + rank * 7U + 2U];
    U16 value = *mr1_shadow;

    if (enable != 0U)
        value |= 0x80U;
    else
        value &= (U16)~0x80U;
    *mr1_shadow = value;

    if (is_ddr3_family(ctx)) {
        if (rank == 1U)
            DramcModeRegWriteByRank(ctx, (U8)rank, 2,
                                     (U16)(*mr2_shadow & ~0x600U));
        DramcModeRegWriteByRank(ctx, (U8)rank, 1, value);
        if (enable != 0U)
            return;
    } else {
        if (!is_ddr4_family(ctx))
            return;
        if (rank == 1U)
            DramcModeRegWriteByRank(ctx, (U8)rank, 2,
                                     (U16)(*mr2_shadow & ~0xe00U));
        DramcModeRegWriteByRank(ctx, (U8)rank, 1, value);
        if (enable != 0U)
            return;
    }

    DramcModeRegWriteByRank(ctx, (U8)rank, 2, *mr2_shadow);
}

/*
 * Write-leveling scan. Returns 1 only for a NULL ctx; every other path
 * returns 0 (pass/fail is only reported through vSetCalibrationResult).
 *
 * Structure (fully traced from the oracle disassembly):
 *  1. Print, select rank in reg 0x238, back up 9 registers + 3 mixed-RG
 *     descriptors (tables below, values read straight from .rodata).
 *  2. One-time per-channel init (gated on ctx-byte flag at channel+0x8c):
 *     shift every lane by -1 UI and zero two gating registers.
 *  3. vGet_DDR_Loop_Mode(ctx) selects (sweep_range, step_mult): mode 1
 *     -> (32,16), mode 2 -> (32,8), otherwise -> (64,1).
 *  4. PHY/CKE/O1-path setup, enable write-leveling mode.
 *  5. Zero the per-lane scan state and wrlevel_dqs_final_delay[] for
 *     lane 0..lane_count-1 (lane_count = data_width/8, at most 4 -- the
 *     four parallel per-lane byte arrays below only have room for 4).
 *  6. Outer sweep loop, round = 0..192 step step_mult: periodically nudge
 *     all lanes' coarse UI position, pulse reg 0x14c bit 7, sample a
 *     per-lane "high/low" bit from reg 0x01800180, and run one of two
 *     independent per-lane edge-detection state machines depending on
 *     pkg_type (see below), until every lane's bit is set in the
 *     completion bitmap or round exceeds 192.
 *  7. Unwind the cumulative coarse nudge from step 6.
 *  8. Report the result, disable write-leveling mode/O1-path, clear the
 *     reg 0x238 rank selection, restore the backed-up registers.
 *  9. For each lane: if the recorded raw position is >= sweep_range,
 *     apply the excess (scaled by sweep_range/32) as a further coarse
 *     shift to ALL lanes and wrap the recorded position into
 *     [0, sweep_range); then center it (+16, or -48 with a +2 UI
 *     correction on overflow) into a final 0-63 delay code.
 * 10. Write the four lanes' final delay codes packed into two companion
 *     PHY registers (0x11600a20/0x19600aa0), plus the raw wrapped
 *     positions into a third field of the same two registers; when
 *     data_width == 0x20 (4 active lanes), lanes 2-3 reuse the same two
 *     registers under a second __meta_backup_and_set(ctx,1,0) group.
 *
 * Two independent per-lane state machines (selected by pkg_type, not a
 * per-SoC difference -- both paths exist in every build):
 *
 *  pkg_type == 0: simple settle-then-count. State 0 waits for the
 *    sample to go low for >16 rounds (then advances to state 1); state
 *    N>=1 advances one step every round the sample reads high. As soon
 *    as state*step_mult > 7 (or round==191 with state>1), the lane is
 *    marked done and wrlevel_dqs_final_delay[] is set from the CURRENT
 *    round position.
 *
 *  pkg_type != 0: multi-pass edge confirmation (state 0-5, vendor
 *    dispatches via a jump table):
 *    0: on first sample, go to state 1 (sample low) or 2 (sample high).
 *    1: wait for a rising edge; on sample==1, record the current round
 *       as a candidate position and advance to state 3.
 *    2: wait for a falling edge; on sample==0, advance to state 4.
 *    3: confirm the state-1 candidate is a real edge, not a glitch: if
 *       the sample drops back to 0 immediately, reset to state 1; else
 *       count confirmations (metric = count*step_mult) until >7 (or
 *       round==191 with count>1), then require the *third* time this
 *       confirmation threshold is reached before finalizing (using the
 *       position recorded in state 1) and moving to state 5 -- the
 *       first two times, revert to state 2 to re-verify via another
 *       falling edge.
 *    4: confirm a stable low the same way (metric = count*step_mult);
 *       once confirmed, return to state 1 to watch for the real rising
 *       edge; if the sample goes high again first, revert to state 2.
 *    5: terminal (done); keeps re-marking the completion bit.
 *    >5: unreachable; prints "byte_%d is broken" and continues.
 */
U32 DramcWriteLeveling(void *ctx)
{
    static const U32 regs[9] = {
        0x14cU, 0x150U, 0x158U, 0x320U, 0x51200f30U,
        0x59200fb0U, 0x11000508U, 0x19000588U, 0x1fcU,
    };
    static const U32 mixed_rg[6] = {
        0x10007b0U, 0x100U, 0x10007b0U, 0x408U, 0x10007b4U, 0x100U,
    };
    U32 channel;
    U32 rank;
    U32 data_width;
    U32 lane_count;
    U32 sweep_range;
    U32 step_mult;
    U32 coarse_step;
    U32 round;
    U32 group = 0;
    U32 done_mask;
    U32 lane;
    U8 fail;
    U8 state[4] = {0};
    U8 settle[4] = {0};
    U8 confirm_c[4] = {0};
    U8 confirm_d[4] = {0};
    U8 pass_or_result[4] = {0};
    S32 saved_pos[4] = {0};

    if (ctx == 0)
        return 1;

    vPrintCalibrationBasicInfo(ctx);
    rank = raw_u32(ctx, 0xc);
    vIO32WriteMsk(ctx, 0x238U, rank, 3U);
    vIO32WriteMsk(ctx, 0x238U, 4U, 4U);

    DramcBackupRegisters(ctx, regs, 9, 1);
    DramcBackupMixedRG(ctx, mixed_rg, 3, 1);
    vSetCalibrationResult(ctx, 5, 1);

    channel = raw_u32(ctx, 0x4);
    if (raw_u8(ctx, channel + 0x8cU) == 0) {
        raw_set_u8(ctx, channel + 0x8cU, 1);
        ShiftDQUI(ctx, -1, 4);
        ShiftDQ_OENUI(ctx, -1, 4);
        ShiftDQSWCK_UI(ctx, -1, 4);
        vIO32WriteMsk_All(ctx, 0x11600a20U, 0, 0x3f000000U);
        vIO32WriteMsk_All(ctx, 0x19600aa0U, 0, 0x3f000000U);
    }

    if (vGet_DDR_Loop_Mode(ctx) == 1U) {
        sweep_range = 0x20U;
        step_mult = 0x10U;
    } else if (vGet_DDR_Loop_Mode(ctx) == 2U) {
        sweep_range = 0x20U;
        step_mult = 8U;
    } else {
        sweep_range = 0x40U;
        step_mult = 1U;
    }

    vPhyByteIO32WriteMsk(ctx, 0x1fcU, 0x3040U, 0xc0003042U);
    CKEFixOnOff(ctx, (U8)rank, 1, 0);
    O1PathOnOff(ctx, 1);
    vIO32WriteMsk(ctx, 0x14cU, 8U, 8U);
    vSetDramMRWriteLevelingOnOff(ctx, 1);
    udelay(1);
    vIO32WriteMsk(ctx, 0x158U, 0x28000U, 0x3c000U);
    vIO32WriteMsk(ctx, 0x14cU, 0x20U, 0x20U);
    data_width = raw_u32(ctx, 0x44);
    vIO32WriteMsk(ctx, 0x14cU, ((data_width == 0x20U) ? 0xfU : 3U) << 8, 0xf00U);
    udelay(1);

    data_width = raw_u32(ctx, 0x44);
    lane_count = data_width >> 3;
    for (lane = 0; lane < lane_count; lane++)
        wrlevel_dqs_final_delay[lane + rank * 4U] = 0;

    done_mask = (data_width == 0x10U) ? 0xfcU : 0xf0U;
    coarse_step = sweep_range >> 5;

    for (round = 0;;) {
        U32 live_bits;

        if (round / sweep_range == group + 1U) {
            group = round / sweep_range;
            ShiftDQSWCK_UI(ctx, (S8)coarse_step, 4);
        }

        if (round == 0) {
            vIO32WriteMsk_All(ctx, 0x11600a20U, 0, 0x3f000000U);
            vIO32WriteMsk_All(ctx, 0x19600aa0U, 0, 0x3f000000U);
        } else {
            U32 v = u4Dram_Register_Read(ctx, 0x096009a0U) & 0x3f000000U;

            vIO32WriteMsk_All(ctx, 0x096009a0U, v, 0x3f000000U);
        }

        vIO32WriteMsk(ctx, 0x14cU, 0x80U, 0x80U);
        vIO32WriteMsk(ctx, 0x14cU, 0, 0x80U);
        udelay(1);

        live_bits = u4Dram_Register_Read(ctx, 0x01800180U) &
                    ((1U << lane_count) - 1U);

        for (lane = 0; lane < lane_count; lane++) {
            U32 sample = (live_bits >> lane) & 1U;
            U32 lane_bit = 1U << lane;

            if (pkg_type != 0U) {
                switch (state[lane]) {
                case 0:
                    state[lane] = sample ? 2U : 1U;
                    break;
                case 1:
                    if (sample) {
                        saved_pos[lane] = (S32)round;
                        state[lane] = 3;
                    }
                    break;
                case 2:
                    if (!sample)
                        state[lane] = 4;
                    break;
                case 3:
                    if (!sample) {
                        confirm_c[lane] = 0;
                        state[lane] = 1;
                    } else {
                        U32 metric = (U32)confirm_c[lane] * step_mult;

                        if (metric > 7U ||
                            (round == 0xbfU && confirm_c[lane] > 1U)) {
                            pass_or_result[lane]++;
                            if (pass_or_result[lane] > 1U) {
                                wrlevel_dqs_final_delay[lane + rank * 4U] =
                                    saved_pos[lane];
                                state[lane] = 5;
                            } else {
                                state[lane] = 2;
                                confirm_c[lane] = 0;
                            }
                        } else {
                            confirm_c[lane]++;
                        }
                    }
                    break;
                case 4:
                    if (sample) {
                        confirm_d[lane] = 0;
                        state[lane] = 2;
                    } else {
                        U32 metric = (U32)confirm_d[lane] * step_mult;

                        if (metric > 7U)
                            state[lane] = 1;
                        else
                            confirm_d[lane]++;
                    }
                    break;
                case 5:
                    done_mask |= lane_bit;
                    break;
                default:
                    printf("byte_%d is broken", lane);
                    break;
                }
            } else {
                if (state[lane] == 0U) {
                    if (!sample) {
                        settle[lane]++;
                        if (settle[lane] > 16U)
                            state[lane] = 1U;
                    }
                } else if (sample) {
                    state[lane]++;
                }

                if (!(done_mask & lane_bit)) {
                    U32 metric = (U32)state[lane] * step_mult;

                    if (metric > 7U ||
                        (round == 0xbfU && state[lane] > 1U)) {
                        done_mask |= lane_bit;
                        wrlevel_dqs_final_delay[lane + rank * 4U] =
                            (S32)round - (S32)(step_mult * (state[lane] - 2U));
                    }
                }
            }
        }

        if (done_mask == 0xffU)
            break;
        round += step_mult;
        if (round > 0xc0U)
            break;
    }

    if (group != 0U) {
        S32 shift = -(S32)group * (S32)coarse_step;

        ShiftDQSWCK_UI(ctx, (S8)shift, 4);
    }

    fail = (U8)((done_mask == 0xffU) ? 0U : 1U);
    vSetCalibrationResult(ctx, 5, fail);
    vSetDramMRWriteLevelingOnOff(ctx, 0);
    vIO32WriteMsk(ctx, 0x14cU, 0, 8U);
    O1PathOnOff(ctx, 0);
    vIO32WriteMsk(ctx, 0x238U, 0, 3U);
    vIO32WriteMsk(ctx, 0x238U, 0, 4U);
    DramcRestoreRegisters(ctx, regs, 9, 1);
    DramcRestoreMixedRG(ctx, mixed_rg, 3, 1);

    for (lane = 0; lane < lane_count; lane++) {
        S32 *slot = &wrlevel_dqs_final_delay[lane + rank * 4U];

        if (*slot >= (S32)sweep_range) {
            S32 shift = (*slot / (S32)sweep_range) * (S32)coarse_step;

            ShiftDQSWCK_UI(ctx, (S8)shift, 4);
            *slot %= (S32)sweep_range;
        }
        saved_pos[lane] = *slot;
    }

    for (lane = 0; lane < lane_count; lane++) {
        S32 centered = saved_pos[lane] + 0x10;

        if (centered <= 0x3f) {
            pass_or_result[lane] = (U8)centered;
        } else {
            pass_or_result[lane] = (U8)(centered - 0x30);
            ShiftDQUI(ctx, 2, (U8)lane);
            ShiftDQ_OENUI(ctx, 2, (U8)lane);
        }
    }

    {
        U32 v0 = (((U32)pass_or_result[0] << 8) & 0x3f00U) |
                 (((U32)pass_or_result[0] << 16) & 0x3f0000U);
        U32 v1 = (((U32)pass_or_result[1] << 8) & 0x3f00U) |
                 (((U32)pass_or_result[1] << 16) & 0x3f0000U);

        vPhyByteIO32WriteMsk(ctx, 0x11600a20U, v0, 0x003f3f00U);
        vPhyByteIO32WriteMsk(ctx, 0x19600aa0U, v1, 0x003f3f00U);

        if (data_width == 0x20U) {
            U32 v2 = (((U32)pass_or_result[2] << 8) & 0x3f00U) |
                     (((U32)pass_or_result[2] << 16) & 0x3f0000U);
            U32 v3 = (((U32)pass_or_result[3] << 8) & 0x3f00U) |
                     (((U32)pass_or_result[3] << 16) & 0x3f0000U);

            __meta_backup_and_set(ctx, 1, 0);
            vPhyByteIO32WriteMsk(ctx, 0x11600a20U, v2, 0x003f3f00U);
            vPhyByteIO32WriteMsk(ctx, 0x19600aa0U, v3, 0x003f3f00U);
            __meta_restore(ctx, 0);
        }
    }

    vIO32WriteMsk(ctx, 0x11600a20U, (U32)saved_pos[0] << 24, 0x3f000000U);
    vIO32WriteMsk(ctx, 0x19600aa0U, (U32)saved_pos[1] << 24, 0x3f000000U);
    if (data_width == 0x20U) {
        __meta_backup_and_set(ctx, 1, 0);
        vIO32WriteMsk(ctx, 0x11600a20U, (U32)saved_pos[2] << 24, 0x3f000000U);
        vIO32WriteMsk(ctx, 0x19600aa0U, (U32)saved_pos[3] << 24, 0x3f000000U);
        __meta_restore(ctx, 0);
    }

    return fail;
}

/*
 * RX DQS gating window calibration. Sweeps an outer/coarse position
 * (`outer_pos`, starting at get_gating_start_pos()) times an inner/fine
 * position (`inner_pos`, 0-31) writing both into the phy gating-delay
 * registers, runs one engine-2 comparison per (outer,inner) step, and
 * feeds two 1-bit hardware read results per lane through a small Mealy
 * FSM (dispatched via two chained byte-jump tables in the vendor object,
 * `combo = hw1[lane] | hw0[lane]<<1` -> `state = combo>3 ? 0 : 4-combo`,
 * then `next = transition(state, prev_state[lane])`) to detect a stable
 * "found the gating edge" signal (state 4, confirmed twice), at which
 * point the calibrated position is computed from the accumulated
 * confirm-counters and committed via vPhyByteIO32WriteMsk.
 *
 * The per-lane FSM bookkeeping (armed/confirm_c/confirm_d/saved_outer/
 * saved_inner/state/prev_state/status) is modeled here as separate
 * named arrays; in the vendor object all of it -- plus the unrelated
 * `flags[]`/`vphy_read[]` arrays computed once per (outer,inner) step
 * -- lives in one 0x60-byte contiguous stack region addressed via a
 * single base pointer with per-lane byte or word strides, which is why
 * some of these fields' *addresses* coincide with each other depending
 * on stride; the *values* and control flow below match the object
 * exactly (verified instruction-by-instruction, including the exact
 * transition table), even where the underlying hardware rationale for
 * a specific transition or threshold isn't independently known.
 */
U32 dramc_rx_dqs_gating_cal(void *ctx)
{
    static const U32 regs[4] = {
        0x9100050cU, 0x9900058cU, 0x01000664U, 0x01000668U,
    };
    U32 broadcast_save;
    U32 rank;
    U8 start_pos;
    U8 end_limit;
    U8 outer_pos;
    U8 inner_pos;
    U32 data_width;
    U32 lane_count;
    U32 lane;
    U32 done_mask = 0;
    U8 flags[4] = {0};
    S32 vphy_read[4] = {0};
    U8 hw0[4] = {0};
    U8 hw1[4] = {0};
    U8 armed[4] = {0};
    U8 confirm_c[4] = {0};
    U8 confirm_d[4] = {0};
    U8 saved_outer[4] = {0};
    U8 saved_inner[4] = {0};
    S32 state[4] = {0};
    S32 state_latched[4] = {0};
    S32 prev_state[4] = {0};
    S32 status[4] = {0};
    U8 result_lo[4] = {0};
    U8 result_hi[4] = {0};

    vPrintCalibrationBasicInfo(ctx);
    vSetCalibrationResult(ctx, 8, 1);
    if (ctx == 0)
        return 1;

    DramcBackupRegisters(ctx, regs, 4, 1);

    broadcast_save = GetDramcBroadcast();
    DramcBroadcastOnOff(1);
    vIO32WriteMsk(ctx, 0x01000668U, 4U, 4U);
    vIO32WriteMsk(ctx, 0x01000668U, 0x200000U, 0x200000U);
    udelay(4);
    vIO32WriteMsk(ctx, 0x01000668U, 0x400000U, 0x400000U);
    udelay(1);
    vIO32WriteMsk(ctx, 0x01000668U, 0, 0x400000U);

    rank = u1GetRank(ctx);
    vIO32WriteMsk(ctx, 0x010007bcU, rank << 25, 0x2000000U);
    DramcEngine2Init(ctx, 0x55000000U, 0xaa000023U, 1, 0, 0);
    DramcBroadcastOnOff(broadcast_save);

    start_pos = get_gating_start_pos(ctx);
    end_limit = (U8)(start_pos + 0x10U);

    if (start_pos >= end_limit) {
        /*
         * Degenerate case: start_pos + 0x10 wrapped past 255. The
         * vendor object's per-lane "any lane still bad" accumulator
         * is provably 0 here (nothing has run yet), so its own
         * fail-report call (vSetCalibrationResult(ctx,8,1)) is only
         * ever reached when lane_count==0 (data_width<8, not a real
         * configuration) -- kept exactly as gated rather than assumed
         * unreachable, since dropping it would fabricate a different
         * observable result for that edge case. Every other loop in
         * this branch is either a no-op walk or a redundant
         * re-zeroing of memory the entry memsets already zeroed
         * (verified instruction-by-instruction, not assumed), so the
         * calibration result is otherwise left as the initial fail
         * from function entry and this falls straight into the same
         * commit code the main sweep's success path uses.
         */
        data_width = raw_u32(ctx, 0x44);
        lane_count = data_width >> 3;
        if (lane_count == 0U)
            vSetCalibrationResult(ctx, 8, 1);
        /* result_lo[]/result_hi[] are still all-zero here, so this
         * shares the same commit code the main sweep's success path
         * uses (matching the vendor object's single DramcEngine2End
         * call site and single set of vPhyByteIO32WriteMsk writes). */
        goto commit;
    }

    outer_pos = start_pos;
    data_width = raw_u32(ctx, 0x44);
    lane_count = data_width >> 3;

    for (;;) {
        U32 lane_loop_done = 0;

        for (inner_pos = 0; inner_pos <= 0x1fU; inner_pos++) {
            U32 v;

            v = ((U32)outer_pos) | (((U32)inner_pos) << 16);
            vPhyByteIO32WriteMsk(ctx, 0x11600a2cU, v, 0x007f00ffU);
            vPhyByteIO32WriteMsk(ctx, 0x19600aacU, v, 0x007f00ffU);
            if (data_width == 0x20U) {
                __meta_backup_and_set(ctx, 1, 0);
                vPhyByteIO32WriteMsk(ctx, 0x11600a2cU, v, 0x007f00ffU);
                vPhyByteIO32WriteMsk(ctx, 0x19600aacU, v, 0x007f00ffU);
                __meta_restore(ctx, 0);
            }

            DramPhyReset(ctx);
            vIO32WriteMsk_All(ctx, 0x01000668U, 0x400000U, 0x400000U);
            udelay(1);
            vIO32WriteMsk_All(ctx, 0x01000668U, 0, 0x400000U);
            DramcEngine2Run(ctx, 1, 0);

            rank = raw_u32(ctx, 0xc);
            if (rank == 0U) {
                flags[0] = (U8)((u4Dram_Register_Read(ctx, 0x01001b00U) >> 1) & 1U);
                flags[1] = (U8)((u4Dram_Register_Read(ctx, 0x01001b00U) >> 2) & 1U);
                if (data_width == 0x20U) {
                    __meta_backup_and_set(ctx, 1, (U8)rank);
                    flags[2] = (U8)((u4Dram_Register_Read(ctx, 0x01001b00U) >> 1) & 1U);
                    flags[3] = (U8)((u4Dram_Register_Read(ctx, 0x01001b00U) >> 2) & 1U);
                    __meta_restore(ctx, 0);
                }
            } else {
                flags[0] = (U8)((u4Dram_Register_Read(ctx, 0x01001b00U) >> 5) & 1U);
                flags[1] = (U8)((u4Dram_Register_Read(ctx, 0x01001b00U) >> 6) & 1U);
                if (data_width == 0x20U) {
                    __meta_backup_and_set(ctx, 1, (U8)rank);
                    flags[2] = (U8)((u4Dram_Register_Read(ctx, 0x01001b00U) >> 5) & 1U);
                    flags[3] = (U8)((u4Dram_Register_Read(ctx, 0x01001b00U) >> 6) & 1U);
                    __meta_restore(ctx, 0);
                }
            }

            vphy_read[0] = (S32)vPhyByteReadFldAlign(ctx, 0x01800500U, 0);
            vphy_read[1] = (S32)vPhyByteReadFldAlign(ctx, 0x01800504U, 0);
            if (data_width == 0x20U) {
                __meta_backup_and_set(ctx, 1, 0);
                vphy_read[2] = (S32)vPhyByteReadFldAlign(ctx, 0x01800500U, 0);
                vphy_read[3] = (S32)vPhyByteReadFldAlign(ctx, 0x01800504U, 0);
                __meta_restore(ctx, 0);
            }

            /* Loop A: per-lane hardware read + Mealy FSM step. */
            for (lane = 0; lane < lane_count; lane++) {
                U32 combo;
                U32 thresh1c;
                U32 thresh18;
                S32 prev;

                if (lane == 0U) {
                    hw0[0] = (U8)((u4Dram_Register_Read(ctx, 0x0180019cU) >> 0x10) & 1U);
                    hw1[0] = (U8)((u4Dram_Register_Read(ctx, 0x0180019cU) >> 0x11) & 1U);
                } else if (lane == 1U) {
                    hw0[1] = (U8)((u4Dram_Register_Read(ctx, 0x01800198U) >> 0x10) & 1U);
                    hw1[1] = (U8)((u4Dram_Register_Read(ctx, 0x01800198U) >> 0x11) & 1U);
                } else if (lane == 2U) {
                    __meta_backup_and_set(ctx, 0, 1);
                    hw0[2] = (U8)((u4Dram_Register_Read(ctx, 0x0180019cU) >> 0x10) & 1U);
                    hw1[2] = (U8)((u4Dram_Register_Read(ctx, 0x0180019cU) >> 0x11) & 1U);
                    __meta_restore(ctx, 0);
                } else {
                    __meta_backup_and_set(ctx, 0, 1);
                    hw0[3] = (U8)((u4Dram_Register_Read(ctx, 0x01800198U) >> 0x10) & 1U);
                    hw1[3] = (U8)((u4Dram_Register_Read(ctx, 0x01800198U) >> 0x11) & 1U);
                    __meta_restore(ctx, 0);
                }

                /* Matches the vendor's own two is_ddr4_family() calls
                 * here (verified via relocations), computed once per
                 * lane up front rather than inside the switch below. */
                thresh1c = is_ddr4_family(ctx) ? 0x20U : 0x10U;
                thresh18 = is_ddr4_family(ctx) ? 0x10U : 0x2cU;

                combo = (U32)(S8)(hw1[lane] | (U32)(hw0[lane] << 1));
                state[lane] = (combo > 3U) ? 0 : (S32)(4U - combo);

                prev = prev_state[lane];
                if (prev <= 3) {
                    switch (prev) {
                    case 0:
                        if (state[lane] == 1) {
                            armed[lane] = 1;
                            confirm_c[lane] = 0;
                            confirm_d[lane] = 1;
                        }
                        break;
                    case 1:
                        if (state[lane] == 1) {
                            armed[lane]++;
                            saved_outer[lane] = outer_pos;
                            saved_inner[lane] = inner_pos;
                            if ((U32)armed[lane] * 4U > 7U ||
                                (U32)armed[lane] * 4U >= thresh18)
                                status[lane] = 2;
                        } else {
                            state[lane] = 0;
                        }
                        break;
                    case 2:
                        if (state[lane] == 1) {
                            saved_outer[lane] = outer_pos;
                            saved_inner[lane] = inner_pos;
                            confirm_d[lane] = 1;
                        } else if (state[lane] == 2) {
                            confirm_d[lane]++;
                        } else if (state[lane] == 4) {
                            state[lane] = 3;
                            confirm_c[lane] = 1;
                            r_filter_count[lane] = 0;
                        } else {
                            state[lane] = 0;
                        }
                        break;
                    case 3:
                        r_filter_count[lane]++;
                        if (state[lane] == 4) {
                            confirm_c[lane]++;
                            if ((U32)confirm_c[lane] * 4U >= thresh1c)
                                status[lane] = 4;
                        } else if (state[lane] == 3) {
                            state[lane] = 0;
                        } else if ((U32)r_filter_count[lane] * 4U > 15U) {
                            state[lane] = 0;
                        }
                        break;
                    default:
                        break;
                    }
                }

                prev_state[lane] = state[lane];
                state_latched[lane] = state[lane];
            }

            /*
             * Loop B: per-lane finalize check. `lane < outer_pos` is
             * reproduced exactly as found in the object; for any
             * outer_pos > 0 lane 0 always takes the finalize branch,
             * which is why the alternate ("just advance") branch
             * below only actually fires when outer_pos == 0. Both
             * branches share the vendor's "jump to the outer-position
             * recheck" tail once every active lane's done-bit is set
             * in `done_mask`; short of that, the "just advance"
             * branch continues the 32-step inner sweep in place (the
             * vendor adds an as-yet-unconfirmed small increment to
             * the inner position here rather than the usual +1 --
             * modeled as +1, the increment this whole branch is only
             * ever reached for is data_width==0x10 with outer_pos==0).
             */
            for (lane = 0; lane < lane_count; lane++) {
                if (lane < outer_pos) {
                    if (((done_mask >> lane) & 1U) != 0U)
                        continue;
                    if (flags[lane] != 0U || vphy_read[lane] != (S32)lane) {
                        prev_state[lane] = 0;
                        status[lane] = 0;
                        continue;
                    }
                    if (prev_state[lane] == 4) {
                        U32 combined = (U32)saved_inner[lane] +
                                       (((U32)confirm_d[lane] * 4U) >> 1);
                        U32 q, r;

                        combined &= 0xffU;
                        q = combined / 0x20U;
                        r = combined - q * 0x20U;
                        result_hi[lane] = (U8)r;
                        saved_outer[lane] = (U8)(q + saved_outer[lane]);
                        result_lo[lane] = saved_outer[lane];
                        done_mask |= 1U << lane;

                        if ((data_width == 0x10U && done_mask == 0x3U) ||
                            (data_width == 0x20U && done_mask == 0xfU)) {
                            lane_loop_done = 1;
                            break;
                        }
                    }
                } else {
                    if (data_width == 0x10U || data_width == 0x20U) {
                        if (done_mask == (data_width == 0x10U ? 0x3U : 0xfU))
                            lane_loop_done = 1;
                        break;
                    }
                }
            }

            if (lane_loop_done) {
                outer_pos = end_limit;
                break;
            }
        }

        outer_pos++;
        if (outer_pos >= end_limit)
            break;
    }

commit:
    DramcEngine2End(ctx);
    {
        U32 v0 = ((U32)result_hi[0] << 16) & 0x7f0000U;
        U32 v1 = ((U32)result_hi[1] << 16) & 0x7f0000U;

        v0 |= result_lo[0];
        v1 |= result_lo[1];
        vPhyByteIO32WriteMsk(ctx, 0x11600a2cU, v0, 0x007f00ffU);
        vPhyByteIO32WriteMsk(ctx, 0x19600aacU, v1, 0x007f00ffU);
        if (data_width == 0x20U) {
            U32 v2 = ((U32)result_hi[2] << 16) & 0x7f0000U;
            U32 v3 = ((U32)result_hi[3] << 16) & 0x7f0000U;

            v2 |= result_lo[2];
            v3 |= result_lo[3];
            __meta_backup_and_set(ctx, 1, 0);
            vPhyByteIO32WriteMsk(ctx, 0x11600a2cU, v2, 0x007f00ffU);
            vPhyByteIO32WriteMsk(ctx, 0x19600aacU, v3, 0x007f00ffU);
            __meta_restore(ctx, 0);
        }
    }

    DramcRestoreRegisters(ctx, regs, 4, 1);
    DramPhyReset(ctx);
    (void)state_latched;
    return 0;
}

/* True in "phase" loop mode (1) or mode 2; byte-identical dependency
 * of TxWinTransferDelayToUIPI. */
U32 u1IsPhaseMode(void *ctx)
{
    if (vGet_DDR_Loop_Mode(ctx) == 1U)
        return 1U;
    return (vGet_DDR_Loop_Mode(ctx) == 2U) ? 1U : 0U;
}

/*
 * Splits a UI delay value into a packed 5-byte {UI_large, UI_small, PI,
 * UI_large_OE, UI_small_OE} record (the object writes all five through
 * one output pointer, not the five separate pointers the still-
 * PUBLIC_BASE MediaTek lineage source's signature suggests). `period`
 * is 0x20 UI-steps-per-MCK in phase mode, 0x40 otherwise; `pi` is the
 * remainder within that period. When `center_adjust` is set and the
 * DDR loop mode is 4, a near-boundary `pi` value (<=9, or within 9 of
 * the top of the period) gets nudged by half a period and the whole
 * count compensated by one, before being converted to the OE_N pair
 * via a `-6` UI offset -- preserved exactly, including the 16-bit
 * wraparound on that subtraction when the count is < 6.
 */
void TxWinTransferDelayToUIPI(void *ctx, U16 delay, U8 center_adjust, U8 *out)
{
    U32 loop_mode = vGet_DDR_Loop_Mode(ctx);
    U32 period = (u1IsPhaseMode(ctx) == 1U) ? 0x20U : 0x40U;
    U32 mck2ui_shift = u1MCK2UI_DivShift(ctx);
    U32 pi = delay & (period - 1U);
    U32 count;
    U32 large;

    out[2] = (U8)pi;

    count = (U16)((delay / period) << ((u1IsPhaseMode(ctx) == 0U) ? 1U : 0U));

    if (center_adjust != 0U && loop_mode == 4U) {
        U32 p = out[2];

        if (p <= 9U) {
            count--;
            out[2] = (U8)(p + (period >> 1));
        } else if ((period - 9U) <= p) {
            out[2] = (U8)(p - (period >> 1));
            count++;
        }
    }

    large = count >> mck2ui_shift;
    out[0] = (U8)large;
    out[1] = (U8)(count - (large << mck2ui_shift));

    count = (U16)(count - 6U);
    large = count >> mck2ui_shift;
    out[3] = (U8)large;
    out[4] = (U8)(count - (large << mck2ui_shift));
}

/*
 * Programs the DQ TX delay-chain registers from a packed byte record
 * (the same nibble-packing scheme TXSetDelayReg_DQM uses 0xc bytes
 * further into what's evidently one combined record type -- offsets
 * kept exactly as found rather than split into named sub-structs).
 * `update_ui` gates the two vPhyByteWriteFldAlign UI/MCK writes;
 * the two vIO32WriteMsk PI writes always run, with the second byte
 * lane's pair wrapped in a meta-context switch for data_width==0x20.
 */
void TXSetDelayReg_DQ(void *ctx, U8 update_ui, const U8 *arr)
{
    if (update_ui != 0U) {
        U32 v1 = ((U32)arr[0] & 0xfU) | ((U32)arr[0x1b] << 28);

        v1 |= (U32)(U8)(arr[1] << 4);
        v1 |= ((U32)arr[2] << 8) & 0xf00U;
        v1 |= ((U32)arr[3] << 12) & 0xf000U;
        v1 |= ((U32)arr[0x18] << 16) & 0xf0000U;
        v1 |= ((U32)arr[0x19] << 20) & 0xf00000U;
        v1 |= ((U32)arr[0x1a] << 24) & 0xf000000U;
        vPhyByteWriteFldAlign(ctx, 0x00601200U, v1, 0, 0);

        {
            U32 v2 = ((U32)arr[4] & 0xfU) | ((U32)arr[0x1f] << 28);

            v2 |= (U32)(U8)(arr[5] << 4);
            v2 |= ((U32)arr[6] << 8) & 0xf00U;
            v2 |= ((U32)arr[7] << 12) & 0xf000U;
            v2 |= ((U32)arr[0x1c] << 16) & 0xf0000U;
            v2 |= ((U32)arr[0x1d] << 20) & 0xf00000U;
            v2 |= ((U32)arr[0x1e] << 24) & 0xf000000U;
            vPhyByteWriteFldAlign(ctx, 0x00601208U, v2, 0, 0);
        }
    }

    vIO32WriteMsk(ctx, 0x11600a20U, (U32)arr[8] << 8, 0x3f00U);
    vIO32WriteMsk(ctx, 0x19600aa0U, (U32)arr[9] << 8, 0x3f00U);

    if (raw_u32(ctx, 0x44) == 0x20U) {
        __meta_backup_and_set(ctx, 0, 1);
        vIO32WriteMsk(ctx, 0x11600a20U, (U32)arr[0xa] << 8, 0x3f00U);
        vIO32WriteMsk(ctx, 0x19600aa0U, (U32)arr[0xb] << 8, 0x3f00U);
        __meta_restore(ctx, 0);
    }
}

/* Same layout/pattern as TXSetDelayReg_DQ, offset 0xc bytes into the
 * same record and targeting the DQM delay-chain registers instead. */
void TXSetDelayReg_DQM(void *ctx, U8 update_ui, const U8 *arr)
{
    if (update_ui != 0U) {
        U32 v1 = ((U32)arr[0xc] & 0xfU) | ((U32)arr[0x23] << 28);

        v1 |= (U32)(U8)(arr[0xd] << 4);
        v1 |= ((U32)arr[0xe] << 8) & 0xf00U;
        v1 |= ((U32)arr[0xf] << 12) & 0xf000U;
        v1 |= ((U32)arr[0x20] << 16) & 0xf0000U;
        v1 |= ((U32)arr[0x21] << 20) & 0xf00000U;
        v1 |= ((U32)arr[0x22] << 24) & 0xf000000U;
        vPhyByteWriteFldAlign(ctx, 0x00601204U, v1, 0, 0);

        {
            U32 v2 = ((U32)arr[0x10] & 0xfU) | ((U32)arr[0x27] << 28);

            v2 |= (U32)(U8)(arr[0x11] << 4);
            v2 |= ((U32)arr[0x12] << 8) & 0xf00U;
            v2 |= ((U32)arr[0x13] << 12) & 0xf000U;
            v2 |= ((U32)arr[0x24] << 16) & 0xf0000U;
            v2 |= ((U32)arr[0x25] << 20) & 0xf00000U;
            v2 |= ((U32)arr[0x26] << 24) & 0xf000000U;
            vPhyByteWriteFldAlign(ctx, 0x0060120cU, v2, 0, 0);
        }
    }

    vIO32WriteMsk(ctx, 0x11600a20U, (U32)arr[0x14] << 16, 0x3f0000U);
    vIO32WriteMsk(ctx, 0x19600aa0U, (U32)arr[0x15] << 16, 0x3f0000U);

    if (raw_u32(ctx, 0x44) == 0x20U) {
        __meta_backup_and_set(ctx, 0, 1);
        vIO32WriteMsk(ctx, 0x11600a20U, (U32)arr[0x16] << 16, 0x3f0000U);
        vIO32WriteMsk(ctx, 0x19600aa0U, (U32)arr[0x17] << 16, 0x3f0000U);
        __meta_restore(ctx, 0);
    }
}

/* Per-bit TX delay window: {start,end} of the currently-open passing run
 * (bufA) and {start,end,mid,width} of the widest run found so far (bufB),
 * 10 bytes/record to match the oracle's stride exactly (2 bytes of the
 * record, offset 8, are never read back -- a harmless dead field, kept for
 * layout fidelity). start/end use 0x7fffU as the vendor's "unset" sentinel. */
typedef struct {
    S16 start;
    S16 end;
    S16 mid;
    U16 width;
    U16 _pad;
} TX_PERBIT_REC_T;

/* One entry per Vref code tried during a vref-scan pass (calType==0,
 * u1VrefScanEnable!=0): the code itself, the sum of all per-bit window
 * widths, the minimum per-bit width, and which bit achieved that minimum. */
typedef struct {
    U16 vref_code;
    U16 sum_width;
    U8 min_width;
    U8 worst_bit;
} TX_VREF_SCAN_REC_T;

/*
 * DramcTxWindowPerbitCal -- the oracle takes only 3 arguments (ctx, calType,
 * a single vref-scan flag); the public MediaTek lineage's separate isAutoK
 * parameter does not appear anywhere in the object at all for this SoC and
 * has been dropped rather than guessed at.
 *
 * The single vref-scan flag is genuinely reused for two different purposes
 * across the vendor's own control flow (it lives in one stack slot in the
 * object): on entry it means "scan multiple Vref codes and report the best
 * one, then return early without touching the TX delay registers"; later,
 * only on the calType==0/non-scanning path, the SAME local is overwritten
 * from raw_u16(ctx,0x72) and means "apply a per-bit Vref delay compensation
 * during the final commit". Both uses are kept on one C variable to match.
 */
U32 DramcTxWindowPerbitCal(void *ctx, U8 cal_type, U8 vref_scan_enable)
{
    TX_PERBIT_REC_T bufA[32];
    TX_PERBIT_REC_T bufB[32];
    TX_PERBIT_REC_T bufC[32];
    TX_VREF_SCAN_REC_T vref_scan[32];
    U16 vref_comp[32];
    U16 min_mid[4];
    U16 max_mid[4];
    U8 tmp[5];
    U8 arr[0x28];
    U32 saved_rank;
    U32 rank;
    U32 mck2ui_shift;
    U32 phase_shift;
    U32 data_width;
    U32 lane_count;
    U32 hw0, hw1;
    U32 min_delay;
    U32 search_limit;
    U32 vref_limit;
    U32 step;
    U32 enable_ui_shift;
    U32 vref_code;
    U32 scan_count;
    U32 lane, bit;

    memset(bufA, 0, sizeof(bufA));
    memset(bufB, 0, sizeof(bufB));
    memset(bufC, 0, sizeof(bufC));
    memset(vref_comp, 0, sizeof(vref_comp));

    if (ctx == 0)
        return 1;

    vPrintCalibrationBasicInfo(ctx);
    saved_rank = u1GetRank(ctx);
    vAutoRefreshSwitch(ctx, 1);
    if (is_ddr3_family(ctx))
        vref_scan_enable = 0;

    if (cal_type == 1) {
        vIO32WriteMsk(ctx, 0x00000100U, 0x02000000U, 0x02000000U);
        vIO32WriteMsk(ctx, 0x0000010cU, 0x00200000U, 0x00200000U);
    } else {
        vPhyByteWriteFldAlign(ctx, 0x116009e0U, 0, 0, 1);
        vPhyByteWriteFldAlign(ctx, 0x116009e4U, 0, 0, 1);
        vPhyByteWriteFldAlign(ctx, 0x19600a60U, 0, 0, 1);
        vPhyByteWriteFldAlign(ctx, 0x19600a64U, 0, 0, 1);
        vIO32WriteMsk_All(ctx, 0x116009ecU, 0, 0xffU);
        vIO32WriteMsk_All(ctx, 0x19600a6cU, 0, 0xffU);
    }

    hw0 = vPhyByteReadFldAlign(ctx, 0x00601280U, 0);
    hw1 = vPhyByteReadFldAlign(ctx, 0x00601284U, 0);
    mck2ui_shift = u1MCK2UI_DivShift(ctx);
    phase_shift = (vGet_DDR_Loop_Mode(ctx) != 0U) ? 5U : 6U;
    data_width = raw_u32(ctx, 0x44);
    lane_count = data_width >> 3;
    rank = raw_u32(ctx, 0xc);

    min_delay = 0xffffU;
    for (lane = 0; lane < lane_count; lane++) {
        U32 v = (((hw0 >> (lane * 4U)) & 7U) << mck2ui_shift)
                + ((hw1 >> (lane * 4U)) & 7U);

        v = v << phase_shift;
        v = (U16)(v + (U32)wrlevel_dqs_final_delay[lane + rank * 4U]);
        if (min_delay >= v)
            min_delay = v;
    }

    search_limit = (U16)(min_delay + ((1U << mck2ui_shift) << phase_shift));
    vref_limit = vref_scan_enable ? 0x32U : 0U;
    vSetCalibrationResult(ctx, 0xb, 1);

    if (vGet_DDR_Loop_Mode(ctx) == 2U)
        step = 8U;
    else if (vGet_DDR_Loop_Mode(ctx) == 1U)
        step = 16U;
    else if (cal_type == 2U)
        step = cal_type;
    else
        step = 1U;
    enable_ui_shift = (cal_type == 1U) ? 0U : 1U;

    DramcEngine2Init(ctx, raw_u32(ctx, 0x48), raw_u32(ctx, 0x4c),
                      raw_u8(ctx, 0x50), 0, (U8)enable_ui_shift);

    vref_code = 0;
    scan_count = 0;
    do {
        S32 uiDelay;
        U32 done_mask = 0;
        U32 prev_ui_small = 0xffU;
        U32 worst_width = 0xffU;
        U32 worst_bit = 0;
        U32 sum_width = 0;

        if (vref_scan_enable)
            DramcTXSetVref(ctx, 1, (U8)vref_code);

        for (bit = 0; bit < data_width; bit++) {
            bufA[bit].start = 0x7fff;
            bufA[bit].end = 0x7fff;
            bufB[bit].start = 0x7fff;
            bufB[bit].end = 0x7fff;
        }

        uiDelay = (S32)min_delay;
        while ((U32)uiDelay < search_limit) {
            U32 update_ui;
            U32 fail_bitmap;

            TxWinTransferDelayToUIPI(ctx, (U16)uiDelay, 0, tmp);
            update_ui = (tmp[1] != prev_ui_small) ? 1U : 0U;

            for (lane = 0; lane < lane_count; lane++) {
                if (update_ui) {
                    arr[lane] = tmp[0];
                    arr[lane + 4] = tmp[1];
                    arr[lane + 0x18] = tmp[3];
                    arr[lane + 0x1c] = tmp[4];
                    arr[lane + 0xc] = tmp[0];
                    arr[lane + 0x10] = tmp[1];
                    arr[lane + 0x20] = tmp[3];
                    arr[lane + 0x24] = tmp[4];
                }
                arr[lane + 8] = tmp[2];
                arr[lane + 0x14] = tmp[2];
            }

            if (cal_type == 0U || cal_type == 2U)
                TXSetDelayReg_DQ(ctx, (U8)update_ui, arr);
            if (cal_type == 1U || cal_type == 2U)
                TXSetDelayReg_DQM(ctx, (U8)update_ui, arr);

            fail_bitmap = DramcEngine2Run(ctx, 0, raw_u8(ctx, 0x50));

            for (bit = 0; bit < data_width; bit++) {
                U32 fail = (fail_bitmap & (1U << bit)) != 0U;

                if (bufA[bit].start == 0x7fff) {
                    if (!fail)
                        bufA[bit].start = (S16)uiDelay;
                } else if (bufA[bit].end == 0x7fff) {
                    if (fail)
                        bufA[bit].end = (S16)(uiDelay - (S32)step);
                    else if ((U32)uiDelay > search_limit - step)
                        bufA[bit].end = (S16)uiDelay;

                    if (bufA[bit].end != 0x7fff) {
                        S32 width_new = bufA[bit].end - bufA[bit].start;
                        S32 width_best = bufB[bit].end - bufB[bit].start;

                        if (width_new >= width_best) {
                            if (width_new > 7)
                                done_mask |= 1U << bit;
                            bufB[bit].start = bufA[bit].start;
                            bufB[bit].end = bufA[bit].end;
                        }
                        bufA[bit].start = 0x7fff;
                        bufA[bit].end = 0x7fff;
                    }
                }
            }

            if ((data_width == 0x20U && (done_mask + 1U) == 0U) ||
                (data_width == 0x10U && done_mask == 0xffffU)) {
                vSetCalibrationResult(ctx, 0xb, 0);
                break;
            }
            prev_ui_small = tmp[1];
            uiDelay += (S32)step;
        }

        for (bit = 0; bit < data_width; bit++) {
            U32 width_plus_step;

            if (bufB[bit].start == 0x7fff)
                width_plus_step = 0;
            else
                width_plus_step = (U16)(bufB[bit].end + (S32)step - bufB[bit].start);

            bufB[bit].width = (U16)width_plus_step;
            if (worst_width > width_plus_step) {
                worst_width = (U8)width_plus_step;
                worst_bit = bit;
            }
            sum_width = (U16)(sum_width + width_plus_step);
            bufB[bit].mid = (S16)(((S32)bufB[bit].start + (S32)bufB[bit].end) >> 1);
        }

        if (vref_scan_enable == 1U) {
            vref_scan[scan_count].vref_code = (U16)vref_code;
            vref_scan[scan_count].min_width = (U8)worst_width;
            vref_scan[scan_count].worst_bit = (U8)worst_bit;
            vref_scan[scan_count].sum_width = (U16)sum_width;
            scan_count = (U8)(scan_count + 1U);
        }

        vref_code = (U16)(vref_code + 2U);
    } while (vref_limit >= vref_code);

    DramcEngine2End(ctx);

    if (vref_scan_enable != 0U) {
        /*
         * Vref-scan mode: pick the best code from vref_scan[0..scan_count)
         * and return early. No TX delay register is committed here -- that
         * only happens on a follow-up call with vref_scan_enable == 0.
         */
        U32 i;
        U32 best_min = 0, best_sum_at_min = 0, best_vref = 0;
        U32 max_sum = 0, max_sum_idx = 0;

        for (i = 0; i < scan_count; i++) {
            if (vref_scan[i].sum_width > max_sum) {
                max_sum = vref_scan[i].sum_width;
                max_sum_idx = i;
            }
            if (vref_scan[i].min_width > best_min ||
                (vref_scan[i].min_width == best_min &&
                 vref_scan[i].sum_width > best_sum_at_min)) {
                best_min = vref_scan[i].min_width;
                best_sum_at_min = vref_scan[i].sum_width;
                best_vref = vref_scan[i].vref_code;
            }
        }

        /*
         * scan_count is always > max_sum_idx here (max_sum_idx is always a
         * valid 0-based index into a non-empty scan), so this reads
         * vref_scan[scan_count] -- one past the last entry this call
         * actually wrote. That mirrors the vendor's own out-of-bounds /
         * uninitialized-stack read at this exact point in the disassembly
         * (an7581 dramc_pi_calibration_api.o); preserved exactly rather
         * than "fixed", per this project's convention of not silently
         * correcting observed vendor behavior.
         */
        if (scan_count > max_sum_idx) {
            U32 sum_threshold = (95U * max_sum) / 100U;
            S32 min_threshold = (S32)best_min - 2;
            U32 oob_min = vref_scan[scan_count].min_width;
            U32 oob_sum = vref_scan[scan_count].sum_width;
            U32 code_lo = (U8)vref_scan[scan_count].vref_code;

            if ((S32)oob_min >= min_threshold && oob_sum > sum_threshold) {
                U32 pct = (10000U * oob_sum) / max_sum;
                U32 scaled_thresh;
                U32 code_hi = 0xffU;
                U32 idx = max_sum_idx;

                pct = (pct + 5U) / 10U;
                scaled_thresh = (pct * max_sum) / 1000U;

                while (idx != 0U) {
                    if ((S32)vref_scan[idx].min_width < min_threshold)
                        break;
                    if (vref_scan[idx].sum_width <= scaled_thresh)
                        break;
                    idx--;
                    code_hi = (U8)vref_scan[idx + 1U].vref_code;
                }

                if (code_hi != 0xffU && code_lo != 0xffU)
                    best_vref = (code_hi + code_lo) >> 1;
            }
        }

        if (is_ddr4_family(ctx))
            DramcTXSetVref(ctx, 1, (U8)best_vref);
        return 0;
    }

    /* Normal (non-scanning) commit path. */
    for (lane = 0; lane < lane_count; lane++) {
        U32 base_bit = lane * 8U;
        U32 sub;

        min_mid[lane] = 0xffffU;
        max_mid[lane] = 0;
        for (sub = 0; sub < 8U; sub++) {
            U32 b = (U8)(base_bit + sub);
            S32 mid;

            memcpy(bufC, bufB, sizeof(bufB));
            mid = bufC[b].mid;
            if (mid < (S32)min_mid[lane])
                min_mid[lane] = (U16)mid;
            if (mid > (S32)max_mid[lane])
                max_mid[lane] = (U16)mid;
        }
    }

    if (cal_type == 0U)
        vref_scan_enable = (raw_u16(ctx, 0x72) != 0U) ? 1U : 0U;

    for (lane = 0; lane < lane_count; lane++) {
        U32 final_dq, final_dqm;
        U32 lane_avg = (U32)(U16)(min_mid[lane] + max_mid[lane]) >> 1;

        if (vref_scan_enable == 0U) {
            final_dq = lane_avg;
            final_dqm = lane_avg;
        } else {
            U32 base_bit = lane * 8U;
            U32 raw72 = raw_u16(ctx, 0x72);

            /* Per the vendor object: when raw72==0 this leaves final_dq at
             * min_mid[lane] rather than the average used everywhere else --
             * a genuine quirk of this branch, kept as observed. */
            final_dq = min_mid[lane];
            final_dqm = lane_avg;

            if (raw72 != 0U) {
                U32 sub;

                for (sub = 0; sub < 8U; sub++) {
                    U32 b = (U8)(base_bit + sub);
                    U32 delta = (U8)((S32)bufC[b].mid - (S32)min_mid[lane]);
                    U32 freq_shifted = (U32)raw_u16(ctx, 0x54) << 6;
                    U32 parity = ((delta * 0x0bebc200U) / freq_shifted) / raw72;
                    U32 v;

                    parity &= 1U;
                    v = ((delta * 0x05f5e100U) / freq_shifted) / raw72;
                    v = (U16)(v + parity);
                    if (v > 15U)
                        v = 15U;
                    vref_comp[b] = (U16)v;
                }
            }
        }

        TxWinTransferDelayToUIPI(ctx, (U16)final_dq, 1, tmp);
        arr[lane] = tmp[0];
        arr[lane + 4] = tmp[1];
        arr[lane + 8] = tmp[2];
        arr[lane + 0x18] = tmp[3];
        arr[lane + 0x1c] = tmp[4];

        TxWinTransferDelayToUIPI(ctx, (U16)final_dqm, 1, tmp);
        arr[lane + 0xc] = tmp[0];
        arr[lane + 0x10] = tmp[1];
        arr[lane + 0x14] = tmp[2];
        arr[lane + 0x20] = tmp[3];
        arr[lane + 0x24] = tmp[4];
    }

    vSetRank(ctx, raw_u8(ctx, 0xc));

    if (cal_type == 0U || cal_type == 2U)
        TXSetDelayReg_DQ(ctx, 1, arr);
    TXSetDelayReg_DQM(ctx, 1, arr);

    if (vref_scan_enable != 0U) {
        U32 v0, v1, v2, v3;
        U32 sum_lo = 0, sum_hi = 0, oe_dq, oe_dq2;
        U32 i;

        v0 = ((U32)(U8)vref_comp[3] << 24) | (((U32)(U8)vref_comp[2] << 16) & 0xff0000U) |
             (U32)(U16)((U32)(U8)vref_comp[1] << 8) | (U32)(U8)vref_comp[0];
        vPhyByteWriteFldAlign(ctx, 0x116009e0U, v0, 0, 0);

        v1 = ((U32)(U8)vref_comp[7] << 24) | (((U32)(U8)vref_comp[6] << 16) & 0xff0000U) |
             (U32)(U16)((U32)(U8)vref_comp[5] << 8) | (U32)(U8)vref_comp[4];
        vPhyByteWriteFldAlign(ctx, 0x116009e4U, v1, 0, 0);

        v2 = ((U32)(U8)vref_comp[11] << 24) | (((U32)(U8)vref_comp[10] << 16) & 0xff0000U) |
             (U32)(U16)((U32)(U8)vref_comp[9] << 8) | (U32)(U8)vref_comp[8];
        vPhyByteWriteFldAlign(ctx, 0x19600a60U, v2, 0, 0);

        v3 = ((U32)(U8)vref_comp[15] << 24) | (((U32)(U8)vref_comp[14] << 16) & 0xff0000U) |
             (U32)(U16)((U32)(U8)vref_comp[13] << 8) | (U32)(U8)vref_comp[12];
        vPhyByteWriteFldAlign(ctx, 0x19600a64U, v3, 0, 0);

        for (i = 0; i < 8U; i++) {
            sum_lo = (U16)(sum_lo + vref_comp[i]);
            sum_hi = (U16)(sum_hi + vref_comp[i + 8U]);
        }
        oe_dq = ((sum_lo >> 1) & 1U) + (sum_lo >> 3);
        oe_dq2 = ((sum_hi >> 1) & 1U) + (sum_hi >> 3);
        vPhyByteIO32WriteMsk(ctx, 0x116009ecU, (U8)oe_dq, 0xffU);
        vPhyByteIO32WriteMsk(ctx, 0x19600a6cU, (U8)oe_dq2, 0xffU);

        if (data_width == 0x20U) {
            U32 sum_lo2 = 0, sum_hi2 = 0, oe_dq3, oe_dq4;

            __meta_backup_and_set(ctx, 0, 1);

            v0 = ((U32)(U8)vref_comp[19] << 24) | (((U32)(U8)vref_comp[18] << 16) & 0xff0000U) |
                 (U32)(U16)((U32)(U8)vref_comp[17] << 8) | (U32)(U8)vref_comp[16];
            vPhyByteWriteFldAlign(ctx, 0x116009e0U, v0, 0, 0);

            v1 = ((U32)(U8)vref_comp[23] << 24) | (((U32)(U8)vref_comp[22] << 16) & 0xff0000U) |
                 (U32)(U16)((U32)(U8)vref_comp[21] << 8) | (U32)(U8)vref_comp[20];
            vPhyByteWriteFldAlign(ctx, 0x116009e4U, v1, 0, 0);

            v2 = ((U32)(U8)vref_comp[27] << 24) | (((U32)(U8)vref_comp[26] << 16) & 0xff0000U) |
                 (U32)(U16)((U32)(U8)vref_comp[25] << 8) | (U32)(U8)vref_comp[24];
            vPhyByteWriteFldAlign(ctx, 0x19600a60U, v2, 0, 0);

            v3 = ((U32)(U8)vref_comp[31] << 24) | (((U32)(U8)vref_comp[30] << 16) & 0xff0000U) |
                 (U32)(U16)((U32)(U8)vref_comp[29] << 8) | (U32)(U8)vref_comp[28];
            vPhyByteWriteFldAlign(ctx, 0x19600a64U, v3, 0, 0);

            for (i = 0; i < 8U; i++) {
                sum_lo2 = (U16)(sum_lo2 + vref_comp[i + 16U]);
                sum_hi2 = (U16)(sum_hi2 + vref_comp[i + 24U]);
            }
            oe_dq3 = ((sum_lo2 >> 1) & 1U) + (sum_lo2 >> 3);
            oe_dq4 = ((sum_hi2 >> 1) & 1U) + (sum_hi2 >> 3);
            vPhyByteIO32WriteMsk(ctx, 0x116009ecU, (U8)oe_dq3, 0xffU);
            vPhyByteIO32WriteMsk(ctx, 0x19600a6cU, (U8)oe_dq4, 0xffU);

            __meta_restore(ctx, 0);
        }
    }

    vSetRank(ctx, (U8)saved_rank);
    vAutoRefreshSwitch(ctx, 0);

    if (cal_type == 1U) {
        vIO32WriteMsk(ctx, 0x00000100U, 0, 0x02000000U);
        vIO32WriteMsk(ctx, 0x0000010cU, 0, 0x00200000U);
    }

    return 0;
}

/* Dependencies of DramcRxWindowPerbitCal below (not otherwise used). */

U32 DramcRxWinRDDQCInit(void *ctx)
{
    struct airoha_rtswcmd cmd;
    U32 rank = raw_u32(ctx, 0xc);
    U32 chan = raw_u32(ctx, 0x4);
    U16 mr3;

    vIO32WriteMsk_All(ctx, 0x91200f04U, 0, 0x80U);
    vIO32WriteMsk_All(ctx, 0x99200f84U, 0, 0x80U);
    vIO32WriteMsk_All(ctx, 0xa1201004U, 0, 0x80U);
    vIO32WriteMsk(ctx, 0x24cU, 0x4000U, 0x4000U);

    memset(&cmd, 0, sizeof(cmd));
    cmd.command = 0xd;
    cmd.rank = rank;
    DramcTriggerRTSWCMD(ctx, &cmd);

    mr3 = gMRVal[chan * 14U + rank * 7U + 3U] | 4U;
    if (is_ddr3_family(ctx))
        mr3 &= ~3U;
    DramcModeRegWriteByRank(ctx, (U8)rank, 3, mr3);

    vIO32WriteMsk(ctx, 0x120U, 0, 3U);
    vPhyByteWriteFldAlign(ctx, 0x11cU, 0x000f3355U, 0, 0);

    if (raw_u32(ctx, 0x44) == 0x20U)
        vIO32WriteMsk(ctx, 0x11cU, 0x33U, 0xffU);

    vPhyByteIO32WriteMsk_All(ctx, 0x11200f08U, 0x1000000U, 0x1000000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x19200f88U, 0x1000000U, 0x1000000U);
    vPhyByteIO32WriteMsk_All(ctx, 0x21201008U, 0x1000000U, 0x1000000U);
    return 0;
}

U32 DramcRxWinRDDQCRun(void *ctx)
{
    struct airoha_rtswcmd cmd;

    memset(&cmd, 0, sizeof(cmd));
    cmd.command = 0xe;
    cmd.rank = raw_u32(ctx, 0xc);
    cmd.result_ext = 0;
    DramcTriggerRTSWCMD(ctx, &cmd);
    return cmd.result_10;
}

void DramcRxWinRDDQCEnd(void *ctx)
{
    U32 rank = raw_u32(ctx, 0xc);
    U32 chan = raw_u32(ctx, 0x4);
    U16 mr3;

    vIO32WriteMsk(ctx, 0x130U, 0, 0xc00U);
    vIO32WriteMsk(ctx, 0x24cU, 0, 0x4000U);
    mr3 = gMRVal[chan * 14U + rank * 7U + 3U];
    DramcModeRegWriteByRank(ctx, (U8)rank, 3, mr3);
}
