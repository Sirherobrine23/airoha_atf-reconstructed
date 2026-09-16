/*
 * AN7581 MD32_initial.c -- functional reconstruction from BL22 oracle.
 *
 * MD32_initializaton_ch() is a direct transcription of the vendor Thumb
 * object. The raw 0x1fcaXXXX accesses are intentional: this AN7581 build
 * programs the MD32/DRAMC block directly instead of going through the newer
 * virtual-register sequence used by AN7583.
 */
#include "recovery_abi.h"

extern U8 DUTTopSetGlobal[];
extern void vSetPHY2ChannelMapping(void *ctx, U32 channel);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern U32 u4Dram_Register_Read(void *ctx, U32 reg);

void MD32_initializaton_ch(void *ctx, U32 channel)
{
    volatile U32 *md32 = mmio32(0x1fca7000U);
    volatile U32 *cfg = mmio32(0x1fca2000U);
    volatile U32 *aux = mmio32(0x1fca3000U);
    U8 *top = DUTTopSetGlobal;

    vSetPHY2ChannelMapping(ctx, (U8)channel);

    *mmio32(0x1fca0000U) = 3U;
    *mmio32(0x1fca3004U) = 0x00410120U;
    *mmio32(0x1fca3004U) = 0x00410100U;

    md32[0x000 / 4] = 0x0fc00000U;
    md32[0x000 / 4] = 0x0fc00001U;
    md32[0x000 / 4] = 0x0fc00801U;
    md32[0x000 / 4] = 0x0fc00801U;
    md32[0x03c / 4] = 0xc3c03000U;
    md32[0x020 / 4] = 0xb0000c00U;
    md32[0x004 / 4] = 0x0000f000U;
    md32[0x004 / 4] = 0x00003000U;
    md32[0x004 / 4] = 0;
    md32[0x008 / 4] = 0;
    md32[0x00c / 4] = 0x40000040U;
    md32[0x03c / 4] = 0xc3c00000U;
    md32[0x03c / 4] = 0xc3000000U;
    md32[0x03c / 4] = 0xc3000000U;
    md32[0x00c / 4] = 0x40000000U;
    md32[0x038 / 4] = 0;
    md32[0x020 / 4] = 0xa0000c00U;
    md32[0x00c / 4] = 0;
    md32[0x004 / 4] = 0x00c00ffcU;
    md32[0x038 / 4] = 0x3fU;
    md32[0x000 / 4] = 0x0fc00809U;
    md32[0x000 / 4] = 0x0fc00801U;
    md32[0x010 / 4] = 0x0010083eU;
    md32[0x014 / 4] = 0x70U;
    md32[0x2d8 / 4] = 0x703cU;
    md32[0x064 / 4] = 0xb49U;
    md32[0x054 / 4] = 0x0c10261cU;
    md32[0x048 / 4] = 0x340a7221U;
    md32[0x04c / 4] = 0x02050c0aU;
    md32[0x050 / 4] = 0x0b205e4aU;
    md32[0x050 / 4] = 0x0f205e4aU;
    md32[0x058 / 4] = 0x66cb020fU;
    md32[0x05c / 4] = 0x0006b13cU;
    md32[0x000 / 4] = 0x44c00801U;
    md32[0x000 / 4] = 0x44c00a01U;
    md32[0x000 / 4] = 0x44c00b01U;

    cfg[0x020 / 4] = 0x00030000U;
    cfg[0x030 / 4] = 0x00030000U;
    cfg[0x060 / 4] = 0x00010000U;
    cfg[0x070 / 4] = 0x00020000U;

    if (top[0xad] == 1U)
        md32[0] = 0x44c00b11U;

    aux[0] = 0x8fdfU;
    aux[0x08 / 4] = 0x10U;

    if ((top[0x76] | top[0x75]) != 0U)
        md32[0x038 / 4] = 0x0c00003fU;

    if (top[0x8d] == 1U)
        md32[0] = 0x44c00b05U;

    if (top[0x07] == 1U) {
        *mmio32(0x1fca01a8U) = 6U;
        vPhyByteIO32WriteMsk(ctx, 0x0100084cU,
                             0x06000000U, 0xffff0000U);
        vPhyByteIO32WriteMsk(ctx, 0x00000168U,
                             0x00400000U, 0x00400000U);
        cfg[0x024 / 4] = 2U;
        cfg[0x034 / 4] = 2U;
        cfg[0x0a4 / 4] = 2U;
    }

    if (top[0xad] == 1U) {
        vIO32WriteMsk(ctx, 0x010008a4U, 0, 0x0003f000U);
        vIO32WriteMsk(ctx, 0x010008a4U, 0, 0x00fc0000U);
    } else if (u4Dram_Register_Read(ctx, 0) & 0x01000000U) {
        vPhyByteIO32WriteMsk(ctx, 0x01000850U, 4U, 0xffffU);
    }

    vSetPHY2ChannelMapping(ctx, 0);
}

void MD32_initializaton(void *ctx)
{
    U32 channel = 0;
    U32 count = raw_u8(ctx, 0x00);

    if (count < 1U)
        count = 1U;

    /* The wrapper's broadcast disable is visible in the oracle's call graph. */
    extern void DramcBroadcastOnOff(U32 enable);
    DramcBroadcastOnOff(0);
    do {
        MD32_initializaton_ch(ctx, channel++);
    } while ((U8)channel < count);
}
