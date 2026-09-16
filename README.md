# EN7523 TF-A source recovery

This package combines the closed EN7523 TF-A objects from `atf.txz` with source
material recovered from the TP-Link XX230v GPL tree and source reconstructed
from ELF symbols/relocations/Thumb-2 disassembly.

## Drop-in tree

Copy the contents of `tree/` into the root of `boot/ATF/atf-airoha/`.

### Recovery status

| file                             | source                                | status                                                        |
| -------------------------------- | ------------------------------------- | ------------------------------------------------------------- |
| `ddr_cal/en7523/dramc.c`         | reconstructed directly from `dramc.o` | strong functional reconstruction; not yet byte-exact verified |
| `dramc_pi_basic_api.c`           | TP-Link GPL                           | all 22 exported functions match the blob by name              |
| `dramc_pi_calibration_api.c`     | TP-Link GPL                           | all 20 exported functions match the blob by name              |
| `dramc_pi_main.c`                | TP-Link GPL                           | exported `DPI_SW_main_PCDDR3` matches                         |
| `hal_io.c`                       | TP-Link GPL                           | all 7 exported functions match the blob by name               |
| `efuse/efuse.c`                  | reconstructed from `efuse.o`          | strong functional reconstruction                              |
| `efuse_load/en7523/efuse_load.c` | reconstructed from `efuse_load.o`     | strong functional reconstruction                              |

The DDR headers are from the GPL source. `atf_compat.h` and small include
changes make the U-Boot-derived code use TF-A `mmio_*` and `udelay()` APIs.

## Important recovered differences from the GPL `dramc.c`

The closed TF-A `dramc.o` is _not_ the GPL `dramc.c` compiled unchanged:

- it prints `EN7523DRAMC V0.1` at entry, using CRLF;
- the ASIC path always calls `DPI_SW_main_PCDDR3()`;
- it sets bits `0x5` at `0x1fb00074` after calibration;
- the size probe base is `0x80080000`;
- it probes 32/64/128/256 MiB and may return 512 MiB;
- `_calculate_dram_size()` returns bytes, not MiB;
- `dramc_main()` returns `dram_size` in bytes, matching `ecnt_system.c` usage.

## eFuse recovery

`efuse.o` exposes only `ef_read_byte()` and `efuse_init()` globally, but the
local `efuse_read_data()` symbol, relocations and strings survived. The object
reads controller `0x1fbf8208`, data registers `0x1fbf8230..0x1fbf823c`, and
builds a 192-byte non-secure cache plus the secure data passed to
`fill_secure_data()`.

`efuse_load.o` retained enough symbols to reconstruct `ef_read_parse()`, MDIO
Clause-22/MMD writes, EN7523 EPHY trims, and the final register trim table.

## Validation oracle

Original objects are under `reference/blobs/`. They were built with:

    GCC: (Buildroot 2015.08.1-g6a3c3ed-dirty) 4.9.3

The next byte-exact step is to reproduce that compiler/toolchain and tune source
and CFLAGS until the ELF sections/relocations match the reference objects.
Do not use SHA256 of the complete `.o` as the only first-stage test: compare
`.text.*`, `.rodata*`, symbols and relocations separately because metadata and
section ordering can differ.

## Not recovered here

`bl1.bin` and `bl31.bin` in the blob package are raw linked images rather than
relocatable objects, so this package does not claim source recovery for them.
The repository's TF-A 2.1/2.3 trees and EN7523 BL31 build artifacts are the
better oracle for that phase.
