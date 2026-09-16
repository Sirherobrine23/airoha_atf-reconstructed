/*
 * TX_RX_auto_gen_library.c -- high-confidence functional reconstruction.
 *
 * Oracle: AN7581/AN7583 BL22 TX_RX_auto_gen_library.o.
 * All ten function sections are byte-identical between AN7581 and AN7583.
 * The table contents and control flow below are taken directly from the
 * relocatable ELF objects.  Parameter names/types are ABI-inferred because
 * the objects contain no DWARF.
 */
#include "recovery_abi.h"

extern U32 MEM_TYPE;
extern U8 DUTTopSetGlobal[];

U32 get_max_val_form_2dim(const U32 value[4])
{
    U32 ret = value[0];
    if (ret < value[1]) ret = value[1];
    if (ret < value[2]) ret = value[2];
    if (ret < value[3]) ret = value[3];
    return ret;
}

U32 get_min_val_form_2dim(const U32 value[4])
{
    U32 ret = value[0];
    if (ret >= value[1]) ret = value[1];
    if (ret >= value[2]) ret = value[2];
    if (ret >= value[3]) ret = value[3];
    return ret;
}

U8 get_dq_ca_p2s_latency(U32 ratio, U32 factor1, U32 factor2)
{
    U8 p2s = (U8)(ratio * (factor1 + 1U) * (factor2 + 1U));
    U8 ret;

    switch (p2s) {
    case 2:  ret = 1; break;
    case 4:  ret = 3; break;
    case 8:  ret = (DUTTopSetGlobal[0x79] == 1) ? 6 : 5; break;
    case 16: ret = (DUTTopSetGlobal[0x79] == 1) ? 11 : 9; break;
    default: ret = 0; break;
    }

    /* Vendor test is exactly: (MEM_TYPE & ~4) == 2. */
    if ((MEM_TYPE & ~4U) == 2U) {
        if (p2s == 4) ret = 5;
        else if (p2s == 8) ret = 7;
    }
    return ret;
}

U8 get_dqsien_p2s_latency(U32 ratio, U32 value)
{
    U8 ret;

    switch (ratio) {
    case 2:  ret = (U8)(value + 2U); break;
    case 4:  ret = (U8)((value << 1) + 4U); break;
    case 8:
        ret = (U8)((value << 2) +
                   ((DUTTopSetGlobal[0x79] == 1) ? 7U : 8U));
        break;
    case 16: ret = (U8)((value << 3) + 18U); break;
    default: ret = 0; break;
    }

    if ((MEM_TYPE & ~4U) == 2U) {
        if (ratio == 4) ret = 5;
        else if (ratio == 8) ret = 7;
    }
    return ret;
}

U8 get_oe_p2s_latency(U32 ratio, U32 value)
{
    switch (ratio) {
    case 2:  return (U8)(value + 1U);
    case 4:  return (U8)((value + 1U) << 1);
    case 8:  return (U8)((value + 1U) << 2);
    case 16: return (U8)((value + 1U) << 3);
    default: return 0;
    }
}

U32 A_div_B_RU(U32 a, U32 b)
{
    U32 q = a / b;
    if (q * b != a)
        ++q;
    return q;
}

static const U8 pc4_rl[24] = {
     9,10,11,12,13,14,15,16,18,20,22,24,
    23,17,19,21,25,26,27,28,29,30,31,32
};
static const U8 pc4_wl[8] = { 9,10,11,12,14,16,18,20 };
static const U8 pc3_rl[14] = { 12,5,13,6,14,7,15,8,16,9,0,10,0,11 };

U32 Get_RL_by_MR_PC4(void *unused_ctx, U32 mr_index, U32 addend)
{
    (void)unused_ctx;
    return addend + ((mr_index <= 23U) ? pc4_rl[mr_index] : 0U);
}

U32 Get_WL_by_MR_PC4(void *unused_ctx, U32 mr_index, U32 addend)
{
    (void)unused_ctx;
    return addend + ((mr_index <= 7U) ? pc4_wl[mr_index] : 0U);
}

U32 Get_RL_by_MR_PC3(void *unused_ctx, U32 mr_value, U32 addend)
{
    U8 idx = (U8)(mr_value - 1U);
    (void)unused_ctx;
    return addend + ((idx <= 13U) ? pc3_rl[idx] : 0U);
}

U32 Get_WL_by_MR_PC3(void *unused0, void *unused1, U32 mr_value, U32 addend)
{
    (void)unused0;
    (void)unused1;
    return addend + ((mr_value <= 7U) ? (U8)(mr_value + 5U) : 0U);
}
