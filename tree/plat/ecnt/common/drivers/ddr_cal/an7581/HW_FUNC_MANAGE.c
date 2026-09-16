/*
 * AN7581 HW_FUNC_MANAGE.c -- functional reconstruction from BL22 oracle.
 *
 * All 18 exported functions are represented. Most are direct control-flow
 * recoveries. DIG_HW_SHUF_IMPCAL_CFG is a mechanical reconstruction of the
 * register packing visible in the Thumb code and should still be validated
 * against the original GCC 10.3.0 object before the blob is deleted.
 */
#include "recovery_abi.h"

extern U8 DUTTopSetGlobal[];
extern U32 MEM_TYPE;
extern U32 DV_p[];
extern U8 HWFUNCEnableGlobal[];

extern void vSetPHY2ChannelMapping(void *ctx, U32 channel);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vIO32WriteMsk_All(void *ctx, U32 reg, U32 value, U32 mask);
extern void vPhyByteWriteFldAlign(void *ctx, U32 reg, U32 value,
                                  U32 field, U32 channel_mask);
extern void DramcBroadcastOnOff(U32 enable);

U8 HW_IMPCAL_CTRL[16];
U8 HW_ZQCAL_CTRL[8];

void print_HW_ZQCAL_config(void) {}
void print_HW_IMPCAL_config(void) {}
void DIG_HW_SHUF_ZQCAL_SWITCH(void) {}
void DIG_HW_SHUF_IMPCAL_SWITCH(void) {}
void DIG_HW_SHUF_SWITCH(void) {}

void DRAMC_HW_FUNC_ZQCAL_config(U8 ctrl[8], U32 unused)
{
    (void)unused;

    ctrl[2] = DUTTopSetGlobal[0xad] == 0;
    if ((MEM_TYPE & ~4U) == 2U)
        ctrl[2] = 0;

    *(U32 *)(void *)(ctrl + 4) = 0x1ffU;
    *(U16 *)(void *)ctrl = 0x1b01U;
}

void DIG_HW_NONSHUF_ZQCAL_CFG(void *ctx, U32 channel)
{
    U32 value;

    vSetPHY2ChannelMapping(ctx, channel);

    value = ((U32)(HW_ZQCAL_CTRL[2] & 1U) << 30) |
            ((U32)(HW_ZQCAL_CTRL[0] & 1U) << 31);

    if ((MEM_TYPE & ~4U) != 2U && channel == 0U)
        value |= 1U << 29;

    vPhyByteIO32WriteMsk(ctx, 0x000001a0U, value, 0xe0000000U);
    vSetPHY2ChannelMapping(ctx, 0);
}

void DIG_HW_SHUF_ZQCAL_CFG(void *ctx, U32 channel, U32 shuffle)
{
    U32 value;

    vSetPHY2ChannelMapping(ctx, channel);
    raw_set_u32(ctx, 0x98, shuffle != 0U);

    value = (U32)HW_ZQCAL_CTRL[1] << 27;
    value |= *(U16 *)(void *)(HW_ZQCAL_CTRL + 4);

    vPhyByteIO32WriteMsk(ctx, 0x0020169cU, value, 0xf800ffffU);

    vSetPHY2ChannelMapping(ctx, 0);
    raw_set_u32(ctx, 0x98, 0);
}

void DIG_HW_NONSHUF_ZQCAL_SWITCH(void *ctx, U32 channel, U32 enable)
{
    vSetPHY2ChannelMapping(ctx, channel);
    vPhyByteIO32WriteMsk(ctx, 0x000001a4U,
                          enable << 31, 0xc0000000U);
    vSetPHY2ChannelMapping(ctx, 0);
}

