/*
 * AN7581 Hal_io.c -- functional reconstruction from BL22 Hal_io.o.
 *
 * Recovered from function-section disassembly, relocations and the public
 * MediaTek Hal_io lineage.  The 0x4c-byte manager is kept as a raw byte
 * object because the original private structure definition is unavailable.
 * This file is not yet claimed byte-identical to the Buildroot GCC 10.3.0
 * object oracle.
 */
#include "recovery_abi.h"

#define BIT(n) (1U << (n))

extern void vSetPHY2ChannelMapping(void *ctx, U32 channel);
extern U32 vGetPHY2ChannelMapping(void *ctx);
extern void vSetRank(void *ctx, U32 rank);
extern U32 u1GetRank(void *ctx);

static U8 dramc_io_manager[0x4c] __attribute__((aligned(4)));

/* Exact 112-byte table from .rodata.REG_BASE_ADDR_LIST. */
const U32 REG_BASE_ADDR_LIST[28] = {
    0x0fc80000U, 0x0fc90000U, 0, 0,
    0x0fc84000U, 0x0fc94000U, 0, 0,
    0x0fc8a000U, 0x0fc9a000U, 0, 0,
    0x0fc88000U, 0x0fc98000U, 0, 0,
    0x0fc60000U, 0, 0, 0,
    0x00006000U, 0, 0, 0,
    0, 0, 0, 0,
};

static inline void dsb_sy(void)
{
#if defined(__arm__) || defined(__aarch64__)
    __asm__ volatile("dsb sy" ::: "memory");
#else
    __asm__ volatile("" ::: "memory");
#endif
}

static inline U8 mgr8(U32 off)
{
    return dramc_io_manager[off];
}

static inline void set_mgr8(U32 off, U8 value)
{
    dramc_io_manager[off] = value;
}

static inline U32 mgr32(U32 off)
{
    return *(U32 *)(void *)(dramc_io_manager + off);
}

static inline void set_mgr32(U32 off, U32 value)
{
    *(U32 *)(void *)(dramc_io_manager + off) = value;
}

static inline U32 meta_off(U32 base, U8 depth, U8 type)
{
    return base + ((U32)depth << 2) + type;
}

void __meta_set(void *ctx, U8 type, U8 value)
{
    U8 depth = mgr8(0x3cU + type);
    set_mgr8(meta_off(0x14, depth, type), value);

    if (type == 0)
        vSetPHY2ChannelMapping(ctx, value);
    else if (type == 1)
        vSetRank(ctx, value);
    else if (type == 2)
        raw_set_u32(ctx, 0x98, value);
}

void __meta_restore(void *ctx, U8 type)
{
    U8 depth = mgr8(0x3cU + type);
    U32 saved_off = meta_off(0x00, depth, type);
    U32 ref_off = meta_off(0x28, depth, type);

    __meta_set(ctx, type, mgr8(saved_off));
    set_mgr8(ref_off, (U8)(mgr8(ref_off) - 1U));
    if (depth != 0)
        set_mgr8(0x3cU + type, (U8)(depth - 1U));
}

void __meta_backup(void *ctx, U8 type)
{
    U8 depth = mgr8(0x3cU + type);
    U32 ref_off = meta_off(0x28, depth, type);
    U8 value;

    /* Matches the vendor comparison exactly (depth <= 4). */
    if (mgr8(ref_off) == 1 && depth <= 4) {
        depth++;
        set_mgr8(0x3cU + type, depth);
    }

    if (type == 0)
        value = (U8)vGetPHY2ChannelMapping(ctx);
    else if (type == 1)
        value = (U8)u1GetRank(ctx);
    else if (type == 2)
        value = (U8)raw_u32(ctx, 0x98);
    else
        value = mgr8(meta_off(0x14, depth, type));

    set_mgr8(meta_off(0x14, depth, type), value);
    set_mgr8(meta_off(0x00, depth, type), value);
    ref_off = meta_off(0x28, depth, type);
    set_mgr8(ref_off, (U8)(mgr8(ref_off) + 1U));
}

