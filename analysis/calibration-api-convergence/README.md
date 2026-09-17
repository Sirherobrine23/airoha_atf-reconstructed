# dramc_pi_calibration_api convergence

Continuation of the v7 handoff's Phase A (PCDDR/Airoha-only helpers). See
`dramc_pi_calibration_api.recovered-core.c` under each SoC's tree for the
promoted functions; the rest of the unit is still the MediaTek
`dramc_pi_calibration_api.c` public-lineage base (`PUBLIC_BASE` in
`analysis/recovery-status.csv`).

## Phase A status: 9/9 done for both SoCs

All nine functions below are byte-identical AN7581 <-> AN7583 at the object
level (same `.text.<func>` bytes and same relocations in the vendor oracle).

- `PCDDR_ShiftDQSUI`
- `PCDDR_ShiftDQS_OENUI`
- `get_gating_start_pos`
- `Dramc_efuse_read_parse`
- `DramcImpedanceEfuseValue`
- `DutyScan_Offset_Convert`
- `CmdOEOnOff` (new this pass)
- `SetRxDqDelay` (new this pass)
- `Get_RX_DelayCell` (new this pass; vendor body is a bare `bx lr`, confirmed no-op on both SoCs, zero relocations)

## New-this-pass validation

`CmdOEOnOff` and `SetRxDqDelay` were reconstructed directly from
`reference/{an7581,an7583}/disasm/bl22/dramc_pi_calibration_api.dis`
instruction-by-instruction and cross-checked against
`reference/{an7581,an7583}/disasm/bl22/dramc_pi_calibration_api.relocs.txt`:

- `CmdOEOnOff`: vendor tail-calls exactly `vPhyByteIO32WriteMsk_All` (rank
  selector == 1) or `vPhyByteIO32WriteMsk` (otherwise), both against register
  `0x21c`, mask `0x300000`. Candidate call graph matches exactly.
- `SetRxDqDelay`: vendor calls `vPhyByteWriteFldAlign` exactly twice, against
  `0x116009f8 + 4*byte_idx` and `0x19600a78 + 4*byte_idx`, with the input
  delay byte broadcast into all four byte lanes of the value word.
  Candidate call graph matches exactly.
- `Get_RX_DelayCell`: vendor object reduces the whole function to `bx lr`
  (2 bytes, zero relocations) on both SoCs. Modeled as an empty function.

See `function-size-check.csv` and `call-count-check.csv`. The only mismatch
is `DutyScan_Offset_Convert`, where the vendor materializes its 15-byte
lookup table via a `memcpy` from `.rodata` and Clang -Os instead inlines the
table with `movw`/`movt` immediates — a known codegen-strategy difference
(see main `README.md`), not a logic error.

## Phase B status: 4/4 impedance functions done, plus a real dependency

Also recovered this pass, both required by the Phase A/B functions above but
previously only `extern` forward declarations:

- `_LoopAryToDelay` (**not** byte-identical cross-SoC: AN7581 has no "field
  absent" gate; AN7583 additionally treats a packed field's bits[31:24] ==
  `0xff` as "skip the read (value 0) and neutralize the write (mask 0)".
  Dead code for every current caller, but a real, faithful SoC difference.)
- `u1MCK2UI_DivShift` (byte-identical cross-SoC; thin wrapper around
  `vGet_Div_Mode`)

Phase B proper:

- `DramcImpedanceByEfuse` -- reads two efuse bytes (DDR4 vs DDR3 bit
  offsets differ, see below), gates each on its own bit 6, and drives the
  DRVP/ODTP (fuse6) and DRVN/ODTN (fuse7) trims. AN7583 additionally gates
  the debug `printf`s on the global `uartDisable`, and both the efuse bit
  offsets and the `%x`/`0x%x` format strings differ cross-SoC.
- `DramcImpedanceDrvSetRG` -- converts a field's current raw value to a
  resistance-like magnitude via a weighted-bit sum (base 10000, doubling
  2500/5000/10000/20000/40000 per set bit, plus 80000 for bit 5 on
  6-bit-wide fields), multiplies by the input `code` and divides by 1175,
  maps the result through an 8-step threshold table (breakpoints at 49,
  149, 249, 350, 450, 549, 649, 749) to a 0-8 grade, then adds or
  subtracts that grade from the original raw value depending on `bit5`
  and writes the clamped byte back. AN7583 adds the same bits[31:24] ==
  `0xff` "field absent" gate as `_LoopAryToDelay`.
