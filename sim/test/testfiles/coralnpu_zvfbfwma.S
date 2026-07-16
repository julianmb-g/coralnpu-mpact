.text
.globl _start
_start:
    # 1. Initialize v4 with 0x11111111
    li x6, 16
    vsetvli x0, x6, e32, m1, ta, ma
    lui x7, 0x11111
    addi x7, x7, 273 # 0x11111111
    vmv.v.x v4, x7
    vmv.v.x v6, x7
    vmv.v.x v8, x7

    # 2. Set vl to 2, e32, m1, tu
    li x6, 2
    vsetvli x0, x6, e32, m1, tu, ma

    # 3. v2 = 1.0f (BF16), v3 = 2.0f (BF16), v4 = 3.0f (FP32), f2 = 1.0f (BF16)
    # v2 = 0x3f803f80
    lui x5, 0x3f804
    addi x5, x5, -128
    vsetvli x0, x6, e16, m1, tu, ma
    vmv.v.x v2, x5
    # v3 = 0x40004000
    lui x5, 0x40004
    vmv.v.x v3, x5
    # v4 = 3.0f (FP32: 0x40400000)
    vsetvli x0, x6, e32, m1, tu, ma
    lui x5, 0x40400
    vmv.v.x v4, x5
    # f2 = 1.0f (BF16: 0x3f80)
    lui x5, 0xFFFF4
    addi x5, x5, -128 # 0xFFFF3f80
    fmv.w.x f2, x5

    # 4. vfwmaccbf16.vv v4, v2, v3
    vfwmaccbf16.vv v4, v2, v3

    # 5. vfwmaccbf16.vf v4, v2, f2
    vfwmaccbf16.vf v4, v2, f2

    # 6. vfwcvtbf16.f.f.v v6, v2
    vfwcvtbf16.f.f.v v6, v2

    # 7. vfncvtbf16.f.f.w v8, v6
    vfncvtbf16.f.f.w v8, v6

    # 8. f2 = 1.0f (FP32)
    lui x5, 0x3f800
    fmv.w.x f2, x5

    # 9. fcvt.bf16.s f4, f2
    fcvt.bf16.s f4, f2

    # 10. fcvt.s.bf16 f6, f4
    fcvt.s.bf16 f6, f4

    ebreak
