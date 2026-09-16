/*
 * TX_path_auto_gen.c -- functional recovery in progress for AN7583.
 *
 * get_TX_path_config() is reconstructed from the BL22 Thumb object with
 * offsets preserved literally because the original structure definitions are
 * not available.  calculate_TX_path_rg_val() and TX_PATH_config() are the
 * remaining large routines in this unit.
 */
#include "recovery_abi.h"

extern void *DV_p[];
extern U32 MEM_TYPE;
extern U8 DUTTopSetGlobal[];
extern U8 DUTShufConfigGlobal[];

extern U32 u1IsPhaseMode(U32 value);
extern U32 Get_WL_by_MR_PC3(void *unused0, void *unused1,
                            U32 mr_value, U32 addend);
extern U32 Get_WL_by_MR_PC4(void *unused_ctx, U32 mr_index, U32 addend);
extern U8 get_dq_ca_p2s_latency(U32 ratio, U32 factor1, U32 factor2);

extern void calculate_TX_path_rg_val(void *config, void *rg, U8 byte, U8 shuffle);
extern void TX_PATH_config(void *ctx, void *rg, U32 shuffle, U32 byte);
extern void vPhyByteIO32WriteMsk(void *ctx, U32 reg, U32 value, U32 mask);
extern void vSetPHY2ChannelMapping(void *ctx, U32 channel);
extern void vSetRank(void *ctx, U32 rank);
extern void vPhyByteWriteFldAlign(void *ctx, U32 reg, U32 value, U32 field,
                                  U32 all_channels);
extern U32 is_ddr3_family(void *ctx);

static U32 load_le32(const U8 *p)
{
    return (U32)p[0] | ((U32)p[1] << 8) |
           ((U32)p[2] << 16) | ((U32)p[3] << 24);
}

void print_TX_path_config(void) {}
void print_TX_path_attribution(void) {}