- `DramcImpedanceSetValue` -- pure dispatch on `type` (0/1/2/3) to up to
  11 `DramcImpedanceDrvSetRG` calls against a fixed set of MMIO
  registers/field descriptors; identical register/field constants on both
  SoCs. `type == 3` calls `DramcImpedanceDrvSetRG(0x012010d4, pos=20)`
  **twice** -- confirmed against the vendor relocations (31 calls, not
  33), so this is preserved as-is rather than de-duplicated. An
  unrecognized `type` prints `"Drv type error \n"`.

See `function-size-check.csv` and `call-count-check.csv` for the full
per-function vendor/candidate comparison. The only mismatches are
`DutyScan_Offset_Convert` (already noted above) and
`DramcImpedanceByEfuse`'s `Dramc_efuse_read_parse` call count: the vendor
GCC tail-merges the DDR3/DDR4 branches' second call into one shared call
site (3 sites for 4 logical calls on AN7581, i.e. the same effect the
`_LoopAryToDelay`/`DramcImpedanceSetValue` shared-tail tricks show
elsewhere), while Clang keeps them separate; the runtime call count matches
either way.

## Phase B is now fully closed for AN7581 (15/55); AN7583 stays at 14/57

`DramcDRVinitSetting` is AN7581-only in `dramc_pi_calibration_api.o` --
AN7583's equivalent function lives in `dramc_pi_basic_api.o` instead (see
`an7583/dramc_pi_basic_api.c`), was already recovered in an earlier pass,
and has **no** `pkg_type` branch. Its `is_ddr3_family()` and
`pkg_type == 0` constants match this AN7581 recovery byte-for-byte
(0x1e/0x26 per lane, default ODT code 13 on all twelve 5-bit lanes of
0x012010d0/0x012010d4), which cross-validates both recoveries
independently. AN7581 additionally branches on the global `pkg_type` for
the non-DDR3 case: package-0 and package-!=0 use different 0x1e/0x26
splits across the six DRVN/DRVP/ODTN/ODTP byte-lane registers, but always
converge on the same ODT-code defaults for 0x012010d0/0x012010d4.

Runtime call count to `vIO32WriteMsk_All` is 34 per invocation on both
vendor and candidate (either `pkg_type` branch); the vendor binary shares
13 of those 34 call instructions as a common tail between the two
branches (the same shared-tail trick used elsewhere in this object), so a
raw relocation count comparison isn't apples-to-apples here -- verified
instead by compiling the candidate at `-O0` (56 call sites, matching the
56 `W()` invocations actually written in the two if/else branches plus
the shared 12) to confirm no calls were dropped, then separately at `-Os`
(34 call sites, matching the vendor's per-branch runtime count) to
confirm the optimizer's merge doesn't change behavior.

## Phase C: 4/7 done (plus the dle_factor_handler dependency)

`DramcZQCalibration` is done (48 bytes, byte-identical AN7581/AN7583).
Despite its name, it does **not** run an actual ZQ calibration loop: it
zero-fills a 20-byte `airoha_rtswcmd` (same struct as
`DramcTriggerRTSWCMD` in `dramc_utility.c`), sets `command = 12` and
`rank`, fires it through `DramcTriggerRTSWCMD`, and then
unconditionally calls `vSetCalibrationResult(ctx, 2, 0)` with no check
of the RTSWCMD response at all -- a fire-and-forget hardware trigger,
not a pass/fail loop. (The EN7523 GPL lineage header would name `2`/`0`
`DRAM_CALIBRATION_CA_TRAIN`/`DRAM_OK`, but that enum ordering isn't
confirmed for this SoC, so the raw values are kept rather than guessing
a name.)

`DramcTXSetVref` is also done (128 bytes, byte-identical AN7581/AN7583).
DDR3 is a no-op; DDR4 sequences a JEDEC-style MR6 VrefDQ training write
(enable bit set -> value added while held -> enable bit cleared to
latch) for the current channel/rank, and updates the low byte of the
cached MR6 shadow (`gMRVal[]`, same indexing already used by
`DDR3_dram_init.c`/`DDR4_dram_init.c`).

`DramcRxdatlatCal` is also done (236/292 bytes AN7581/AN7583 -- **not**
byte-identical, see below), together with its dependency
`dle_factor_handler` (144 bytes, byte-identical). It scans the 32 UI
positions of the DATLAT delay line for the first contiguous run of
passing `DramcEngine2Run()` comparisons (capped at run length 5; once a
run ends, later successes are not counted as a new run), centers
`dle_factor_handler()` on that run, or restores the pre-scan baseline
and reports failure if nothing ever passed.

AN7583 differs from AN7581 in two real ways here, not just codegen:

- the `DramcEngine2Run()` result is truncated to 8 bits before the
  pass/fail check when `ctx+0x44 == 8` (a data-width mode where only
  the low byte of the per-lane mismatch mask matters);
- after reporting the result, `ctx+0xbd` selects a "rank 1" path: if
  clear, the just-applied `0x012010b8`/`0x0020168c` field values are
  cached into the named global `Rx_datlat_K_result_rg_rk1[2]`; if set,
  that cached pair is copied verbatim into fixed hardware registers at
  `0x1fc8a510`/`0x1fc8a490` instead of re-measuring -- i.e. rank 1
  mirrors rank 0's result on AN7583. AN7581 has no such path at all
  (matches the earlier utility.c finding that AN7581 hard-codes
  single-rank support).

Both `DramcRxdatlatCal` candidates show one extra `DramcEngine2End()`
call versus the vendor (a harmless Clang tail-duplication of a call
immediately preceding a branch -- see `call-count-check.csv`), and the
AN7583 candidate shows the `Rx_datlat_K_result_rg_rk1` global address
materialized twice instead of the vendor's once (same class of
difference: the vendor caches the address in one register across both
branches, Clang recomputes it per branch).

