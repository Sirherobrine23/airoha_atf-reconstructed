# Vendor BL22 compiler reconstruction

The original AN7581/AN7583 BL22 objects identify their compiler as:

```
GCC: (Buildroot -g413d1bb) 10.3.0
```

The Airoha build tree and SDK metadata identify the toolchain as:

```
Target: arm-buildroot-linux-gnueabi
configured default CPU: cortex-a7
float ABI: soft
ABI: aapcs-linux
configured default mode: arm
vendor path: /opt/trendchip/buildroot-gcc1030-glibc232_kernel5_4
```

The TF-A build overrides the configured ARM mode for AArch32 through
`AARCH32_INSTRUCTION_SET=T32`, therefore BL22 C objects are Thumb-2.

## Effective code-generation flags recovered from TF-A

The common TF-A Makefile contributes:

```
-mthumb
-mno-unaligned-access
-ffunction-sections
-fdata-sections
-ffreestanding
-fno-builtin
-fno-common
-Os
-std=gnu99
```

`plat/ecnt/common/drivers/flash/flash.mk`, which is included by the EN7523
platform makefile, contributes:

```
-march=armv8-a+crc
```

The object attributes agree with this reconstruction:

```
Tag_CPU_name: 8-A
Tag_CPU_arch: v8
Tag_THUMB_ISA_use: Thumb-2
Tag_ABI_enum_size: int
Tag_ABI_optimization_goals: Aggressive Size
```

The vendor toolchain itself defaults to Cortex-A7 tuning and soft-float.  When a
non-vendor GCC 10.3 is used as a codegen proxy, also pass:

```
-mcpu=cortex-a7 -mfloat-abi=soft
```

Do not assume proxy-GCC equality proves byte-exact recovery: the exact Trendchip
Buildroot compiler, assembler, preprocessor defines and header set remain the
final oracle environment.

## PIE

The vendor DRAMC objects use ordinary absolute data relocations such as
`R_ARM_ABS32`; no evidence of a PIE code model is present in these objects.
The convergence helper therefore does not add `-fpie` by default.

## Sources used to derive this

- `work_atf/Makefile`
- `work_atf/make_helpers/defaults.mk`
- `work_atf/plat/ecnt/en7523/platform.mk`
- `work_atf/plat/ecnt/common/drivers/flash/flash.mk`
- `.ARM.attributes` and `.comment` from the original BL22 objects
