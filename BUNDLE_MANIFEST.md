# AN7581 / AN7583 DRAMC recovery bundle v5

## Purpose

Research + integration bundle for replacing the proprietary AN7581/AN7583 BL22
DRAMC objects with reconstructed source. The original objects are treated as the
binary oracle.

## v5 changes

- Promoted AN7583 `dramtest.c` to canonical functional source.
- Recovered all 15 exported functions from `dramtest.o`.
- Added the complete `dramtest.o` oracle metadata/disassembly.
- Optimizer-neutral Clang validation matches raw relocated call sequences for 13/15 dramtest functions; the remaining two differ only in compiler block/call-site merging.
- Added function-size and call/relocation comparison reports.
- Recovered the effective vendor BL22 code-generation flags from the TF-A build.
- Added helper scripts for compilation with the real Trendchip GCC and
  per-function byte comparison.
- Refreshed recovery status: RX/TX auto-gen and AN7583 self-refresh are no longer
  marked partial/unrecovered.

## Canonical reconstructed/source-base tree

Both SoCs contain:

- `ANA_init_config.c`
- `DDR3_dram_init.c`
- `DDR4_dram_init.c`
- `DIG_NONSHUF_config.c`
- `DIG_SHUF_config.c`
- `DRAMC_SUBSYS_config.c`
- `HW_FUNC_MANAGE.c`
- `Hal_io.c`
- `IPM_actiming_setting_DDR3.c`
- `IPM_actiming_setting_DDR4.c`
- `MD32_initial.c`
- `RX_path_auto_gen.c`
- `TX_RX_auto_gen_library.c`
- `TX_path_auto_gen.c`
- `dramc.c`
- `dramc_actiming.c`
- `dramc_dv_dut.c`
- `dramc_dvfs.c` — object-derived, 3/3 functions cross-SoC byte-identical
- `dramc_pi_basic_api.c`
- `dramc_pi_calibration_api.c`
- `dramc_pi_main.c`
- `dramc_utility.c`
- `recovery_abi.h`

AN7583 additionally contains canonical:

- `dramc_selfrefresh_api.c`
- `dramtest.c`

## Research-only files

`*.recovered-fragments.c` are retained in the full research bundle for audit
history, but excluded from the apply-ready patch and source-only bundle so they
cannot produce duplicate definitions.

## Analysis/oracle material

- `analysis/recovery-status.csv`
- `analysis/dramtest-v5-size-check.csv`
- `analysis/dramtest-v5-call-check.csv`
- `analysis/dramtest-v5-call-analysis.csv`
- `analysis/dramtest-v5-string-check.txt`
- cross-SoC/function maps retained from previous rounds
- `an7581/oracle/` and `an7583/oracle/`
- `VENDOR_COMPILER.md`

## Patch policy

Generated patch files contain only canonical source/header files. They exclude:

- `*.recovered-fragments.c`
- oracle dumps/disassembly
- compile-check objects
- reports and temporary analysis products

This keeps the patch suitable for review/integration without accidentally
compiling duplicate recovery fragments.

## Confidence terminology

- `EXACT_DATA`: data/table bytes were compared byte-for-byte.
- `FUNCTIONAL_HIGH`: object-driven reconstruction with strong cross-checks, but
  exact compiler convergence still pending.
- `FUNCTIONAL`: object-driven source representing recovered semantics/ABI.
- `PUBLIC_BASE`: public MediaTek lineage source; Airoha per-function convergence
  is still required.
- `BYTE_EXACT`: reserved for an actual matching oracle comparison; not claimed
  globally by this bundle.

## v7 promotion

`dramc_utility.recovered-core.c` and `dramc_pi_basic_api.recovered-core.c` are
promoted to the canonical `dramc_utility.c` and `dramc_pi_basic_api.c` names.
The prior MT819x lineage candidates are retained as `.public-base.c`.

Basic API coverage from the vendor ELF symbol tables:
- AN7581: 32/32 functions represented.
- AN7583: 35/35 functions represented.

Utility coverage:
- AN7581: 62/62 ELF functions represented.
- AN7583: 62/62 ELF functions represented.

`vReplaceDVInit()` source call multiplicities match vendor relocations exactly
for every external callee on both SoCs. See
`analysis/basic-api-convergence/vReplace-call-count.csv`.
