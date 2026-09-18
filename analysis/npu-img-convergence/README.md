# ecnt_npu_img convergence

`ecnt_npu_img.o` (`bl23`) is not ARM code: it is a raw RISC-V (RV32IMAC)
binary blob for the platform's NPU/MD32 coprocessor, wrapped in a
dummy ARM-flavored ELF purely so the ARM linker can pull it in as data.
Its four symbols (`ecnt_npu_img`, `ecnt_npu_img_end`, `ecnt_npu_data`,
`ecnt_npu_data_end`) just mark the boundaries of two concatenated
byte ranges inside one `.text` section -- a code blob (0x0-0x3ddc) and
a small data blob (0x3ddc-0x3e0c, 48 bytes).

## Key finding: this blob is byte-identical across all three SoCs, and byte-identical to a real, debug-info-carrying build

`reference/{an7581,an7583,en7523}/blobs/bl23/ecnt_npu_img.o` all carry
the **exact same 15884-byte payload** (`cmp` byte-for-byte identical).
This is a single shared NPU firmware image, not something that varies
per host SoC.

The user supplied the actual MD32RV SDK build tree that produces this
firmware (`plat/ecnt/common/drivers/NPU/md32rv-sdk/`), now checked in
under `reference/common/drivers/NPU/md32rv-sdk/`:

- `software/main/main` -- the linked RISC-V ELF, **not stripped, full
  DWARF debug info** (functions, parameters, locals, struct/type
  layouts, line tables).
- `software/main/iNIC_client.o`, `software/main/iNIC_client_api.o` --
  two of the object files that went into that link, also with debug
  info.
- `init/env/start.o` -- the startup object (from `start.S`).
- The Makefiles and `init/env/fpga/{settings.mk,7523_ld_sram.lds}`
  build configuration.
- `software/main/main.elf.txt` -- `llvm-objdump -h -S -d` dump of
  `main`, generated here (the vendor Makefile's own `dasm` target
  does exactly this).

Reproducing the vendor's own build recipe from `Makefile`'s `software:`
target confirms this is the exact source build for the blob:

```
llvm-objcopy -j .init -j .text -j .rodata -j .eh_frame -j .lalign -O binary main npu_rv32.bin   # 15836 bytes
llvm-objcopy -j .dalign -j .data -j .bss -j .heap -O binary main npu_data.bin                    # 48 bytes
```

`npu_rv32.bin` matches `ecnt_npu_img[0:0x3ddc]` byte-for-byte, and
`npu_data.bin` matches `ecnt_npu_data[0:0x30]` byte-for-byte, for all
three SoCs' oracle objects. There is no ambiguity about what this
firmware is or what built it -- unlike the ARM Thumb DRAMC units, this
is a confirmed, not inferred, match.

## What "the same thing" means here

We do not have the literal `.c` files (`iNIC_client.c`,
`iNIC_client_api.c`, `memcpy.c`, `memset.c`, `strcpy.c`, `strlen.c`,
`start.S`) -- only compiled objects with DWARF. The DWARF is rich
enough (parameter names, local variable names and types, full struct
layouts, line numbers) that this reconstruction is higher-confidence
per function than the ARM Thumb work has been, but it is still a
reconstruction: source text has to be written from
disassembly + DWARF, then validated.