Remaining Phase C, per the handoff, roughly in size order:

- `DramcWriteLeveling` (1612 B)
- `dramc_rx_dqs_gating_cal` (1948 B)
- `DramcTxWindowPerbitCal` (2504 B)
- `DramcRxWindowPerbitCal` (2540 B)

These four are the largest and highest-risk functions in the whole
object; expect them to take substantially longer per function than
anything done so far.

## Dependencies recovered for DramcWriteLeveling

Before tackling `DramcWriteLeveling` itself, its five not-yet-recovered
callees were done first (all byte-identical AN7581/AN7583, table
content cross-checked against the raw `.rodata` bytes via
`llvm-objcopy --dump-section`):

- `ShiftDQUI` / `ShiftDQUI_AllRK` -- same `_LoopAryToDelay` wrapper
  pattern as `PCDDR_ShiftDQSUI`, but over all 8 DQ byte lanes (count 8)
  across two registers (0x60120c/0x601208 UI, 0x601204/0x601200 MCK).
  `_AllRK` is a 4-byte tail-jump alias to the non-`_AllRK` name in the
  vendor object.
- `ShiftDQ_OENUI` / `ShiftDQ_OENUI_AllRK` -- same, for the OE_N fields.
- `ShiftDQSWCK_UI` -- applies the same shift to both
  `PCDDR_ShiftDQSUI` and `PCDDR_ShiftDQS_OENUI`.
- `O1PathOnOff` -- turns the O1 (1x-frequency) datapath on/off; ends
  with a fixed 1us delay.
- `vSetDramMRWriteLevelingOnOff` -- sets/clears the MR1 write-leveling
  bit plus a family-specific MR2 tweak on rank 1, restoring MR2's cache
  on disable.

