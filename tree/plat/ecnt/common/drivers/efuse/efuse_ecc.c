/*
 * AN7583 ECC eFuse driver reconstructed from vendor BL22 efuse_ecc.o.
 *
 * Reference SHA256:
 *   42c6e75962e875cf99018fb606f7d33afd3d706886a535f37d2ab4f57bbe1068
 * Compiler recorded in .comment:
 *   GCC: (Buildroot -g413d1bb) 10.3.0
 *
 * Functional reconstruction from symbols, relocations and Thumb-2
 * disassembly. Byte identity has not yet been established.
 */
#include <stdint.h>
#include <stddef.h>

#include <lib/mmio.h>

extern void tf_log(const char *fmt, ...);
extern int printf(const char *fmt, ...);
extern void fill_secure_data(uint8_t *data, uint8_t offset, size_t len);
extern void *get_secure_data_base(unsigned int index);
extern void ef_read_parse(unsigned int start, unsigned int len,
                          unsigned char *data);
extern void SET_PACKAGE_ID(unsigned int id);
extern int is_asic(void);
extern int get_valid(void);
extern int uartDisable;

#define EFUSE_P1_CTRL           0x1fbf8400U
#define EFUSE_P1_DATA           0x1fbf8500U
#define EFUSE_P2_CTRL           0x1faa0000U
#define EFUSE_P2_DATA32         0x1faa0100U
#define EFUSE_SENTINEL          0xdeadbeefU
#define EFUSE_TIMEOUT           0x00100000U
#define EFUSE_PRE_MAGIC         0xa07923b6U

uint8_t s_efuse_data[0x140];
uint8_t secureConfigOffset[6] = { 5, 7, 8, 10, 11, 12 };

static uintptr_t efuse_ctrl(unsigned int page)
{
        return page == 1 ? EFUSE_P1_CTRL : EFUSE_P2_CTRL;
}

static int efuse_wait_timeout(unsigned int page)
{
        uintptr_t base = efuse_ctrl(page);
        volatile uint32_t timeout = EFUSE_TIMEOUT;

        while (!(mmio_read_32(base) & 1U) && timeout)
                --timeout;

        return timeout == 0;
}

int efuse_preliminary(unsigned int page)
{
        uintptr_t base = efuse_ctrl(page);

        if (efuse_wait_timeout(page))
                return 1;

        mmio_write_32(base + 8, EFUSE_PRE_MAGIC);
        return 0;
}

void efuse_postsetting(unsigned int page)
{
        mmio_write_32(efuse_ctrl(page) + 8, 0);
}

int efuse_reload_page(unsigned int page)
{
        uintptr_t base = efuse_ctrl(page);
        uint32_t value = mmio_read_32(base);

        mmio_write_32(base, value | 0x00010000U);
        return efuse_wait_timeout(page) ? 1 : 0;
}

int efuse_setup_page1_secure_data(unsigned int start, unsigned int len,
                                  unsigned int secure_offset)
{
        volatile uint8_t *page = (volatile uint8_t *)(uintptr_t)EFUSE_P1_DATA;
        unsigned int i;

        /*
         * Page-1 secure bytes are stored twice.  The closed object ORs the
         * two len-byte copies and emits the result one byte at a time.
         */
        for (i = 0; i < len; ++i) {
                uint8_t data = page[start + i] | page[start + len + i];
                fill_secure_data(&data, (uint8_t)(secure_offset + i), 1);
        }

        return 0;
}

int efuse_setup_page2_secure_data(unsigned int start, unsigned int count,
                                  unsigned int secure_offset)
{
        volatile uint32_t *page =
                (volatile uint32_t *)(uintptr_t)EFUSE_P2_DATA32;
        uint32_t *dst = (uint32_t *)get_secure_data_base(start);
        unsigned int i;
        int ret = 0;

        dst = (uint32_t *)((uintptr_t)dst + (secure_offset & ~3U));

        for (i = start; i < start + count; ++i) {
                if (i <= 0x35U)
                        *dst++ = page[i];
                else
                        ret |= 2;
        }

        return ret;
}

int efuse_check_remark(void)
{
        uint8_t value = 0;
        ef_read_parse(3, 1, &value);
        return value & 1U;
}

int efuse_test(void)
{
        return 0;
}

unsigned char ef_read_byte(unsigned int index)
{
        if (mmio_read_32(EFUSE_P1_CTRL + 0x400) == EFUSE_SENTINEL)
                return 0;

        return mmio_read_8(EFUSE_P1_DATA + index);
}

void en7523_packageID_init(void)
{
        uint16_t raw;
        unsigned int id;

        raw = ef_read_byte(0) | ((uint16_t)ef_read_byte(1) << 8);
        if ((raw & 0x08U) != 0)
                id = (raw >> 9) & 0x1fU;
        else
                id = (raw >> 4) & 0x1fU;

        SET_PACKAGE_ID(id);
}

unsigned int read_iddq(void)
{
        return ef_read_byte(0xbb);
}

int efuse_init(void)
{
        int ret;

        if (mmio_read_32(EFUSE_P1_CTRL + 0x400) == EFUSE_SENTINEL ||
            !is_asic()) {
                printf("\033[31;1m don't support efuse \n\033[0m\n");
                return 0;
        }

        ret = efuse_reload_page(1);
        if (ret)
                return ret;

        ret = efuse_setup_page1_secure_data(0, 2, 0);
        if (ret) {
                printf("\033[31;1m fail:2 \n\033[0m\n");
                return ret;
        }

        ret = efuse_setup_page1_secure_data(0x82, 8, 2);
        if (ret) {
                printf("\033[31;1m fail:3 \n\033[0m\n");
                return ret;
        }

        ret = efuse_setup_page1_secure_data(0x92, 0x10, 0x0a);
        if (ret) {
                printf("\033[31;1m fail:4 \n\033[0m\n");
                return ret;
        }

        ret = get_valid();
        if (ret) {
                if (!uartDisable) {
                        tf_log("\0242-2-5\n");
                        tf_log("\024SECURE_VALID\n");
                }

                ret = efuse_reload_page(2);
                if (ret)
                        return ret;

                /* r0 is zero on successful reload in the vendor object. */
                ret = efuse_setup_page2_secure_data(0, 0x35, 0x1c);
                if (ret) {
                        printf("\033[31;1m fail:page2 setup \n\033[0m\n");
                        return ret;
                }

                if (!uartDisable)
                        printf("Secure key exist\n");
                return 0;
        }

        if (!uartDisable) {
                tf_log("\0242-2-6\n");
                tf_log("\024SECURE_INVALID\n");
                printf("Secure key does not exist\n");
        }

        return 0;
}