void DRAMC_HW_FUNC_IMPCAL_config(U8 ctrl[16], U32 index)
{
    U8 *cfg = (U8 *)(uintptr_t)DV_p[5U + index];
    U32 data_rate = *(U32 *)(void *)cfg;
    U32 low = data_rate <= 2666U;
    U32 div = (U32)cfg[7] * 160U;

    ctrl[0] = low ? 0 : 1;
    ctrl[3] = ctrl[0] + 0x1eU;
    ctrl[4] = low ? 3 : 0x10;
    ctrl[5] = 2;
    ctrl[6] = 1;
    ctrl[11] = 8;
    ctrl[12] = 4;
    ctrl[7] = 0x3f;
    ctrl[8] = 0x30;
    ctrl[9] = 0x21;
    ctrl[15] = (U8)(data_rate / div + 1U);
    ctrl[10] = low ? 0x0f : 0x1b;
    ctrl[13] = 3;
}

static U32 pack_imp_5bit_pair(U8 a, U8 b)
{
    U32 v = 0;

    v |= ((U32)a << 5)  & 0x000003e0U;
    v |= ((U32)a << 15) & 0x000f8000U;
    v |= ((U32)a << 25) & 0x3e000000U;
    v |= ((U32)b)       & 0x0000001fU;
    v |= ((U32)b << 10) & 0x00007c00U;
    v |= ((U32)b << 20) & 0x01f00000U;
    return v;
}

void DIG_HW_SHUF_IMPCAL_CFG(void *ctx, U32 channel, U32 shuffle)
{
    const U8 *c = HW_IMPCAL_CTRL;
    U32 value;
    U32 common;

    vSetPHY2ChannelMapping(ctx, channel);
    raw_set_u32(ctx, 0x98, shuffle != 0U);

    vPhyByteIO32WriteMsk(ctx, 0x01201068U,
                          (U32)c[1] << 31, 0x80000000U);

    value = pack_imp_5bit_pair(c[3], c[4]);
    if (c[0] == 0U)
        value |= 0xc0000000U;
    vPhyByteWriteFldAlign(ctx, 0x012010c8U, value, 0U, 0U);

    value = pack_imp_5bit_pair(c[3], c[4]);
    if (c[0] == 0U)
        value |= 0x80000000U;
    vPhyByteIO32WriteMsk(ctx, 0x012010ccU, value, 0xbfffffffU);

    value = pack_imp_5bit_pair(c[6], c[5]);
    vPhyByteIO32WriteMsk(ctx, 0x012010d0U, value, 0x3fffffffU);
    vPhyByteIO32WriteMsk(ctx, 0x012010d4U, value, 0x3fffffffU);

    vPhyByteIO32WriteMsk(ctx, 0x012010dcU,
                          c[14] & 0x3fU, 0x3fU);

    value = ((U32)(c[4] & 0x1fU)      ) |
            ((U32)(c[3] & 0x1fU) <<  8) |
            ((U32)(c[4] & 0x1fU) << 16) |
            ((U32)(c[3] & 0x1fU) << 24);
    vPhyByteIO32WriteMsk(ctx, 0x01201060U, value, 0x1f1f1f1fU);

    value = ((U32)(c[4] & 0x1fU)) |
            ((U32)(c[3] & 0x1fU) << 8);
    vPhyByteIO32WriteMsk(ctx, 0x01201064U, value, 0x00001f1fU);

    value = ((U32)c[15] & 0x7U) |
            ((U32)c[11] << 28) |
            ((U32)c[13] << 20) |
            (((U32)c[12] << 17) & 0x000e0000U);
    vPhyByteIO32WriteMsk(ctx, 0x012010c4U, value, 0xfffff1f7U);

    value = (((U32)c[7] << 24) & 0x7f000000U) |
            (((U32)c[8] << 16) & 0x007f0000U) |
            (((U32)c[9] <<  8) & 0x00007f00U) |
            (U32)c[10];
    vPhyByteIO32WriteMsk(ctx, 0x09200e90U, value, 0x7f7f7fffU);

    common = 0x00777000U;
    if (c[2] & 1U)
        common |= 0x00000700U;

    if (c[0] == 0U) {
        common |= 0x70000000U;
        common |= 0x00000077U;
        common |= 0x00000004U;
    }

    vPhyByteIO32WriteMsk(ctx, 0x0120108cU, common, 0x70777777U);

    value = c[0] == 0U ? 0x00007777U : 0U;
    vPhyByteIO32WriteMsk(ctx, 0x01201090U, value, 0x00007777U);

    raw_set_u32(ctx, 0x98, 0);
    vSetPHY2ChannelMapping(ctx, 0);
}

