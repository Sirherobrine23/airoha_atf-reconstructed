# GitHub cross-reference findings

'

## Strongest external reference

`Yuzhii0718/mt7986-dram-analysis` contains multiple MT7986 `dram.o` revisions and reports the same PCDDR function family used by AN7581/AN7583, including `PC3_dram_init_single_rank`, `PC4_dram_init_single_rank`, `Get_RL_by_MR_PC4`, `Get_WL_by_MR_PC3`, `get_TX_path_config`, `MD32_DFS_PRE`, and the DRAMC calibration routines.

The public MT7981/MT7986 TF-A platform Makefiles also name the same missing source units: `HW_FUNC_MANAGE.c`, `MD32_initial.c`, `RX_path_auto_gen.c`, `TX_path_auto_gen.c`, `TX_RX_auto_gen_library.c`, and `pcddr_cal/DDR3_dram_init.c` / `DDR4_dram_init.c`. Public releases generally ship the aggregate `dram.o` instead of those sources.

## What was recovered in this pass

* `TX_RX_auto_gen_library.c`: all 10 functions reconstructed; object function sections are byte-identical between AN7581 and AN7583.
* AN7581 `dramc.c`: all functions represented; semantics reconstructed directly from BL22 Thumb-2 disassembly. External API C types remain ABI-inferred.
* AN7581 DDR3/DDR4 init: four functions per file reconstructed from object control flow and constants.
* AN7583 DDR3/DDR4 init: reconstructed separately; captures the AN7583-only fast-delay flag and DDR3 context override.
* AN7583 `dramc.recovered-fragments.c`: directly recovered compact routines; the more complex scrambler/RBUS/main path remains for the next pass.

## Validation level

This is a **functional reconstruction**, not a byte-identical source claim. Exact validation still requires the vendor-compatible Buildroot GCC 10.3.0 toolchain and the original include/type layout.

## v5: dramtest lineage and compiler convergence

The older Airoha/EcoNet tree exposes the same DRAM pattern-walker family in
`boot/bootrom/bootram/lib/dramtest.c`: `dram_pat_set`, `dram_pat_cmp`,
`dram_incrPat_cmp`, `dram_antiIncrPat_cmp` and `dramTest`.  This was used as a
lineage/reference source only.  The AN7583 BL22 `dramtest.o` changes the ABI and
wrapper logic, so the object disassembly/relocations remain authoritative.

The Airoha TF-A Makefile also identifies the expected 32-bit Trendchip toolchain
path as `/opt/trendchip/buildroot-gcc1030-glibc232_kernel5_4` and the BL22 build
selects Thumb-2, `-Os`, function/data sections, freestanding/no-builtin/no-common,
`-mno-unaligned-access`, plus `-march=armv8-a+crc` from `flash.mk`.

A public Bootlin ARMv7 GCC 10.3.0 toolchain exists and could serve as a secondary
compiler-version proxy, but it is hard-float by default and is not the exact
Trendchip Buildroot toolchain.  It was not available to execute in this recovery
environment, so all current local size comparisons remain Clang-based.
