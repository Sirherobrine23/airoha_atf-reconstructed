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

## Next phase (Phase B — impedance)

Per the handoff, the next block is:

- `DramcImpedanceByEfuse`
- `DramcDRVinitSetting`
- `DramcImpedanceDrvSetRG`
- `DramcImpedanceSetValue`

(`DramcImpedanceEfuseValue` above is Phase B's entry point but was already
recovered in an earlier pass; `DramcImpedanceSetValue` is currently only an
`extern` forward declaration used by it and still needs its own body pulled
from the oracle.)
