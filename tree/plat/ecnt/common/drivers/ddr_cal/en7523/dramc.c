/*
 * EN7523 DRAM top-level glue reconstructed from the vendor dramc.o.
 *
 * Reference object:
 *   SHA256 84d6eec1936575c9b66e9e12e2421c08b78224d2ba43816a23e6d49e8a769f51
 * Compiler recorded in .comment:
 *   GCC: (Buildroot 2015.08.1-g6a3c3ed-dirty) 4.9.3
 *
 * This source is functionally reconstructed from symbols, relocations and
 * Thumb-2 disassembly. It has not yet been proven byte-identical.
 */
#include "dramc.h"

U32 pkg_type = 0;
U32 dram_size = 0;
U32 xtal_sel = PLL_Fin_25M;

unsigned int _calculate_dram_size(void)
{
	volatile U32 *checkaddr1;
	volatile U32 *checkaddr2;
	U32 check_size = 0x02000000;
	U32 val1, val2;
	unsigned int tries = 5;

	printf("Calculate size.\r\n");

	/*
	 * The object decrements before probing.  With tries=5 this probes
	 * 32/64/128/256 MiB and leaves 512 MiB as the maximum result.
	 */
	while (--tries) {
		checkaddr1 = (volatile U32 *)(uintptr_t)DRAM_START;
		checkaddr2 = (volatile U32 *)(uintptr_t)(DRAM_START + check_size);

		*checkaddr1 = 0x12345678;
		udelay(2);
		*checkaddr2 = 0x87654321;
		udelay(2);

		val2 = *checkaddr2;
		val1 = *checkaddr1;

		while (val2 != 0x87654321) {
			*checkaddr2 = 0x87654321;
			val2 = *checkaddr2;
			val1 = *checkaddr1;
		}

		if (val1 == 0x12345678) {
			check_size <<= 1;
			continue;
		}

		if (val1 == val2)
			break;

		printf("dram r/w error!\r\n");
		return 0;
	}

	printf("DRAM size=");
	printf("%d", check_size >> 20);
	printf("MB\r\n");

	/* The vendor object returns bytes, not MiB. */
	return check_size;
}

unsigned int dramc_main(void)
{
	U32 val;

	printf("\r\nEN7523DRAMC V0.1\r\n");

	writel(1, 0x1fb00040);
	writel(0, 0x1fb00040);

	val = readl(0x1fb0009c) & 1;
	if (val == 1) {
		xtal_sel = (readl(0x1fa20254) >> 19) & 1;
		pkg_type = (readl(0x1fa20254) >> 14) & 3;

		/* The closed object contains only the PCDDR3 path. */
		DPI_SW_main_PCDDR3();

		/* Present in dramc.o immediately after DDR calibration. */
		writel(readl(0x1fb00074) | 0x5, 0x1fb00074);
	}

	dram_size = _calculate_dram_size();
	return dram_size;
}
