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

## Phase C: DramcWriteLeveling done for both SoCs (plus the dle_factor_handler dependency)

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

- `DramcWriteLeveling` -- done for both SoCs (AN7581 1612 B, AN7583
  1208 B; confirmed structurally different, each traced and
  reconstructed independently). See below.
- `dramc_rx_dqs_gating_cal` (1948 B)
- `DramcTxWindowPerbitCal` (2504 B)
- `DramcRxWindowPerbitCal` (2540 B)

These are the largest and highest-risk functions in the whole object;
expect them to take substantially longer per function than anything
done so far.

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

## DramcWriteLeveling: AN7581 done (27/55); AN7583 confirmed different, not yet done

**AN7581's `DramcWriteLeveling` is fully reconstructed and validated**
(`reference/an7581/disasm/bl22/dramc_pi_calibration_api.dis` line 1426,
61 relocations; vendor 1612 bytes, Clang -Os candidate 1738 bytes).
Structure, in order:

1. `if (ctx == 0) return 1;` then `vPrintCalibrationBasicInfo(ctx)`,
   `vIO32WriteMsk(ctx, 0x238, rank, 3)`, `vIO32WriteMsk(ctx, 0x238, 4, 4)`.
2. Backup via two fixed `.rodata` tables (confirmed via
   `llvm-objcopy --dump-section`): `regs[9] = {0x14c, 0x150, 0x158, 0x320,
   0x51200f30, 0x59200fb0, 0x11000508, 0x19000588, 0x1fc}` through
   `DramcBackupRegisters(ctx, regs, 9, 1)`, and `mixed_rg[3]` (2 words
   each) `= {{0x10007b0,0x100},{0x10007b0,0x408},{0x10007b4,0x100}}`
   through `DramcBackupMixedRG(ctx, mixed_rg, 3, 1)`. Both restored at
   the end via the matching `DramcRestoreRegisters`/`DramcRestoreMixedRG`.
3. `vSetCalibrationResult(ctx, 5, 1)` (provisional fail), then a
   per-channel one-time init gated on a byte flag at
   `*(ctx + *(ctx+4) + 0x8c)`: if unset, set it, sweep
   `ShiftDQUI(ctx,-1,4)` / `ShiftDQ_OENUI(ctx,-1,4)` /
   `ShiftDQSWCK_UI(ctx,-1,4)` (byte_idx 4 hits `_LoopAryToDelay`'s
   default/all-8-lanes case), then zero byte lanes of
   `0x11600a20`/`0x19600aa0` via `vIO32WriteMsk_All`.
4. `vGet_DDR_Loop_Mode(ctx)` selects `{sweep_range, step_mult}`:
   mode 1 -> `{0x20, 0x10}`, mode 2 -> `{0x20, 8}`, else -> `{0x40, 1}`.
5. `vPhyByteIO32WriteMsk(ctx, 0x1fc, 0x3040, 0xc0003042)`,
   `CKEFixOnOff(ctx, rank, 1, 0)`, `O1PathOnOff(ctx, 1)`, MR
   write-leveling enable (`vIO32WriteMsk(0x14c, 8, 8)` +
   `vSetDramMRWriteLevelingOnOff(ctx, 1)` + `udelay(1)`), a
   `0x158`/`0x14c` timing-window setup gated on `data_width == 0x20`,
   then `udelay(1)` again. `lane_count = data_width >> 3`, and
   `wrlevel_dqs_final_delay[lane + rank*4]` is zeroed for each active
   lane.
6. The sweep loop: increments `round` by `step_mult` each iteration
   (0 to 0xc0 max), shifts `ShiftDQSWCK_UI` by one coarse step every
   time `round` crosses a `sweep_range` boundary, toggles the sample
   strobe (`0x14c` bit 7), then reads live DQS bits from
   `0x01800180` (round 0) or `0x096009a0` (later rounds) and updates a
   **per-lane FSM that is gated on the global `pkg_type`**:
   - `pkg_type != 0`: a 6-state edge-detector (dispatched conceptually
     like a `tbb` jump table in the vendor) that tracks a candidate
     edge position (`saved_pos`), requires two confirmations
     (`confirm_c`/`confirm_d` counters scaled by `step_mult`, threshold
     7, with a `round == 0xbf` early-accept case) before locking in
     `wrlevel_dqs_final_delay`, and prints
     `"byte_%d is broken"` (string confirmed via
     `.rodata.DramcWriteLeveling.str1.1`) on the unreachable/default
     state.
   - `pkg_type == 0`: a simpler settle-then-count FSM (`settle[lane]`
     must exceed 16 samples of a low strobe before arming, then counts
     highs until `state*step_mult > 7` or the `round == 0xbf` early-out,
     recording `round - step_mult*(state-2)` as the final delay).
   Loop exits once `done_mask == 0xff` (all active lanes done) or
   `round > 0xc0`.
7. Undo any leftover coarse-step group shift, report pass/fail via
   `vSetCalibrationResult(ctx, 5, ...)`, disable MR write-leveling and
   O1 path, restore the backed-up registers/mixed-RG.
8. Fold any `wrlevel_dqs_final_delay` value `>= sweep_range` back into
   an additional `ShiftDQSWCK_UI` coarse shift plus a `%= sweep_range`,
   then re-center each lane's final value by `+0x10`: values `<= 0x3f`
   are written as-is; values that overflow get `-0x30` plus a
   compensating `ShiftDQUI(ctx,2,lane)` / `ShiftDQ_OENUI(ctx,2,lane)`
   fine shift.
