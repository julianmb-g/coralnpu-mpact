.section .text.init
.global _start
_start:
  la sp, _stack_top
  la gp, __global_pointer$
  la a0, __BSS_START__
  la a1, __BSS_END__
1:
  bgeu a0, a1, 2f
  sw zero, 0(a0)
  addi a0, a0, 4
  j 1b
2:
  // Enable FPU and Vector Unit
.set MSTATUS_FS_SHIFT, 13
.set MSTATUS_VS_SHIFT, 9
.set MSTATUS_DIRTY, 3
.set MSTATUS_ENABLE_FPU_VECTOR, (MSTATUS_DIRTY << MSTATUS_FS_SHIFT) | (MSTATUS_DIRTY << MSTATUS_VS_SHIFT)
  li t0, MSTATUS_ENABLE_FPU_VECTOR
  csrs mstatus, t0

  // Initialize Vector Registers
  vsetvli t0, x0, e32, m4, ta, ma
  vmv.v.i v0, 0
  vmv.v.i v4, 0
  vmv.v.i v8, 0
  vmv.v.i v12, 0
  vmv.v.i v16, 0
  vmv.v.i v20, 0
  vmv.v.i v24, 0
  vmv.v.i v28, 0

  call main
  
  // Pass return code to _exit
  call _exit
  
  j .
