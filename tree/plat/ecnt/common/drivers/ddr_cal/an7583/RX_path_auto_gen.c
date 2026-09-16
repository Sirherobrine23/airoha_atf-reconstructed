/*
 * RX_path_auto_gen.c -- functional recovery in progress for AN7583.
 *
 * get_RX_path_config() and the tiny min/max helpers are reconstructed from
 * the BL22 Thumb object.  Structure fields are kept as byte offsets until the
 * original MediaTek/Airoha typedefs can be recovered with confidence.
 */
#include "recovery_abi.h"

extern void *DV_p[];
extern U32 MEM_TYPE;
extern U8 DUTTopSetGlobal[];
extern U8 DUTShufConfigGlobal[];

extern U32 Get_RL_by_MR_PC3(void *unused_ctx, U32 mr_value, U32 addend);
extern U32 Get_RL_by_MR_PC4(void *unused_ctx, U32 mr_index, U32 addend);
extern U8 get_dqsien_p2s_latency(U32 ratio, U32 value);
extern U8 get_oe_p2s_latency(U32 ratio, U32 value);
extern U8 get_dq_ca_p2s_latency(U32 ratio, U32 factor1, U32 factor2);
extern U32 u1IsPhaseMode(U32 value);

static U32 rx_load_le32(const U8 *p)
{
    return (U32)p[0] | ((U32)p[1] << 8) |
           ((U32)p[2] << 16) | ((U32)p[3] << 24);
}

U32 get_max_val(const U32 v[4])
{
    U32 r = v[0];
    if (r < v[1]) r = v[1];
    if (r < v[2]) r = v[2];
    if (r < v[3]) r = v[3];
    return r;
}

U32 get_min_val(const U32 v[4])
{
    U32 r = v[0];
    if (r >= v[1]) r = v[1];
    if (r >= v[2]) r = v[2];
    if (r >= v[3]) r = v[3];
    return r;
}

void print_RX_path_config(void) {}
void print_RX_path_rg_val(void) {}