void DIG_HW_NONSHUF_IMPCAL_SWITCH(void *ctx, U32 channel, U32 enable)
{
    vSetPHY2ChannelMapping(ctx, channel);
    vPhyByteIO32WriteMsk(ctx, 0x010006c4U,
                          enable << 31, 0x80000000U);
    vSetPHY2ChannelMapping(ctx, 0);
}

void DIG_HW_NONSHUF_DQSG_SWITCH(void *ctx, U32 channel, U32 enable)
{
    U32 value = (enable & 1U) ? 0x04400002U : 0U;

    vSetPHY2ChannelMapping(ctx, channel);
    vPhyByteIO32WriteMsk(ctx, 0x010006acU,
                          0x02000000U, 0x02000000U);
    vPhyByteIO32WriteMsk(ctx, 0x010006acU,
                          value, 0x04400002U);
    vSetPHY2ChannelMapping(ctx, 0);
}

void DIG_HW_ATTRIBUTE_INIT(void)
{
    DRAMC_HW_FUNC_ZQCAL_config(HW_ZQCAL_CTRL, 0);
    DRAMC_HW_FUNC_IMPCAL_config(HW_IMPCAL_CTRL, 0);
}

void DIG_HW_NONSHU_MISC_FIX(void *ctx)
{
    vIO32WriteMsk_All(ctx, 0x00000110U, 0U, 0x20000000U);
    vIO32WriteMsk_All(ctx, 0x00000110U, 0U, 0x40000000U);
    vIO32WriteMsk_All(ctx, 0x0000017cU, 0U, 0x200U);

    vPhyByteWriteFldAlign(ctx, 0x00000278U, 0x33333333U, 0U, 1U);
    vPhyByteWriteFldAlign(ctx, 0x0000027cU, 0x33333333U, 0U, 1U);

    vIO32WriteMsk_All(ctx, 0x000002f0U, 0x61U, 0x7ffU);
    vIO32WriteMsk_All(ctx, 0x00000330U, 0U, 0x2U);
    vIO32WriteMsk_All(ctx, 0x00000170U, 0U, 0x4U);
}

void DIG_HW_NONSHUF_CFG(void *ctx)
{
    DramcBroadcastOnOff(0);
    DIG_HW_NONSHUF_ZQCAL_CFG(ctx, 0);
    DIG_HW_NONSHUF_ZQCAL_CFG(ctx, 1);
    DIG_HW_NONSHU_MISC_FIX(ctx);
    DramcBroadcastOnOff(1);
}

void DIG_HW_SHUF_CFG(void *ctx, U32 channel, U32 shuffle)
{
    DIG_HW_SHUF_ZQCAL_CFG(ctx, channel, shuffle);
    DRAMC_HW_FUNC_IMPCAL_config(HW_IMPCAL_CTRL, shuffle);
    DIG_HW_SHUF_IMPCAL_CFG(ctx, channel, shuffle);
}

void DIG_HW_NONSHUF_SWITCH(void *ctx, U32 channel)
{
    DIG_HW_NONSHUF_ZQCAL_SWITCH(ctx, channel, HWFUNCEnableGlobal[6]);
    DIG_HW_NONSHUF_IMPCAL_SWITCH(ctx, channel, HWFUNCEnableGlobal[5]);
    DIG_HW_NONSHUF_DQSG_SWITCH(ctx, channel, HWFUNCEnableGlobal[0]);
}
