/* SPDX-License-Identifier: BSD-3-Clause */
/* Oracle-derived Airoha PCDDR calibration API recovery core. */
#include "recovery_abi.h"

typedef int8_t S8;
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
extern S32 is_ddr4_family(void *ctx);
extern S32 is_ddr3_family(void *ctx);
extern U32 uartDisable;
extern int printf(const char *fmt, ...);
extern void DramcTriggerRTSWCMD(void *ctx, void *opaque);
extern void vSetCalibrationResult(void *ctx, U8 cal_type, U8 result);
static void _LoopAryToDelay(void *ctx, REG_TRANSFER_T *ui_reg,
                             REG_TRANSFER_T *mck_reg, U8 count,
                             S8 shift_ui, U8 byte_idx);
void DramcImpedanceSetValue(void *ctx, U32 code, U32 bit5, U32 type);

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
 *   bits[31:24] : 0xff means "field absent" -- skip the MMIO read
 *                 (value 0) and neutralize the MMIO write (mask 0).
 *                 AN7581 has no equivalent check; this is AN7583-only.
 *   bits[19:18] : 1 selects the PHY-space accessors (vPhyByteReadFldAlign/
 *                 vPhyByteWriteFldAlign); otherwise the generic MMIO
 *                 accessors (u4Dram_Register_Read/vIO32WriteMsk) are used
 *   bits[15:8]  : field width in bits
 *   bits[7:0]   : field bit position
 *
 * None of the current PCDDR_ShiftDQSUI/PCDDR_ShiftDQS_OENUI table
 * entries set bits[31:24], so this sentinel path is currently dead code
 * that only affects the AN7583 object's size relative to AN7581.
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
        else if (((ui_field >> 24) & 0xffU) == 0xffU)
            ui_val = 0;
        else {
            U32 raw = u4Dram_Register_Read(ctx, ui_reg_addr);
            U32 mask = (0xffffffffU >> (32U - ui_width)) << ui_pos;

            ui_val = (raw & mask) >> ui_pos;
        }

        if (((mck_field >> 18) & 3U) == 1U)
            mck_val = vPhyByteReadFldAlign(ctx, mck_reg_addr, mck_field);
        else if (((mck_field >> 24) & 0xffU) == 0xffU)
            mck_val = 0;
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
            U32 mask = (((ui_field >> 24) & 0xffU) == 0xffU) ? 0U :
                       (0xffffffffU >> (32U - ui_width)) << ui_pos;

            vIO32WriteMsk(ctx, ui_reg_addr, new_ui << ui_pos, mask);
        }

        if (((mck_field >> 18) & 3U) == 1U)
            vPhyByteWriteFldAlign(ctx, mck_reg_addr, new_mck, mck_field, 0);
        else {
            U32 mask = (((mck_field >> 24) & 0xffU) == 0xffU) ? 0U :
                       (0xffffffffU >> (32U - mck_width)) << mck_pos;

            vIO32WriteMsk(ctx, mck_reg_addr, new_mck << mck_pos, mask);
        }

        lane += stride;
    }
}

/*
 * Same weighted-bit resistance-grade computation as AN7581 (see that
 * recovered-core for the full derivation), but AN7583 additionally
 * treats field bits[31:24] == 0xff as "field absent": the read is
 * skipped (raw = 0) and the write mask is forced to 0, matching the
 * same sentinel _LoopAryToDelay uses on this SoC. None of the current
 * DramcImpedanceSetValue field constants set bits[31:24], so this path
 * is currently dead code, only affecting AN7583's object size relative
 * to AN7581.
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
    else if (((field >> 24) & 0xffU) == 0xffU)
        raw = 0;
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
        U32 mask = (((field >> 24) & 0xffU) == 0xffU) ? 0U :
                   (0xffffffffU >> (32U - width)) << pos;

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
 * Identical addresses/fields to AN7581.
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
 * DRV/ODT impedance trim from efuse, gated by DDR type. Unlike AN7581,
 * the debug prints are additionally gated on the global uartDisable
 * flag, and the efuse bit offsets and format strings differ.
 */
void DramcImpedanceByEfuse(void *ctx)
{
    U8 fuse6 = 0;
    U8 fuse7 = 0;

    if (is_ddr4_family(ctx)) {
        Dramc_efuse_read_parse(0x2deU, 7, &fuse6);
        Dramc_efuse_read_parse(0x2d7U, 7, &fuse7);
    } else {
        if (!is_ddr3_family(ctx))
            return;
        Dramc_efuse_read_parse(0x2d0U, 7, &fuse6);
        Dramc_efuse_read_parse(0x2c9U, 7, &fuse7);
    }

    if ((fuse6 >> 6) & 1U) {
        if (uartDisable == 0U) {
            printf("DRVP driving setting info: 0x%x\n", fuse6);
            printf("ODTP driving setting info: 0x%x\n", fuse6);
        }
        DramcImpedanceEfuseValue(ctx, fuse6, 0);
        DramcImpedanceEfuseValue(ctx, fuse6, 2);
    }

    if ((fuse7 >> 6) & 1U) {
        if (uartDisable == 0U) {
            printf("DRVN driving setting info: 0x%x\n", fuse7);
            printf("ODTN driving setting info: 0x%x\n", fuse7);
        }
        DramcImpedanceEfuseValue(ctx, fuse7, 1);
        DramcImpedanceEfuseValue(ctx, fuse7, 3);
    }
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
