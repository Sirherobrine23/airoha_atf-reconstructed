/*
 * AN7581 PHY eFuse loader reconstructed from vendor BL22 efuse_load.o.
 * SHA256: ba564059e09970ff8f689a51b6931d18080807247450f85cafa6c66ecb4ed3d9
 * GCC: (Buildroot -g413d1bb) 10.3.0
 *
 * BL22 and BL23 copies are byte-identical.  Functional reconstruction;
 * byte identity of a rebuilt object has not yet been established.
 */
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>

extern unsigned char ef_read_byte(unsigned int index);

void ef_read_parse(unsigned int start, unsigned int len, unsigned char *data)
{
        unsigned int bit;

        if (start + len > 0x600U)
                return;

        memset(data, 0, (len >> 3) + !!(len & 7U));
        for (bit = 0; bit < len; ++bit) {
                unsigned int src = start + bit;
                unsigned int v = (ef_read_byte(src >> 3) >> (src & 7U)) & 1U;
                data[bit >> 3] |= (unsigned char)(v << (bit & 7U));
        }
}

static void _efuse2rg(unsigned int start, volatile uint32_t *reg,
                      unsigned int bits, unsigned int shift)
{
        uint32_t data = 0;
        uint32_t mask = (0xffffffffU >> (32U - bits)) << shift;
        uint32_t value;

        ef_read_parse(start, bits, (unsigned char *)&data);
        value = mmio_read_32((uintptr_t)reg);
        value ^= ((value ^ (data << shift)) & mask);
        mmio_write_32((uintptr_t)reg, value);
}

void phy_config_efuse_load(void)
{
        uint32_t enabled = 0;

        ef_read_parse(0x1c, 1, (unsigned char *)&enabled);
        if (enabled == 1) {
                _efuse2rg(0x57, (volatile uint32_t *)0x1fac0304U, 5, 19);
                _efuse2rg(0x52, (volatile uint32_t *)0x1fae0304U, 5, 19);
                _efuse2rg(0x25, (volatile uint32_t *)0x1fac0910U, 5, 24);
                _efuse2rg(0x2a, (volatile uint32_t *)0x1fac0914U, 5, 24);
                _efuse2rg(0x2f, (volatile uint32_t *)0x1fac0b00U, 6, 10);
                _efuse2rg(0x39, (volatile uint32_t *)0x1fae0910U, 5, 24);
                _efuse2rg(0x3e, (volatile uint32_t *)0x1fae0914U, 5, 24);
                _efuse2rg(0x43, (volatile uint32_t *)0x1fae0b00U, 6, 10);
                _efuse2rg(0x25, (volatile uint32_t *)0x1fa9a02cU, 5, 7);
                _efuse2rg(0x2a, (volatile uint32_t *)0x1fa9a044U, 5, 4);
                _efuse2rg(0x2f, (volatile uint32_t *)0x1fa9a048U, 6, 8);
        }

        enabled = 0;
        ef_read_parse(0x20, 1, (unsigned char *)&enabled);
        if (enabled == 1) {
                _efuse2rg(0x2a8, (volatile uint32_t *)0x1efbdf20U, 16, 0);
                _efuse2rg(0x2b8, (volatile uint32_t *)0x1efbdf20U, 16, 16);
                _efuse2rg(0x2d8, (volatile uint32_t *)0x1efbdf24U, 16, 0);
                _efuse2rg(0x2e8, (volatile uint32_t *)0x1efbdf24U, 16, 16);
                _efuse2rg(0x2c8, (volatile uint32_t *)0x1efbdf28U, 16, 0);
                _efuse2rg(0x2f8, (volatile uint32_t *)0x1efbdf28U, 16, 16);
        }

        enabled = 0;
        ef_read_parse(0x1d, 1, (unsigned char *)&enabled);
        if (enabled == 1) {
                _efuse2rg(0x1d, (volatile uint32_t *)0x1fa5b77cU, 1, 8);
                _efuse2rg(0x65, (volatile uint32_t *)0x1fa5b77cU, 2, 0);
                _efuse2rg(0x67, (volatile uint32_t *)0x1fa5a144U, 2, 8);
                _efuse2rg(0x1d, (volatile uint32_t *)0x1fa5c77cU, 1, 8);
                _efuse2rg(0x69, (volatile uint32_t *)0x1fa5c77cU, 2, 0);
                _efuse2rg(0x6b, (volatile uint32_t *)0x1fa5a1fcU, 2, 16);
        }

        enabled = 0;
        ef_read_parse(0x1e, 1, (unsigned char *)&enabled);
        if (enabled == 1) {
                _efuse2rg(0x1e, (volatile uint32_t *)0x1fa8b77cU, 1, 8);
                _efuse2rg(0x76, (volatile uint32_t *)0x1fa8b77cU, 2, 0);
                _efuse2rg(0x78, (volatile uint32_t *)0x1fa8a114U, 2, 24);
                _efuse2rg(0x1e, (volatile uint32_t *)0x1fa7b77cU, 1, 8);
                _efuse2rg(0x83, (volatile uint32_t *)0x1fa7b77cU, 2, 0);
                _efuse2rg(0x85, (volatile uint32_t *)0x1fa7a114U, 2, 24);
        }
}
