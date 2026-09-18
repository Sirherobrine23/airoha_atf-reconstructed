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

## dramc_rx_dqs_gating_cal: AN7581 done; AN7583 confirmed different, not yet done

Reconstructed from a full instruction-by-instruction trace of
`reference/an7581/disasm/bl22/dramc_pi_calibration_api.dis` (offset
0x0-0x798, vendor 1948 bytes, candidate 1588 bytes). This function
resisted the first attempt at a clean per-field semantic model (see
git history of this file for the earlier "structure mapped, body not
yet written" checkpoint) because its per-lane bookkeeping is packed
into one 0x60-byte stack region addressed through a single base
pointer at multiple different byte/word strides -- some of those
strides land on completely unrelated scalars (e.g. one stride-by-lane
combination transiently aliases the *inner scan position* variable
for lane 0 only, an artifact of a 4-byte word store zeroing the other
three lanes' slots). The eventual approach: build the exact
address-arithmetic map offset-by-offset from the raw disassembly bytes
(not by pattern-matching `DramcWriteLeveling`), verify it against a
mechanically-derived control-flow graph rather than by eye, and only
then assign each field a name.

Confirmed structure, in order:

1. `vPrintCalibrationBasicInfo(ctx)`, `vSetCalibrationResult(ctx, 8, 1)`
   (provisional fail, cal_type 8), `if (ctx == 0) return 1`.
2. Backup a 4-word register table (`.rodata+0x1bc`:
   `{0x9100050c, 0x9900058c, 0x01000664, 0x01000668}`) via
   `DramcBackupRegisters(ctx, table, 4, 1)`; restored at the very end
   via `DramcRestoreRegisters` + `DramPhyReset(ctx)`.
3. A gating-enable pulse sequence on `0x01000668` (set bit 2, set bit
   0x200000, `udelay(4)`, set bit 0x400000, `udelay(1)`, clear bit
   0x400000) bracketed by `GetDramcBroadcast()`/`DramcBroadcastOnOff(1)`
   and `DramcBroadcastOnOff(<saved>)`; `rank = u1GetRank(ctx)` written
   into bit 25 of `0x010007bc`; `DramcEngine2Init(ctx, 0x55000000,
   0xaa000023, 1, 0, 0)`.
4. `start_pos = get_gating_start_pos(ctx)`, `end_limit = start_pos +
   0x10` (as a `U8`, i.e. it can wrap). If it wrapped
   (`start_pos >= end_limit`), the vendor's own "any lane still bad"
   accumulator is provably 0 at this point (nothing has run yet),
   which makes its fail-report call unreachable for any real
   `data_width` (only fires when `lane_count == 0`) and every other
   loop in that branch either a no-op walk or a redundant re-zero of
   memory the entry `memset`s already zeroed -- so this degenerate
   case is modeled as falling straight into the same commit code the
   normal sweep's success path uses, with all-zero results (verified
   instruction-by-instruction that this is the actual net effect, not
   assumed).
5. Main sweep: an outer/coarse position (`outer_pos`, starting at
   `start_pos`) times an inner/fine position (`inner_pos`, 0-31).
   Each step writes both into `0x11600a2c`/`0x19600aac` (mask
   `0x007f00ff`, second pair under `__meta_backup_and_set(ctx,1,0)`
   for `data_width==0x20`), pulses `0x01000668` bit 0x400000 around a
   `DramPhyReset`, runs `DramcEngine2Run(ctx, 1, 0)`, then reads two
   bits of `0x01001b00` (bit 1/2 for rank 0, bit 5/6 for rank != 0,
   each duplicated under `__meta_backup_and_set(ctx,1,rank)` for
   `data_width==0x20`) and two `vPhyByteReadFldAlign` reads from
   `0x01800500`/`0x01800504` (also duplicated for `data_width==0x20`).
6. Per-lane FSM step (`lane` 0..`lane_count-1`, `lane_count =
   data_width>>3`): two more hardware bits per lane (even lanes from
   `0x0180019c`, odd lanes from `0x01800198`, lanes 2/3 wrapped in
   `__meta_backup_and_set(ctx,0,1)` -- a *different* meta "type" than
   the rank-context above, selecting a channel/group instead), then
   `combo = hw1[lane] | hw0[lane]<<1`, `state = combo>3 ? 0 : 4-combo`
   (the vendor's first `tbb` byte-jump table, confirmed to reduce to
   this arithmetic), then a proper Mealy transition
   `next = f(state, prev_state[lane])` (the vendor's second chained
   `tbb`) driving four per-lane counters (`armed`, `confirm_c`,
   `confirm_d`, plus saved `(outer_pos, inner_pos)` at first
   detection) toward a terminal `status` value (2 = confirmed via one
   threshold, 4 = confirmed via a DDR3/DDR4-dependent second
   threshold), with a global per-lane safety-timeout counter
   (`r_filter_count[]`, a real `.bss` array, confirmed 4 bytes on
   AN7581 vs 2 bytes on AN7583) forcing a reset back to state 0 if a
   lane never settles. `state`/`prev_state` shift each pass
   (`prev_state[lane] = state[lane]`, the transition table's history
   input for next time).
7. Per-lane finalize check: once a lane's hardware self-check passes
   (a `flags[]`/`vphy_read[]` sanity pair from step 5) and
   `prev_state[lane] == 4`, compute the calibrated position from
   `saved_inner[lane]`, `confirm_d[lane]`, and `saved_outer[lane]`
   (`(saved_inner + confirm_d*2) / 32` remainder/quotient split,
   quotient folded into a re-centered `saved_outer`), mark the lane
   done in a `done_mask` bitmask, and -- once every active lane's bit
   is set -- short-circuit the entire outer/inner sweep by forcing
   `outer_pos = end_limit` and falling into the shared commit/teardown
   tail (matching how the degenerate case above reaches that same
   tail). Otherwise the sweep continues to the next inner position,
   or the next outer position once the inner 0-31 range is exhausted.
8. Commit: `DramcEngine2End(ctx)`, then pack each lane's calibrated
   position into `0x11600a2c`/`0x19600aac` (`hi<<16 & 0x7f0000 | lo`,
   mask `0x007f00ff`), lanes 2/3 under the same
   `__meta_backup_and_set(ctx,1,0)` bracket as step 5, for
   `data_width==0x20`.

Validated by compiling with `-Wall -Wextra` (clean) and diffing
per-callee relocation counts against the vendor oracle at both `-Os`
and `-O0`: at `-O0`, every callee's call count matches the vendor
**exactly** (`u4Dram_Register_Read` 16, `vPhyByteIO32WriteMsk` 8,
`vPhyByteReadFldAlign` 4, `__meta_backup_and_set` 7, `vIO32WriteMsk` 5,
`udelay` 3, `vSetCalibrationResult` 2, `vIO32WriteMsk_All` 2,
`is_ddr4_family` 2, `DramcBroadcastOnOff` 2, `DramPhyReset` 2, and all
the singles) with one open exception: `__meta_restore` is 7 in the
source/`-O0` build against the vendor's 5, not yet explained (the
matching `__meta_backup_and_set` count suggests the vendor merges two
of its restore call sites as a shared tail, but which two hasn't been
confirmed). `memset`'s 4 vendor calls have no candidate equivalent
since the C source zero-initializes the arrays it split that 0x60-byte
region into instead of calling `memset` on one buffer -- functionally
identical, not a logic gap.

