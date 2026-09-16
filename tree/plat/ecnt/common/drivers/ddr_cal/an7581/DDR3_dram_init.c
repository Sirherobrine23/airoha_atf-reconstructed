/* AN7581 DDR3 init reconstructed from DDR3_dram_init.o. */
#include "recovery_abi.h"

extern void DramcModeRegWriteByRank(void *ctx, U32 rank, U32 mr, U32 value);
extern void DramcZQCalibration(void *ctx, U32 rank);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void CKE_FIX_OFF(void *ctx, U32 option, U32 rank);
extern void CKE_FIX_ON(void *ctx, U32 option, U32 rank);
extern void udelay(U32 usec);
extern U16 gMRVal[];
extern U8 DV_p[];

void DDR3_MRS(void *ctx, U32 mr, U32 value, U32 rank)
{
    DramcModeRegWriteByRank(ctx, (U8)rank, (U8)mr, (U16)value);
}

void DDR3_SWZQ(void *ctx, U32 rank)
{
    DramcZQCalibration(ctx, rank);
}

void PC3_dram_init_single_rank(void *ctx, const U8 cfg[8], U32 rank)
{
    U16 mr0 = 0x1100U | ((cfg[3] << 3) & 0x70U) | (cfg[5] & 3U) |
              ((cfg[7] << 9) & 0xe00U) | ((cfg[6] << 3) & 8U) |
              ((cfg[3] << 2) & 4U);
    U16 mr1 = ((cfg[4] << 3) & 0x18U) | ((cfg[1] << 2) & 4U) |
              ((cfg[1] << 7) & 0x200U) | ((cfg[1] << 5) & 0x40U);
    U16 mr2 = ((cfg[2] << 9) & 0x600U) | ((cfg[0] << 3) & 0x38U);
    U32 ch, channels = raw_u32(ctx, 0);

    udelay(1);
    DramcModeRegWriteByRank(ctx, rank, 2, mr2);
    DramcModeRegWriteByRank(ctx, rank, 3, 0);
    DramcModeRegWriteByRank(ctx, rank, 1, mr1);
    DramcModeRegWriteByRank(ctx, rank, 0, mr0);
    DramcZQCalibration(ctx, rank);
    udelay(1);

    for (ch = 0; ch < channels; ++ch) {
        U16 *dst = &gMRVal[ch * 14U + rank * 7U];
        dst[0] = mr0; dst[1] = mr1; dst[2] = mr2; dst[3] = 0;
    }
}

void PC3_DRAM_INIT(void *ctx)
{
    U32 ranks_raw = raw_u32(ctx, 8);
    U32 ranks = (ranks_raw == 1) ? 1U : 2U;
    const U8 *cfg = *(const U8 **)(DV_p + 0x0c);

    vPhyByteIO32WriteMsk(ctx, 0x010007bcU, 0, 0x4000U);
    udelay(1);
    CKE_FIX_OFF(ctx, 1, 0);
    if (ranks_raw != 1) CKE_FIX_OFF(ctx, 1, 1);
    udelay(200);
    vPhyByteIO32WriteMsk(ctx, 0x010007bcU, 0x4000U, 0x4000U);
    udelay(500);
    CKE_FIX_OFF(ctx, 0, 0);
    if (ranks_raw != 1) CKE_FIX_OFF(ctx, 0, 1);
    CKE_FIX_ON(ctx, 1, 0);
    if (ranks_raw != 1) CKE_FIX_ON(ctx, 1, 1);

    PC3_dram_init_single_rank(ctx, cfg, 0);
    if (ranks == 2) PC3_dram_init_single_rank(ctx, cfg, 1);
}