void __meta_backup_and_set(void *ctx, U8 type, U8 value)
{
    __meta_backup(ctx, type);
    __meta_set(ctx, type, value);
}

void __meta_advance(void *ctx, U8 type)
{
    U8 depth = mgr8(0x3cU + type);
    U32 off = meta_off(0x14, depth, type);
    U8 value = (U8)(mgr8(off) + 1U);
    set_mgr8(off, value);
    __meta_set(ctx, type, value);
}

U32 __meta_process_complete(void *ctx, U8 type)
{
    U8 limit = type == 0 ? raw_u8(ctx, 0x00) : raw_u8(ctx, 0x08);
    U8 depth = mgr8(0x3cU + type);
    U8 value = mgr8(meta_off(0x14, depth, type));
    return value >= limit;
}

U32 __meta_loop_complete_for_next(void *ctx, U8 type, U8 next)
{
    U32 done = __meta_process_complete(ctx, type);
    if (done)
        __meta_set(ctx, type, next);
    return done;
}

U32 __meta_get(void *ctx, U8 type)
{
    U8 depth = mgr8(0x3cU + type);
    U8 value;

    if (type == 0)
        value = (U8)vGetPHY2ChannelMapping(ctx);
    else if (type == 1)
        value = (U8)u1GetRank(ctx);
    else if (type == 2)
        value = (U8)raw_u32(ctx, 0x98);
    else
        value = mgr8(meta_off(0x14, depth, type));

    set_mgr8(meta_off(0x14, depth, type), value);
    return mgr8(meta_off(0x14, depth, type));
}

void channel_msk_set(void *ctx, U32 mask)
{
    (void)ctx;
    set_mgr32(0x40, mask);
}

void channel_msk_backup(void *ctx)
{
    (void)ctx;
    set_mgr8(0x44, (U8)mgr32(0x40));
    set_mgr8(0x45, (U8)(mgr8(0x45) + 1U));
}

void channel_msk_restore(void *ctx)
{
    (void)ctx;
    set_mgr32(0x40, mgr8(0x44));
    set_mgr8(0x45, (U8)(mgr8(0x45) - 1U));
}

void channel_msk_backup_and_set(void *ctx, U32 mask)
{
    channel_msk_backup(ctx);
    set_mgr32(0x40, mask);
}

void channel_msk_op_enable(void)
{
    set_mgr8(0x46, 1);
}

void channel_msk_op_disable(void)
{
    set_mgr8(0x46, 0);
}

U32 NeedSwapARToBR(void)
{
    return 0;
}

U32 u1PhyByteSwapOnOff(U32 enable)
{
    set_mgr8(0x47, (U8)enable);
    return 0;
}

U32 u4PhyAoByteSwap(void)
{
    return 0;
}

U32 u4RegBaseAddrTranslate(void *ctx, U32 channel, U32 reg)
{
    U32 type = (reg >> 23) & 0xfU;
    U32 offset = reg & 0xfffffU;
    U32 shuffle = raw_u32(ctx, 0x98);
    U32 mode = raw_u32(ctx, 0x0c);
    U32 base = REG_BASE_ADDR_LIST[(type << 2) + channel];

    switch (type) {
    case 0:
    case 2: {
        U32 bit21 = (reg >> 21) & 1U;
        if (bit21 && shuffle)
            base += shuffle * (type == 0 ? 0x700U : 0x900U);
        if ((reg & BIT(22)) && mode == 1) {
            if (type == 0)
                offset += 0x200U;
            else
                offset += bit21 ? 0x280U : 0x200U;
        }
        break;
    }
    case 1:
        if (mode == 1) {
            if ((offset - 0x600U) <= 0xffU)
                offset += 0x100U;
            else if ((offset - 0x980U) <= 0x200U)
                offset += 0x200U;
        }
        break;
    case 3:
        if (mode == 1) {
            if ((offset - 0x600U) <= 0x2fU ||
                (offset - 0x660U) <= 0x2fU ||
                (offset - 0x6c0U) <= 0x2fU)
                offset += 0x30U;
            else if ((offset - 0x420U) <= 0x13U)
                offset += 0x14U;
        }
        break;
    case 5:
        base = 0x6000U;
        break;
    default:
        break;
    }

    return 0x10000000U + base + offset;
}