void get_TX_path_config(void *config_, U8 shuffle, U8 byte)
{
    U8 *config = config_;
    const U8 *pc3 = DV_p[3];
    const U8 *pc4 = DV_p[4];
    const U8 *freq = DV_p[2];
    const U8 *shu = DV_p[(U32)byte + 5U];
    const U8 *shuf_cfg = DUTShufConfigGlobal + (U32)byte * 0x26U;
    const U32 top_mode = load_le32(DUTTopSetGlobal);
    U32 q;
    U32 ratio;
    U32 wl;
    U8 p2s;

    (void)shuffle; /* BL22 AN7583 object does not consume r1. */

    /* Initial values come from the PC3/PC4 and per-shuffle DV blocks. */
    config[0x71] = DUTTopSetGlobal[0xa0];
    config[0x74] = 0;
    raw_set_u32(config, 0x08, raw_u32(shu, 0x00));
    config[0x60] = raw_u8(freq, 0x06);

    config[0x72] = u1IsPhaseMode(top_mode) == 0;
    config[0x73] = u1IsPhaseMode(top_mode) != 0;

    if (MEM_TYPE == 2U) {
        wl = Get_WL_by_MR_PC3(NULL, NULL, raw_u8(pc3, 0), raw_u8(pc3, 4));
        raw_set_u32(config, 0x0c, wl);
        raw_set_u32(config, 0x10,
                    Get_WL_by_MR_PC3(NULL, NULL,
                                     raw_u8(pc3, 0), raw_u8(pc3, 4)));

        raw_set_u32(config, 0x24, load_le32(DUTTopSetGlobal + 0x6f));
        raw_set_u32(config, 0x28, load_le32(DUTTopSetGlobal + 0x6b));
        raw_set_u32(config, 0x2c, load_le32(DUTTopSetGlobal + 0x67));
        raw_set_u32(config, 0x30, load_le32(DUTTopSetGlobal + 0x63));
    } else if (MEM_TYPE == 6U) {
        wl = Get_WL_by_MR_PC4(NULL, raw_u8(pc4, 0), raw_u8(pc4, 4));
        raw_set_u32(config, 0x0c, wl);
        raw_set_u32(config, 0x10,
                    Get_WL_by_MR_PC4(NULL, raw_u8(pc4, 0), raw_u8(pc4, 4)));

        raw_set_u32(config, 0x24, load_le32(DUTTopSetGlobal + 0x6f));
        raw_set_u32(config, 0x28, load_le32(DUTTopSetGlobal + 0x6b));
        raw_set_u32(config, 0x2c, load_le32(DUTTopSetGlobal + 0x67));
        raw_set_u32(config, 0x30, load_le32(DUTTopSetGlobal + 0x63));
    }

    /* Generic PCDDR timing/configuration fields. */
    config[0x4c] = raw_u8(freq, 1);
    config[0x4d] = 0x20;
    config[0x03] = raw_u8(freq, 0);
    config[0x01] = 2;
    config[0x02] = 2;

    q = 1000000U / raw_u32(config, 0x08);
    raw_set_u32(config, 0x54, q);
    raw_set_u32(config, 0x58, q * raw_u8(freq, 0));

    ratio = raw_u8(freq, 1) / raw_u8(freq, 0);
    config[0x44] = (U8)ratio;
    raw_set_u32(config, 0x50, q * raw_u8(freq, 1));
    raw_set_u32(config, 0x5c,
                (q * raw_u8(freq, 0)) / (raw_u8(freq, 6) + 1U));

    config[0x4a] = 2;
    config[0x62] = 4;
    raw_set_u32(config, 0x64, 400);
    raw_set_u32(config, 0x68, 500);
    raw_set_u32(config, 0x6c, 0x10640b01U);
    raw_set_u32(config, 0x7c, raw_u32(shu, 0x0c));

    /* Vendor branch is selected only when freq[1] == 4. */
    if (config[0x4c] == 4U) {
        if (DUTTopSetGlobal[0x76] == 1U) {
            config[0x07] = (DUTTopSetGlobal[0x73] |
                            DUTTopSetGlobal[0x74]) == 0U;
            config[0x49] = config[0x07] == 1U ? 1U : 5U;
        } else {
            config[0x07] = 1;
            config[0x49] = 1;
        }
    } else {
        config[0x07] = 0;
        config[0x49] = 5;
    }

    if (MEM_TYPE == 2U || MEM_TYPE == 6U) {
        config[0x04] = 0;
        raw_set_u32(config, 0x24, 0);
        raw_set_u32(config, 0x28, 0);
        raw_set_u32(config, 0x2c, 0);
        raw_set_u32(config, 0x30, 0);
        config[0x46] = 0;

        config[0x05] = DUTTopSetGlobal[9] != 0U;
        config[0x06] = (U8)MEM_TYPE;
        config[0x45] = 1;
        config[0x00] = 1;

        raw_set_u32(config, 0x14, (q * 2U) / 10U);
        raw_set_u32(config, 0x20, (q * 2U) / 10U);
        raw_set_u32(config, 0x18, (q * 4U) / 10U);
        raw_set_u32(config, 0x1c, (q * 4U) / 10U);
    }

    config[0x61] = 0;
    config[0x77] = 0;
    config[0x76] = shuf_cfg[0x19] == 1U;

    p2s = shuf_cfg[0x19];
    config[0x4b] = get_dq_ca_p2s_latency(config[0x4c], 0, p2s);
    config[0x48] = get_dq_ca_p2s_latency(config[0x4c], 0, p2s);
    config[0x47] = get_dq_ca_p2s_latency(config[0x44], raw_u8(freq, 6), p2s);

    config[0x70] = 1;
    if (config[0x4c] == 16U)
        config[0x75] = 5;
    else if (config[0x4c] == 8U)
        config[0x75] = 7;
    else
        config[0x75] = 11;

    config[0x78] = raw_u8(shu, 9);
}


extern U32 A_div_B_RU(U32 a, U32 b);

/*
 * Recovered from .text.calculate_TX_path_rg_val.  The complete function body
 * is byte-identical between AN7581 and AN7583 BL22; only the surrounding
 * get/config register-programming functions differ between SoCs.
 */
