/*
 * EN7523 PHY eFuse loader reconstructed from vendor efuse_load.o.
 *
 * SHA256: 221324c68ae0270ff15a1d28dd5e738ea381011eed308bfcabbe33a939fa5182
 * GCC: (Buildroot 2015.08.1-g6a3c3ed-dirty) 4.9.3
 *
 * BL22 and BL23 contain byte-identical copies of this object.
 * Functional reconstruction; byte identity has not yet been established.
 */
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>

extern unsigned char ef_read_byte(unsigned int index);

void ef_read_parse(unsigned int start, unsigned int len, unsigned char *data)
{
	unsigned int bit;
	unsigned int end = start + len;

	if (end > 0x600U)
		return;

	memset(data, 0, (len >> 3) + ((len & 7U) != 0));

	for (bit = start; bit < end; ++bit) {
		unsigned int out_bit = bit - start;
		unsigned char v = ef_read_byte(bit >> 3);

		v = (v >> (bit & 7U)) & 1U;
		data[out_bit >> 3] |= v << (out_bit & 7U);
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
	value &= ~mask;
	value |= (data << shift) & mask;
	mmio_write_32((uintptr_t)reg, value);
}

void miiStationWrite(unsigned int phy, unsigned int reg, unsigned int data)
{
	const uintptr_t mdio = 0x1fb5f01cU;
	unsigned int timeout = 10000;
	uint32_t cmd;

	while ((int32_t)mmio_read_32(mdio) < 0 && --timeout)
		;

	cmd = (data & 0xffffU) | 0x80000000U | 0x00050000U |
	      (reg << 25) | (phy << 20);
	mmio_write_32(mdio, cmd);

	timeout = 10000;
	while ((int32_t)mmio_read_32(mdio) < 0 && --timeout)
		;
}

void MiiExtStationWrite(unsigned int phy, unsigned int devad,
			unsigned int reg, unsigned int data)
{
	miiStationWrite(phy, 0x1f, 0);
	miiStationWrite(phy, 0x0d, devad);
	miiStationWrite(phy, 0x0e, reg);
	miiStationWrite(phy, 0x0d, devad + 0x4000U);
	miiStationWrite(phy, 0x0e, data);
}

#define PARSE6(bit, p) ef_read_parse((bit), 6, (p))
#define PARSE7(bit, p) ef_read_parse((bit), 7, (p))

void ephyEfuseWrite(void)
{
	unsigned char a, b;
	unsigned int bank;
	unsigned int off = 0;

	ef_read_parse(0x6c, 1, &a);
	if (a != 1)
		return;

	ef_read_parse(0x165, 3, &a);
	MiiExtStationWrite(9, 0x1f, 0x115, a);

	PARSE6(0x168, &a);
	MiiExtStationWrite(9, 0x1e, 0xe0, (unsigned int)a << 8);

	/* Two identical fuse layouts, separated by 0x7c bits. */
	for (bank = 0; bank < 2; ++bank, off += 0x7c) {
		unsigned int phy_a = 9 + bank;
		unsigned int phy_b = 11 + bank;
		unsigned int v;

		PARSE6(off + 0x6d, &a); PARSE6(off + 0xa9, &b);
		v = b | ((unsigned int)a << 10);
		MiiExtStationWrite(phy_a, 0x1e, 0x12, v);

		PARSE6(off + 0x73, &a); PARSE6(off + 0xaf, &b);
		MiiExtStationWrite(phy_a, 0x1e, 0x17, b | ((unsigned int)a << 8));

		PARSE6(off + 0x79, &a);
		MiiExtStationWrite(phy_a, 0x1e, 0x19, (unsigned int)a << 8);
		PARSE6(off + 0x7f, &a);
		MiiExtStationWrite(phy_a, 0x1e, 0x21, (unsigned int)a << 8);

		PARSE6(off + 0x9d, &a); PARSE6(off + 0x85, &b);
		MiiExtStationWrite(phy_a, 0x1e, 0x16, b | ((unsigned int)a << 10));
		PARSE6(off + 0xa3, &a); PARSE6(off + 0x8b, &b);
		MiiExtStationWrite(phy_a, 0x1e, 0x18, b | ((unsigned int)a << 8));
		PARSE6(off + 0x91, &b);
		MiiExtStationWrite(phy_a, 0x1e, 0x20, b);
		PARSE6(off + 0x97, &b);
		MiiExtStationWrite(phy_a, 0x1e, 0x22, b);

		PARSE6(off + 0xb5, &a); PARSE6(off + 0xbb, &b);
		MiiExtStationWrite(phy_a, 0x1e, 0x172, b | ((unsigned int)a << 8));
		PARSE6(off + 0xc1, &a); PARSE6(off + 0xc7, &b);
		MiiExtStationWrite(phy_a, 0x1e, 0x173, b | ((unsigned int)a << 8));
		PARSE7(off + 0xcd, &a); PARSE7(off + 0xd4, &b);
		MiiExtStationWrite(phy_a, 0x1e, 0x174,
				   (b | 0x80U) | ((unsigned int)(a | 0x80U) << 8));
		PARSE7(off + 0xdb, &a); PARSE7(off + 0xe2, &b);
		MiiExtStationWrite(phy_a, 0x1e, 0x175,
				   (b | 0x80U) | ((unsigned int)(a | 0x80U) << 8));

		PARSE6(off + 0x16e, &a); PARSE6(off + 0x1aa, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x12, b | ((unsigned int)a << 10));
		PARSE6(off + 0x174, &a); PARSE6(off + 0x1b0, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x17, b | ((unsigned int)a << 8));
		PARSE6(off + 0x17a, &a);
		MiiExtStationWrite(phy_b, 0x1e, 0x19, (unsigned int)a << 8);
		PARSE6(off + 0x180, &a);
		MiiExtStationWrite(phy_b, 0x1e, 0x21, (unsigned int)a << 8);

		PARSE6(off + 0x19e, &a); PARSE6(off + 0x186, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x16, b | ((unsigned int)a << 10));
		PARSE6(off + 0x1a4, &a); PARSE6(off + 0x18c, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x18, b | ((unsigned int)a << 8));
		PARSE6(off + 0x192, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x20, b);
		PARSE6(off + 0x198, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x22, b);

		PARSE6(off + 0x1b6, &a); PARSE6(off + 0x1bc, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x172, b | ((unsigned int)a << 8));
		PARSE6(off + 0x1c2, &a); PARSE6(off + 0x1c8, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x173, b | ((unsigned int)a << 8));
		PARSE7(off + 0x1ce, &a); PARSE7(off + 0x1d5, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x174,
				   (b | 0x80U) | ((unsigned int)(a | 0x80U) << 8));
		PARSE7(off + 0x1dc, &a); PARSE7(off + 0x1e3, &b);
		MiiExtStationWrite(phy_b, 0x1e, 0x175,
				   (b | 0x80U) | ((unsigned int)(a | 0x80U) << 8));
	}
}

void phy_config_efuse_load(void)
{
	/* These 18 addresses are the complete .rodata payload in efuse_load.o. */
	volatile uint32_t *rg_a[9] = {
		(volatile uint32_t *)0x1fa93910U,
		(volatile uint32_t *)0x1fa93914U,
		(volatile uint32_t *)0x1fa93b00U,
		(volatile uint32_t *)0x1fa95910U,
		(volatile uint32_t *)0x1fa95914U,
		(volatile uint32_t *)0x1fa95b00U,
		(volatile uint32_t *)0x1fad0910U,
		(volatile uint32_t *)0x1fad0914U,
		(volatile uint32_t *)0x1fad0b00U,
	};
	volatile uint32_t *rg_b[9] = {
		(volatile uint32_t *)0x1fa72c2cU,
		(volatile uint32_t *)0x1fa72c44U,
		(volatile uint32_t *)0x1fa72c48U,
		(volatile uint32_t *)0x1fa77c2cU,
		(volatile uint32_t *)0x1fa77c44U,
		(volatile uint32_t *)0x1fa77c48U,
		(volatile uint32_t *)0x1fa81c2cU,
		(volatile uint32_t *)0x1fa81c44U,
		(volatile uint32_t *)0x1fa81c48U,
	};
	uint32_t enabled = 0;
	unsigned int i;
	unsigned int bit = 0x24;

	ephyEfuseWrite();

	ef_read_parse(0x6b, 1, (unsigned char *)&enabled);
	if (enabled != 1)
		return;

	for (i = 0; i < 3; ++i, bit += 0x15) {
		unsigned int n = i * 3;

		_efuse2rg(bit - 5, rg_a[n],     5, 24);
		_efuse2rg(bit - 5, rg_b[n],     5, 7);
		_efuse2rg(bit,     rg_a[n + 1], 5, 24);
		_efuse2rg(bit,     rg_b[n + 1], 5, 4);
		_efuse2rg(bit + 6, rg_a[n + 2], 6, 10);
		_efuse2rg(bit + 6, rg_b[n + 2], 6, 8);
	}

	_efuse2rg(0x267, (volatile uint32_t *)0x1fad0304U, 5, 0x13);
	_efuse2rg(0x26c, (volatile uint32_t *)0x1fad1304U, 5, 0x13);
	_efuse2rg(0x285, (volatile uint32_t *)0x1efbdf20U, 16, 0);
	_efuse2rg(0x59,  (volatile uint32_t *)0x1faf4638U, 5, 0x10);
	_efuse2rg(0x5e,  (volatile uint32_t *)0x1faf4638U, 5, 0x18);
	_efuse2rg(0x63,  (volatile uint32_t *)0x1faf3004U, 6, 0x12);
}