The vendor toolchain is a MediaTek-internal fork of **clang 7.0.0**
(`.comment`: "clang version 7.0.0 ... toolchain/clang@80901a1a ...
toolchain/llvm@5675420b"), linked with **LLD 7.0.0**, targeting
`rv32imac`/`ilp32` (`init/env/fpga/settings.mk`). This sandbox has
mainline clang/LLVM 18.1.3 with a working `riscv32`/`riscv64` backend
(`clang --target=riscv32-unknown-elf
-march=rv32imac -mabi=ilp32` compiles cleanly), but an 11-major-version
gap in the RISC-V backend means **byte-exact recompilation should not
be expected**, the same lesson already learned from the GCC-vs-Clang
gap on the ARM Thumb side. Validation here follows the same playbook
already established for `dramc_pi_calibration_api`: compile the
candidate, then compare per-function disassembly shape / call graph
against the oracle rather than raw bytes, and call out any genuine
divergence explicitly rather than assuming a match.

## Function inventory (from `main`'s symbol table + DWARF compile units)

`main`'s DWARF line tables reference exactly these source files:
`iNIC_client.c`, `iNIC_client_api.c`, `memcpy.c`, `memset.c`,
`sgmii_api.c` (!), `start.S`, `strcpy.c`, `strlen.c`, plus two
compiler-rt helper `.S` files (`save-restore.S`,
`unaligned-load-store.S`) and `string.h`/`yvals.h` headers. Notably
**no `main.c` or `core[0-7]_main.c`** despite the SDK Makefile listing
them -- `iNIC_client.c` itself defines a function literally named
`main`, which is what this particular firmware image actually links
in as its entry's C-level driver; the multi-core `core*_main.c`
variants are not part of this image at all.

`iNIC_client.o` defines (36 functions, incl. `main`):
`regRead32`, `regWrite32`, `serial_outc`, `dump_data`,
`get_cpuTmrTime_by_msTime`, `prom_puts`, `prom_print_hex`,
`trap_entry`, `core_id_get`, `core_char_get`, `reset_all_npu_modules`,
`__mdelay`, `_init`, `skb_headerinit`, `skb_init`, `alloc_skb`,
`skb_put`, `copy_data`, `npu_packet_send`, `skb_push`, `skb_reserve`,
`rom_in_csum`, `rom_eth_send`, `free_skb`, `rom_send_packet`,
`rom_send_rrq`, `rom_rcv_notify_ack`, `rom_send_error`, `rom_rcv_data`,
`rom_rcv_error`, `rom_rcv_packet`, `skb_pull`, `rom_eth_rcv`,
`npu_packet_rcv`, `npu_inic_client`, `main`.

`iNIC_client_api.o` defines (8 functions):
`npu_bridge_inic_buf_init`, `npu_bridge_inic_ingress_pkt`,
`npu_bridge_inic_get_header_buf_addr`, `checkAESyncState`,
`npu_bridge_inic_egress_pkt_common`, `npu_bridge_inic_release_hdr_buf`,
`npu_bridge_inic_create_pkt`, `npu_bridge_inic_drop_pkt`.

`regRead32`, `regWrite32`, `reset_all_npu_modules`, and
`checkAESyncState` do not appear in `main`'s own symbol table (either
inlined at `-O2`/LTO-style whole-program optimization, or dead-code
eliminated); `main`'s `check_ponphy_sync_state` (38 bytes) is
suspiciously close in role to `checkAESyncState` and needs checking
when that function is reconstructed, rather than assumed to be the
same thing under a different name.

By size, the largest/highest-value targets in `main`:
`rom_rcv_data` (0x7f4/2036 B), `rom_rcv_packet` (0x4e6/1254 B),
`rom_send_packet` (0x4b8/1208 B), `skb_init` (0x698/1688 B),
`npu_inic_client` (0x3e6/998 B), `rom_eth_rcv` (0x37a/890 B),
`alloc_skb` (0x35e/862 B), `trap_entry` (0x2ec/748 B),
`rom_rcv_notify_ack` (0x2d6/726 B), `npu_packet_send` (0x278/632 B),
`npu_bridge_inic_create_pkt` (0xc8/200 B). The `rom_*` naming plus
`rom_send_rrq` (RRQ = TFTP Read ReQuest) and `romhdr_t` (seen in
struct dumps: `magic_no`/`cmd_opcode`/`cmd_id`/`length`/`checksum`/
`block_no`/`file_checksum`/`cmd_subopcode`/`rom_ver`/`mem_addr`/
`mem_value`) strongly suggest a TFTP-like ROM/firmware download
protocol running over the internal NIC bridge (`npu_bridge_inic_*`),
not literal boot-ROM code.

`__riscv_save_N`/`__riscv_restore_N` and `__mrv_unaligned_load_i32`/
`__mrv_unaligned_store_i32` are standard compiler-rt RISC-V ABI
helpers (register-spill save/restore trampolines, unaligned access
trampolines) pulled in from `save-restore.S`/`unaligned-load-store.S`
-- public, well-known compiler-rt code, not vendor logic. Not worth
reconstructing as "recovered" source; if needed for a from-scratch
relink they can be pulled from upstream LLVM compiler-rt directly.

## Status

Reference material added, build-reproduction confirmed byte-exact.
No `.c` source written yet. Next: reconstruct `iNIC_client_api.c`
(smallest file, 8 functions, all thin wrappers) first, then work
through `iNIC_client.c` roughly in size order, each function validated
against its own DWARF signature/locals and disassembly, landing in
`tree/plat/ecnt/common/drivers/NPU/md32rv-sdk/software/main/` (no
per-SoC subdirectory -- this driver is confirmed SoC-common, same
convention as `tree/plat/ecnt/common/drivers/efuse/`).