## dramc_rx_dqs_gating_cal for AN7583: done, independently traced

Confirmed a separate function from AN7581's (1568 bytes vs 1948, 52
combined `tbb`+call relocation sites vs 73, its own `r_filter_count`
global 2 bytes wide vs AN7581's 4) and traced independently rather
than adapted from AN7581's source. Same overall algorithm (outer/inner
position sweep, per-lane Mealy FSM via two chained `tbb` tables
reducing to the identical `state = combo>3 ? 0 : 4-combo` plus
transition table), with these confirmed, preserved differences:

- Only 2 lanes ever (`data_width` 0x8 or 0x10, never AN7581's 0x20) --
  no `__meta_backup_and_set`/`__meta_restore` anywhere in the function.
- The rank-select `vIO32WriteMsk(ctx, 0x010007bc, rank<<25, ...)` right
  after `u1GetRank()` uses a literal mask of **0** in the vendor
  object, making that specific write an observable no-op -- kept
  exactly as found rather than "corrected" to AN7581's `0x2000000`.
- The wrap-around/degenerate branch and the main sweep's success exit
  are **the same code**, reached via the vendor's own jump back to the
  function's top-of-loop recheck once `outer_pos` is forced to
  `end_limit` (confirmed by tracing the actual branch target, not
  assumed by analogy to AN7581, where the same sharing pattern was
  verified independently). Modeled here as a single `for (;;)` whose
  top checks `outer_pos >= end_limit` and does the shared commit/
  teardown, so both entry paths run through identical code.
- A genuine AN7581-absent feature: if `*(ctx+0xbd)` is set (rank 1),
  that shared exit skips computing/committing `result_lo[]`/
  `result_hi[]` entirely and instead mirrors whatever rank 0's own
  earlier run of this function cached into `dqs_gating_K_result_rg_rk1[]`
  (an already-declared AN7583-only global, `tree/.../an7583/
  dramc_selfrefresh_api.c`) via `vSetRank(ctx,1)` +
  `vPhyByteWriteFldAlign()` + `vSetRank(ctx,0)` -- the same "rank 1
  mirrors rank 0" pattern already confirmed for AN7583's
  `DramcRxdatlatCal`, now confirmed here too.

Validated the same way as AN7581: compiling with `-Wall -Wextra`
(clean) and diffing per-callee relocation counts against the vendor
oracle at both `-Os` and `-O0`. At `-O0`, **every single callee's call
count matches the vendor exactly** (`u4Dram_Register_Read` 8,
`vIO32WriteMsk` 5, `vPhyByteReadFldAlign` 4, `vPhyByteIO32WriteMsk` 4,
`udelay` 3, `vSetRank` 2, `vSetCalibrationResult` 2,
`vPhyByteWriteFldAlign` 2, `vIO32WriteMsk_All` 2, `is_ddr4_family` 2,
`DramcBroadcastOnOff` 2, `DramPhyReset` 2, and all the singles
including `DramcEngine2End` 1) -- no open discrepancies at all, a
cleaner result than AN7581's one unresolved `__meta_restore` count.
`memset`'s 3 vendor calls have no candidate equivalent for the same
reason as AN7581 (C initializers instead of one `memset` per buffer).

## DramcTxWindowPerbitCal (AN7581): done, zero call-count discrepancies

Before the main function, it depends on three helpers that were only
present in the still-PUBLIC_BASE MediaTek lineage source
(`TxWinTransferDelayToUIPI`, `TXSetDelayReg_DQ`, `TXSetDelayReg_DQM`,
plus `u1IsPhaseMode` as `TxWinTransferDelayToUIPI`'s own dependency) --
all four recovered from their own oracle objects and validated
(3 of 4 relocation counts match the vendor exactly; the fourth,
`TxWinTransferDelayToUIPI`, shows `vGet_Div_Mode` instead of
`u1MCK2UI_DivShift` because Clang -Os inlines that one-line same-TU
helper, exposing its internal call -- the established codegen pattern
already seen throughout this file). Notably, both `TxWinTransferDelayToUIPI`'s
and `TXSetDelayReg_DQ`/`_DQM`'s oracle signatures differ from what the
public lineage suggests: they pack their outputs/inputs into one
shared byte record via a single pointer rather than several separate
pointers/arrays -- confirmed by tracing the actual byte offsets, not
assumed from the public source.

The main function itself (oracle 2504 bytes, candidate 2604 -- a +100
byte/+4% delta, consistent with the range seen across every other
function in this file) was fully instruction-level traced (925
disassembled instructions, 141 branch instructions) using a
mechanical CFG-parsing script rather than by-eye reading, the same
technique `dramc_rx_dqs_gating_cal` required. This is the densest
function reconstructed in this file so far:

- Three 0x140-byte (320-byte) per-bit stack buffers (`bufA`/`bufB`/
  `bufC`, 32 records of 10 bytes each: `S16 start, end, mid; U16
  width, _pad;`), dynamically indexed via runtime `mul`/`mla`
  (`record_ptr = base + 10*bit_index`), not fixed offsets.
- Oracle signature is only 3 parameters (`ctx, cal_type,
  vref_scan_enable`) versus the public lineage's 4 (`calType,
  u1VrefScanEnable, isAutoK`) -- `isAutoK` does not appear in the
  object at all for this SoC/build and was dropped rather than
  guessed at.
- A single stack slot for `vref_scan_enable` is genuinely reused by
  the vendor for two different meanings across the function: on entry
  it means "scan multiple Vref codes instead of committing delays";
  later, only on the non-scanning path, it is overwritten from
  `raw_u16(ctx,0x72)` and means "apply a per-bit Vref delay
  compensation during the final commit". Modeled as one C variable
  with a mid-function reassignment to match the object exactly.
- **Two structurally distinct exit modes**, confirmed by tracing where
  each branch actually terminates (not assumed): when
  `vref_scan_enable != 0` on entry, the function sweeps every Vref
  code in `{0,2,4,...,48}`, running a full per-bit window search at
  each one and recording `{vref_code, sum_width, min_width,
  worst_bit}` into a 6-byte-record scratch array; it then picks the
  Vref code that maximizes the worst-bit window width (with a
  sum-width tie-break and a backward-search refinement step), calls
  `DramcTXSetVref` once more if DDR4, and **returns immediately --
  it never reaches the TX delay commit code in this mode.** The
  vref-analysis block contains one confirmed vendor quirk preserved
  exactly rather than fixed: it reads `vref_scan[scan_count]`, one
  past the last entry actually written this call (an
  out-of-bounds/uninitialized-stack read in the disassembly, not a
  transcription artifact).
- When `vref_scan_enable == 0` on entry: per-bit sweep of `uiDelay`
  from a write-leveling-derived baseline up to a computed
  `search_limit`, calling `DramcEngine2Run` each step and updating
  `bufA`/`bufB` with signed 16-bit (`ldrsh`/`strh`)
  wraparound-sensitive window-open/close/best-so-far logic per bit
  (find the widest passing delay run per bit) -- a genuine
  window-search algorithm, not a small state machine like the
  gating/write-leveling functions. Then a per-lane min/max-of-midpoint
  pass (with a redundant `memcpy(bufC, bufB, ...)` executed on every
  one of the 8 sub-iterations per lane -- kept exactly as observed,
  not hoisted, since the vendor's own compiled code has it inside the
  loop), a commit phase calling `TXSetDelayReg_DQ`/`TXSetDelayReg_DQM`,
  and (only when `raw_u16(ctx,0x72) != 0`) an extra per-bit Vref
  compensation pass writing packed nibble values to
  `0x116009e0`/`0x116009e4`/`0x19600a60`/`0x19600a64` plus OE writes to
  `0x116009ec`/`0x19600a6c`, the upper half of a 32-bit-wide config
  wrapped in `__meta_backup_and_set(ctx,0,1)`/`__meta_restore(ctx,0)`
  (note: `type=0` here, not the `type=1` seen in every prior use of
  this helper pair in this file -- confirmed by re-reading the
  registers at the call site, not assumed to match precedent).
- Confirmed vendor quirk in the Vref-compensation branch: when
  `raw_u16(ctx,0x72) == 0`, the per-lane "final DQ delay" is left at
  `min_mid[lane]` rather than the `(min_mid+max_mid)/2` average used
  everywhere else in the function -- an asymmetry between the DQ and
  DQM final values that is only reachable when `cal_type != 0` (since
  `cal_type == 0` always forces this flag from the same
  `raw_u16(ctx,0x72)` check just before the loop). Preserved exactly.

Validation (an7581, `-O0`): **zero discrepancies** -- every one of the
24 distinct callees matches the vendor's static call-site count
exactly (`vPhyByteWriteFldAlign` 12, `vGet_DDR_Loop_Mode` 3 -- called
three separate times by the vendor rather than cached in a local,
`vIO32WriteMsk` 4, `vPhyByteIO32WriteMsk` 4, `memset` 4,
`TxWinTransferDelayToUIPI`/`TXSetDelayReg_DQ`/`TXSetDelayReg_DQM`/
`DramcTXSetVref`/`vAutoRefreshSwitch`/`vSetCalibrationResult`/`vSetRank`
2 each, `vPhyByteReadFldAlign` 2, `vIO32WriteMsk_All` 2, and all the
singles including `memcpy` 1, `__meta_backup_and_set` 1,
`__meta_restore` 1) -- matching AN7583's `dramc_rx_dqs_gating_cal`
result as the cleanest validation achieved in this file so far.

## DramcTxWindowPerbitCal (AN7583): done, zero call-count discrepancies -- a genuinely different, larger function

AN7583's copy (2732 bytes oracle, 2902 candidate) is *larger* than
AN7581's (2504 bytes), breaking the "AN7583 is simpler" pattern every
other function in this file has shown -- and an independent
1174-instruction trace confirms it is not just a smaller-scale replay
of AN7581's algorithm but has substantial additional logic AN7581 has
no equivalent of at all. Before the main function, six dependencies
needed their own independent traces from AN7583's own objects (none
were assumed from AN7581's):

- `u1IsPhaseMode` and `TxWinTransferDelayToUIPI` are confirmed
  instruction-for-instruction semantically identical to AN7581's (same
  call counts/order); AN7581's build merely expands one conditional as
  an explicit branch where AN7583's expands it as a `clz`-based bit
  trick -- a Clang codegen difference, not an algorithmic one.
- `TXSetDelayReg_DQ`/`_DQM` use a genuinely smaller, 2-lane-only 0x14-byte
  record layout (vs AN7581's 0x28-byte, 4-lane layout) with no
  `data_width==0x20` case at all, consistent with this SoC never
  exceeding 2 DQ byte lanes (confirmed independently by
  `dramc_rx_dqs_gating_cal`'s own trace above).
- Two helpers AN7581 has **no equivalent of at all**:
  `TxWinTransferDelayToUIPIByHighSpeed(ctx, ui_large, ui_small,
  high_nibble)` recomputes one lane's UI-large/UI-small nibble pair
  *relative to the current hardware register readback* (via
  `u4Dram_Register_Read`) rather than from an absolute delay, storing
  the result into ctx-resident scratch fields (`ctx+0xc2..0xcd`); and
  `TXUpdateDelayReg_DQ_DQM(ctx)` commits those scratch fields to the
  same registers.

Confirmed genuine differences in the main function itself (each
verified by re-reading the exact instructions at that point, not
inferred from AN7581):

- `wrlevel_dqs_final_delay` is indexed `[lane + rank*2]`, not AN7581's
  `*4` (matching the same *2 stride already established for this SoC
  in `dramc_rx_dqs_gating_cal`/`DramcWriteLeveling`).
- The per-bit delay sweep starts at `min_delay - 0x10`, not plain
  `min_delay` -- a wider search margin than AN7581 uses.
- The "all bits done" fast-exit compares `data_width` against 8 (1
  lane) instead of AN7581's 32; this SoC's `data_width` is only ever 8
  or 16, never 32.
- The vref-scan analysis applies its result via
  `DramcTXSetVref(ctx, 0, best_vref)` (range argument 0), not AN7581's
  range 1.
- A **table-driven** Vref-compensation OE commit: a 16-byte row from
  `DLY_RG_Mapping[raw_u32(ctx,0xb0)]` remaps which of the 16
  `vref_comp[]` entries lands in which packed nibble position before
  writing to the same four `0x116009e?`/`0x19600a6?` registers AN7581
  writes directly. In the observed object all 3 rows happen to be the
  identity permutation, but the code performs a genuine table lookup
  and is modeled as one rather than assumed to always be the identity.
- A **12-register rank-1 mirror/cache pair**, the same "rank 1 mirrors
  rank 0" pattern used by `dramc_rx_dqs_gating_cal` (keyed off the same
  `raw_u8(ctx,0xbd)` flag): when 0, cache 12 just-committed TX
  delay-chain registers into `Tx_win_K_result_rg_rk1[12]`; when nonzero,
  write those cached values to rank 1 instead of recalibrating it, then
  run the "high speed" combined DQ/DQM commit using whichever of
  `ctx+0xbe`/`ctx+0xc0` a *prior call with the other `cal_type`* left
  cached (a genuinely cross-call stateful mechanism -- DQ tuning with
  `cal_type==0` populates one field, DQM tuning with `cal_type==1`
  populates the other, and a later rank-1 pass combines both).
- A **direct MMIO register write** at the literal address `0x1fc8000c`
  (no named symbol, no `vIO32WriteMsk`/`vPhyByteWriteFldAlign` helper --
  a raw read-modify-write) combining a cached rank-0 Vref code
  (`Tx_vref_K_result_rg_rk0[0]`) and a cached rank-1 Vref code
  (`Tx_vref_K_result_rg_rk1[0]`) into one register's two nibble fields,
  gated by the same `raw_u8(ctx,0xbd)` flag.

One implementation pitfall caught during validation: the 12-register
cache-read/mirror-write pairs must be written as 12 separate unrolled
calls, not a loop over an array -- the oracle has 12 distinct call
sites for each of `vPhyByteReadFldAlign`/`vPhyByteWriteFldAlign` here;
an initial loop-based draft collapsed them to 1 call site each and
undercounted `vPhyByteReadFldAlign` (3 vs oracle 14) and
`vPhyByteWriteFldAlign` (9 vs oracle 20).

Validation (an7583, `-O0`, call + tail-call-jump sites combined):
**zero discrepancies** across all 24 distinct callees, including the
two new-to-AN7583 helpers (`TXUpdateDelayReg_DQ_DQM` 1,
`TxWinTransferDelayToUIPIByHighSpeed` 2), `vPhyByteReadFldAlign` 14,
`vPhyByteWriteFldAlign` 20, and `vSetRank` 6 -- matching AN7581's clean
result for this same function.

## DramcRxWindowPerbitCal (AN7581): done, zero call-count discrepancies

Its three previously-unaddressed dependencies (`DramcRxWinRDDQCInit`,
`DramcRxWinRDDQCRun`, `DramcRxWinRDDQCEnd`) were recovered and validated
for both SoCs first -- see the git history for that commit. (Its other
dependencies -- `GetEyeScanEnable`, `SetRxDqDelay`,
`DramcEngine2Init/Run/End`, `DramPhyReset`, `u4Dram_Register_Read`,
`__meta_backup_and_set`/`__meta_restore` -- were already recovered
elsewhere in this file or in `dramc_utility.c`.)

The main function itself (2540 bytes oracle, 2564 candidate -- a +24
byte/+1% delta, the tightest match of any function in this file) was
fully instruction-level traced (1024 instructions), including one pass
where by-eye reading of the final commit section produced a wrong
conclusion (assuming the four early `u4Dram_Register_Read` results fed
the final UI/PI writes) that a second, more careful re-read caught: the
stack slots those four reads land in are silently reused later by an
unrelated per-lane sum computation before anything reads them back, so
the four reads are dead other than their call sites/side effects. Key
findings:

- Oracle signature is 3 arguments: `(ctx, mode_sel, custom_delay)`.
  `mode_sel` selects between an RDDQC-hardware-assisted path (calls
  `DramcRxWinRDDQCInit`/`Run`/`End`, `mode_sel != 1`) and an
  Engine2-test-pattern path (`mode_sel == 1`, calls `DramcEngine2Init`/
  `Run`/`End` directly instead). `custom_delay`, when non-NULL, points
  at a caller-supplied 4-byte array of per-lane initial RX delay bytes
  (`0xff` in a lane means "search a coarse candidate for this lane
  instead of using a fixed one"); NULL means "default of 14 for every
  lane, no search".
- **Two nested searches**, confirmed by tracing the exact loop-exit
  conditions rather than assumed from `DramcTxWindowPerbitCal`'s
  single-level Vref scan: an inner sweep (RX's equivalent of TX's delay
  sweep) finds the widest passing `uiDelay` run per bit for the
  *current* coarse candidate, using a different, RX-specific delay
  encoding than TX -- `uiDelay<=0` is a negated broadcast into the OE
  registers (`0x11600a0c`/`0x19600a8c`), `uiDelay>0` a direct broadcast
  into the UI/PI registers (`0x11600a08`/`0x19600a88`) *plus* an
  explicit `SetRxDqDelay()` per lane, an asymmetry confirmed by
  re-reading both branches rather than assumed symmetric; an outer loop
  (0 to 31 candidates, but only run multiple times when at least one
  lane is in "auto" mode) tries successive coarse candidates and
  remembers, independently per lane, which candidate gave that lane the
  widest minimum per-bit window (sum-width tie-break), with an
  early-stop heuristic that -- confirmed by tracing the exact
  instructions, not an omission -- only ever examines lanes 0 and 1
  even on a 4-lane config.
- The per-bit window-close reference point (`sweep_start_minus1`, used
  only for the edge case where a bit fails on the very first sweep
  step) is fixed for the whole inner sweep at "starting threshold - 1",
  *not* recomputed per step the way `DramcTxWindowPerbitCal` recomputes
  "current uiDelay - step" -- confirmed by checking every write to that
  stack slot, not assumed to mirror TX.
- A persisted-across-calls `S16 s2RxDelayPreCal` global caches the
  `uiDelay` at which the very first bit opened its window, to seed the
  starting threshold on a *later* call (`mode_sel == 0`: threshold =
  cached value - 10, clamped to -126) rather than always starting from
  a frequency-derived threshold -- a "resume near where we left off"
  optimization with no equivalent in `DramcTxWindowPerbitCal`.
- A DDR3-only quirk (confirmed via the object's single `memcpy` call
  site, invoked 8 times per lane at runtime): after the search, every
  lane's 8 per-bit results are overwritten with bit 0's result,
  flattening the per-bit window to one value per lane.
- The final commit computes, per lane, a magnitude (from the per-bit
  `delta` field, itself only ever initialized to 0 and otherwise
  unused elsewhere in the function) and a group-sum, writes the
  magnitude to the OE registers and the group-sum to the UI/PI
  registers, then does a **second**, differently-shaped commit: 4
  register-groups (`0x116009f8..0x11600a04`, mirrored to a "channel B"
  block at `+0x8000080`) each packing two records' `raw8` field. Only
  `best[lane*8].raw8` (bit 0 of each lane) is ever written anywhere in
  the object; the other 28 of 32 records' `raw8` is a confirmed
  preserved-as-observed uninitialized-stack read, the same class of
  quirk as `DramcTxWindowPerbitCal`'s `vref_scan[scan_count]` OOB read.
- Also writes two new BSS globals not used elsewhere in this file --
  `gFinalRXVrefDQ` (4 bytes) and `gFinalRXVrefDQForSpeedUp` (16 bytes)
  -- with a flat, computed index (channel\*4/16 + rank-selector +
  odt + lane) rather than the public lineage's declared
  `[CHANNEL_NUM][RANK_MAX][2]`/`[...][2][2]` shapes, which don't
  actually fit the object's measured sizes.
- The oracle's own tail is a side-effect-free empty busy-loop (`for (r
  = 0; data_width > r; r += 4) {}`) followed by an unconditional
  `return 0`; simplified to the equivalent direct `return 0` since the
  loop is provably inert for any `data_width`.

Validation (an7581, `-O0`, call + tail-call-jump sites combined):
**zero discrepancies** across all 24 distinct callees, including
`__meta_backup_and_set`/`__meta_restore` 5 each, `vIO32WriteMsk` 8,
`vPhyByteIO32WriteMsk`/`_All` 8/8, `u4Dram_Register_Read` 4, and
`DramPhyReset` 3 -- matching the cleanest results already achieved for
`DramcTxWindowPerbitCal` on both SoCs.

AN7583's copy has not been examined at all yet beyond the dependency
recovery and the size/call-count signal in the table below (call counts
differ substantially from AN7581's -- notably `vPhyByteReadFldAlign` 10
vs AN7581's 0, and `vPhyByteWriteFldAlign` 10 vs AN7581's 4 -- so, as
with `DramcTxWindowPerbitCal`, it should **not** be assumed to share
AN7581's structure and needs its own independent trace):

| callee | an7581 | an7583 |
|---|---|---|
| `DramPhyReset` | 3 | 3 |
| `DramcEngine2End/Init/Run` | 1/1/1 | 1/1/1 |
| `DramcRxWinRDDQCEnd/Init/Run` | 1/1/1 | 1/1/1 |
| `GetEyeScanEnable` | 1 | 1 |
| `SetRxDqDelay` | 2 | 2 |
| `__meta_backup_and_set`/`__meta_restore` | 5/5 | 0/0 |
| `is_ddr3_family` | 1 | 1 |
| `memcpy` | 1 | 1 |
| `u1GetRank` | 1 | 1 |
| `u4Dram_Register_Read` | 4 | 2 |
| `vAutoRefreshSwitch` | 2 | 2 |
| `vIO32WriteMsk`/`vIO32WriteMsk_All` | 8/4 | 4/4 |
| `vPhyByteIO32WriteMsk`/`_All` | 8/8 | 4/8 |
| `vPhyByteReadFldAlign`/`WriteFldAlign` | 0/4 | 10/10 |
| `vPrintCalibrationBasicInfo` | 2 | 2 |
| `vSetCalibrationResult` | 3 | 3 |
| `vSetRank` | 1 | 3 |

Left documented rather than guessed at, per this project's standing
rule, until there is time to give both SoCs' copies the same
address-by-address verification rigor the two `DramcTxWindowPerbitCal`
reconstructions received.
