/*
 * EN7523 eFuse driver reconstructed from vendor efuse.o.
 *
 * SHA256: 4169bd9646d406a33bddf64d32f5d046cd4986a2ab52f0b951f510680f4179a9
 * GCC: (Buildroot 2015.08.1-g6a3c3ed-dirty) 4.9.3
 *
 * BL22 and BL23 contain byte-identical copies of this object.
 * Functional reconstruction; byte identity has not yet been established.
 */
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <plat_private.h>

#define EN7523_EFUSE_CTRL	0x1fbf8208U
#define EN7523_EFUSE_DATA0	0x1fbf8230U
#define EN7523_EFUSE_DATA1	0x1fbf8234U
#define EN7523_EFUSE_DATA2	0x1fbf8238U
#define EN7523_EFUSE_DATA3	0x1fbf823cU
#define EN7523_EFUSE_BUSY	(1U << 30)
#define EN7523_EFUSE_TIMEOUT	0x10000000U
#define EN7523_NS_EFUSE_SIZE	0xc0U

uint8_t ns_efuse_data[EN7523_NS_EFUSE_SIZE];

static int efuse_read_data(unsigned int index, uint32_t data[4], unsigned int merge)
{
	uint32_t ctl;
	uint32_t timeout;
	uint32_t value[4];

	ctl = mmio_read_32(EN7523_EFUSE_CTRL);
	if (ctl & EN7523_EFUSE_BUSY) {
		NOTICE("efuse_read_data busy\n");
		return 4;
	}

	ctl &= 0xfe00ff3fU;
	ctl |= EN7523_EFUSE_BUSY | 0x40U | ((index & 0x1ffU) << 16);
	mmio_write_32(EN7523_EFUSE_CTRL, ctl);

	timeout = EN7523_EFUSE_TIMEOUT;
	do {
		ctl = mmio_read_32(EN7523_EFUSE_CTRL);
		--timeout;
		if (!(ctl & EN7523_EFUSE_BUSY))
			break;
	} while (timeout);

	/* The original also treats a clear on the last count as timeout. */
	if (!timeout) {
		NOTICE("efuse_read_data timeout\n");
		return 3;
	}

	value[0] = mmio_read_32(EN7523_EFUSE_DATA0);
	value[1] = mmio_read_32(EN7523_EFUSE_DATA1);
	value[2] = mmio_read_32(EN7523_EFUSE_DATA2);
	value[3] = mmio_read_32(EN7523_EFUSE_DATA3);

	if (merge) {
		data[0] |= value[0];
		data[1] |= value[1];
		data[2] |= value[2];
		data[3] |= value[3];
	} else {
		data[0] = value[0];
		data[1] = value[1];
		data[2] = value[2];
		data[3] = value[3];
	}

	return 0;
}

unsigned char ef_read_byte(unsigned int index)
{
	if (index > 0xbfU)
		return 0;

	return ns_efuse_data[index];
}

int efuse_init(void)
{
	uint32_t data[4];
	unsigned int i;

	/* Non-secure eFuse window: 12 x 16-byte rows = 192 bytes. */
	for (i = 0; i < 12; ++i) {
		if (efuse_read_data(i << 4, data, 0))
			return 1;
		memcpy(ns_efuse_data + (i << 4), data, 16);
	}

	/*
	 * Secure data is assembled by ORing rows 12..21 with rows 22..31.
	 * Each combined 16-byte row is handed to the trusted-boot store.
	 */
	for (i = 12; i < 22; ++i) {
		if (efuse_read_data(i << 4, data, 0))
			return 1;
		if (efuse_read_data((i + 10) << 4, data, 1))
			return 1;

		fill_secure_data((uint8_t *)data, ((i - 12) << 4) & 0xf0, 16);

		if (i == 12) {
			NOTICE("SECURE_AERA : %x \n", data[0]);
			if (data[0] & 1U) {
				NOTICE("2-2-5\n");
				NOTICE("SECURE_VAILD\n");
			} else {
				NOTICE("2-2-6\n");
				NOTICE("SECURE_INVAILD\n");
				dyn_disable_auth();
				break;
			}
		}
	}

	return 0;
}
