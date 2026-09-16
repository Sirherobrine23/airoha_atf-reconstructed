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

/* RX_PATH_auto_gen_and_set() remains SoC-specific and is recovered separately. */
