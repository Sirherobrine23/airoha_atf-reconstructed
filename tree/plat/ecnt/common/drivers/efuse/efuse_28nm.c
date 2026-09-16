/*
 * AN7581 eFuse driver reconstructed from vendor BL22 efuse_28nm.o.
 *
 * Reference SHA256:
 *   e4ec75838fae47cbdaf05af7736a4a69c101158baaf2fa3fe3c911f316c71fab
 * Compiler recorded in .comment:
 *   GCC: (Buildroot -g413d1bb) 10.3.0
 *
 * The control flow, MMIO addresses, constants and public ABI below were
 * recovered from symbols, relocations and Thumb-2 disassembly.  This is a
 * functional reconstruction; byte identity has not yet been established.
 */
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>

extern void tf_log(const char *fmt, ...);
extern int printf(const char *fmt, ...);
extern void fill_secure_data(uint8_t *data, uint8_t offset, size_t len);
extern void SET_PACKAGE_ID(unsigned int id);
extern int uartDisable;

#define SCU_BASE                0x1fb00000U
#define EFUSE_PAGE1_BASE        0x1fbf8200U
#define EFUSE_PAGE2_BASE        0x1faa0000U
#define EFUSE_PAGE_SPLIT        0x40U
#define EFUSE_READY             1U
#define EFUSE_TIMEOUT           0x10000000U
#define EFUSE_UNLOCK0           0xde7502bcU
#define EFUSE_UNLOCK1           0x78d39bf1U
#define NS_EFUSE_SIZE           0xc0U
#define S_EFUSE_SIZE            0x140U

uint8_t ns_efuse_data[NS_EFUSE_SIZE];
uint8_t s_efuse_data[S_EFUSE_SIZE];

static int efuse_read_data_from_efuse(unsigned int index, uint32_t *data,
                                      unsigned int merge)
{
        uintptr_t base;
        uint32_t timeout;
        uint32_t value;

        /* The vendor object returns success without touching data off-ASIC. */
        if (!(mmio_read_32(SCU_BASE + 0x9c) & 1U))
                return 0;

        if (index <= 0x3fU) {
                base = EFUSE_PAGE1_BASE;
        } else {
                index -= EFUSE_PAGE_SPLIT;
                base = EFUSE_PAGE2_BASE;
        }

        mmio_write_32(base + 0x34, EFUSE_UNLOCK0);
        mmio_write_32(base + 0x8c, EFUSE_UNLOCK1);
        mmio_write_32(base + 0x04, index);
        mmio_write_32(base + 0x00, 1);
        mmio_write_32(base + 0x34, 0);
        mmio_write_32(base + 0x8c, 0);

        timeout = EFUSE_TIMEOUT;
        while (mmio_read_32(base + 0x08) != EFUSE_READY && --timeout)
                ;

        if (mmio_read_32(base + 0x08) != EFUSE_READY) {
                tf_log("\nefuse_read_data_from_efuse timeout\n");
                return 4;
        }

        value = mmio_read_32(base + 0x14);
        if (merge)
                *data |= value;
        else
                *data = value;

        return 0;
}

unsigned char ef_read_byte(unsigned int index)
{
        if (index <= 0xbfU)
                return ns_efuse_data[index];

        if (index < 0x200U)
                return s_efuse_data[index - 0xc0U];

        tf_log("\nef_read_byte index error !\n");
        return 1;
}

unsigned int read_iddq(void)
{
        return ns_efuse_data[0xbb];
}

void en7523_packageID_init(void)
{
        uint16_t raw = ns_efuse_data[0] | ((uint16_t)ns_efuse_data[1] << 8);
        unsigned int id;

        /* Bit 3 selects the alternate five-bit package-ID field. */
        if (ns_efuse_data[0] & 0x08U)
                id = (raw >> 9) & 0x1fU;
        else
                id = (raw >> 4) & 0x1fU;

        SET_PACKAGE_ID(id);
}

int efuse_init(void)
{
        uint32_t data = 0;
        unsigned int i;

        /* 48 words -> the 192-byte non-secure byte-addressable window. */
        for (i = 0; i < 0x30U; ++i) {
                data = 0;
                if (efuse_read_data_from_efuse(i, &data, 0))
                        return 1;
                memcpy(ns_efuse_data + i * sizeof(data), &data, sizeof(data));
        }

        /*
         * Secure rows 0x30..0x57 are combined with rows 0x58..0x7f and
         * exported four bytes at a time.  This exactly follows the vendor
         * object's index arithmetic.
         */
        for (i = 0x30U; i < 0x58U; ++i) {
                data = 0;
                if (efuse_read_data_from_efuse(i, &data, 0))
                        return 1;
                if (efuse_read_data_from_efuse(i + 0x28U, &data, 1))
                        return 1;

                fill_secure_data((uint8_t *)&data,
                                 (uint8_t)(((i << 2) + 0x40U) & 0xfcU),
                                 sizeof(data));

                if (i == 0x30U) {
                        tf_log("\024SECURE_AREA : %x \n", data);
                        if (data & 1U) {
                                if (!uartDisable) {
                                        tf_log("\0242-2-5\n");
                                        tf_log("\024SECURE_VALID\n");
                                }
                        } else {
                                if (!uartDisable) {
                                        tf_log("\0242-2-6\n");
                                        tf_log("\024SECURE_INVALID\n");
                                        printf("Secure key does not exist\n");
                                }
                                return 0;
                        }
                }
        }

        if (!uartDisable)
                printf("Secure key exist\n");

        return 0;
}
