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

## Next phase (Phase C)

Phase C per the handoff:

- `DramcZQCalibration`
- `DramcWriteLeveling`
- `dramc_rx_dqs_gating_cal`
- `DramcRxWindowPerbitCal`
- `DramcRxdatlatCal`
- `DramcTXSetVref`
- `DramcTxWindowPerbitCal`

These are the largest and highest-risk functions in the object (up to
~2.5 KB each); expect them to take substantially longer per function than
Phase A/B.