void calculate_TX_path_rg_val(void *config_, void *rg_, U8 byte, U8 shuffle)
{
    U8 *config = config_;
    U8 *rg = rg_;
    U32 numer[4];
    U32 remv[4];
    U32 extra[4];
    U32 outer;

    raw_set_u32(config, 0x34, 0);
    raw_set_u32(config, 0x38, 0);
    raw_set_u32(config, 0x3c, 0);
    raw_set_u32(config, 0x40, 0);

    /* Present in the oracle even though the result is dead. */
    (void)A_div_B_RU(0, raw_u32(config, 0x54));

    for (outer = 0; outer < 2U; outer++) {
        U32 inner;
        U32 outer_word = raw_u32(config, 0x0c + 4U * outer);

        for (inner = 0; inner < 2U; inner++) {
            U32 k = outer * 2U + inner;
            U32 unit = raw_u32(config, 0x54);
            U32 t;
            U32 q;
            U32 r;
            U32 phase;
            U32 tmp;
            U32 b0;

            t = raw_u32(config, 0x14 + 4U * k);
            t += raw_u32(config, 0x58) * config[0x45];
            t -= unit >> 1;
            t += raw_u32(config, 0x5c) * config[0x47];
            t -= raw_u32(config, 0x50) *
                 ((U32)config[0x07] + config[0x04] + 2U);
            t -= raw_u32(config, 0x50) * config[0x05] *
                 ((U32)config[0x06] - 1U - config[0x78]);
            t += unit * ((U32)config[0x77] - config[0x4b] +
                         (U32)config[0x00] * config[0x03] +
                         2U * config[0x03] * outer_word);
            t += raw_u32(config, 0x24 + 4U * k);

            if (shuffle == 1U && MEM_TYPE == 6U &&
                DUTTopSetGlobal[0x77] == 1U)
                t += unit << 2;

            if (DUTTopSetGlobal[0x05] == 1U)
                t -= (raw_u32(config, 0x7c) >> 2) << 3;

            numer[k] = t;
            q = t / unit;
            r = t - q * unit;
            remv[k] = r;
            extra[k] = 0;
            raw_set_u32(rg, 0x30 + 4U * k, q);

            if (config[0x72] == 1U && r > (unit << 1)) {
                U32 twice = unit << 1;
                extra[k] = r - twice;
                remv[k] = twice;
            }

            phase = (remv[k] * config[0x4d]) / unit;
            raw_set_u32(rg, 0x40 + 4U * k, phase);
            raw_set_u32(rg, 0x80 + 4U * k, extra[k] >> 3);

            if (DUTTopSetGlobal[0x05] == 1U) {
                U32 d = raw_u32(config, 0x7c);
                raw_set_u32(rg, 0x80, d);
                raw_set_u32(rg, 0x84, d);
                raw_set_u32(rg, 0x88, d);
                raw_set_u32(rg, 0x8c, d);
            }

            if (config[0x72] == 0U && phase > 3U) {
                if (phase <= 11U) {
                    phase = 8U;
                } else if (phase <= 19U) {
                    phase = 16U;
                } else if (phase <= 29U) {
                    phase = 24U;
                } else {
                    phase = 0U;
                    q++;
                    raw_set_u32(rg, 0x30 + 4U * k, q);
                }
                raw_set_u32(rg, 0x40 + 4U * k, phase);
            }

            tmp = A_div_B_RU(raw_u32(config, 0x64), unit);
            q = raw_u32(rg, 0x30 + 4U * k);
            q += config[0x4c] *
                 ((U32)config[0x05] * ((U32)config[0x06] - 1U) +
                  config[0x04]);
            q -= tmp;
            q -= 1U;
            if (MEM_TYPE == 6U && DUTTopSetGlobal[0x77] == 1U &&
                shuffle == 1U)
                q += 4U;
            raw_set_u32(rg, 0x10 + 4U * k, q);

            b0 = (U32)config[0x00] * config[0x03];
            b0 += (U32)config[0x45] -
                  ((U32)config[0x49] + config[0x4c]) - config[0x62];
            b0 -= (U32)config[0x07] * config[0x4c];
            b0 += config[0x4c] * ((U32)config[0x78] * config[0x05]);
            b0 += 2U * config[0x03] * outer_word;
            b0 += numer[k] / unit;
            if (MEM_TYPE == 6U && DUTTopSetGlobal[0x77] == 1U &&
                shuffle == 1U)
                b0 += 4U;
            raw_set_u32(rg, 0xb0 + 4U * k, b0);

            if (MEM_TYPE == 2U ||
                (MEM_TYPE == 6U && DUTTopSetGlobal[0x77] == 1U &&
                 shuffle == 1U)) {
                raw_set_u32(rg, 0xd0 + 4U * k,
                            (numer[k] << 5) / unit);
            } else if (MEM_TYPE != 2U) {
                U32 mode = DUTShufConfigGlobal[(U32)byte * 0x26U + 7U];
                switch (mode) {
                case 0:
                    raw_set_u32(rg, 0xd0 + 4U * k, 0);
                    break;
                case 1:
                    raw_set_u32(rg, 0xd0 + 4U * k, outer ^ 1U);
                    break;
                case 2:
                    raw_set_u32(rg, 0xd0 + 4U * k, outer);
                    break;
                case 3:
                    raw_set_u32(rg, 0xd0 + 4U * k, outer + 1U);
                    break;
                default:
                    /* Oracle leaves the generated slot untouched. */
                    break;
                }
            }

            /* Second dead rounding call retained from the vendor object. */
            (void)A_div_B_RU(raw_u32(config, 0x64), unit);

            {
                U32 ratio = config[0x4c];
                U32 coarse = raw_u32(rg, 0x30 + 4U * k);
                U32 tail = raw_u32(rg, 0xb0 + 4U * k);
                U32 a = coarse < 6U ? 0U : coarse - 6U;
                U32 b = tail < 6U ? 0U : tail - 6U;

                raw_set_u32(rg, 0x50 + 4U * k, coarse);
                raw_set_u32(rg, 0x20 + 4U * k, coarse / ratio);
                raw_set_u32(rg, 0x30 + 4U * k, coarse % ratio);
                raw_set_u32(rg, 0x60 + 4U * k,
                            raw_u32(rg, 0x40 + 4U * k));
                raw_set_u32(rg, 0x70 + 4U * k,
                            raw_u32(rg, 0x40 + 4U * k));
                raw_set_u32(rg, 0x90 + 4U * k, tail / ratio);
                raw_set_u32(rg, 0xb0 + 4U * k, tail % ratio);
                raw_set_u32(rg, 0x00 + 4U * k, a / ratio);
                raw_set_u32(rg, 0x10 + 4U * k, a % ratio);
                raw_set_u32(rg, 0xa0 + 4U * k, b / ratio);
                raw_set_u32(rg, 0xc0 + 4U * k, b % ratio);
            }
        }
    }

    /* Final generated attributes / mode selection. */
    rg[0xe0] = config[0x70];

    if (config[0x70] == 0U) {
        U32 code = raw_u32(rg, 0x00);
        U32 max_code;
        U32 flag8 = config[0x4c] == 8U;

        rg[0xe1] = 0;
        *(U16 *)(rg + 0xe2) = 0;
        raw_set_u32(rg, 0xe4, 0);
        rg[0xe8] = 0;

        max_code = raw_u32(rg, 0x04);
        if (max_code < code) max_code = code;
        if (max_code < raw_u32(rg, 0x0c)) max_code = raw_u32(rg, 0x0c);
        if (max_code < raw_u32(rg, 0x08)) max_code = raw_u32(rg, 0x08);

        if (config[0x71] == 1U) {
            U32 a = raw_u32(rg, 0x00);
            U32 b = raw_u32(rg, 0x04);
            rg[0xe9] = 0;
            if (a >= raw_u32(rg, 0x08)) a = raw_u32(rg, 0x08);
            if (a != 0U) a -= flag8;
            rg[0xea] = (U8)a;
            if (b >= raw_u32(rg, 0x0c)) b = raw_u32(rg, 0x0c);
            if (b != 0U) b -= flag8;
            rg[0xeb] = (U8)b;
            rg[0xec] = 0;

            if (config[0x4c] == 16U) {
                rg[0xed] = max_code ? (U8)(max_code - 1U) : 0U;
            } else {
                rg[0xed] = max_code <= 1U ? 0U : (U8)(max_code - 2U);
            }
            rg[0xee] = rg[0xed];
        } else {
            U32 data_rate = raw_u32(config, 0x08);
            rg[0xe9] = 1;
            *(U16 *)(rg + 0xea) = 0;
            rg[0xec] = data_rate > 0x0e94U;

            if (config[0x4c] == 4U) {
                rg[0xed] = 1;
                rg[0xee] = 2;
            } else if (config[0x4c] == 8U) {
                if (data_rate <= 0x640U) {
                    rg[0xed] = 0;
                    rg[0xee] = 1;
                } else if (data_rate <= 0x0e95U) {
                    rg[0xed] = 1;
                    rg[0xee] = 2;
                } else if (data_rate <= 0x10aaU) {
                    rg[0xed] = 2;
                    rg[0xee] = 3;
                }
            }
        }

        rg[0xef] = 7;
        rg[0xf0] = 1;
        rg[0xf1] = config[0x46];
        if (config[0x72] == 0U)
            rg[0xf1] = config[0x60] == 0U ? 0x10U : 0x18U;

        rg[0xf2] = (max_code + 6U < config[0x75]) ?
                   config[0x75] : (U8)(max_code + 7U);
    } else {
        U32 ratio = config[0x4c];
        U32 phase_extra;
        U32 x;
        U32 r0v = raw_u32(rg, 0x00);
        U32 r1v = raw_u32(rg, 0x04);
        U32 r2v = raw_u32(rg, 0x08);
        U32 r3v = raw_u32(rg, 0x0c);
        U32 max_code;

        phase_extra = (DV_p[2] && raw_u8(DV_p[2], 1) == 4U &&
                       ((MEM_TYPE & ~4U) == 2U)) ? 1U : 0U;
        rg[0xe1] = (U8)((config[0x76] == 1U ? 3U : 2U) + phase_extra);
        rg[0xe8] = 0;

        {
            U32 mode4 = ratio == 4U;
            U32 hi_sub;
            U32 lo_sub;
            U32 tail = raw_u32(rg, 0xc0);
            U32 base = raw_u32(rg, 0xa0) + mode4;

            if (ratio == 16U) {
                hi_sub = 8U;
                lo_sub = 8U;
            } else if (ratio == 8U) {
                hi_sub = 6U;
                lo_sub = 2U;
            } else {
                hi_sub = 2U;
                lo_sub = 0U;
            }

            x = base + (tail >= ratio - hi_sub);
            rg[0xe2] = (U8)x;
            rg[0xe3] = (U8)((base + 1U) + (tail >= ratio - lo_sub));

            x = ((r1v >= r0v ? r0v : r1v) - 1U) + mode4;
            rg[0xe4] = (U8)(x + 2U);
            if ((ratio >> 1) <= (raw_u32(rg, 0x14) >= raw_u32(rg, 0x10) ?
                                  raw_u32(rg, 0x10) : raw_u32(rg, 0x14)))
                x++;
            rg[0xe5] = (U8)(x + 2U);

            x = ((r3v >= r2v ? r2v : r3v) - 1U) + mode4;
            rg[0xe6] = (U8)(x + 2U);
            if ((ratio >> 1) <= (raw_u32(rg, 0x1c) >= raw_u32(rg, 0x18) ?
                                  raw_u32(rg, 0x18) : raw_u32(rg, 0x1c)))
                x++;
            rg[0xe7] = (U8)(x + 2U);

            if (ratio == 4U) {
                rg[0xe3] = 0;
                rg[0xe5] = 0;
                rg[0xe7] = 0;
            }
        }

        max_code = r1v;
        if (max_code < r0v) max_code = r0v;
        if (max_code < r3v) max_code = r3v;
        if (max_code < r2v) max_code = r2v;

        if (config[0x71] == 1U) {
            U32 flag8 = ratio == 8U;
            U32 a = r0v >= r2v ? r2v : r0v;
            U32 b = r1v >= r3v ? r3v : r1v;
            rg[0xe9] = 0;
            if (a) a -= flag8;
            if (b) b -= flag8;
            rg[0xea] = (U8)a;
            rg[0xeb] = (U8)b;
            rg[0xec] = 0;
            rg[0xed] = ratio == 16U ?
                       (max_code ? (U8)(max_code - 1U) : 0U) :
                       (max_code <= 1U ? 0U : (U8)(max_code - 2U));
            rg[0xee] = rg[0xed];
        } else {
            U32 data_rate = raw_u32(config, 0x08);
            rg[0xe9] = 1;
            *(U16 *)(rg + 0xea) = 0;
            rg[0xec] = data_rate > 0x0e94U;
            if (ratio == 4U) {
                rg[0xed] = 1; rg[0xee] = 2;
            } else if (ratio == 8U) {
                if (data_rate <= 0x640U) { rg[0xed] = 0; rg[0xee] = 1; }
                else if (data_rate <= 0x0e95U) { rg[0xed] = 1; rg[0xee] = 2; }
                else if (data_rate <= 0x10aaU) { rg[0xed] = 2; rg[0xee] = 3; }
            }
        }

        rg[0xef] = 7;
        rg[0xf0] = 1;
        rg[0xf1] = config[0x46];
        if (config[0x72] == 0U)
            rg[0xf1] = config[0x60] == 0U ? 0x10U : 0x18U;
        rg[0xf2] = (max_code + 6U < config[0x75]) ?
                   config[0x75] : (U8)(max_code + 7U);
    }

    if ((DUTTopSetGlobal[0x76] | DUTTopSetGlobal[0x75]) != 0U) {
        rg[0xf3] = 6;
        rg[0xf4] = 6;
    } else {
        rg[0xf3] = 3;
        rg[0xf4] = config[0x4c] == 4U ? 4U : 3U;
    }
    rg[0xf5] = 0;
    *(U16 *)(rg + 0xf6) = 1;
    rg[0xf8] = config[0x4c] == 4U;
    rg[0xf9] = config[0x4c] == 8U ? 0U : 1U;
    *(U16 *)(rg + 0xfa) = 0x100;
    rg[0xfc] = config[0x4c] == 8U;
    rg[0xfd] = config[0x4c] == 4U;
}

