/*
 * AN7583 MD32_initial.c -- functional reconstruction from BL22 oracle.
 *
 * Register addresses, masks, values and conditionals are transcribed from
 * MD32_initial.o. This is intended as a functional source replacement; exact
 * GCC 10.3.0 code-generation equivalence still needs the original toolchain.
 */
#include "recovery_abi.h"

extern U8 DUTTopSetGlobal[];
extern void vSetPHY2ChannelMapping(void *ctx, U32 channel);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vPhyByteWriteFldAlign(void *ctx, U32 reg, U32 value,
                                  U32 field, U32 all_channels);
extern U32 u4Dram_Register_Read(void *ctx, U32 reg);
extern void DramcBroadcastOnOff(U32 enable);

void MD32_initializaton_ch(void *ctx, U32 channel)
{
    U8 *top = DUTTopSetGlobal;
    U32 alt = top[0xad] != 1U;
    U32 v;

    vSetPHY2ChannelMapping(ctx, (U8)channel);

    vPhyByteIO32WriteMsk(ctx, 0x02040000U, 1U, 1U);
    vPhyByteIO32WriteMsk(ctx, 0x02043004U, 0x100U, 0x100U);
    vPhyByteIO32WriteMsk(ctx, 0x02043004U, 0U, 0x30U);

    vPhyByteIO32WriteMsk(ctx, 0x02047000U, 0U, 1U);
    vPhyByteIO32WriteMsk(ctx, 0x02047000U, 1U, 1U);
    vPhyByteIO32WriteMsk(ctx, 0x02047000U, 0x800U, 0x800U);
    vPhyByteIO32WriteMsk(ctx, 0x02047000U, 0U, 0x20U);

    vPhyByteIO32WriteMsk(ctx, 0x0204703cU,
                         alt ? 0x03000000U : 0x01000000U,
                         0x03000000U);
    vPhyByteIO32WriteMsk(ctx, 0x02047020U,
                         0x30000c00U, 0x70000c00U);

    vPhyByteIO32WriteMsk(ctx, 0x02047004U, 0U, 0x00000003U);
    vPhyByteIO32WriteMsk(ctx, 0x02047004U, 0U, 0x0000c000U);
    vPhyByteIO32WriteMsk(ctx, 0x02047004U, 0U, 0x00003000U);
    vPhyByteIO32WriteMsk(ctx, 0x02047008U, 0U, 0x000c0000U);
    vPhyByteIO32WriteMsk(ctx, 0x0204700cU, 0U, 0x00000030U);
    vPhyByteIO32WriteMsk(ctx, 0x0204703cU, 0U, 0x00003000U);
    vPhyByteIO32WriteMsk(ctx, 0x0204703cU, 0U, 0x00c00000U);
    vPhyByteIO32WriteMsk(ctx, 0x0204703cU, 0U, 0x00000c00U);
    vPhyByteIO32WriteMsk(ctx, 0x0204700cU, 0U, 0x00000040U);
    vPhyByteIO32WriteMsk(ctx, 0x02047038U, 0U, 0x01000000U);
    vPhyByteIO32WriteMsk(ctx, 0x02047020U, 0U, 0x10000000U);
    vPhyByteIO32WriteMsk(ctx, 0x0204700cU, 0U, 0x40000000U);

    /* MD32 control lane packing selected by DUTTopSetGlobal[0xad]. */
    v = alt ? 0x00c00ffcU : 0x00400554U;
    vPhyByteIO32WriteMsk(ctx, 0x02047004U, v, 0x00c00ffcU);

    v = alt ? 0x3fU : 0x15U;
    vPhyByteIO32WriteMsk(ctx, 0x02047038U, v, 0x3fU);

    /* Pulse bit 3. */
    vPhyByteIO32WriteMsk(ctx, 0x02047000U, 8U, 8U);
    vPhyByteIO32WriteMsk(ctx, 0x02047000U, 0U, 8U);

    vPhyByteIO32WriteMsk(ctx, 0x02047010U,
                         0x0010083eU, 0x0010083eU);
    vPhyByteIO32WriteMsk(ctx, 0x02047014U, 0x70U, 0x70U);
    vPhyByteIO32WriteMsk(ctx, 0x020472d8U, 0x703cU, 0x703cU);
    vPhyByteIO32WriteMsk(ctx, 0x02047064U, 0xb49U, 0x1fffU);

    /* field == 0 in the oracle: these calls replace the whole register. */
    vPhyByteWriteFldAlign(ctx, 0x02047054U, 0x0c10261cU, 0U, 0U);
    vPhyByteWriteFldAlign(ctx, 0x02047048U, 0x340a7221U, 0U, 0U);
    vPhyByteWriteFldAlign(ctx, 0x0204704cU, 0x02050c0aU, 0U, 0U);
    vPhyByteWriteFldAlign(ctx, 0x02047050U, 0x0b205e4aU, 0U, 0U);
    vPhyByteWriteFldAlign(ctx, 0x02047050U, 0x0f205e4aU, 0U, 0U);

    vPhyByteIO32WriteMsk(ctx, 0x02047058U,
                         0x66cb020fU, 0xffff03ffU);
    vPhyByteIO32WriteMsk(ctx, 0x0204705cU,
                         0x0006b13cU, 0x003ff3ffU);

    vPhyByteIO32WriteMsk(ctx, 0x02047000U,
                         0x44000000U, 0xff000000U);
    vPhyByteIO32WriteMsk(ctx, 0x02047000U, 0x200U, 0x200U);
    vPhyByteIO32WriteMsk(ctx, 0x02047000U, 0x100U, 0x100U);

    vIO32WriteMsk(ctx, 0x02042020U, 0x00030000U, 0xffffffffU);
    vIO32WriteMsk(ctx, 0x02042030U, 0x00030000U, 0xffffffffU);
    vIO32WriteMsk(ctx, 0x02042060U, 0x00010000U, 0xffffffffU);
    vIO32WriteMsk(ctx, 0x02042070U, 0x00020000U, 0xffffffffU);

    if (top[0xad] == 1U)
        vIO32WriteMsk(ctx, 0x02047000U, 0x10U, 0x10U);

    vPhyByteIO32WriteMsk(ctx, 0x02043000U, 0U, 0x1000U);
    vPhyByteIO32WriteMsk(ctx, 0x02043008U, 0x10U, 0xfffU);

    if ((top[0x76] | top[0x75]) != 0U)
        vPhyByteIO32WriteMsk(ctx, 0x02047038U,
                             0x0c000000U, 0x0c000000U);

    if (top[0x8d] == 1U)
        vPhyByteIO32WriteMsk(ctx, 0x02047000U, 4U, 4U);

    if (top[0x07] == 1U) {
        vPhyByteIO32WriteMsk(ctx, 0x020401a8U, 2U, 2U);
        vPhyByteIO32WriteMsk(ctx, 0x020401a8U, 4U, 4U);
        vPhyByteIO32WriteMsk(ctx, 0x0100084cU,
                             0x06000000U, 0xffff0000U);
        vPhyByteIO32WriteMsk(ctx, 0x00000168U,
                             0x00400000U, 0x00400000U);
    }

    vIO32WriteMsk(ctx, 0x02042024U, 2U, 0xffffffffU);
    vIO32WriteMsk(ctx, 0x02042034U, 2U, 0xffffffffU);
    vIO32WriteMsk(ctx, 0x020420a4U, 2U, 0xffffffffU);

    if (top[0xad] == 1U) {
        vIO32WriteMsk(ctx, 0x010008a4U, 0U, 0x0003f000U);
        vIO32WriteMsk(ctx, 0x010008a4U, 0U, 0x00fc0000U);
    } else if (u4Dram_Register_Read(ctx, 0U) & 0x01000000U) {
        vPhyByteIO32WriteMsk(ctx, 0x01000850U, 4U, 0xffffU);
    }

    vSetPHY2ChannelMapping(ctx, 0U);
}

void MD32_initializaton(void *ctx)
{
    U32 channel = 0;
    U32 count = raw_u8(ctx, 0x00);

    if (count < 1U)
        count = 1U;

    DramcBroadcastOnOff(0U);
    do {
        MD32_initializaton_ch(ctx, channel++);
    } while ((U8)channel < count);
}