void get_RX_path_config(void *config_, U8 shuffle, U8 byte)
{
    U8 *config = config_;
    const U8 *freq = DV_p[2];
    const U8 *pc3 = DV_p[3];
    const U8 *pc4 = DV_p[4];
    const U8 *shu = DV_p[(U32)byte + 5U];
    const U8 *shuf_cfg = DUTShufConfigGlobal + (U32)byte * 0x26U;
    U32 rl;
    U32 min_rl;
    U32 max_rl;
    U32 base;

    (void)shuffle; /* BL22 AN7583 does not consume r1 in this routine. */

    raw_set_u32(config, 0x54, raw_u32(shu, 0));
    config[0x48] = raw_u8(freq, 1);
    config[0x4a] = raw_u8(freq, 0);
    config[0x50] = raw_u8(shu, 6);

    if (MEM_TYPE == 2U) {
        rl = Get_RL_by_MR_PC3(NULL, raw_u8(pc3, 3), raw_u8(pc3, 4));
        raw_set_u32(config, 0x08, rl);
        raw_set_u32(config, 0x0c, rl);
        raw_set_u32(config, 0x18, rx_load_le32(DUTTopSetGlobal + 0x5f));
        raw_set_u32(config, 0x1c, rx_load_le32(DUTTopSetGlobal + 0x5b));
        raw_set_u32(config, 0x20, rx_load_le32(DUTTopSetGlobal + 0x57));
        raw_set_u32(config, 0x24, rx_load_le32(DUTTopSetGlobal + 0x53));
        raw_set_u32(config, 0x60, 0);
        raw_set_u32(config, 0x64, 0x168);
    } else if (MEM_TYPE == 6U) {
        rl = Get_RL_by_MR_PC4(NULL, raw_u8(pc4, 3), raw_u8(pc4, 4));
        raw_set_u32(config, 0x08, rl);
        raw_set_u32(config, 0x0c, rl);
        raw_set_u32(config, 0x18, 0);
        raw_set_u32(config, 0x1c, 0);
        raw_set_u32(config, 0x20, 0);
        raw_set_u32(config, 0x24, 0);
        raw_set_u32(config, 0x60, 0);
        raw_set_u32(config, 0x64, 0x168);
    }

    config[0x03] = 2;
    config[0x2a] = MEM_TYPE == 2U ? 1U : 2U;
    raw_set_u32(config, 0x30, 100);
    raw_set_u32(config, 0x34, MEM_TYPE == 2U ? 0U : 0x10U);
    config[0x28] = 0;
    raw_set_u32(config, 0x2c, 0x0b);
    config[0x04] = 2;

    base = 1000000U / raw_u32(config, 0x54);
    raw_set_u32(config, 0x3c, base);
    config[0x53] = 1;
    config[0x38] = 2;
    config[0x5c] = config[0x48] / config[0x4a];

    rl = raw_u32(config, 0x08);
    {
        U32 rl2 = raw_u32(config, 0x0c);
        min_rl = rl2 >= rl ? rl : rl2;
        max_rl = rl2 < rl ? rl : rl2;
    }
    raw_set_u32(config, 0x10, min_rl);
    raw_set_u32(config, 0x14, max_rl);

    config[0x51] = shuf_cfg[0x19];
    config[0x29] = get_dqsien_p2s_latency(config[0x48], config[0x51]);
    config[0x4e] = get_oe_p2s_latency(config[0x48], config[0x51]);
    config[0x01] = get_dq_ca_p2s_latency(config[0x5c], raw_u8(freq, 6),
                                         config[0x51]);

    raw_set_u32(config, 0x44, base * config[0x48]);
    config[0x49] = config[0x48] / config[0x38];
    raw_set_u32(config, 0x40, base * config[0x4a]);

    config[0x00] = 1;
    config[0x4b] = 0;
    *(U16 *)(config + 0x4c) = 2;
    config[0x68] = 1;
    config[0x52] = 0x0b;
    raw_set_u32(config, 0x58,
                raw_u32(config, 0x40) / (raw_u8(freq, 6) + 1U));
    config[0x02] = 1;
    config[0x4f] = MEM_TYPE == 2U ? 8U : 0x10U;
    config[0x69] = DUTTopSetGlobal[0xa0];
    raw_set_u32(config, 0x7c, DUTTopSetGlobal[0x85]);
    config[0x6a] = u1IsPhaseMode(rx_load_le32(DUTTopSetGlobal)) == 0;
    raw_set_u32(config, 0x70, 1);
    raw_set_u32(config, 0x6c, 2);
    raw_set_u32(config, 0x74, 1);
    raw_set_u32(config, 0x78, DUTTopSetGlobal[0x89] != 1U);
    config[0x6b] = shuf_cfg[7];
    config[0x80] = raw_u8(shu, 0x0a);
}



extern U32 A_div_B_RU(U32 a, U32 b);

/*
 * Recovered from .text.calculate_RX_path_rg_val.  The vendor object uses
 * anonymous generated structures; offsets are deliberately kept literal.
 * The function body is byte-identical between AN7581 and AN7583 BL22.
 */