static U32 tx_pack_nibble_groups(const U8 *rg, U32 rank_off,
                                 U32 low0_off, U32 low1_off,
                                 U32 high0_off, U32 high1_off)
{
    U32 a = raw_u32(rg, rank_off + low0_off) & 0xfU;
    U32 b = raw_u32(rg, rank_off + low1_off) & 0xfU;
    U32 c = raw_u32(rg, rank_off + high0_off) & 0xfU;
    U32 d = raw_u32(rg, rank_off + high1_off) & 0xfU;

    return a | (b << 4) | (a << 8) | (b << 12) |
           (c << 16) | (d << 20) | (c << 24) | (d << 28);
}

static U32 tx_repeat_byte(U32 value)
{
    value &= 0xffU;
    return value | (value << 8) | (value << 16) | (value << 24);
}

static U32 tx_mix_6bit(U32 even, U32 odd)
{
    even &= 0x3fU;
    odd &= 0x3fU;
    return odd | (even << 8) | (odd << 16) | (even << 24);
}

/* Recovered from AN7583 BL22 .text.TX_PATH_config. */
void TX_PATH_config(void *ctx, void *rg_, U32 channel, U32 byte)
{
    U8 *rg = rg_;
    U8 *top = DUTTopSetGlobal;
    U32 channel8 = (U8)channel;
    U32 byte_sel = byte != 0U;
    U32 top_flags = (U32)top[0x73] | top[0x74];
    U32 special_lane = !(top[0x76] != 0U && top_flags != 0U) &&
                       top[0x75] != 0U && channel == 0U;
    U32 value;
    U32 a, b, src, d0, d4, f1;

    vSetPHY2ChannelMapping(ctx, channel8);
    raw_set_u32(ctx, 0x98, byte_sel);

    value = ((U32)(rg[0xf3] & 0xfU)) |
            ((U32)(rg[0xf4] & 0xfU) << 4) |
            ((U32)(rg[0xf5] & 1U) << 8) |
            ((U32)(rg[0xf6] & 1U) << 11) |
            ((U32)(rg[0xf7] & 1U) << 12) |
            ((U32)(rg[0xf8] & 1U) << 13) |
            ((U32)(rg[0xf9] & 1U) << 14) |
            ((U32)(rg[0xfa] & 0xfU) << 16) |
            ((U32)(rg[0xfb] & 0xfU) << 20) |
            ((U32)(rg[0xfc] & 0xfU) << 24) |
            ((U32)(rg[0xfd] & 0xfU) << 28);
    vPhyByteIO32WriteMsk(ctx, 0x002016c8U, value, 0xffff79ffU);

    value = ((U32)(rg[0xe0] & 1U) << 31) |
            ((U32)(rg[0xe1] & 0xfU)) |
            ((U32)(rg[0xe2] & 0x1fU) << 16) |
            ((U32)(rg[0xe3] & 0x1fU) << 4) |
            ((U32)(rg[0xe8] & 7U) << 24);
    vPhyByteIO32WriteMsk(ctx, 0x002016acU, value, 0x871f01ffU);

    value = ((U32)(rg[0xe9] & 1U) << 31) |
            ((U32)(rg[0xea] & 0xfU) << 16) |
            ((U32)(rg[0xeb] & 0xfU) << 24);
    vPhyByteIO32WriteMsk(ctx, 0x002016a8U, value, 0x8f0f0000U);

    value = ((U32)(rg[0xed] & 0xfU) << 4) |
            ((U32)(rg[0xec] & 0xfU) << 8) |
            (U32)(rg[0xee] & 0xfU);
    vPhyByteIO32WriteMsk(ctx, 0x00201698U, value, 0x31000fffU);

    value = ((U32)rg[0xef] << 12) | ((U32)rg[0xf0] << 7);
    vPhyByteIO32WriteMsk(ctx, 0x00201610U, value, 0x0000f080U);
    vPhyByteIO32WriteMsk(ctx, 0x03a016e8U,
                         ((U32)rg[0xf2] << 22) & 0x07c00000U,
                         0x07c00000U);

    vSetRank(ctx, 0);

    value = tx_pack_nibble_groups(rg, 0, 0x20, 0x24, 0x00, 0x04);
    vPhyByteWriteFldAlign(ctx, 0x00601200U, value, 0, 0);
    vPhyByteWriteFldAlign(ctx, 0x00601204U, value, 0, 0);

    value = tx_pack_nibble_groups(rg, 0, 0x30, 0x34, 0x10, 0x14);
    vPhyByteWriteFldAlign(ctx, 0x00601208U, value, 0, 0);
    vPhyByteWriteFldAlign(ctx, 0x0060120cU, value, 0, 0);

    value = tx_pack_nibble_groups(rg, 0, 0x90, 0x94, 0xa0, 0xa4);
    vPhyByteWriteFldAlign(ctx, 0x00601280U, value, 0, 0);
    value = tx_pack_nibble_groups(rg, 0, 0xb0, 0xb4, 0xc0, 0xc4);
    vPhyByteWriteFldAlign(ctx, 0x00601284U, value, 0, 0);

    a = raw_u32(rg, 0x40) & 0x3fU;
    b = raw_u32(rg, 0x44) & 0x3fU;
    src = special_lane ? b : a;
    value = (src << 8) | (src << 16);
    vPhyByteIO32WriteMsk(ctx, 0x11600a20U, value, 0x003f3f00U);
    vPhyByteIO32WriteMsk(ctx, 0x19600aa0U, value, 0x003f3f00U);

    value = (raw_u32(rg, 0x70) & 0x1fffU) |
            ((raw_u32(rg, 0x74) & 0x1fffU) << 16);
    vPhyByteIO32WriteMsk(ctx, 0x00601210U, value, 0x1fff1fffU);
    vPhyByteIO32WriteMsk(ctx, 0x0060121cU, value, 0x1fff1fffU);
    vPhyByteIO32WriteMsk(ctx, 0x00601214U, value, 0x1fff1fffU);
    vPhyByteIO32WriteMsk(ctx, 0x00601220U, value, 0x1fff1fffU);

    if (special_lane)
        value = (b * 0x01010101U) & 0x3f3f3f3fU;
    else
        value = tx_mix_6bit(a, b);
    vPhyByteIO32WriteMsk(ctx, 0x00601224U, value, 0x3f3f3f3fU);

    value = tx_repeat_byte(raw_u32(rg, 0x80));
    vPhyByteWriteFldAlign(ctx, 0x116009e0U, value, 0, 0);
    vPhyByteWriteFldAlign(ctx, 0x116009e4U, value, 0, 0);
    vPhyByteIO32WriteMsk(ctx, 0x116009ecU, raw_u8(rg, 0x80), 0xffU);

    value = tx_repeat_byte(raw_u32(rg, 0x84));
    vPhyByteWriteFldAlign(ctx, 0x19600a60U, value, 0, 0);
    vPhyByteWriteFldAlign(ctx, 0x19600a64U, value, 0, 0);
    vPhyByteIO32WriteMsk(ctx, 0x19600a6cU, raw_u8(rg, 0x84), 0xffU);

    value = tx_repeat_byte(raw_u32(rg, 0x80));
    vPhyByteWriteFldAlign(ctx, 0x116009e8U, value, 0, 0);
    value = tx_repeat_byte(raw_u32(rg, 0x84));
    vPhyByteWriteFldAlign(ctx, 0x19600a68U, value, 0, 0);

    value = ((U32)(rg[0xe4] & 0x1fU) << 8) |
            (U32)(rg[0xe5] & 0x1fU);
    vPhyByteIO32WriteMsk(ctx, 0x00601234U, value, 0x1f1fU);

    d0 = raw_u32(rg, 0xd0) & 0x3fU;
    d4 = raw_u32(rg, 0xd4) & 0x3fU;
    if (top[0x76] != 0U) {
        if (top_flags != 0U) {
            vPhyByteIO32WriteMsk(ctx, 0x11600a20U,
                                 d0 << 24, 0x3f000000U);
            vPhyByteIO32WriteMsk(ctx, 0x19600aa0U,
                                 d4 << 24, 0x3f000000U);
        } else {
            vPhyByteIO32WriteMsk(ctx, 0x11600a20U,
                                 d4 << 24, 0x3f000000U);
            vPhyByteIO32WriteMsk(ctx, 0x19600aa0U,
                                 d4 << 24, 0x3f000000U);
        }
    } else {
        U32 d = (top[0x75] != 0U && channel == 0U) ? d4 : d0;
        vPhyByteIO32WriteMsk(ctx, 0x11600a20U,
                             d << 24, 0x3f000000U);
        vPhyByteIO32WriteMsk(ctx, 0x19600aa0U,
                             d << 24, 0x3f000000U);
    }

    f1 = (U32)(rg[0xf1] & 0x3fU) << 16;
    if (!is_ddr3_family(ctx)) {
        value = f1 | 0x14000000U;
    } else if (raw_u8(ctx, 0xce) == 1U) {
        value = f1 | 0x1b000000U;
    } else {
        value = f1 | 0x0e000000U;
    }
    vPhyByteIO32WriteMsk(ctx, 0x096009a0U,
                         value, 0x3f3f3f00U);

    if (MEM_TYPE == 2U) {
        if (top[0x76] == 0U) {
            value = (U32)(rg[0xf1] & 0x3fU);
            value = (value << 8) | (value << 16) | (value << 24);
            vPhyByteIO32WriteMsk(ctx, 0x21600b20U,
                                 value, 0x3f3f3f00U);
        }
        vPhyByteIO32WriteMsk(ctx, 0x00201684U,
                             0x000f0100U, 0x007f0100U);
    }
}


void TX_PATH_auto_gen_and_set(void *ctx, U32 shuffle, U32 byte)
{
    /* Oracle stack frame is exactly 0x184 bytes: config[0x80] + rg[0x104]. */
    U8 work[0x184];
    void *config = work;
    void *rg = work + 0x80;
    U8 shu8 = (U8)shuffle;

    get_TX_path_config(config, shu8, (U8)byte);
    calculate_TX_path_rg_val(config, rg, (U8)byte, shu8);
    TX_PATH_config(ctx, rg, shuffle, byte);

    vPhyByteIO32WriteMsk(ctx, 0x00201610U,
        DUTShufConfigGlobal[byte * 0x26U + 0x19U] == 1 ?
            0x00500000U : 0x00400000U,
        0x00f00000U);
    vPhyByteIO32WriteMsk(ctx, 0x00201684U,
        (U32)work[6] << 28, 0xf0000000U);
}

/*
 * TX_PATH_config() is functionally recovered above.
 */
