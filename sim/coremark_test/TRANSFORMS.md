# Source Code Transformations

To adhere to the **Single-Precision Mandate (ADR 027)** and the **Unified Assembly Generation Mandate (ADR 023)** while maintaining compliance with the **Authenticity Mandate (ADR 002)**, the following surgical transformations are applied to the authentic Coremark source code *within the transient build container only*.

## 1. Include Guards for `coremark.h`
**Rationale:** The authentic `coremark.h` lacks include guards, preventing it from being included multiple times in a single translation unit (required for unified assembly generation).
**Transformation:** Wrap the entire file in `#ifndef COREMARK_H_GUARD`.

## 2. Flexible Typedefs for `secs_ret`, `MATDAT`, and `MATRES`
**Rationale:** The authentic `coremark.h` hardcodes these types to `double` or `ee_s16` when `HAS_FLOAT=1`, which causes ABI mismatches and software emulation on the `coralnpu_m3` target (which only supports single-precision hardware floats).
**Transformation:** Wrap these typedefs in `#ifndef` guards to allow `core_portme.h` to override them with `float`.

## 3. Matrix Macro Guards in `core_matrix.c`
**Rationale:** Authentic `core_matrix.c` redefines `matrix_clip`, `matrix_big`, and `bit_extract`, causing redefinition warnings/errors when these are provided by the porting layer for optimization.
**Transformation:** Wrap these macro definitions in `#ifndef` guards.

These transformations are considered part of the **Adaptive Porting Layer Integration (ADR 001)** and are strictly limited to enabling hardware compatibility without altering the benchmark's core algorithmic logic.
