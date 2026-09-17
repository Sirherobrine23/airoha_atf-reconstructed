# AN7581 / AN7583 DRAMC recovery v7

This revision promotes the object-derived `dramc_utility.c` and
`dramc_pi_basic_api.c` recovery cores to the canonical source filenames.
The previous MT819x-derived candidates are retained as `*.public-base.c` for
lineage/reference only.

## Major v7 milestone

- `dramc_utility.o`: every ELF function has a C representation (62/62 for both SoCs).
- `dramc_pi_basic_api.o`: every ELF function has a C representation
  (AN7581 32/32, AN7583 35/35).
- `vReplaceDVInit()` is now reconstructed for both SoCs from the object oracle.
- Per-callee source call counts in `vReplaceDVInit()` exactly match the vendor
  relocations for both SoCs.
- AN7581-specific corrections recovered during validation:
  - extra PHY write at `0x090004ac`: `0x29400000 / 0x7fe00000`
  - `0x010006c0` is a PHY masked write, not a DRAMC masked write.

These are functional source reconstructions, not byte-exact recompilations.
The original compiler is Buildroot GCC 10.3.0 / Trendchip cortex-a7 soft-float.
