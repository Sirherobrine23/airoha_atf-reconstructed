/*
 * dramc_dv_dut.c -- functional reconstruction from the vendor BL22 object.
 *
 * DUTTopSetGlobal remains byte-offset based because the original private
 * structure declaration is absent. Control flow and stored values below are
 * taken directly from the Thumb oracle.
 */
#include "recovery_abi.h"

#include <stdint.h>

extern U8 DUTTopSetGlobal[];
extern U8 DUTShufConfigGlobal[];
extern U8 HWFUNCEnableGlobal[];
extern U32 MEM_TYPE;
extern U32 GetDataRateByFreq(void *ctx);
extern U32 vGet_Div_Mode(void *ctx);
extern U32 is_ddr3_family(void *ctx);
extern U32 is_ddr4_family(void *ctx);

extern U32 pkg_type;
extern U32 gpio_31;


void DramcDUTTopSet(void *ctx)
{
    U8 *d = DUTTopSetGlobal;
    uintptr_t p = (uintptr_t)ctx;
    U32 type = raw_u32(ctx, 0x18);
    U32 mode73 = 0;

    if (type == 3U || pkg_type == 0U || (pkg_type == 1U && gpio_31 == 0U))
        raw_set_u32(ctx, 0x44, 0x10U);
    else if (pkg_type == 1U && gpio_31 != 0U)
        raw_set_u32(ctx, 0x44, 0x20U);

    d[0xae] = 0; d[0xad] = 0;
    d[0xb3] = (U8)(MEM_TYPE >> 0); d[0xb4] = (U8)(MEM_TYPE >> 8);
    d[0xb5] = (U8)(MEM_TYPE >> 16); d[0xb6] = (U8)(MEM_TYPE >> 24);
    d[0xaf] = 0; d[0xb0] = 0; d[0xb1] = 0; d[0xb2] = 0;
    d[0xac] = raw_u32(ctx, 0x08) == 1U;
    d[0xab] = 0; d[0x04] = 0; d[0xa9] = 0; d[0xa8] = 0;
    d[0xa7] = 0; d[0xa5] = 0; d[0xa4] = 0; d[0xa1] = 0;
    d[0xa0] = 0; d[0x9f] = 0; d[0x9e] = 0;
    d[0xaa] = 1; d[0xa6] = 1; d[0xa3] = 1; d[0xa2] = 1; d[0x9d] = 1;
    d[0x9c] = 0; d[0x96] = 0; d[0x94] = 0; d[0x93] = 0; d[0x92] = 0;
    d[0x91] = 0; d[0x90] = 0; d[0x8e] = 0; d[0x8d] = 0; d[0x8c] = 0;
    d[0x8b] = 0; d[0x89] = 0; d[0x88] = 0; d[0x86] = 0; d[0x85] = 0;
    d[0x84] = 0; d[0x83] = 0;
    d[0x9b] = 1; d[0x9a] = 1; d[0x99] = 1;
    d[0x98] = 2; d[0x97] = 4; d[0x95] = 8;
    d[0x8f] = 1; d[0x8a] = 1; d[0x87] = 1; d[0x82] = 3;

    if (is_ddr4_family(ctx)) {
        U8 v = pkg_type == 1U ? 2U : 4U;
        d[0x81] = v; d[0x80] = v;
    }
    if (is_ddr3_family(ctx)) {
        U8 v = pkg_type == 1U ? 1U : 3U;
        d[0x81] = v; d[0x80] = v;
    }
    if (is_ddr4_family(ctx))
        mode73 = raw_u32(ctx, 0x44) == 0x20U;

    d[0x7f] = 0;
    d[0x7e] = 0;
    d[0x7d] = 0;
    d[0x7c] = 0;
    d[0x7b] = 0;
    d[0x7a] = 0;
    d[0x79] = 0;
    d[0x78] = 0;
    d[0x77] = 0;
    d[0x75] = 0;
    d[0x74] = 0;
    d[0x5f] = 0;
    d[0x60] = 0;
    d[0x61] = 0;
    d[0x62] = 0;
    d[0x5b] = 0;
    d[0x5c] = 0;
    d[0x5d] = 0;
    d[0x5e] = 0;
    d[0x57] = 0;
    d[0x58] = 0;
    d[0x59] = 0;
    d[0x5a] = 0;
    d[0x53] = 0;
    d[0x54] = 0;
    d[0x55] = 0;
    d[0x56] = 0;
    d[0x6f] = 0;
    d[0x70] = 0;
    d[0x71] = 0;
    d[0x72] = 0;
    d[0x6b] = 0;
    d[0x6c] = 0;
    d[0x6d] = 0;
    d[0x6e] = 0;
    d[0x67] = 0;
    d[0x68] = 0;
    d[0x69] = 0;
    d[0x6a] = 0;
    d[0x63] = 0;
    d[0x64] = 0;
    d[0x65] = 0;
    d[0x66] = 0;
    d[0x12] = 0;
    d[0x0e] = 0;
    d[0x0f] = 0;
    d[0x0b] = 0;
    d[0x0a] = 0;
    d[0x09] = 0;
    d[0x08] = 0;
    d[0x07] = 0;
    d[0x05] = 0;
    d[0x73] = (U8)mode73;
    d[0x76] = 1;
    raw_set_u32(ctx, 0xb8, 0);
    d[0x0c] = 0x55; d[0x0d] = 8; d[0x10] = 1; d[0x11] = 1; d[0x06] = 1;
    d[0x00] = (U8)(p >> 0); d[0x01] = (U8)(p >> 8);
    d[0x02] = (U8)(p >> 16); d[0x03] = (U8)(p >> 24);
}

void DramcDUTShuSet(void *ctx)
{
    U32 rate = GetDataRateByFreq(ctx);
    U8 *d = DUTShufConfigGlobal;

    d[0x22] = (U8)(rate >> 0);
    d[0x23] = (U8)(rate >> 8);
    d[0x24] = (U8)(rate >> 16);
    d[0x25] = (U8)(rate >> 24);

    d[0x1e] = 1; d[0x1f] = 0; d[0x1d] = 1; d[0x20] = 1;
    d[0x21] = vGet_Div_Mode(ctx) == 1 ? 8 : 4;
    d[0x19] = 0; d[0x18] = 0; d[0x17] = 0;
    d[0x12] = 4; d[0x16] = 0; d[0x0f] = 0x3f; d[0x13] = 1;
    d[0x10] = 0; d[0x0e] = 0; d[0x0d] = 0x0f; d[0x0c] = 0;
    d[0x0b] = 0; d[0x08] = 0; d[0x07] = 0; d[0x06] = 0;
    d[0x05] = 0; d[0x04] = 0; d[0x00] = 0; d[0x01] = 0;
    d[0x02] = 0; d[0x03] = 0;
}

void DramcHWFuncSet(void)
{
    U32 i;
    for (i = 0; i < 7; i++)
        HWFUNCEnableGlobal[i] = 0;
}