All were validated against the oracle's relocation/call-target sets;
`vSetDramMRWriteLevelingOnOff` shows the vendor's 5 logical
`DramcModeRegWriteByRank` calls compiled down to 3 call sites under
Clang -Os (it merges the two branches' identical trailing calls) --
confirmed as a harmless codegen difference, not a dropped call, by
reading the generated assembly directly.

## DramcWriteLeveling itself: structure mapped, body not yet written

**Important finding first:** AN7581's `DramcWriteLeveling` is 1612
bytes; AN7583's is only 1208 bytes (`reference/an7583/disasm/bl22/
dramc_pi_calibration_api.dis` line 1220). That is too large a gap to be
codegen noise -- the two SoCs' write-leveling sequences genuinely
differ in structure, not just constants. Whoever continues this must
disassemble and decode AN7583's copy independently rather than
assuming it is a byte-identical (or even structurally identical)
sibling; don't reuse the AN7581 analysis below for it.

AN7581's `DramcWriteLeveling` (`reference/an7581/disasm/bl22/
dramc_pi_calibration_api.dis` line 1426, 61 relocations) has been read
and phase-mapped in full but **not yet transcribed to C** -- the tail
half is a genuine 6-state per-byte-lane edge-detection FSM (dispatched
through a `tbb` jump table at offset 0x4cc) with counters packed into a
stack-allocated array, and committing an unverified guess at that would
be worse than leaving it documented. What follows is everything needed
to pick this up without redoing the analysis:

Phases confirmed by full disassembly read:

1. `if (ctx == 0) return 1;`
2. `vPrintCalibrationBasicInfo(ctx)`, then `vIO32WriteMsk(ctx, 0x238, rank, 3)` and `vIO32WriteMsk(ctx, 0x238, 4, 4)`.
3. Two fixed tables are copied from the object's shared `.rodata` (confirmed via `llvm-objcopy --dump-section`, offsets are into the AN7581 blob's `.rodata`, not necessarily the same in AN7583's):
   - `regs[9]` at `.rodata+0x180`: `{0x14c, 0x150, 0x158, 0x320, 0x51200f30, 0x59200fb0, 0x11000508, 0x19000588, 0x1fc}`, passed to `DramcBackupRegisters(ctx, regs, 9, 1)`.
   - `mixed_rg[3]` (2 words each) at `.rodata+0x1a4`: `{{0x10007b0,0x100},{0x10007b0,0x408},{0x10007b4,0x100}}`, passed to `DramcBackupMixedRG(ctx, mixed_rg, 3, 1)`.
   - Restored at the end via `DramcRestoreRegisters`/`DramcRestoreMixedRG` with the same tables/counts.
4. `vSetCalibrationResult(ctx, 5, 1)` (provisional fail).
5. Per-channel one-time init gated on a byte flag at `*(ctx + *(ctx+4) + 0x8c)`: if unset, set it, then `ShiftDQUI(ctx, -1, 4)`, `ShiftDQ_OENUI(ctx, -1, 4)`, `ShiftDQSWCK_UI(ctx, -1, 4)` (byte_idx 4 hits `_LoopAryToDelay`'s default case, i.e. sweeps all 8 lanes with stride 1 -- confirmed against the already-recovered `_LoopAryToDelay`), then two `vIO32WriteMsk_All` calls zeroing byte lanes of `0x11600a20`/`0x19600aa0`.
6. `vGet_DDR_Loop_Mode(ctx)`: branches into either a `phase` sweep of 32 (mode==1) or a nested rank×32 sweep (other modes, with a mode==2-specific set of constants) -- this branch (`beq 0x3d6`) has NOT been fully traced yet for the mode==1 path.
7. `CKEFixOnOff`, `O1PathOnOff(ctx,1)`, more `vIO32WriteMsk` setup, `vSetDramMRWriteLevelingOnOff(ctx,1)`, `udelay(1)`.
8. The FSM: for each rank/lane index, reads a per-lane byte-array record (base `sp+0x80`, each record ~32 bytes: state at -108, three counters at -104/-100/-96, an edge counter at -92, and a saved delay word at -76), drives `dle`-style register reads and `ShiftDQSWCK_UI`/`ShiftDQUI`/`ShiftDQ_OENUI` shifts, and on state 5 (failure) calls `printf("byte_%d is broken", ...)` (string confirmed via `.rodata.DramcWriteLeveling.str1.1`). States 0-5 dispatch via a `tbb [pc, lr]` byte jump table at 0x4d0.
9. Final results are written into `wrlevel_dqs_final_delay`, a `static S32 [RANK_MAX][DQS_BYTE_NUMBER]` array (declared at `dramc_pi_calibration_api.c:156` in the still-PUBLIC_BASE MediaTek lineage source -- not yet in any recovered/validated file, so it needs a fresh `extern S32 wrlevel_dqs_final_delay[][8];`-style declaration here, matching the object's actual flattened word-array indexing rather than assuming the public-base shape is exactly right), then the backup registers/mixed-RG are restored and the function returns.

Recommended approach for finishing this: trace phase 6-8 instruction-by-instruction the same way the rest of this file's functions were done (one register at a time, cross-checking every constant against `.rodata`/relocations), rather than trying to shortcut the FSM from the phase summary above -- the summary is a map, not a substitute for the full trace.