void calculate_RX_path_rg_val(void *config_, void *rg_, U8 channel)
{
    U8 *config = config_;
    U8 *rg = rg_;
    U32 numer[4];
    U32 quot[4];
    U32 count = (U32)config[0x03] * config[0x04];
    U32 max_src = get_max_val((const U32 *)(config + 0x18));
    U32 min_src = get_min_val((const U32 *)(config + 0x18));
    U32 base_step = raw_u32(config, 0x3c);
    U32 ui_step = raw_u32(config, 0x44);
    U32 byte_ui = config[0x48];
    U32 bias;
    U32 i;
    U32 max_dqs;
    U32 max_dqsi;
    U32 max_pi;

    (void)channel; /* present in the ABI, unused by the oracle function */

    bias = A_div_B_RU(max_src + 2U * base_step - min_src, ui_step);

    raw_set_u32(rg, 0x160,
                config[0x50] <= 1U ? 1U : (byte_ui == 0x10U));

    /* First pass: derive absolute delay and quotient/remainder components. */
    for (i = 0; i < count; i++) {
        U32 rank_w = raw_u32(config, i < config[0x03] ? 0x08 : 0x0c);
        U32 t;

        t = raw_u32(config, 0x40) * config[0x00];
        t += raw_u32(config, 0x58) * config[0x01];
        t -= raw_u32(config, 0x34);
        t += ui_step * config[0x80];
        t -= base_step * ((U32)config[0x53] * byte_ui);
        t -= base_step * ((U32)config[0x28] + config[0x29] +
                          config[0x2a] + config[0x4b]);
        t += raw_u32(config, 0x18 + 4U * i);
        t += base_step * config[0x4a] *
             ((U32)config[0x02] + 2U * rank_w);

        numer[i] = t;
        quot[i] = t / ui_step;
        raw_set_u32(rg, 0x20 + 4U * i,
                    (t - quot[i] * ui_step) / base_step);
    }

    max_pi = get_min_val(quot);
    raw_set_u32(rg, 0x50, max_pi - bias);

    /* Second pass: split each lane into coarse/UI/PI pieces. */
    for (i = 0; i < count; i++) {
        U32 t = numer[i];
        U32 q = (t - max_pi * ui_step) / base_step;
        U32 coarse;
        U32 rem;
        U32 ui;
        U32 fine;

        if (q < byte_ui)
            coarse = bias;
        else
            coarse = q / byte_ui + bias;

        raw_set_u32(rg, 0x00 + 4U * i, coarse);
        raw_set_u32(rg, 0x10 + 4U * i, coarse);

        rem = t - ui_step * (raw_u32(rg, 0x50) + coarse);
        ui = rem / base_step;
        raw_set_u32(rg, 0x20 + 4U * i, ui);

        fine = (U32)config[0x49] + ui;
        if (byte_ui <= fine) {
            fine -= byte_ui;
            raw_set_u32(rg, 0x10 + 4U * i, coarse + 1U);
        }
        raw_set_u32(rg, 0x30 + 4U * i, fine);

        fine = A_div_B_RU((rem - base_step * ui) << 5, base_step);
        if (fine == 0x20U)
            fine = 0x1fU;

        if (config[0x6a] == 0U) {
            if (fine <= 4U)
                fine = 0;
            else if (fine <= 12U)
                fine = 8;
            else if (fine <= 20U)
                fine = 16;
            else
                fine = 24;
        }
        raw_set_u32(rg, 0x40 + 4U * i, fine);
    }

    /* Convert coarse+fine pieces to absolute UI positions. */
    for (i = 0; i < count; i++) {
        raw_set_u32(rg, 0x20 + 4U * i,
                    raw_u32(rg, 0x20 + 4U * i) +
                    raw_u32(rg, 0x00 + 4U * i) * byte_ui);
        raw_set_u32(rg, 0x30 + 4U * i,
                    raw_u32(rg, 0x30 + 4U * i) +
                    raw_u32(rg, 0x10 + 4U * i) * byte_ui);
    }

    max_dqs = get_max_val((const U32 *)(rg + 0x20));

    {
        U32 t;
        U32 x;
        U32 middle;

        t = raw_u32(config, 0x40) * config[0x00];
        t += raw_u32(config, 0x58) * config[0x01];
        t += 3U * ui_step;
        t += base_step * ((U32)config[0x4a] *
                          ((U32)config[0x02] +
                           2U * raw_u32(config, 0x14)) +
                          5U * config[0x38]);
        t += max_src;
        middle = A_div_B_RU(t, ui_step);
        middle += raw_u32(config, 0x70) -
                  (raw_u32(config, 0x74) + raw_u32(config, 0x6c));
        middle += config[0x80];
        x = raw_u32(config, 0x74) + raw_u32(config, 0x78) + middle;

        raw_set_u32(rg, 0x78, x - 1U);
        raw_set_u32(rg, 0x70, middle + 1U);
        raw_set_u32(rg, 0x74, x);
        raw_set_u32(rg, 0x8c, x);

        if (x <= 7U)
            t = 0;
        else if (x <= 13U)
            t = 1;
        else if (x <= 18U)
            t = 3;
        else
            t = 7;
        raw_set_u32(rg, 0x90, t);
    }

    raw_set_u32(rg, 0x7c, 0);
    raw_set_u32(rg, 0x80, 0);
    raw_set_u32(rg, 0x84, 0);
    raw_set_u32(rg, 0x88, 0);

    /* RX input-buffer delay compensation. */
    {
        U32 p = (U32)config[0x29] + (byte_ui >> 1);
        U32 ref = config[0x4c];
        U32 d;

        if (p < ref) {
            d = (ref - p) * base_step + 2000U;
            raw_set_u32(rg, 0x54, A_div_B_RU(d, ui_step));
        } else {
            d = (p - ref) * base_step;
            raw_set_u32(rg, 0x54,
                        d > 2000U ? A_div_B_RU(d - 2000U, ui_step) : 0U);
        }
    }

    if (2U * ui_step < 8000U)
        raw_set_u32(rg, 0x5c,
                    A_div_B_RU((4000U - ui_step) << 1, ui_step));
    else
        raw_set_u32(rg, 0x5c, 0);

    {
        U32 p = (U32)config[0x29] + 2U * config[0x2a] +
                config[0x4b] + ((U32)config[0x4f] - 1U) +
                config[0x4d];
        U32 t = base_step * p + ui_step + base_step + (ui_step >> 1);
        U32 six = 6U * ui_step;
        U32 d;

        if (six > t)
            d = six - t;
        else
            d = t + ui_step - six;

        raw_set_u32(rg, 0x58,
                    ui_step > d ? A_div_B_RU(d, ui_step) : 0U);
    }

    {
        U32 v = raw_u32(rg, 0x58);
        raw_set_u32(rg, 0x60, v > 2U ? v - 2U : 0U);
    }

    {
        U32 v = raw_u32(rg, 0x50);
        raw_set_u32(rg, 0x94, v > 1U ? v - 2U : 0U);
        raw_set_u32(rg, 0x98, v > 1U ? v - 2U : 0U);
        raw_set_u32(rg, 0x9c, v == 0U ? 0U : v - 1U);
        raw_set_u32(rg, 0x12c, 0);

        {
            U32 lhs = (U32)config[0x52] + 2U + config[0x4e] +
                      raw_u32(rg, 0x9c) * byte_ui;
            U32 rhs = (U32)config[0x29] + v * byte_ui;
            raw_set_u32(rg, 0x128, lhs > rhs ? lhs - rhs : 0U);
        }

        raw_set_u32(rg, 0x64, config[0x68]);
        raw_set_u32(rg, 0x68,
                    raw_u32(config, 0x54) > 2400U ? 2U : 1U);
        raw_set_u32(rg, 0x6c, config[0x49] == 2U ? 1U : 2U);

        if (v <= 2U) {
            raw_set_u32(rg, 0xa0, 0);
        } else if (config[0x68] == 0U || config[0x69] == 1U) {
            raw_set_u32(rg, 0xa0, v - 2U);
        } else if (raw_u32(config, 0x54) == 0x10aaU) {
            raw_set_u32(rg, 0xa0, v - 4U);
        } else if (raw_u32(config, 0x54) <= 2400U) {
            raw_set_u32(rg, 0xa0, v - 2U);
        } else {
            raw_set_u32(rg, 0xa0, v - 3U);
        }
    }

    max_pi = get_max_val((const U32 *)(rg + 0x00));
    max_dqsi = get_max_val((const U32 *)(rg + 0x10));
    raw_set_u32(rg, 0x104, raw_u32(rg, 0x50) +
                (max_pi < max_dqsi ? max_dqsi : max_pi));
    raw_set_u32(rg, 0x108, 2);
    raw_set_u32(rg, 0x10c, 3);
    raw_set_u32(rg, 0x110, 3);
    raw_set_u32(rg, 0x114, config[0x69]);
    raw_set_u32(rg, 0x118, 0);
    raw_set_u32(rg, 0x11c, 1);
    raw_set_u32(rg, 0x120, 0);
    raw_set_u32(rg, 0x124, raw_u32(rg, 0x94) + 1U);

    raw_set_u32(rg, 0x130, max_pi == 0U ? 0U : max_pi - 1U);
    raw_set_u32(rg, 0x134, max_dqs < 15U ? 0U : max_dqs - 15U);
    raw_set_u32(rg, 0x138, max_dqsi == 0U ? 0U : max_dqsi - 1U);
    raw_set_u32(rg, 0x13c, 0);

    if (config[0x69] == 0U) {
        if (base_step == 4U) {
            raw_set_u32(rg, 0xfc, 0);
            raw_set_u32(rg, 0x100, 1);
        } else {
            U32 is_1200 = raw_u32(config, 0x54) == 1200U;
            raw_set_u32(rg, 0xfc, is_1200);
            raw_set_u32(rg, 0x100, is_1200 ? 2U : 1U);
        }
    }

    raw_set_u32(rg, 0x140, 1);
    raw_set_u32(rg, 0x144, 0x60);
    raw_set_u32(rg, 0x148, 1);
    raw_set_u32(rg, 0x14c, 0x3f);
    raw_set_u32(rg, 0x150, 1);
    raw_set_u32(rg, 0x154, 0x60);
    raw_set_u32(rg, 0x158, 1);
    raw_set_u32(rg, 0x15c, 0x3f);
    raw_set_u32(rg, 0xa8, 0);
    raw_set_u32(rg, 0xac, 0);
    raw_set_u32(rg, 0xb0, 0);
    raw_set_u32(rg, 0xb4, 1);
    raw_set_u32(rg, 0xb8, 1);

    for (i = 0; i < count; i++) {
        raw_set_u32(rg, 0xbc + 4U * i, 0);
        raw_set_u32(rg, 0xcc + 4U * i, 0);
        raw_set_u32(rg, 0xdc + 4U * i, 0);
        raw_set_u32(rg, 0xec + 4U * i, 0);
    }
}


