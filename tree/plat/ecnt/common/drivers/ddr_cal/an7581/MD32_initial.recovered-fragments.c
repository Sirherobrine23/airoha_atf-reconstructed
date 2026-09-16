/* Shared wrapper recovered from MD32_initial.o. */
#include "recovery_abi.h"

extern void DramcBroadcastOnOff(U32 enable);
extern void MD32_initializaton_ch(void *ctx, U32 channel);

void MD32_initializaton(void *ctx)
{
    U32 channel = 0;
    U32 count = raw_u8(ctx, 0x00);
    if (count < 1)
        count = 1;

    DramcBroadcastOnOff(0);
    do {
        MD32_initializaton_ch(ctx, channel++);
    } while ((U8)channel < count);
}

/* MD32_initializaton_ch() remains chip-specific and unrecovered. */
