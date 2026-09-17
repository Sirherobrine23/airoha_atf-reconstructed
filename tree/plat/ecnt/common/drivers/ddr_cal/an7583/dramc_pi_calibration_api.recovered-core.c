/* SPDX-License-Identifier: BSD-3-Clause */
/* Oracle-derived Airoha PCDDR calibration API recovery core. */
#include "recovery_abi.h"

typedef int8_t S8;
typedef struct {
    U32 reg;
    U32 field;
} REG_TRANSFER_T;

extern void *memset(void *, int, size_t);
extern void *memcpy(void *, const void *, size_t);
extern U8 ef_read_byte(U32 index);
extern U32 u4Dram_Register_Read(void *ctx, U32 reg);
extern void _LoopAryToDelay(void *ctx, REG_TRANSFER_T *ui_reg,
                            REG_TRANSFER_T *mck_reg, U8 count,
                            S8 shift_ui, U8 byte_idx);
extern void __meta_backup_and_set(void *ctx, U8 type, U8 value);
extern void __meta_advance(void *ctx, U8 type);
extern U32 __meta_process_complete(void *ctx, U8 type);
extern void __meta_restore(void *ctx, U8 type);
extern void DramcImpedanceSetValue(void *ctx, U32 code, U32 bit5, U32 type);

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