U32 DramcVirt2Phys(void *ctx, U32 reg)
{
    return u4RegBaseAddrTranslate(ctx, __meta_get(ctx, 0), reg);
}

U32 u4Dram_Register_Read(void *ctx, U32 reg)
{
    volatile U32 *p = (volatile U32 *)(uintptr_t)
        u4RegBaseAddrTranslate(ctx, raw_u32(ctx, 0x04), reg);
    dsb_sy();
    return *p;
}

void vDram_Register_Write(void *ctx, U32 reg, U32 value)
{
    volatile U32 *p = (volatile U32 *)(uintptr_t)
        u4RegBaseAddrTranslate(ctx, raw_u32(ctx, 0x04), reg);
    *p = value;
    dsb_sy();
}

void vIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask)
{
    volatile U32 *p = (volatile U32 *)(uintptr_t)
        u4RegBaseAddrTranslate(ctx, raw_u32(ctx, 0x04), reg);
    U32 old;
    dsb_sy();
    old = *p;
    *p = old ^ ((value ^ old) & mask);
    dsb_sy();
}

void vIO32WriteMsk_All(void *ctx, U32 reg, U32 value, U32 mask)
{
    U32 count = raw_u8(ctx, 0x00);
    U32 type = (reg >> 23) & 0xfU;
    U32 ch;

    if (raw_u32(ctx, 0x44) == 0x20U && type == 2)
        count++;

    for (ch = 0; ch < count; ch++) {
        volatile U32 *p = (volatile U32 *)(uintptr_t)
            u4RegBaseAddrTranslate(ctx, ch, reg);
        U32 old;
        dsb_sy();
        old = *p;
        *p = old ^ ((value ^ old) & mask);
        dsb_sy();
    }
}

void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask)
{
    vIO32WriteMsk(ctx, reg, value, mask);
}

void vPhyByteIO32WriteMsk_All(void *ctx, U32 reg, U32 value, U32 mask)
{
    vIO32WriteMsk_All(ctx, reg, value, mask);
}

U32 vPhyByteReadFldAlign(void *ctx, U32 reg, U32 field)
{
    volatile U32 *p = (volatile U32 *)(uintptr_t)
        u4RegBaseAddrTranslate(ctx, raw_u32(ctx, 0x04), reg);
    U32 shift, width, mask;

    dsb_sy();
    if (field == 0)
        return *p;

    shift = field & 0xffU;
    width = (field >> 8) & 0xffU;
    mask = (0xffffffffU >> (32U - width)) << shift;
    return (*p & mask) >> shift;
}

void vPhyByteWriteFldAlign(void *ctx, U32 reg, U32 value, U32 field,
                           U32 all_channels)
{
    U32 start, end, shift, width, mask, encoded;
    U32 type = (reg >> 23) & 0xfU;
    U32 ch;

    if (all_channels == 1) {
        start = 0;
        end = raw_u8(ctx, 0x00);
        if (raw_u32(ctx, 0x44) == 0x20U && type == 2)
            end++;
    } else {
        start = raw_u8(ctx, 0x04);
        end = start + 1U;
    }

    if (field == 0) {
        mask = 0xffffffffU;
        encoded = value;
    } else {
        shift = field & 0xffU;
        width = (field >> 8) & 0xffU;
        mask = (0xffffffffU >> (32U - width)) << shift;
        encoded = (value << shift) & mask;
    }

    for (ch = start; ch < end; ch++) {
        volatile U32 *p = (volatile U32 *)(uintptr_t)
            u4RegBaseAddrTranslate(ctx, ch, reg);
        U32 old;
        dsb_sy();
        old = *p;
        *p = (old & ~mask) | encoded;
        dsb_sy();
    }
}

void vPhyByteWriteFldMulti2(void *ctx, ...)
{
    (void)ctx;
}