extern void vSetPHY2ChannelMapping(void *ctx, U32 channel);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vSetRank(void *ctx, U32 rank);

static U32 rx_pack_mode(U32 code)
{
    U32 half = code >> 1;
    return ((code << 4) & 0x10U) | ((code >> 2) & 1U) |
           ((code << 5) & 0x20U) | (half & 2U) |
           ((half << 2) & 4U) | ((half << 3) & 8U);
}

void RX_PATH_auto_gen_and_set(void *ctx, U32 channel, U32 shuffle)
{
    U8 work[0x1e8];
    U8 *config = work;
    U8 *rg = work + 0x84;
    U32 v;
    U32 rank;
    U32 base;

    get_RX_path_config(config, (U8)channel, (U8)shuffle);
    calculate_RX_path_rg_val(config, rg, (U8)channel);
    vSetPHY2ChannelMapping(ctx, (U8)channel);
    raw_set_u32(ctx, 0x98, shuffle != 0U);

    v = ((raw_u32(rg, 0x5c) << 24) & 0x07000000U) |
        ((raw_u32(rg, 0x60) << 28) & 0x70000000U) |
        (raw_u32(rg, 0x160) & 1U) |
        ((raw_u32(rg, 0x54) << 16) & 0x00070000U) |
        ((raw_u32(rg, 0x58) << 20) & 0x00700000U) |
        ((raw_u32(rg, 0x64) << 8) & 0x100U) |
        ((raw_u32(rg, 0x68) << 9) & 0xe00U) |
        ((raw_u32(rg, 0x6c) << 14) & 0xc000U) |
        ((raw_u32(rg, 0x160) << 1) & 2U);
    vPhyByteIO32WriteMsk(ctx, 0x012010a4U, v, 0x7777cf03U);

    v = ((raw_u32(rg, 0x94) << 16) & 0x001f0000U) |
        ((raw_u32(rg, 0x104) << 21) & 0x03e00000U) |
        (raw_u32(rg, 0xa0) & 0x1fU) |
        ((raw_u32(rg, 0x98) << 10) & 0x7c00U) |
        ((raw_u32(rg, 0x9c) << 5) & 0x3e0U);
    vPhyByteIO32WriteMsk(ctx, 0x012010a0U, v, 0x03ff7fffU);

    v = (raw_u32(rg, 0x114) & 1U) |
        (raw_u32(rg, 0x128) << 24) |
        ((raw_u32(rg, 0x118) << 5) & 0x20U) |
        ((raw_u32(rg, 0x12c) << 16) & 0x00ff0000U) |
        ((raw_u32(rg, 0x120) << 7) & 0x80U) |
        ((raw_u32(rg, 0x124) << 8) & 0x0f00U) |
        ((raw_u32(rg, 0x11c) << 4) & 0x10U) | 0x40U;
    vPhyByteIO32WriteMsk(ctx, 0x012010f8U, v, 0xffff0ff1U);

    vPhyByteIO32WriteMsk(ctx, 0x11200f20U, raw_u8(rg, 0x134), 0xffU);
    vPhyByteIO32WriteMsk(ctx, 0x19200fa0U, raw_u8(rg, 0x134), 0xffU);

    v = ((raw_u32(rg, 0x100) << 25) & 0x0e000000U) |
        (raw_u32(rg, 0xfc) << 29) | 0x11008000U;
    vPhyByteIO32WriteMsk(ctx, 0x91200f04U, v, 0xff008000U);
    vPhyByteIO32WriteMsk(ctx, 0x99200f84U, v, 0xff008000U);

    vPhyByteIO32WriteMsk(ctx, 0x012010ecU,
                         DUTTopSetGlobal[0x89] & 1U, 1U);
    v = ((raw_u32(rg, 0x70) << 8) & 0x1f00U) |
        ((raw_u32(rg, 0x78) << 16) & 0x001f0000U) |
        (raw_u32(rg, 0x74) & 0x1fU);
    vPhyByteIO32WriteMsk(ctx, 0x012010b8U, v, 0x001f1f1fU);

    v = ((raw_u32(rg, 0x88) << 6) & 0x40U) |
        ((raw_u32(rg, 0x84) << 7) & 0x80U) |
        (raw_u32(rg, 0x7c) << 20) |
        ((raw_u32(rg, 0x80) << 8) & 0x000fff00U);
    vPhyByteIO32WriteMsk(ctx, 0x012010fcU, v, 0xffffffdfU);

    vPhyByteIO32WriteMsk(ctx, 0x0020168cU,
                         rx_pack_mode(raw_u32(rg, 0x90)), 0x3fU);

    v = (raw_u32(rg, 0xb4) & 1U) |
        (raw_u32(rg, 0xa8) << 31) |
        ((raw_u32(rg, 0xb0) << 4) & 0xffU) |
        ((raw_u32(rg, 0xac) << 30) & 0x40000000U) |
        ((raw_u32(rg, 0xb8) << 29) & 0x20000000U);
    vPhyByteIO32WriteMsk(ctx, 0x012010a8U, v, 0xc00000f1U);

    /* AN7583 programs one rank per invocation.  The oracle writes config[4]=1
     * before entering this block and performs it once even for channel/rank 1. */
    config[0x04] = 1;
    rank = (U8)channel;
    base = rank;
    vSetRank(ctx, rank);
    vPhyByteIO32WriteMsk(ctx, 0x01600b60U,
                         raw_u32(rg, 0x50) & 0x1fU, 0x1fU);

    v = ((raw_u32(rg, 4U * (base + 0x10U)) << 16) & 0x007f0000U) |
        raw_u8(rg, 4U * (base + 8U));
    vPhyByteIO32WriteMsk(ctx, 0x11600a2cU, v, 0x007f00ffU);
    v = ((raw_u32(rg, 4U * (base + 0x11U)) << 16) & 0x007f0000U) |
        raw_u8(rg, 4U * (base + 9U));
    vPhyByteIO32WriteMsk(ctx, 0x19600aacU, v, 0x007f00ffU);

    vPhyByteIO32WriteMsk(ctx, 0x11600a30U,
                         raw_u8(rg, 0xdc + 4U * base), 0xffU);
    vPhyByteIO32WriteMsk(ctx, 0x19600ab0U,
                         raw_u8(rg, 0xe0 + 4U * base), 0xffU);

    v = ((raw_u32(rg, 0x20 + 4U * base) << 8) & 0xff00U) |
        (raw_u32(rg, 0x40 + 4U * base) & 0x7fU);
    vPhyByteIO32WriteMsk(ctx, 0x11600a24U, v, 0xff7fU);
    v = ((raw_u32(rg, 0x24 + 4U * base) << 8) & 0xff00U) |
        (raw_u32(rg, 0x44 + 4U * base) & 0x7fU);
    vPhyByteIO32WriteMsk(ctx, 0x19600aa4U, v, 0xff7fU);

    v = (raw_u32(rg, 0x30 + 4U * base) << 24) |
        ((raw_u32(rg, 0x20 + 4U * base) << 8) & 0xffffU) |
        (raw_u32(rg, 0x40 + 4U * base) & 0x7fU);
    vPhyByteIO32WriteMsk(ctx, 0x13e00a28U, v, 0xff00ff7fU);
    v = (raw_u32(rg, 0x34 + 4U * base) << 24) |
        ((raw_u32(rg, 0x24 + 4U * base) << 8) & 0xffffU) |
        (raw_u32(rg, 0x44 + 4U * base) & 0x7fU);
    vPhyByteIO32WriteMsk(ctx, 0x1be00aa8U, v, 0xff00ff7fU);

    vSetRank(ctx, 0);
    v = ((raw_u32(rg, 0x140) << 7) & 0x80U) |
        (raw_u32(rg, 0x144) & 0x7fU) |
        ((raw_u32(rg, 0x148) << 15) & 0x8000U) |
        ((raw_u32(rg, 0x14c) << 8) & 0x7f00U);
    vPhyByteIO32WriteMsk(ctx, 0x01600b68U, v, 0xffffU);
}