9. Final packed writes: lanes 0/1 always go into
   `vPhyByteIO32WriteMsk(0x11600a20/0x19600aa0, ...)` (bits [13:8] and
   [21:16] both set to the same re-centered byte-position value), and
   when `data_width == 0x20` (4 active lanes), lanes 2/3 get the same
   treatment additionally wrapped in
   `__meta_backup_and_set(ctx,1,0)`/`__meta_restore(ctx,0)` to target
   the second meta-context. A second, separate pair of
   `vIO32WriteMsk` calls (mask `0x3f000000`, byte position [31:24])
   writes the un-recentered `saved_pos` for lanes 0/1 unconditionally
   and lanes 2/3 again under the same `__meta_backup_and_set`/
   `__meta_restore` bracket when `data_width == 0x20`.

Validated by compiling the candidate with
`-Wall -Wextra` (clean, rc=0) and diffing per-callee relocation counts
against the vendor oracle (`call-count-check.csv`): every callee count
matches exactly except two well-understood Clang -Os artifacts also
seen elsewhere in this file -- `vIO32WriteMsk` shows 2 extra call sites
(codegen duplication, not extra logical calls) and `pkg_type` shows 4
loads where the source reads it once per lane (compiler
rematerializes the global read instead of caching it in a register
across the whole loop body).

**Correction applied after the initial pass:** the vendor's final
`pop.w {..., pc}` is preceded by `mov r0, r5` where `r5` is the same
pass/fail flag just passed to `vSetCalibrationResult(ctx, 5, r5)` --
i.e. the function returns that flag (0 on success, 1 on failure), not
a hardcoded 0. The first reconstruction pass had `return 0;`
unconditionally at the end; fixed to `return fail;` after re-reading
`reference/an7581/disasm/bl22/dramc_pi_calibration_api.dis` lines
1833-1839 directly. Same fix applies to AN7583's version below (its
own `mov r0, r5` before its single `pop.w` at offset 0x328).

**AN7583's `DramcWriteLeveling` is now also reconstructed and
validated**, from its own independent instruction-by-instruction trace
(NOT derived by stripping the `pkg_type` branch out of AN7581's C
source, per the caution originally logged here). Vendor 1208 bytes,
Clang -Os candidate 1260 bytes, 73 relocations. It is structurally
simpler than AN7581 (no `pkg_type`, `printf`, or
`__meta_backup_and_set`/`__meta_restore` relocations at all -- it has
only the `pkg_type == 0`-style settle-then-count FSM, no 6-state
edge-detector, and no dual meta-context final write for lanes 2/3),
but the independent trace turned up real, confirmed differences beyond
that missing branch, all preserved in
`an7583/dramc_pi_calibration_api.recovered-core.c`:

- `wrlevel_dqs_final_delay` is indexed `[lane + rank*2]`, not AN7581's
  `[lane + rank*4]` -- confirmed from the vendor's own
  `add.w r3, r3, r5, lsl #1` (multiply by 2) at every zero-init/
  fold-back/FSM-finalize site that touches this array, versus AN7581's
  `lsl #2` (multiply by 4) at the equivalent sites.
- The MR-timing-window field select compares `data_width` against
  `0x10` (giving field `3` vs `1`), not AN7581's compare against
  `0x20` (giving field `0xf` vs `3`) -- confirmed via
  `cmp r3, #0x10` immediately preceding the `it ne` at the
  corresponding offset, versus AN7581's `cmp r3, #0x20` at its own
  equivalent site.
- The settle FSM multiplies the settle counter by `step_mult` (an
  `smulbb`) *before* comparing it to the arm threshold of 16; AN7581
  compares the raw unscaled counter directly (`cmp r2, #0x10` with no
  preceding multiply). Changes how many samples are needed to arm
  depending on `vGet_DDR_Loop_Mode()`.
- Every round after the first writes the live round position
  (`round << 24`, mask `0x3f000000`) into both `0x11600a20` and
  `0x19600aa0`; AN7581 does not touch those two registers again until
  after the sweep loop exits (it only reads/refreshes `0x096009a0` on
  rounds after the first). AN7583 also swaps which round (0 vs. later)
  does the `0x096009a0` refresh versus the `0x11600a20`/`0x19600aa0`
  writes, relative to how AN7581 arranges the analogous round==0 vs.
  round!=0 branch.
- The final packing section only ever writes lanes 0/1 into
  `0x11600a20`/`0x19600aa0` -- there is no `data_width == 0x20` branch
  and no second (lanes 2/3) write pass at all, consistent with the
  missing meta-context relocations above. A 4-lane configuration's
  lanes 2/3 results are still computed by the FSM and folded/
  recentered on the stack, but are never committed to hardware.
- Confirmed only 3 `vPhyByteIO32WriteMsk` relocations (not 5, matching
  the missing lane-2/3 write pass) and 13 `vIO32WriteMsk` + 4
  `vIO32WriteMsk_All` relocations, both reproduced exactly at the
  *source* level (verified by compiling at `-O0`, which shows the
  same 13/5 call sites the C source literally contains); the `-Os`
  candidate shows 12/4 because Clang merges one call from each into a
  shared tail with an identical call already on the other branch --
  the same shared-tail trick documented elsewhere in this file, not a
  dropped call.

Also carries the same return-value fix as AN7581 above: the vendor's
`mov r0, r5` before its single `pop.w {..., pc}` returns the pass/fail
flag, not a hardcoded value; the candidate's final `return fail;`
matches this directly (there was no separate wrong-return-value bug
to fix here since this function was traced fresh, but the fix is
called out to make clear both SoCs' functions now agree on this
point).

`DramcWriteLeveling` is now closed for both SoCs (AN7581 at 27/55
overall, AN7583 at 26/57 overall). Of the four functions this section
originally listed as the largest/highest-risk remainder,
`dramc_rx_dqs_gating_cal`, `DramcTxWindowPerbitCal`, and
`DramcRxWindowPerbitCal` are what's left.
