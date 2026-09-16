/*
 * AN7583 eFuse bit parser reconstructed from vendor BL22 efuse_load.o.
 * SHA256: d132a2539b8475044900029896313a6225f16a48825d200654bc270e93628b62
 * GCC: (Buildroot -g413d1bb) 10.3.0
 */
#include <stdint.h>
#include <string.h>

extern void tf_log(const char *fmt, ...);
extern int printf(const char *fmt, ...);
extern unsigned char ef_read_byte(unsigned int index);

void ef_read_parse(unsigned int start, unsigned int len, unsigned char *data)
{
        unsigned int bit;

        if (start + len > 0x600U) {
                tf_log("ef_read_parse: range over 0x600\n");
                return;
        }

        memset(data, 0, (len >> 3) + !!(len & 7U));
        for (bit = 0; bit < len; ++bit) {
                unsigned int src = start + bit;
                unsigned int v = (ef_read_byte(src >> 3) >> (src & 7U)) & 1U;
                data[bit >> 3] |= (unsigned char)(v << (bit & 7U));
        }
}

void ef_read_parse_new(unsigned int start, unsigned int end,
                       unsigned char *base_addr)
{
        unsigned int len = end - start + 1U;
        unsigned int bit;

        printf("input base_addr = %p\n", base_addr);
        printf("len = %d\n", len);

        if (start + len > 0x600U) {
                tf_log("ef_read_parse: range over 0x600\n");
                return;
        }

        memset(base_addr, 0, (len >> 3) + !!(len & 7U));
        for (bit = 0; bit < len; ++bit) {
                unsigned int src = start + bit;
                unsigned int v = (ef_read_byte(src >> 3) >> (src & 7U)) & 1U;
                base_addr[bit >> 3] |= (unsigned char)(v << (bit & 7U));
        }
}

void phy_config_efuse_load(void)
{
}

void phy_config_efuse_load_new(unsigned char *base_addr)
{
        printf("phy_config_efuse_load_new\n");
        printf("base_addr = %d\n", (unsigned int)(uintptr_t)base_addr);
        ef_read_parse_new(0x1b, 0x398, base_addr);
        ef_read_parse_new(0x616, 0x69f, base_addr + 0x70);
}
