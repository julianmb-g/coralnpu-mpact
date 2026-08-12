// Copyright 2026 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

# ADR 015, ADR 026: Symbolic MSTATUS Configuration
.equiv MSTATUS_MPP_M,    0x1800
.equiv MSTATUS_FS_CLEAN, 0x4000
.equiv MSTATUS_VS_CLEAN, 0x0400
.equiv MSTATUS_INITIAL, (MSTATUS_MPP_M | MSTATUS_FS_CLEAN | MSTATUS_VS_CLEAN)

.section .text.init
.global _start
_start:
.option push
.option norelax
  la gp, __global_pointer$
.option pop
  la sp, _stack_top

  # ADR 015, ADR 026: Initialize mstatus to Clean state
  li t0, MSTATUS_INITIAL
  csrw mstatus, t0

  # ADR 031: Initialize writable FP and Vector CSRs to zero
  csrw fcsr, zero
  csrw vstart, zero
#if defined(__riscv_zve32f)
  csrw vxsat, zero
  csrw vxrm, zero
  csrw vcsr, zero
#endif

  # ADR 031: Initialize Scalar FP registers f0-f31 to zero
  fcvt.s.w f0, zero
  fcvt.s.w f1, zero
  fcvt.s.w f2, zero
  fcvt.s.w f3, zero
  fcvt.s.w f4, zero
  fcvt.s.w f5, zero
  fcvt.s.w f6, zero
  fcvt.s.w f7, zero
  fcvt.s.w f8, zero
  fcvt.s.w f9, zero
  fcvt.s.w f10, zero
  fcvt.s.w f11, zero
  fcvt.s.w f12, zero
  fcvt.s.w f13, zero
  fcvt.s.w f14, zero
  fcvt.s.w f15, zero
  fcvt.s.w f16, zero
  fcvt.s.w f17, zero
  fcvt.s.w f18, zero
  fcvt.s.w f19, zero
  fcvt.s.w f20, zero
  fcvt.s.w f21, zero
  fcvt.s.w f22, zero
  fcvt.s.w f23, zero
  fcvt.s.w f24, zero
  fcvt.s.w f25, zero
  fcvt.s.w f26, zero
  fcvt.s.w f27, zero
  fcvt.s.w f28, zero
  fcvt.s.w f29, zero
  fcvt.s.w f30, zero
  fcvt.s.w f31, zero

  # ADR 031: Initialize Vector Registers v0-v31
  vsetvli t0, x0, e32, m8, ta, ma
  vmv.v.i v0, 0
  vmv.v.i v8, 0
  vmv.v.i v16, 0
  vmv.v.i v24, 0

#if defined(ENABLE_M4)
  # ADR 031: Initialize mtype and matrix tiles for coralnpu_m4
  li a3, 0x843
  li a4, 0x843
  .word 0x82E6F057 # msetmtype a3, a4
  .word 0x41E06057 # mzero mt0
  .word 0x41E06157 # mzero mt1
  .word 0x41E06257 # mzero mt2
  .word 0x41E06357 # mzero mt3
  .word 0x41E06457 # mzero mt4
  .word 0x41E06557 # mzero mt5
  .word 0x41E06657 # mzero mt6
  .word 0x41E06757 # mzero mt7
  .word 0x41E06857 # mzero mt8
  .word 0x41E06957 # mzero mt9
  .word 0x41E06A57 # mzero mt10
  .word 0x41E06B57 # mzero mt11
  .word 0x41E06C57 # mzero mt12
  .word 0x41E06D57 # mzero mt13
  .word 0x41E06E57 # mzero mt14
  .word 0x41E06F57 # mzero mt15
#endif

  # Zero-initialize the BSS section
  la a0, __BSS_START__
  la a1, __BSS_END__
1:
  bgeu a0, a1, 2f
  sw zero, 0(a0)
  addi a0, a0, 4
  j 1b
2:
  
  li a0, 0
  li a1, 0
  call main
  
  # Pass return code to _exit
  call _exit
  
  j .

.section .tohost, "aw", @progbits
.align 6
.global tohost
tohost:
  .dword 0

.align 7
.global fromhost
fromhost:
  .dword 0
