.file	"core_portme.c"
.option nopic
.text
.align	2
.globl	_fstat
.type	_fstat, @function
_fstat:
  li	a5,8192
  sw	a5,4(a1)
  li	a0,0
  ret
.size	_fstat, .-_fstat
.align	2
.globl	_isatty
.type	_isatty, @function
_isatty:
  li	a0,1
  ret
.size	_isatty, .-_isatty
.align	2
.globl	_close
.type	_close, @function
_close:
  li	a0,-1
  ret
.size	_close, .-_close
.align	2
.globl	_lseek
.type	_lseek, @function
_lseek:
  li	a0,0
  ret
.size	_lseek, .-_lseek
.align	2
.globl	_read
.type	_read, @function
_read:
  li	a0,0
  ret
.size	_read, .-_read
.align	2
.globl	_write
.type	_write, @function
_write:
  addi	sp,sp,-176
  sw	s2,160(sp)
  sw	ra,172(sp)
  mv	s2,a2
  beq	a2,zero,.L_portme_22
  sw	s0,168(sp)
  addi	a3,sp,79
  lui	s0,%hi(.LANCHOR_portme_0)
  sw	s1,164(sp)
  sw	s3,156(sp)
  sw	s4,152(sp)
  sw	s5,148(sp)
  sw	s6,144(sp)
  andi	a3,a3,-64
  mv	s5,a1
  add	s3,a1,a2
  addi	s0,s0,%lo(.LANCHOR_portme_0)
  addi	s4,sp,15
  li	s1,64
  li	s6,1
.L_portme_11:
  lbu	a5,0(s5)
  li	a2,64
  li	a1,0
  mv	a0,a3
  sb	a5,15(sp)
  call	memset
  sw	s1,0(a0)
  sw	zero,4(a0)
  sw	s6,8(a0)
  sw	zero,12(a0)
  sw	s4,16(a0)
  sw	zero,20(a0)
  sw	s6,24(a0)
  mv	a3,a0
  sw	zero,28(a0)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a0,0(s0)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	s6,64(s0)
.L_portme_25:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s0)
  lw	a5,132(s0)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_25
  li	a5,0
  sw	a5,128(s0)
  li	a6,0
  sw	a6,132(s0)
  sw	zero,64(s0)
  addi	s5,s5,1
  bne	s5,s3,.L_portme_11
  lw	s0,168(sp)
  lw	s1,164(sp)
  lw	s3,156(sp)
  lw	s4,152(sp)
  lw	s5,148(sp)
  lw	s6,144(sp)
.L_portme_22:
  lw	ra,172(sp)
  mv	a0,s2
  lw	s2,160(sp)
  addi	sp,sp,176
  jr	ra
.size	_write, .-_write
.align	2
.globl	ee_printf
.type	ee_printf, @function
ee_printf:
  addi	sp,sp,-976
  sw	a4,960(sp)
  sw	ra,940(sp)
  sw	a1,948(sp)
  sw	a2,952(sp)
  sw	a3,956(sp)
  sw	a5,964(sp)
  sw	a6,968(sp)
  sw	a7,972(sp)
  lbu	a5,0(a0)
  addi	a4,sp,948
  sw	a4,44(sp)
  beq	a5,zero,.L_portme_232
  sw	s7,908(sp)
  lui	s7,%hi(.L_portme_138)
  addi	a4,s7,%lo(.L_portme_138)
  sw	s9,900(sp)
  lui	s9,%hi(.L_portme_36)
  sw	s1,932(sp)
  sw	s11,892(sp)
  lui	s1,%hi(.LANCHOR_portme_0)
  addi	s11,sp,175
  sw	a4,4(sp)
  addi	a4,s9,%lo(.L_portme_36)
  sw	s0,936(sp)
  sw	s3,924(sp)
  sw	s2,928(sp)
  sw	s4,920(sp)
  sw	s5,916(sp)
  sw	s6,912(sp)
  sw	s8,904(sp)
  sw	s10,896(sp)
  mv	s0,a0
  andi	s11,s11,-64
  addi	s1,s1,%lo(.LANCHOR_portme_0)
  sw	a4,16(sp)
  li	s3,37
.L_portme_27:
  bne	a5,s3,.L_portme_28
  lbu	a5,1(s0)
  li	a4,48
  beq	a5,a4,.L_portme_29
  addi	s0,s0,1
  li	s6,0
.L_portme_30:
  li	a4,108
  beq	a5,a4,.L_portme_280
.L_portme_32:
  beq	a5,s3,.L_portme_33
  addi	a4,a5,-88
  andi	a4,a4,0xff
  li	a3,32
  bgtu	a4,a3,.L_portme_34
  lw	a3,4(sp)
  slli	a4,a4,2
  add	a4,a4,a3
  lw	a4,0(a4)
  jr	a4
.section	.rodata
.align	2
.align	2
.L_portme_138:
.word	.L_portme_75
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_41
.word	.L_portme_56
.word	.L_portme_34
.word	.L_portme_39
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_38
.word	.L_portme_34
.word	.L_portme_43
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_75
.text
.L_portme_28:
  li	s4,64
  mv	a2,s4
  li	a1,0
  mv	a0,s11
  sb	a5,48(sp)
  call	memset
  sw	s4,0(s11)
  sw	zero,4(s11)
  li	a5,1
  sw	a5,8(s11)
  sw	zero,12(s11)
  addi	s2,sp,48
  sw	s2,16(s11)
  sw	zero,20(s11)
  sw	a5,24(s11)
  sw	zero,28(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	s11,0(s1)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a5,64(s1)
.L_portme_253:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s1)
  lw	a5,132(s1)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_253
  li	a5,0
  sw	a5,128(s1)
  li	a6,0
  sw	a6,132(s1)
  sw	zero,64(s1)
.L_portme_34:
  lbu	a5,1(s0)
  addi	s0,s0,1
  bne	a5,zero,.L_portme_27
.L_portme_281:
  lw	s0,936(sp)
  lw	s1,932(sp)
  lw	s2,928(sp)
  lw	s3,924(sp)
  lw	s4,920(sp)
  lw	s5,916(sp)
  lw	s6,912(sp)
  lw	s7,908(sp)
  lw	s8,904(sp)
  lw	s9,900(sp)
  lw	s10,896(sp)
  lw	s11,892(sp)
.L_portme_232:
  lw	ra,940(sp)
  addi	sp,sp,976
  jr	ra
.L_portme_38:
  lw	a5,44(sp)
  lw	s5,0(a5)
  addi	a5,a5,4
  sw	a5,44(sp)
  beq	s5,zero,.L_portme_34
  lbu	a5,0(s5)
  beq	a5,zero,.L_portme_34
  lui	s10,%hi(.LANCHOR_portme_0)
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  addi	s2,sp,48
  addi	a3,s11,384
  li	s4,64
  li	s6,1
.L_portme_97:
  li	a2,64
  li	a1,0
  mv	a0,a3
  sb	a5,48(sp)
  call	memset
  sw	s4,384(s11)
  sw	zero,388(s11)
  sw	s6,392(s11)
  sw	zero,396(s11)
  sw	s2,400(s11)
  sw	zero,404(s11)
  sw	s6,408(s11)
  addi	s5,s5,1
  mv	a3,a0
  sw	zero,412(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a0,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	s6,64(s10)
.L_portme_243:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_243
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  sw	a6,132(s10)
  sw	zero,64(s10)
  lbu	a5,0(s5)
  bne	a5,zero,.L_portme_97
  lbu	a5,1(s0)
  addi	s0,s0,1
  bne	a5,zero,.L_portme_27
  j	.L_portme_281
.L_portme_39:
  lw	a5,44(sp)
  lui	a4,%hi(.LC_portme_0)
  lw	a2,%lo(.LC_portme_0)(a4)
  addi	a5,a5,7
  andi	a5,a5,-8
  lw	s8,4(a5)
  lw	s6,0(a5)
  lw	a3,%lo(.LC_portme_0+4)(a4)
  slli	s2,s8,1
  addi	a5,a5,8
  mv	a0,s6
  srli	a1,s2,1
  sw	a5,44(sp)
  call	__gtdf2
  bgt	a0,zero,.L_portme_282
  lui	a5,%hi(.LC_portme_2)
  lw	a2,%lo(.LC_portme_2)(a5)
  lw	a3,%lo(.LC_portme_2+4)(a5)
  mv	a0,s6
  srli	a1,s2,1
  call	__adddf3
  mv	s5,a1
  mv	s4,a0
  call	__fixdfsi
  mv	s10,a0
  call	__floatsidf
  mv	a2,a0
  mv	a3,a1
  mv	a0,s4
  mv	a1,s5
  call	__subdf3
  bne	s10,zero,.L_portme_283
  li	a5,48
  sb	a5,48(sp)
  li	s10,6
  li	s7,5
  li	s9,4
  li	a6,3
  li	a7,2
  li	s4,1
  li	s2,7
  addi	t1,sp,49
.L_portme_105:
  lui	s5,%hi(.LC_portme_3)
  lw	a2,%lo(.LC_portme_3)(s5)
  lw	a3,%lo(.LC_portme_3+4)(s5)
  sw	a6,28(sp)
  sw	a7,24(sp)
  sw	t1,20(sp)
  call	__muldf3
  sw	a1,12(sp)
  sw	a0,8(sp)
  call	__fixdfsi
  lw	t1,20(sp)
  addi	a3,a0,48
  add	s10,s10,sp
  sb	a3,0(t1)
  call	__floatsidf
  mv	a2,a0
  mv	a3,a1
  lw	a0,8(sp)
  lw	a1,12(sp)
  add	s2,s2,sp
  call	__subdf3
  lw	a2,%lo(.LC_portme_3)(s5)
  lw	a3,%lo(.LC_portme_3+4)(s5)
  call	__muldf3
  sw	a1,12(sp)
  sw	a0,8(sp)
  call	__fixdfsi
  lw	a7,24(sp)
  addi	a3,a0,48
  add	a7,a7,sp
  sb	a3,48(a7)
  call	__floatsidf
  mv	a2,a0
  mv	a3,a1
  lw	a0,8(sp)
  lw	a1,12(sp)
  call	__subdf3
  lw	a2,%lo(.LC_portme_3)(s5)
  lw	a3,%lo(.LC_portme_3+4)(s5)
  call	__muldf3
  sw	a1,12(sp)
  sw	a0,8(sp)
  call	__fixdfsi
  lw	a6,28(sp)
  addi	a3,a0,48
  add	a6,a6,sp
  sb	a3,48(a6)
  call	__floatsidf
  mv	a2,a0
  mv	a3,a1
  lw	a0,8(sp)
  lw	a1,12(sp)
  call	__subdf3
  lw	a2,%lo(.LC_portme_3)(s5)
  lw	a3,%lo(.LC_portme_3+4)(s5)
  call	__muldf3
  sw	a1,12(sp)
  sw	a0,8(sp)
  call	__fixdfsi
  add	a4,s9,sp
  addi	a3,a0,48
  sb	a3,48(a4)
  call	__floatsidf
  mv	a2,a0
  mv	a3,a1
  lw	a0,8(sp)
  lw	a1,12(sp)
  call	__subdf3
  lw	a2,%lo(.LC_portme_3)(s5)
  lw	a3,%lo(.LC_portme_3+4)(s5)
  call	__muldf3
  sw	a1,12(sp)
  sw	a0,8(sp)
  call	__fixdfsi
  add	a5,s7,sp
  addi	a4,a0,48
  sb	a4,48(a5)
  call	__floatsidf
  mv	a2,a0
  mv	a3,a1
  lw	a0,8(sp)
  lw	a1,12(sp)
  call	__subdf3
  lw	a2,%lo(.LC_portme_3)(s5)
  lw	a3,%lo(.LC_portme_3+4)(s5)
  call	__muldf3
  call	__fixdfsi
  addi	a5,a0,48
  sb	a5,48(s10)
  sb	zero,48(s2)
  mv	a0,s6
  mv	a1,s8
  li	a2,0
  li	a3,0
  call	__ltdf2
  bge	a0,zero,.L_portme_107
.L_portme_292:
  li	s2,64
  li	a4,45
  addi	a0,s11,320
  mv	a2,s2
  li	a1,0
  sb	a4,43(sp)
  call	memset
  sw	s2,320(s11)
  sw	zero,324(s11)
  li	a4,1
  sw	a4,328(s11)
  sw	zero,332(s11)
  addi	s5,sp,43
  sw	s5,336(s11)
  sw	zero,340(s11)
  sw	a4,344(s11)
  sw	zero,348(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lui	s10,%hi(.LANCHOR_portme_0)
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  sw	a0,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a4,64(s10)
.L_portme_245:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_245
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  sw	a6,132(s10)
  sw	zero,64(s10)
  j	.L_portme_107
.L_portme_41:
  lw	a4,44(sp)
  li	s4,64
  addi	a0,s11,128
  lw	a3,0(a4)
  mv	a2,s4
  addi	a4,a4,4
  li	a1,0
  sw	a4,44(sp)
  sb	a3,48(sp)
  call	memset
  sw	s4,128(s11)
  sw	zero,132(s11)
  li	a4,1
  sw	a4,136(s11)
  sw	zero,140(s11)
  addi	s2,sp,48
  sw	s2,144(s11)
  sw	zero,148(s11)
  sw	a4,152(s11)
  sw	zero,156(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lui	s10,%hi(.LANCHOR_portme_0)
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  sw	a0,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a4,64(s10)
.L_portme_249:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_249
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  sw	a6,132(s10)
  sw	zero,64(s10)
.L_portme_284:
  lbu	a5,1(s0)
  addi	s0,s0,1
  bne	a5,zero,.L_portme_27
  j	.L_portme_281
.L_portme_29:
  lbu	a5,2(s0)
  li	a4,9
  addi	s6,a5,-48
  andi	a3,s6,0xff
  bleu	a3,a4,.L_portme_31
  li	a4,108
  addi	s0,s0,2
  li	s6,0
  bne	a5,a4,.L_portme_32
.L_portme_280:
  lbu	a5,1(s0)
  addi	s0,s0,1
  beq	a5,s3,.L_portme_33
  addi	a4,a5,-88
  andi	a4,a4,0xff
  li	a3,32
  bgtu	a4,a3,.L_portme_34
  lw	a3,16(sp)
  slli	a4,a4,2
  add	a4,a4,a3
  lw	a4,0(a4)
  jr	a4
.section	.rodata
.align	2
.align	2
.L_portme_36:
.word	.L_portme_35
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_41
.word	.L_portme_40
.word	.L_portme_34
.word	.L_portme_39
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_38
.word	.L_portme_34
.word	.L_portme_37
.word	.L_portme_34
.word	.L_portme_34
.word	.L_portme_35
.text
.L_portme_31:
  lbu	a5,3(s0)
  addi	s0,s0,3
  j	.L_portme_30
.L_portme_33:
  li	s4,64
  li	a4,37
  addi	a0,s11,64
  mv	a2,s4
  li	a1,0
  sb	a4,48(sp)
  call	memset
  sw	s4,64(s11)
  sw	zero,68(s11)
  li	a4,1
  sw	a4,72(s11)
  sw	zero,76(s11)
  addi	s2,sp,48
  sw	s2,80(s11)
  sw	zero,84(s11)
  sw	a4,88(s11)
  sw	zero,92(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lui	s10,%hi(.LANCHOR_portme_0)
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  sw	a0,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a4,64(s10)
.L_portme_251:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_251
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  sw	a6,132(s10)
  sw	zero,64(s10)
  j	.L_portme_284
.L_portme_75:
  lw	a3,44(sp)
  lw	a4,0(a3)
  addi	a3,a3,4
  sw	a3,44(sp)
  bne	a4,zero,.L_portme_76
.L_portme_286:
  li	a5,48
  sb	a5,48(sp)
  addi	s2,sp,48
  li	a5,1
.L_portme_83:
  ble	s6,a5,.L_portme_285
  li	a3,48
  vsetvli	a4,zero,e8,m1,ta,ma
  vmv.v.x	v1,a3
  sub	a3,s6,a5
  vsetvli	a4,a3,e8,m1,ta,ma
  add	a5,s2,a5
  vse8.v	v1,0(a5)
  sub	a2,a3,a4
  beq	a3,a4,.L_portme_78
  add	a5,a5,a4
  vsetvli	zero,a2,e8,m1,ta,ma
  vse8.v	v1,0(a5)
.L_portme_78:
  lui	s10,%hi(.LANCHOR_portme_0)
  add	s6,s2,s6
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  addi	s5,sp,43
  addi	a3,s11,448
  li	s4,64
  li	s8,1
.L_portme_89:
  lbu	a5,-1(s6)
  li	a2,64
  li	a1,0
  mv	a0,a3
  sb	a5,43(sp)
  call	memset
  sw	s4,448(s11)
  sw	zero,452(s11)
  sw	s8,456(s11)
  sw	zero,460(s11)
  sw	s5,464(s11)
  sw	zero,468(s11)
  sw	s8,472(s11)
  mv	a3,a0
  sw	zero,476(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a0,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	s8,64(s10)
.L_portme_241:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_241
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  sw	a6,132(s10)
  sw	zero,64(s10)
  addi	s6,s6,-1
  bne	s2,s6,.L_portme_89
  lbu	a5,1(s0)
  addi	s0,s0,1
  bne	a5,zero,.L_portme_27
  j	.L_portme_281
.L_portme_35:
  lw	a4,44(sp)
  addi	a3,a4,4
  lw	a4,0(a4)
  sw	a3,44(sp)
  beq	a4,zero,.L_portme_286
.L_portme_76:
  li	a3,88
  beq	a5,a3,.L_portme_146
  li	a5,0
  addi	s2,sp,48
  li	a1,9
.L_portme_82:
  andi	a3,a4,15
  addi	a2,a3,87
  bgtu	a3,a1,.L_portme_81
  addi	a2,a3,48
.L_portme_81:
  addi	a5,a5,1
  add	a3,s2,a5
  srli	a4,a4,4
  sb	a2,-1(a3)
  bne	a4,zero,.L_portme_82
  j	.L_portme_83
.L_portme_43:
  lw	a4,44(sp)
  lw	a5,0(a4)
  addi	a4,a4,4
  sw	a4,44(sp)
  bne	a5,zero,.L_portme_145
.L_portme_291:
  li	a5,48
  sb	a5,48(sp)
  li	a4,1
  add	s2,sp,a5
.L_portme_47:
  ble	s6,a4,.L_portme_287
  li	a3,48
  vsetvli	a5,zero,e8,m1,ta,ma
  vmv.v.x	v1,a3
  sub	a3,s6,a4
  vsetvli	a5,a3,e8,m1,ta,ma
  add	a4,s2,a4
  vse8.v	v1,0(a4)
  sub	a2,a3,a5
  beq	a3,a5,.L_portme_46
  add	a4,a4,a5
  vsetvli	zero,a2,e8,m1,ta,ma
  vse8.v	v1,0(a4)
.L_portme_46:
  lui	s10,%hi(.LANCHOR_portme_0)
  add	s6,s2,s6
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  addi	s5,sp,43
  addi	a3,s11,640
  li	s4,64
  li	s8,1
.L_portme_51:
  lbu	a5,-1(s6)
  li	a2,64
  li	a1,0
  mv	a0,a3
  sb	a5,43(sp)
  call	memset
  sw	s4,640(s11)
  sw	zero,644(s11)
  sw	s8,648(s11)
  sw	zero,652(s11)
  sw	s5,656(s11)
  sw	zero,660(s11)
  sw	s8,664(s11)
  mv	a3,a0
  sw	zero,668(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a0,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	s8,64(s10)
.L_portme_235:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_235
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  sw	a6,132(s10)
  sw	zero,64(s10)
  addi	s6,s6,-1
  bne	s2,s6,.L_portme_51
  lbu	a5,1(s0)
  addi	s0,s0,1
  bne	a5,zero,.L_portme_27
  j	.L_portme_281
.L_portme_56:
  lw	a5,44(sp)
  lw	s4,0(a5)
  addi	a5,a5,4
  sw	a5,44(sp)
  mv	a5,s4
  blt	s4,zero,.L_portme_288
.L_portme_57:
  bne	s4,zero,.L_portme_289
  li	a5,48
  lui	s10,%hi(.LANCHOR_portme_0)
  sb	a5,48(sp)
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  li	a4,1
  add	s2,sp,a5
.L_portme_66:
  ble	s6,a4,.L_portme_290
  li	a3,48
  vsetvli	a5,zero,e8,m1,ta,ma
  vmv.v.x	v1,a3
  sub	a3,s6,a4
  vsetvli	a5,a3,e8,m1,ta,ma
  add	a4,s2,a4
  vse8.v	v1,0(a4)
  sub	a2,a3,a5
  beq	a3,a5,.L_portme_65
  add	a4,a4,a5
  vsetvli	zero,a2,e8,m1,ta,ma
  vse8.v	v1,0(a4)
.L_portme_65:
  add	s6,s2,s6
  addi	s5,sp,43
  addi	a3,s11,512
  li	s4,64
  li	s8,1
.L_portme_70:
  lbu	a5,-1(s6)
  li	a2,64
  li	a1,0
  mv	a0,a3
  sb	a5,43(sp)
  call	memset
  sw	s4,512(s11)
  sw	zero,516(s11)
  sw	s8,520(s11)
  sw	zero,524(s11)
  sw	s5,528(s11)
  sw	zero,532(s11)
  sw	s8,536(s11)
  mv	a3,a0
  sw	zero,540(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a0,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	s8,64(s10)
.L_portme_239:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_239
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  sw	a6,132(s10)
  sw	zero,64(s10)
  addi	s6,s6,-1
  bne	s2,s6,.L_portme_70
  lbu	a5,1(s0)
  addi	s0,s0,1
  bne	a5,zero,.L_portme_27
  j	.L_portme_281
.L_portme_37:
  lw	a5,44(sp)
  addi	a4,a5,4
  lw	a5,0(a5)
  sw	a4,44(sp)
  beq	a5,zero,.L_portme_291
.L_portme_145:
  li	a6,-858992640
  addi	a6,a6,-819
  li	a4,0
  addi	s2,sp,48
  li	a7,9
.L_portme_44:
  mulhu	a3,a5,a6
  addi	a4,a4,1
  mv	a1,a5
  add	a0,s2,a4
  srli	a3,a3,3
  slli	a2,a3,2
  add	a2,a2,a3
  slli	a2,a2,1
  sub	a5,a5,a2
  addi	a5,a5,48
  sb	a5,-1(a0)
  mv	a5,a3
  bgtu	a1,a7,.L_portme_44
  j	.L_portme_47
.L_portme_40:
  lw	a5,44(sp)
  lw	s4,0(a5)
  addi	a4,a5,4
  sw	a4,44(sp)
  mv	a5,s4
  bge	s4,zero,.L_portme_57
.L_portme_288:
  li	s5,64
  li	a4,45
  addi	a0,s11,576
  mv	a2,s5
  li	a1,0
  sb	a4,48(sp)
  call	memset
  sw	s5,576(s11)
  sw	zero,580(s11)
  li	a4,1
  sw	a4,584(s11)
  sw	zero,588(s11)
  addi	s2,sp,48
  sw	s2,592(s11)
  sw	zero,596(s11)
  sw	a4,600(s11)
  sw	zero,604(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lui	s10,%hi(.LANCHOR_portme_0)
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  sw	a0,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a4,64(s10)
.L_portme_237:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_237
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  sw	a6,132(s10)
  sw	zero,64(s10)
  neg	a5,s4
.L_portme_60:
  li	a6,-858992640
  addi	a6,a6,-819
  li	a4,0
  li	a7,9
.L_portme_63:
  mulhu	a3,a5,a6
  addi	a4,a4,1
  mv	a1,a5
  add	a0,s2,a4
  srli	a3,a3,3
  slli	a2,a3,2
  add	a2,a2,a3
  slli	a2,a2,1
  sub	a5,a5,a2
  addi	a5,a5,48
  sb	a5,-1(a0)
  mv	a5,a3
  bgtu	a1,a7,.L_portme_63
  j	.L_portme_66
.L_portme_282:
  lui	a5,%hi(.LC_portme_1)
  vsetivli	zero,4,e8,m1,ta,ma
  addi	a5,a5,%lo(.LC_portme_1)
  vle8.v	v1,0(a5)
  addi	s2,sp,48
  mv	a0,s6
  mv	a1,s8
  li	a2,0
  li	a3,0
  vse8.v	v1,0(s2)
  li	s4,4
  call	__ltdf2
  blt	a0,zero,.L_portme_292
.L_portme_107:
  lbu	a7,48(sp)
  beq	a7,zero,.L_portme_34
  addi	a5,s11,256
  addi	s2,sp,48
  addi	s7,sp,49
  sw	a5,8(sp)
  sub	a5,s7,s2
  lui	s10,%hi(.LANCHOR_portme_0)
  addi	a5,a5,-1
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  addi	s8,sp,43
  addi	s5,s11,192
  li	s6,64
  li	s9,1
  beq	a5,s4,.L_portme_293
.L_portme_114:
  li	a2,64
  li	a1,0
  mv	a0,s5
  sb	a7,43(sp)
  call	memset
  sw	s6,192(s11)
  sw	zero,196(s11)
  sw	s9,200(s11)
  sw	zero,204(s11)
  sw	s8,208(s11)
  sw	zero,212(s11)
  sw	s9,216(s11)
  sw	zero,220(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	s5,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	s9,64(s10)
.L_portme_247:
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_247
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  lbu	a7,0(s7)
  sw	a6,132(s10)
  sw	zero,64(s10)
  addi	s7,s7,1
  beq	a7,zero,.L_portme_34
  sub	a5,s7,s2
  addi	a5,a5,-1
  bne	a5,s4,.L_portme_114
.L_portme_293:
  lw	a0,8(sp)
  li	a4,46
  li	a2,64
  li	a1,0
  sb	a4,42(sp)
  sw	a7,20(sp)
  call	memset
  sw	s6,256(s11)
  sw	zero,260(s11)
  sw	s9,264(s11)
  sw	zero,268(s11)
  addi	a4,sp,42
  sw	a4,272(s11)
  sw	zero,276(s11)
  sw	s9,280(s11)
  sw	zero,284(s11)
  #APP
  # 82 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a5,8(sp)
  sw	a5,0(s10)
  #APP
  # 89 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	s9,64(s10)
  #APP
  # 91 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  lw	a7,20(sp)
  or	a4,a4,a5
  bne	a4,zero,.L_portme_118
.L_portme_115:
  #APP
  # 94 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lw	a4,128(s10)
  lw	a5,132(s10)
  or	a4,a4,a5
  beq	a4,zero,.L_portme_115
.L_portme_118:
  li	a5,0
  sw	a5,128(s10)
  li	a6,0
  sw	a6,132(s10)
  sw	zero,64(s10)
  j	.L_portme_114
.L_portme_283:
  li	a6,-858992640
  addi	a6,a6,-819
  mv	a5,s10
  li	s4,0
  li	a7,9
  ble	s10,zero,.L_portme_294
.L_portme_104:
  mulhu	a3,a5,a6
  mv	a4,a5
  mv	a2,s4
  addi	s4,s4,1
  srli	a5,a3,3
  bgt	a4,a7,.L_portme_104
  addi	s2,sp,48
  li	a6,1717985280
  add	a3,s2,a2
  addi	a6,a6,1639
  addi	a7,sp,47
.L_portme_106:
  mulh	a4,s10,a6
  srai	a5,s10,31
  addi	a3,a3,-1
  srai	a4,a4,2
  sub	a4,a4,a5
  slli	a5,a4,2
  add	a5,a5,a4
  slli	a5,a5,1
  sub	a5,s10,a5
  addi	a5,a5,48
  sb	a5,1(a3)
  mv	s10,a4
  bne	a7,a3,.L_portme_106
  add	t1,s2,s4
  addi	a7,a2,2
  addi	s2,a2,7
  addi	a6,a2,3
  addi	s9,a2,4
  addi	s7,a2,5
  addi	s10,a2,6
  j	.L_portme_105
.L_portme_287:
  mv	s6,a4
  j	.L_portme_46
.L_portme_285:
  mv	s6,a5
  j	.L_portme_78
.L_portme_290:
  mv	s6,a4
  j	.L_portme_65
.L_portme_146:
  li	a5,0
  addi	s2,sp,48
  li	a1,9
.L_portme_79:
  andi	a3,a4,15
  addi	a2,a3,48
  bleu	a3,a1,.L_portme_85
  addi	a2,a3,55
.L_portme_85:
  addi	a5,a5,1
  add	a3,s2,a5
  srli	a4,a4,4
  sb	a2,-1(a3)
  bne	a4,zero,.L_portme_79
  j	.L_portme_83
.L_portme_294:
  li	a7,1
  mv	s4,a7
  li	s10,5
  li	s7,4
  li	s9,3
  li	a6,2
  li	s2,6
  addi	t1,sp,48
  j	.L_portme_105
.L_portme_289:
  lui	s10,%hi(.LANCHOR_portme_0)
  addi	s10,s10,%lo(.LANCHOR_portme_0)
  addi	s2,sp,48
  j	.L_portme_60
.size	ee_printf, .-ee_printf
.align	2
.globl	_exit
.type	_exit, @function
_exit:
  #APP
  # 260 "coralnpu_port/core_portme.c" 1
  .word 0x08000073

  # 0 "" 2
  #NO_APP
  lui	a5,%hi(.LANCHOR_portme_0)
  srai	a4,a0,31
  slli	a3,a0,1
  addi	a5,a5,%lo(.LANCHOR_portme_0)
  srli	a0,a0,31
  ori	a3,a3,1
  slli	a4,a4,1
  add	a4,a0,a4
  sw	a3,0(a5)
  sw	a4,4(a5)
  #APP
  # 267 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  li	a4,1
  sw	a4,64(a5)
.L_portme_296:
  j	.L_portme_296
.size	_exit, .-_exit
.section	.rodata.str1.4,"aMS",@progbits,1
.align	2
.LC_portme_4:
.string	"ABORT CALLED!\n"
.text
.align	2
.globl	abort
.type	abort, @function
abort:
  lui	a0,%hi(.LC_portme_4)
  addi	sp,sp,-16
  addi	a0,a0,%lo(.LC_portme_4)
  sw	ra,12(sp)
  call	ee_printf
  li	a0,1
  call	_exit
.size	abort, .-abort
.align	2
.globl	start_time
.type	start_time, @function
start_time:
  #APP
  # 276 "coralnpu_port/core_portme.c" 1
  csrr a4, mcycle
  # 0 "" 2
  #NO_APP
  lui	a5,%hi(start_time_val)
  sw	a4,%lo(start_time_val)(a5)
  ret
.size	start_time, .-start_time
.align	2
.globl	stop_time
.type	stop_time, @function
stop_time:
  #APP
  # 276 "coralnpu_port/core_portme.c" 1
  csrr a4, mcycle
  # 0 "" 2
  #NO_APP
  lui	a5,%hi(stop_time_val)
  sw	a4,%lo(stop_time_val)(a5)
  ret
.size	stop_time, .-stop_time
.align	2
.globl	get_time
.type	get_time, @function
get_time:
  lui	a4,%hi(stop_time_val)
  lui	a5,%hi(start_time_val)
  lw	a0,%lo(stop_time_val)(a4)
  lw	a5,%lo(start_time_val)(a5)
  sub	a0,a0,a5
  ret
.size	get_time, .-get_time
.align	2
.globl	time_in_secs
.type	time_in_secs, @function
time_in_secs:
  lui	a5,%hi(.LC_portme_5)
  flw	fa5,%lo(.LC_portme_5)(a5)
  fcvt.s.wu	fa0,a0
  addi	sp,sp,-16
  sw	ra,12(sp)
  fmul.s	fa0,fa0,fa5
  call	__extendsfdf2
  lw	ra,12(sp)
  addi	sp,sp,16
  jr	ra
.size	time_in_secs, .-time_in_secs
.align	2
.globl	portable_malloc
.type	portable_malloc, @function
portable_malloc:
  lui	a2,%hi(current_alloc_offset)
  lw	a4,%lo(current_alloc_offset)(a2)
  addi	a5,a0,63
  andi	a5,a5,-64
  li	a3,1998848
  add	a5,a5,a4
  addi	a3,a3,1152
  bgtu	a5,a3,.L_portme_306
  lui	a3,%hi(static_memblock)
  addi	a3,a3,%lo(static_memblock)
  sw	a5,%lo(current_alloc_offset)(a2)
  add	a0,a4,a3
  ret
.L_portme_306:
  li	a0,0
  ret
.size	portable_malloc, .-portable_malloc
.align	2
.globl	portable_free
.type	portable_free, @function
portable_free:
  ret
.size	portable_free, .-portable_free
.align	2
.globl	reset_portable_malloc
.type	reset_portable_malloc, @function
reset_portable_malloc:
  lui	a5,%hi(current_alloc_offset)
  sw	zero,%lo(current_alloc_offset)(a5)
  ret
.size	reset_portable_malloc, .-reset_portable_malloc
.align	2
.globl	_sbrk
.type	_sbrk, @function
_sbrk:
  lui	a3,%hi(heap_end.0)
  lw	a5,%lo(heap_end.0)(a3)
  lui	a4,%hi(__heap_start)
  addi	a4,a4,%lo(__heap_start)
  beq	a5,zero,.L_portme_315
.L_portme_311:
  add	a0,a5,a0
  bltu	a0,a4,.L_portme_313
  lui	a4,%hi(__heap_end)
  addi	a4,a4,%lo(__heap_end)
  bgtu	a0,a4,.L_portme_313
  sw	a0,%lo(heap_end.0)(a3)
  mv	a0,a5
  ret
.L_portme_313:
  li	a5,-1
  mv	a0,a5
  ret
.L_portme_315:
  sw	a4,%lo(heap_end.0)(a3)
  mv	a5,a4
  j	.L_portme_311
.size	_sbrk, .-_sbrk
.align	2
.globl	portable_init
.type	portable_init, @function
portable_init:
  li	a5,1
  sb	a5,0(a0)
  ret
.size	portable_init, .-portable_init
.align	2
.globl	portable_fini
.type	portable_fini, @function
portable_fini:
  addi	sp,sp,-16
  sw	ra,12(sp)
  sb	zero,0(a0)
  li	a0,0
  call	_exit
.size	portable_fini, .-portable_fini
.globl	default_num_contexts
.globl	fromhost
.globl	fromhost_ready
.globl	tohost
.globl	tohost_ready
.globl	seed5_volatile
.globl	seed4_volatile
.globl	seed3_volatile
.globl	seed2_volatile
.globl	seed1_volatile
.section	.srodata.cst8,"aM",@progbits,8
.align	3
.LC_portme_0:
.word	-4194306
.word	1105199103
.section	.srodata.cst4,"aM",@progbits,4
.align	2
.LC_portme_1:
.byte	111
.byte	118
.byte	102
.byte	0
.section	.srodata.cst8
.align	3
.LC_portme_2:
.word	-1598689905
.word	1050724087
.align	3
.LC_portme_3:
.word	0
.word	1076101120
.section	.srodata.cst4
.align	2
.LC_portme_5:
.word	854498299
.bss
.align	6
.type	static_memblock, @object
.size	static_memblock, 2000000
static_memblock:
.zero	2000000
.section	.sbss,"aw",@nobits
.align	2
.type	heap_end.0, @object
.size	heap_end.0, 4
heap_end.0:
.zero	4
.type	current_alloc_offset, @object
.size	current_alloc_offset, 4
current_alloc_offset:
.zero	4
.type	stop_time_val, @object
.size	stop_time_val, 4
stop_time_val:
.zero	4
.type	start_time_val, @object
.size	start_time_val, 4
start_time_val:
.zero	4
.type	seed5_volatile, @object
.size	seed5_volatile, 4
seed5_volatile:
.zero	4
.type	seed3_volatile, @object
.size	seed3_volatile, 4
seed3_volatile:
.zero	4
.type	seed2_volatile, @object
.size	seed2_volatile, 4
seed2_volatile:
.zero	4
.type	seed1_volatile, @object
.size	seed1_volatile, 4
seed1_volatile:
.zero	4
.section	.sdata,"aw"
.align	2
.type	default_num_contexts, @object
.size	default_num_contexts, 4
default_num_contexts:
.word	1
.type	seed4_volatile, @object
.size	seed4_volatile, 4
seed4_volatile:
.word	500
.section	.tohost,"aw"
.align	6
.set	.LANCHOR_portme_0,. + 0
.type	tohost, @object
.size	tohost, 8
tohost:
.zero	8
.zero	56
.type	tohost_ready, @object
.size	tohost_ready, 8
tohost_ready:
.zero	8
.zero	56
.type	fromhost_ready, @object
.size	fromhost_ready, 8
fromhost_ready:
.zero	8
.zero	56
.type	fromhost, @object
.size	fromhost, 8
fromhost:
.zero	8
.globl	__extendsfdf2
.globl	__ltdf2
.globl	__muldf3
.globl	__subdf3
.globl	__floatsidf
.globl	__fixdfsi
.globl	__adddf3
.globl	__gtdf2
.ident	"GCC: (Alpine Linux) 15.2.0"
.section	.note.GNU-stack,"",@progbits
.file	"core_list_join.c"
.option nopic
.text
.align	2
.globl	calc_func
.type	calc_func, @function
calc_func:
  lh	a6,0(a0)
  srai	a5,a6,7
  andi	a5,a5,1
  beq	a5,zero,.L_list_2
  andi	a0,a6,127
  ret
.L_list_2:
  srai	a4,a6,3
  andi	a4,a4,15
  addi	sp,sp,-32
  slli	a5,a4,4
  add	a2,a4,a5
  sw	s0,24(sp)
  sw	ra,28(sp)
  sw	s1,20(sp)
  andi	a3,a6,7
  lhu	a5,56(a1)
  mv	a7,a1
  mv	s0,a0
  mv	a4,a2
  beq	a3,zero,.L_list_4
  li	a2,1
  bne	a3,a2,.L_list_20
  mv	a2,a5
  addi	a0,a7,40
  mv	a1,a4
  sw	a6,12(sp)
  sw	a7,8(sp)
  call	core_bench_matrix
  lw	a7,8(sp)
  lw	a6,12(sp)
  mv	s1,a0
  lhu	a5,60(a7)
  bne	a5,zero,.L_list_18
  lhu	a5,56(a7)
  sh	a0,60(a7)
.L_list_6:
  mv	a1,a5
  sw	a7,12(sp)
  sw	a6,8(sp)
  call	crcu16
  lw	a6,8(sp)
  lw	a7,12(sp)
  mv	a5,a0
  andi	a6,a6,-256
  andi	a0,s1,127
  or	a6,a0,a6
  sh	a5,56(a7)
  ori	a6,a6,128
  lw	ra,28(sp)
  sh	a6,0(s0)
  lw	s0,24(sp)
  lw	s1,20(sp)
  addi	sp,sp,32
  jr	ra
.L_list_20:
  zext.h	a0,a6
  mv	s1,a6
  j	.L_list_6
.L_list_4:
  li	a3,33
  bgtu	a2,a3,.L_list_7
  li	a4,34
.L_list_7:
  lh	a3,2(a7)
  lh	a2,0(a7)
  lw	a1,20(a7)
  lw	a0,24(a7)
  sw	a6,12(sp)
  sw	a7,8(sp)
  call	core_bench_state
  lw	a7,8(sp)
  lw	a6,12(sp)
  mv	s1,a0
  lhu	a5,62(a7)
  bne	a5,zero,.L_list_18
  sh	a0,62(a7)
.L_list_18:
  lhu	a5,56(a7)
  j	.L_list_6
.size	calc_func, .-calc_func
.align	2
.globl	cmp_complex
.type	cmp_complex, @function
cmp_complex:
  lh	a6,0(a0)
  addi	sp,sp,-32
  sw	s0,24(sp)
  srai	a5,a6,7
  sw	s1,20(sp)
  sw	ra,28(sp)
  andi	a5,a5,1
  mv	s1,a1
  mv	s0,a2
  andi	a7,a6,127
  bne	a5,zero,.L_list_23
  srai	a4,a6,3
  andi	a4,a4,15
  slli	a5,a4,4
  add	a2,a4,a5
  sw	s2,16(sp)
  andi	a3,a6,7
  lhu	a5,56(s0)
  mv	t1,a0
  mv	a4,a2
  beq	a3,zero,.L_list_24
  li	a2,1
  bne	a3,a2,.L_list_52
  sw	a0,12(sp)
  mv	a2,a5
  mv	a1,a4
  addi	a0,s0,40
  sw	a6,8(sp)
  call	core_bench_matrix
  lhu	a5,60(s0)
  lw	a6,8(sp)
  lw	t1,12(sp)
  mv	s2,a0
  bne	a5,zero,.L_list_50
  lhu	a5,56(s0)
  sh	a0,60(s0)
.L_list_26:
  mv	a1,a5
  sw	t1,12(sp)
  sw	a6,8(sp)
  call	crcu16
  lw	a6,8(sp)
  andi	a7,s2,127
  lw	t1,12(sp)
  andi	a6,a6,-256
  or	a6,a7,a6
  lw	s2,16(sp)
  ori	a6,a6,128
  sh	a0,56(s0)
  sh	a6,0(t1)
.L_list_23:
  lh	a6,0(s1)
  srai	a5,a6,7
  andi	a5,a5,1
  beq	a5,zero,.L_list_30
  lw	ra,28(sp)
  lw	s0,24(sp)
  andi	a5,a6,127
  lw	s1,20(sp)
  sub	a0,a7,a5
  addi	sp,sp,32
  jr	ra
.L_list_30:
  srai	a4,a6,3
  andi	a4,a4,15
  slli	a5,a4,4
  add	a2,a4,a5
  sw	s2,16(sp)
  andi	a3,a6,7
  lhu	a5,56(s0)
  mv	a4,a2
  beq	a3,zero,.L_list_32
  li	a2,1
  bne	a3,a2,.L_list_53
  mv	a2,a5
  mv	a1,a4
  addi	a0,s0,40
  sw	a7,12(sp)
  sw	a6,8(sp)
  call	core_bench_matrix
  lhu	a5,60(s0)
  lw	a6,8(sp)
  lw	a7,12(sp)
  mv	s2,a0
  bne	a5,zero,.L_list_51
  lhu	a5,56(s0)
  sh	a0,60(s0)
.L_list_34:
  mv	a1,a5
  sw	a7,12(sp)
  sw	a6,8(sp)
  call	crcu16
  lw	a6,8(sp)
  andi	a5,s2,127
  sh	a0,56(s0)
  andi	a6,a6,-256
  lw	a7,12(sp)
  lw	ra,28(sp)
  lw	s0,24(sp)
  or	a6,a5,a6
  ori	a6,a6,128
  sh	a6,0(s1)
  lw	s2,16(sp)
  lw	s1,20(sp)
  sub	a0,a7,a5
  addi	sp,sp,32
  jr	ra
.L_list_52:
  zext.h	a0,a6
  mv	s2,a6
  j	.L_list_26
.L_list_53:
  zext.h	a0,a6
  mv	s2,a6
  j	.L_list_34
.L_list_32:
  li	a3,33
  bgtu	a2,a3,.L_list_35
  li	a4,34
.L_list_35:
  lh	a3,2(s0)
  lh	a2,0(s0)
  lw	a1,20(s0)
  lw	a0,24(s0)
  sw	a7,12(sp)
  sw	a6,8(sp)
  call	core_bench_state
  lhu	a5,62(s0)
  lw	a6,8(sp)
  lw	a7,12(sp)
  mv	s2,a0
  bne	a5,zero,.L_list_51
  sh	a0,62(s0)
.L_list_51:
  lhu	a5,56(s0)
  j	.L_list_34
.L_list_24:
  li	a3,33
  bgtu	a2,a3,.L_list_27
  li	a4,34
.L_list_27:
  lh	a3,2(s0)
  lh	a2,0(s0)
  lw	a1,20(s0)
  lw	a0,24(s0)
  sw	t1,12(sp)
  sw	a6,8(sp)
  call	core_bench_state
  lhu	a5,62(s0)
  lw	a6,8(sp)
  lw	t1,12(sp)
  mv	s2,a0
  bne	a5,zero,.L_list_50
  sh	a0,62(s0)
.L_list_50:
  lhu	a5,56(s0)
  j	.L_list_26
.size	cmp_complex, .-cmp_complex
.align	2
.globl	cmp_idx
.type	cmp_idx, @function
cmp_idx:
  beq	a2,zero,.L_list_56
  lh	a0,2(a0)
  lh	a5,2(a1)
  sub	a0,a0,a5
  ret
.L_list_56:
  lh	a5,0(a0)
  zext.h	a4,a5
  srli	a4,a4,8
  andi	a5,a5,-256
  or	a5,a5,a4
  sh	a5,0(a0)
  lh	a5,0(a1)
  lh	a0,2(a0)
  zext.h	a4,a5
  srli	a4,a4,8
  andi	a5,a5,-256
  or	a5,a5,a4
  sh	a5,0(a1)
  lh	a5,2(a1)
  sub	a0,a0,a5
  ret
.size	cmp_idx, .-cmp_idx
.align	2
.globl	copy_info
.type	copy_info, @function
copy_info:
  vsetivli	zero,2,e16,m1,ta,ma
  vle16.v	v1,0(a1)
  vse16.v	v1,0(a0)
  ret
.size	copy_info, .-copy_info
.align	2
.globl	core_bench_list
.type	core_bench_list, @function
core_bench_list:
  lh	a7,4(a0)
  addi	sp,sp,-64
  sw	s0,56(sp)
  sw	s6,32(sp)
  sw	ra,60(sp)
  sw	s1,52(sp)
  sw	s2,48(sp)
  sw	s3,44(sp)
  sw	s9,20(sp)
  sw	s10,16(sp)
  lw	s0,36(a0)
  mv	s6,a0
  ble	a7,zero,.L_list_149
  mv	s9,a1
  li	a2,0
  li	a6,0
  li	a0,0
  li	s1,0
.L_list_76:
  andi	s10,a2,0xff
  blt	s9,zero,.L_list_234
  mv	a5,s0
  bne	s0,zero,.L_list_63
  j	.L_list_235
.L_list_67:
  lw	a5,0(a5)
  beq	a5,zero,.L_list_66
.L_list_63:
  lw	a4,4(a5)
  lh	a4,2(a4)
  bne	s9,a4,.L_list_67
.L_list_66:
  li	a3,0
  j	.L_list_69
.L_list_152:
  mv	s0,a4
.L_list_69:
  lw	a4,0(s0)
  sw	a3,0(s0)
  mv	a3,s0
  bne	a4,zero,.L_list_152
  beq	a5,zero,.L_list_236
  lw	a4,4(a5)
  addi	a0,a0,1
  lh	a4,0(a4)
  andi	a3,a4,1
  beq	a3,zero,.L_list_72
  srai	a4,a4,9
  andi	a4,a4,1
  add	s1,s1,a4
.L_list_72:
  lw	a4,0(a5)
  beq	a4,zero,.L_list_71
  lw	a3,0(a4)
  sw	a3,0(a5)
  lw	a5,0(s0)
  sw	a5,0(a4)
  sw	a4,0(s0)
.L_list_71:
  blt	s9,zero,.L_list_65
.L_list_64:
  addi	a2,a2,1
  addi	s9,s9,1
  sext.h	a2,a2
  sext.h	s9,s9
  bne	a7,a2,.L_list_76
.L_list_75:
  slli	a5,a0,2
  sub	a5,a5,a6
  add	s1,s1,a5
  zext.h	s1,s1
.L_list_59:
  bgt	a1,zero,.L_list_237
.L_list_86:
  lw	a5,0(s0)
  mv	s2,s0
  lw	s3,0(a5)
  lw	a3,4(a5)
  lw	a2,4(s3)
  lw	a4,0(s3)
  sw	a2,4(a5)
  sw	a3,4(s3)
  sw	a4,0(a5)
  sw	zero,0(s3)
  blt	s9,zero,.L_list_238
  bne	s0,zero,.L_list_115
  j	.L_list_239
.L_list_117:
  lw	s2,0(s2)
  beq	s2,zero,.L_list_116
.L_list_115:
  lw	a5,4(s2)
  lh	a5,2(a5)
  bne	a5,s9,.L_list_117
.L_list_120:
  lw	a5,4(s0)
  mv	a1,s1
  lh	a0,0(a5)
  call	crc16
  lw	s2,0(s2)
  mv	s1,a0
  bne	s2,zero,.L_list_120
.L_list_256:
  lw	s2,0(s0)
.L_list_109:
  lw	a3,4(s2)
  lw	a4,4(s3)
  lw	a5,0(s2)
  sw	a3,4(s3)
  sw	a4,4(s2)
  sw	a5,0(s3)
  li	t4,1
  sw	s3,0(s2)
  mv	t6,t4
  beq	s0,zero,.L_list_121
.L_list_245:
  li	s2,0
  li	a4,0
  li	t5,0
.L_list_141:
  addi	t5,t5,1
  mv	a3,s0
  li	a5,0
.L_list_122:
  bge	a5,t4,.L_list_125
  lw	a3,0(a3)
  addi	a5,a5,1
  bne	a3,zero,.L_list_122
.L_list_125:
  mv	a2,s0
  mv	a1,t4
  mv	s0,a3
.L_list_123:
  ble	a5,zero,.L_list_133
.L_list_135:
  beq	a1,zero,.L_list_128
  bne	s0,zero,.L_list_126
.L_list_128:
  mv	a3,a4
  addi	a5,a5,-1
  mv	a4,a2
  lw	a2,0(a2)
  beq	a3,zero,.L_list_240
.L_list_132:
  sw	a4,0(a3)
.L_list_247:
  bgt	a5,zero,.L_list_135
.L_list_133:
  snez	a3,s0
  ble	a1,zero,.L_list_138
  beq	a3,zero,.L_list_229
  beq	a5,zero,.L_list_167
.L_list_126:
  lw	a7,4(s0)
.L_list_129:
  lw	a0,4(a2)
  lh	t1,2(a7)
  lh	a3,0(a0)
  lh	t3,2(a0)
  zext.h	a6,a3
  srli	a6,a6,8
  andi	a3,a3,-256
  or	a3,a3,a6
  sh	a3,0(a0)
  lh	a3,0(a7)
  zext.h	a0,a3
  srli	a0,a0,8
  andi	a3,a3,-256
  or	a3,a3,a0
  sh	a3,0(a7)
  ble	t3,t1,.L_list_241
  mv	a3,a4
  addi	a1,a1,-1
  mv	a4,s0
  lw	s0,0(s0)
  bne	a3,zero,.L_list_132
.L_list_240:
  mv	s2,a4
  j	.L_list_123
.L_list_242:
  sw	a5,0(a4)
  snez	a3,s0
  mv	a4,a5
  beq	a1,zero,.L_list_169
.L_list_243:
  beq	a3,zero,.L_list_169
.L_list_167:
  mv	a5,s0
  addi	a1,a1,-1
  lw	s0,0(s0)
  bne	a4,zero,.L_list_242
  mv	s2,a5
  snez	a3,s0
  mv	a4,a5
  bne	a1,zero,.L_list_243
.L_list_169:
  mv	a4,a5
.L_list_138:
  bne	a3,zero,.L_list_141
.L_list_229:
  sw	zero,0(a4)
  beq	t5,t6,.L_list_244
  mv	s0,s2
  slli	t4,t4,1
  bne	s0,zero,.L_list_245
.L_list_121:
  sw	zero,0(zero)
.L_list_231:
  lw	ra,60(sp)
  lw	s0,56(sp)
  lw	s2,48(sp)
  lw	s3,44(sp)
  lw	s6,32(sp)
  lw	s9,20(sp)
  lw	s10,16(sp)
  mv	a0,s1
  lw	s1,52(sp)
  addi	sp,sp,64
  jr	ra
.L_list_241:
  lw	a0,0(a2)
  addi	a5,a5,-1
  beq	a4,zero,.L_list_246
  mv	a3,a4
  mv	a4,a2
  sw	a4,0(a3)
  mv	a2,a0
  j	.L_list_247
.L_list_246:
  mv	a4,a2
  mv	s2,a2
  mv	a2,a0
  bgt	a5,zero,.L_list_129
  j	.L_list_133
.L_list_249:
  lw	a5,0(zero)
  addi	a6,a6,1
  lw	a5,4(a5)
  lb	a5,1(a5)
  andi	a5,a5,1
  add	s1,s1,a5
.L_list_65:
  addi	a2,a2,1
  sext.h	a2,a2
  beq	a7,a2,.L_list_75
.L_list_74:
  andi	s10,a2,0xff
  beq	s0,zero,.L_list_77
.L_list_82:
  mv	a4,s0
  j	.L_list_79
.L_list_248:
  lw	a4,0(a4)
  beq	a4,zero,.L_list_78
.L_list_79:
  lw	a5,4(a4)
  lbu	a5,0(a5)
  bne	a5,s10,.L_list_248
.L_list_78:
  li	a3,0
  j	.L_list_84
.L_list_153:
  mv	s0,a5
.L_list_84:
  lw	a5,0(s0)
  sw	a3,0(s0)
  mv	a3,s0
  bne	a5,zero,.L_list_153
  beq	a4,zero,.L_list_77
  lw	a5,4(a4)
  addi	a0,a0,1
  lh	a5,0(a5)
  andi	a3,a5,1
  beq	a3,zero,.L_list_80
  srai	a5,a5,9
  andi	a5,a5,1
  add	s1,s1,a5
.L_list_80:
  lw	a5,0(a4)
  beq	a5,zero,.L_list_83
  lw	a3,0(a5)
  sw	a3,0(a4)
  lw	a4,0(s0)
  sw	a4,0(a5)
  sw	a5,0(s0)
.L_list_83:
  addi	a5,a2,1
  sext.h	a2,a5
  ble	a7,a2,.L_list_75
  andi	s10,a5,0xff
  j	.L_list_82
.L_list_77:
  lw	a5,0(s0)
  addi	a2,a2,1
  sext.h	a2,a2
  lw	a5,4(a5)
  addi	a6,a6,1
  lb	a5,1(a5)
  andi	a5,a5,1
  add	s1,s1,a5
  bgt	a7,a2,.L_list_74
  slli	a5,a0,2
  sub	a5,a5,a6
  add	s1,s1,a5
  zext.h	s1,s1
  j	.L_list_59
.L_list_234:
  beq	s0,zero,.L_list_249
  mv	a5,s0
  j	.L_list_61
.L_list_68:
  lw	a5,0(a5)
  beq	a5,zero,.L_list_66
.L_list_61:
  lw	a4,4(a5)
  lbu	a4,0(a4)
  bne	s10,a4,.L_list_68
  j	.L_list_66
.L_list_236:
  lw	a5,0(s0)
  addi	a6,a6,1
  lw	a5,4(a5)
  lb	a5,1(a5)
  andi	a5,a5,1
  add	s1,s1,a5
  j	.L_list_71
.L_list_244:
  lw	s0,0(s2)
  beq	s0,zero,.L_list_231
.L_list_145:
  lw	a5,4(s2)
  mv	a1,s1
  lh	a0,0(a5)
  call	crc16
  lw	s0,0(s0)
  mv	s1,a0
  bne	s0,zero,.L_list_145
  j	.L_list_231
.L_list_237:
  sw	s5,36(sp)
  sw	s4,40(sp)
  sw	s7,28(sp)
  sw	s8,24(sp)
  sw	s11,12(sp)
  li	s5,1
  beq	s0,zero,.L_list_87
.L_list_254:
  li	s7,0
  li	s3,0
  li	s8,0
.L_list_107:
  addi	s8,s8,1
  mv	a5,s0
  li	s2,0
.L_list_88:
  bge	s2,s5,.L_list_91
  lw	a5,0(a5)
  addi	s2,s2,1
  bne	a5,zero,.L_list_88
.L_list_91:
  mv	s4,s0
  mv	s11,s5
  mv	s0,a5
.L_list_89:
  ble	s2,zero,.L_list_99
.L_list_101:
  beq	s11,zero,.L_list_94
  bne	s0,zero,.L_list_95
.L_list_94:
  mv	a5,s3
  addi	s2,s2,-1
  mv	s3,s4
  lw	s4,0(s4)
  beq	a5,zero,.L_list_250
.L_list_98:
  sw	s3,0(a5)
.L_list_258:
  bgt	s2,zero,.L_list_101
.L_list_99:
  snez	a5,s0
  ble	s11,zero,.L_list_104
  beq	a5,zero,.L_list_228
  beq	s2,zero,.L_list_158
.L_list_95:
  lw	a1,4(s0)
  lw	a0,4(s4)
  mv	a2,s6
  call	cmp_complex
  ble	a0,zero,.L_list_251
  mv	a5,s3
  addi	s11,s11,-1
  mv	s3,s0
  lw	s0,0(s0)
  bne	a5,zero,.L_list_98
.L_list_250:
  mv	s7,s3
  j	.L_list_89
.L_list_252:
  sw	a3,0(s3)
  snez	a5,s0
  mv	s3,a3
  beq	s11,zero,.L_list_168
.L_list_253:
  beq	a5,zero,.L_list_168
.L_list_158:
  mv	a3,s0
  addi	s11,s11,-1
  lw	s0,0(s0)
  bne	s3,zero,.L_list_252
  mv	s7,a3
  snez	a5,s0
  mv	s3,a3
  bne	s11,zero,.L_list_253
.L_list_168:
  mv	s3,a3
.L_list_104:
  bne	a5,zero,.L_list_107
.L_list_228:
  sw	zero,0(s3)
  li	a5,1
  beq	s8,a5,.L_list_160
  mv	s0,s7
  slli	s5,s5,1
  bne	s0,zero,.L_list_254
.L_list_87:
  sw	zero,0(zero)
  bge	s9,zero,.L_list_255
  lw	s4,40(sp)
  lw	s5,36(sp)
  lw	s7,28(sp)
  lw	s8,24(sp)
  lw	s11,12(sp)
.L_list_110:
  lw	s2,0(zero)
  mv	s3,s0
  li	s0,0
  beq	s2,zero,.L_list_109
.L_list_259:
  lw	a5,4(s0)
  mv	a1,s1
  lh	a0,0(a5)
  call	crc16
  lw	s2,0(s2)
  mv	s1,a0
  bne	s2,zero,.L_list_120
  j	.L_list_256
.L_list_251:
  lw	a3,0(s4)
  addi	s2,s2,-1
  beq	s3,zero,.L_list_257
  mv	a5,s3
  mv	s3,s4
  sw	s3,0(a5)
  mv	s4,a3
  j	.L_list_258
.L_list_257:
  mv	s3,s4
  mv	s7,s4
  mv	s4,a3
  bgt	s2,zero,.L_list_95
  j	.L_list_99
.L_list_116:
  lw	s2,0(s0)
  bne	s2,zero,.L_list_259
  j	.L_list_109
.L_list_235:
  lw	a5,0(zero)
  addi	a6,a6,1
  lw	a5,4(a5)
  lb	a5,1(a5)
  andi	a5,a5,1
  add	s1,s1,a5
  j	.L_list_64
.L_list_149:
  li	s10,0
  mv	s9,a1
  li	s1,0
  j	.L_list_59
.L_list_239:
  lw	s2,0(zero)
  bne	s2,zero,.L_list_259
  j	.L_list_109
.L_list_238:
  bne	s0,zero,.L_list_113
  j	.L_list_260
.L_list_119:
  lw	s2,0(s2)
  beq	s2,zero,.L_list_116
.L_list_113:
  lw	a5,4(s2)
  lbu	a5,0(a5)
  bne	s10,a5,.L_list_119
  lw	a5,4(s0)
  mv	a1,s1
  lh	a0,0(a5)
  call	crc16
  lw	s2,0(s2)
  mv	s1,a0
  bne	s2,zero,.L_list_120
  j	.L_list_256
.L_list_160:
  mv	s0,s7
  lw	s4,40(sp)
  lw	s5,36(sp)
  lw	s7,28(sp)
  lw	s8,24(sp)
  lw	s11,12(sp)
  j	.L_list_86
.L_list_255:
  lw	s4,40(sp)
  lw	s5,36(sp)
  lw	s7,28(sp)
  lw	s8,24(sp)
  lw	s11,12(sp)
  li	s2,0
  li	s3,0
  j	.L_list_109
.L_list_260:
  mv	s0,s3
  j	.L_list_110
.size	core_bench_list, .-core_bench_list
.align	2
.globl	core_list_init
.type	core_list_init, @function
core_list_init:
  li	a5,-858992640
  addi	a5,a5,-819
  mulhu	a5,a0,a5
  vsetivli	zero,2,e16,m1,ta,ma
  li	t1,-32768
  vid.v	v1
  xori	a4,t1,-128
  vmul.vx	v2,v1,a4
  addi	a4,t1,128
  sw	zero,0(a1)
  mv	a0,a1
  vadd.vx	v2,v2,a4
  srli	a5,a5,4
  addi	a4,a5,-2
  slli	a7,a4,3
  add	a7,a1,a7
  sw	a7,4(a1)
  vse16.v	v2,0(a7)
  slli	t4,a4,2
  addi	a1,a1,16
  add	t4,a7,t4
  addi	a5,a7,4
  addi	a6,a0,8
  bleu	a7,a1,.L_list_299
  addi	a3,a7,8
  bleu	t4,a3,.L_list_299
  vmul.vx	v2,v1,t1
  sw	zero,8(a0)
  sw	a6,0(a0)
  sw	a5,12(a0)
  vadd.vi	v2,v2,-1
  vse16.v	v2,0(a5)
.L_list_262:
  beq	a4,zero,.L_list_263
  li	a5,-113
  vmul.vx	v1,v1,a5
  li	a6,0
  li	a5,120
  zext.h	t1,a6
  vadd.vx	v2,v1,a5
  xor	a5,a2,t1
  slli	a5,a5,3
  vmv.v.x	v1,a5
  li	t5,-32768
  addi	a5,a1,8
  vslide1down.vx	v1,v1,t1
  vmv.s.x	v3,zero
  xori	t5,t5,-1
  addi	t3,a3,4
  bleu	a7,a5,.L_list_264
.L_list_330:
  bleu	t4,t3,.L_list_329
  vand.vv	v1,v1,v2
  lw	t1,0(a0)
  addi	a6,a6,1
  vredor.vs	v1,v1,v3
  sw	t1,0(a1)
  sw	a1,0(a0)
  sw	a3,4(a1)
  sh	t5,2(a3)
  vmv.x.s	t1,v1
  slli	a1,t1,8
  add	a1,a1,t1
  sh	a1,0(a3)
  beq	a4,a6,.L_list_264
.L_list_266:
  zext.h	t1,a6
  mv	a1,a5
  xor	a5,a2,t1
  slli	a5,a5,3
  vmv.v.x	v1,a5
  mv	a3,t3
  addi	a5,a1,8
  vslide1down.vx	v1,v1,t1
  addi	t3,a3,4
  bgtu	a7,a5,.L_list_330
.L_list_264:
  lw	a6,0(a0)
.L_list_263:
  lw	a7,0(a6)
  beq	a7,zero,.L_list_271
  li	a5,-858992640
  addi	a5,a5,-819
  mulhu	a4,a4,a5
  li	t3,16384
  addi	t3,t3,-1
  li	a1,512
  li	a3,1
  srli	a4,a4,2
  j	.L_list_273
.L_list_331:
  sh	a3,2(a6)
  addi	a1,a1,256
  beq	t1,zero,.L_list_271
  addi	a3,a3,1
.L_list_272:
  mv	a6,a7
  mv	a7,t1
.L_list_273:
  xor	t1,a3,a2
  andi	a5,a1,1792
  or	a5,a5,t1
  lw	a6,4(a6)
  lw	t1,0(a7)
  and	a5,a5,t3
  bgtu	a4,a3,.L_list_331
  sh	a5,2(a6)
  addi	a3,a3,1
  beq	t1,zero,.L_list_271
  addi	a1,a1,256
  j	.L_list_272
.L_list_271:
  li	t5,1
  mv	t2,t5
  beq	a0,zero,.L_list_274
.L_list_337:
  li	t6,0
  li	a2,0
  li	t0,0
.L_list_294:
  addi	t0,t0,1
  mv	a5,a0
  li	a4,0
.L_list_275:
  bge	a4,t5,.L_list_278
  lw	a5,0(a5)
  addi	a4,a4,1
  bne	a5,zero,.L_list_275
.L_list_278:
  mv	a3,a0
  mv	a1,t5
  mv	a0,a5
  ble	a4,zero,.L_list_286
.L_list_288:
  beq	a1,zero,.L_list_281
  bne	a0,zero,.L_list_279
.L_list_281:
  lw	a5,0(a3)
  addi	a4,a4,-1
  beq	a2,zero,.L_list_303
.L_list_285:
  sw	a3,0(a2)
.L_list_287:
  mv	a2,a3
  mv	a3,a5
  bgt	a4,zero,.L_list_288
.L_list_286:
  snez	a5,a0
  ble	a1,zero,.L_list_291
  beq	a5,zero,.L_list_322
  beq	a4,zero,.L_list_332
.L_list_279:
  lw	t1,4(a0)
.L_list_282:
  lw	a6,4(a3)
  lh	t3,2(t1)
  lh	a5,0(a6)
  lh	t4,2(a6)
  zext.h	a7,a5
  srli	a7,a7,8
  andi	a5,a5,-256
  or	a5,a5,a7
  sh	a5,0(a6)
  lh	a5,0(t1)
  zext.h	a6,a5
  srli	a6,a6,8
  andi	a5,a5,-256
  or	a5,a5,a6
  sh	a5,0(t1)
  ble	t4,t3,.L_list_333
  mv	a5,a3
  addi	a1,a1,-1
  mv	a3,a0
  lw	a0,0(a0)
  bne	a2,zero,.L_list_285
.L_list_303:
  mv	t6,a3
  j	.L_list_287
.L_list_332:
  mv	a4,a2
  j	.L_list_292
.L_list_334:
  sw	a2,0(a4)
  snez	a5,a0
  mv	a4,a2
  beq	a1,zero,.L_list_291
.L_list_335:
  beq	a5,zero,.L_list_322
.L_list_292:
  mv	a2,a0
  addi	a1,a1,-1
  lw	a0,0(a0)
  bne	a4,zero,.L_list_334
  mv	t6,a2
  mv	a4,a2
  snez	a5,a0
  bne	a1,zero,.L_list_335
.L_list_291:
  bne	a5,zero,.L_list_294
.L_list_322:
  sw	zero,0(a2)
  beq	t0,t2,.L_list_336
  mv	a0,t6
  slli	t5,t5,1
  bne	a0,zero,.L_list_337
.L_list_274:
  sw	zero,0(zero)
  ret
.L_list_333:
  lw	a5,0(a3)
  addi	a4,a4,-1
  bne	a2,zero,.L_list_285
  mv	a2,a3
  mv	t6,a3
  mv	a3,a5
  bgt	a4,zero,.L_list_282
  j	.L_list_286
.L_list_329:
  addi	a6,a6,1
  beq	a4,a6,.L_list_264
  mv	t3,a3
  mv	a5,a1
  j	.L_list_266
.L_list_299:
  mv	a1,a6
  mv	a3,a5
  li	a6,0
  j	.L_list_262
.L_list_336:
  mv	a0,t6
  ret
.size	core_list_init, .-core_list_init
.align	2
.globl	core_list_insert_new
.type	core_list_insert_new, @function
core_list_insert_new:
  lw	a6,0(a2)
  addi	a7,a6,8
  bgeu	a7,a4,.L_list_341
  lw	a4,0(a3)
  addi	t1,a4,4
  bleu	a5,t1,.L_list_341
  sw	a7,0(a2)
  lw	a5,0(a0)
  vsetivli	zero,2,e16,m1,ta,ma
  vle16.v	v1,0(a1)
  sw	a5,0(a6)
  sw	a6,0(a0)
  sw	a4,4(a6)
  lw	a5,0(a3)
  mv	a0,a6
  addi	a5,a5,4
  sw	a5,0(a3)
  lw	a5,4(a6)
  vse16.v	v1,0(a5)
  ret
.L_list_341:
  li	a6,0
  mv	a0,a6
  ret
.size	core_list_insert_new, .-core_list_insert_new
.align	2
.globl	core_list_remove
.type	core_list_remove, @function
core_list_remove:
  mv	a5,a0
  lw	a0,0(a0)
  lw	a3,4(a5)
  lw	a2,4(a0)
  lw	a4,0(a0)
  sw	a2,4(a5)
  sw	a3,4(a0)
  sw	a4,0(a5)
  sw	zero,0(a0)
  ret
.size	core_list_remove, .-core_list_remove
.align	2
.globl	core_list_undo_remove
.type	core_list_undo_remove, @function
core_list_undo_remove:
  lw	a3,4(a1)
  lw	a4,4(a0)
  lw	a5,0(a1)
  sw	a3,4(a0)
  sw	a4,4(a1)
  sw	a5,0(a0)
  sw	a0,0(a1)
  ret
.size	core_list_undo_remove, .-core_list_undo_remove
.align	2
.globl	core_list_find
.type	core_list_find, @function
core_list_find:
  lh	a4,2(a1)
  blt	a4,zero,.L_list_359
  bne	a0,zero,.L_list_349
  ret
.L_list_350:
  lw	a0,0(a0)
  beq	a0,zero,.L_list_358
.L_list_349:
  lw	a5,4(a0)
  lh	a5,2(a5)
  bne	a5,a4,.L_list_350
  ret
.L_list_359:
  beq	a0,zero,.L_list_358
  lh	a4,0(a1)
  j	.L_list_348
.L_list_351:
  lw	a0,0(a0)
  beq	a0,zero,.L_list_358
.L_list_348:
  lw	a5,4(a0)
  lbu	a5,0(a5)
  bne	a5,a4,.L_list_351
.L_list_358:
  ret
.size	core_list_find, .-core_list_find
.align	2
.globl	core_list_reverse
.type	core_list_reverse, @function
core_list_reverse:
  beq	a0,zero,.L_list_367
  li	a4,0
  j	.L_list_362
.L_list_363:
  mv	a0,a5
.L_list_362:
  lw	a5,0(a0)
  sw	a4,0(a0)
  mv	a4,a0
  bne	a5,zero,.L_list_363
.L_list_367:
  ret
.size	core_list_reverse, .-core_list_reverse
.align	2
.globl	core_list_mergesort
.type	core_list_mergesort, @function
core_list_mergesort:
  addi	sp,sp,-48
  sw	s2,32(sp)
  sw	s5,20(sp)
  sw	s6,16(sp)
  sw	s7,12(sp)
  sw	s10,0(sp)
  li	s5,1
  sw	ra,44(sp)
  sw	s0,40(sp)
  sw	s1,36(sp)
  sw	s3,28(sp)
  sw	s4,24(sp)
  sw	s8,8(sp)
  sw	s9,4(sp)
  mv	s2,a0
  mv	s7,a1
  mv	s6,a2
  mv	s10,s5
  beq	s2,zero,.L_list_369
.L_list_417:
  li	s9,0
  li	s1,0
  li	s8,0
.L_list_389:
  addi	s9,s9,1
  mv	a5,s2
  li	s0,0
.L_list_370:
  ble	s5,s0,.L_list_373
  lw	a5,0(a5)
  addi	s0,s0,1
  bne	a5,zero,.L_list_370
.L_list_373:
  mv	s3,s2
  mv	s4,s5
  mv	s2,a5
.L_list_371:
  ble	s0,zero,.L_list_381
.L_list_383:
  beq	s4,zero,.L_list_376
  bne	s2,zero,.L_list_377
.L_list_376:
  mv	a5,s1
  addi	s0,s0,-1
  mv	s1,s3
  lw	s3,0(s3)
  beq	a5,zero,.L_list_412
.L_list_380:
  sw	s1,0(a5)
.L_list_419:
  bgt	s0,zero,.L_list_383
.L_list_381:
  snez	a5,s2
  ble	s4,zero,.L_list_386
  beq	a5,zero,.L_list_408
  beq	s0,zero,.L_list_398
.L_list_377:
  lw	a1,4(s2)
  lw	a0,4(s3)
  mv	a2,s6
  jalr	s7
  ble	a0,zero,.L_list_413
  mv	a5,s1
  addi	s4,s4,-1
  mv	s1,s2
  lw	s2,0(s2)
  bne	a5,zero,.L_list_380
.L_list_412:
  mv	s8,s1
  j	.L_list_371
.L_list_414:
  sw	a4,0(s1)
  snez	a5,s2
  mv	s1,a4
  beq	s4,zero,.L_list_399
.L_list_415:
  beq	a5,zero,.L_list_399
.L_list_398:
  mv	a4,s2
  addi	s4,s4,-1
  lw	s2,0(s2)
  bne	s1,zero,.L_list_414
  mv	s8,a4
  snez	a5,s2
  mv	s1,a4
  bne	s4,zero,.L_list_415
.L_list_399:
  mv	s1,a4
.L_list_386:
  bne	a5,zero,.L_list_389
.L_list_408:
  sw	zero,0(s1)
  beq	s9,s10,.L_list_416
  mv	s2,s8
  slli	s5,s5,1
  bne	s2,zero,.L_list_417
.L_list_369:
  sw	zero,0(zero)
.L_list_410:
  lw	ra,44(sp)
  lw	s0,40(sp)
  lw	s1,36(sp)
  lw	s3,28(sp)
  lw	s4,24(sp)
  lw	s5,20(sp)
  lw	s6,16(sp)
  lw	s7,12(sp)
  lw	s8,8(sp)
  lw	s9,4(sp)
  lw	s10,0(sp)
  mv	a0,s2
  lw	s2,32(sp)
  addi	sp,sp,48
  jr	ra
.L_list_413:
  lw	a4,0(s3)
  addi	s0,s0,-1
  beq	s1,zero,.L_list_418
  mv	a5,s1
  mv	s1,s3
  sw	s1,0(a5)
  mv	s3,a4
  j	.L_list_419
.L_list_418:
  mv	s8,s3
  mv	s1,s3
  mv	s3,a4
  bgt	s0,zero,.L_list_377
  j	.L_list_381
.L_list_416:
  mv	s2,s8
  j	.L_list_410
.size	core_list_mergesort, .-core_list_mergesort
.ident	"GCC: (Alpine Linux) 15.2.0"
.section	.note.GNU-stack,"",@progbits
.file	"core_state.c"
.option nopic
.text
.align	2
.globl	core_init_state
.type	core_init_state, @function
core_init_state:
  addi	a7,a0,-1
  li	t4,1
  bleu	a7,t4,.L_state_17
  addi	sp,sp,-16
  add	a1,a1,t4
  lui	a6,%hi(.LANCHOR_state_0)
  csrr	t1,vlenb
  slli	t0,a1,27
  sw	s0,12(sp)
  sw	s1,8(sp)
  li	t3,7
  andi	a4,a1,7
  addi	a6,a6,%lo(.LANCHOR_state_0)
  addi	t1,t1,-2
  li	a3,0
  li	t5,4
  li	t6,44
  srli	a5,t0,30
  beq	a4,t3,.L_state_6
.L_state_38:
  bgtu	a4,t5,.L_state_7
  addi	a4,a4,-3
  slli	a5,a5,2
  zext.h	a4,a4
  add	a5,a6,a5
  bgtu	a4,t4,.L_state_36
  lw	a5,16(a5)
  li	t2,8
.L_state_9:
  addi	t0,a3,1
  add	s0,t2,t0
  bgeu	s0,a7,.L_state_10
.L_state_39:
  add	a3,a2,a3
  sub	a4,a3,a5
  addi	a4,a4,-1
  addi	a1,a1,1
  bleu	a4,t1,.L_state_37
  vsetvli	a4,t2,e8,m1,ta,ma
  vle8.v	v1,0(a5)
  sub	t0,t2,a4
  vse8.v	v1,0(a3)
  beq	t2,a4,.L_state_5
  add	a5,a5,a4
  vsetvli	zero,t0,e8,m1,ta,ma
  vle8.v	v1,0(a5)
  add	a4,a3,a4
  vse8.v	v1,0(a4)
.L_state_5:
  add	a3,a3,t2
  sb	t6,0(a3)
  slli	t0,a1,27
  andi	a4,a1,7
  mv	a3,s0
  srli	a5,t0,30
  bne	a4,t3,.L_state_38
.L_state_6:
  slli	a5,a5,2
  li	t2,8
  addi	t0,a3,1
  add	a5,a6,a5
  add	s0,t2,t0
  lw	a5,48(a5)
  bltu	s0,a7,.L_state_39
.L_state_10:
  bgeu	a3,a0,.L_state_24
  li	a4,1
  bgeu	a0,t0,.L_state_40
.L_state_14:
  vsetvli	a5,zero,e8,m1,ta,ma
  vmv.v.i	v1,0
  add	a5,a2,a3
.L_state_15:
  vsetvli	a3,a4,e8,m1,ta,ma
  vse8.v	v1,0(a5)
  sub	a4,a4,a3
  add	a5,a5,a3
  bne	a4,zero,.L_state_15
.L_state_24:
  lw	s0,12(sp)
  lw	s1,8(sp)
  addi	sp,sp,16
  jr	ra
.L_state_36:
  lw	a5,0(a5)
  li	t2,4
  j	.L_state_9
.L_state_7:
  slli	a5,a5,2
  add	a5,a6,a5
  lw	a5,32(a5)
  li	t2,8
  j	.L_state_9
.L_state_40:
  sub	a4,a0,a3
  j	.L_state_14
.L_state_37:
  lbu	s1,0(a5)
  add	t0,a2,t0
  li	a4,4
  sb	s1,0(a3)
  lbu	s1,1(a5)
  sb	s1,0(t0)
  lbu	t0,2(a5)
  sb	t0,2(a3)
  lbu	t0,3(a5)
  sb	t0,3(a3)
  beq	t2,a4,.L_state_5
  lbu	a4,4(a5)
  sb	a4,4(a3)
  lbu	a4,5(a5)
  sb	a4,5(a3)
  lbu	a4,6(a5)
  sb	a4,6(a3)
  lbu	a5,7(a5)
  sb	a5,7(a3)
  j	.L_state_5
.L_state_17:
  li	a3,0
  li	a4,1
  bltu	a0,t4,.L_state_27
  sub	a4,a0,a3
.L_state_27:
  vsetvli	a5,zero,e8,m1,ta,ma
  vmv.v.i	v1,0
  add	a5,a2,a3
.L_state_29:
  vsetvli	a3,a4,e8,m1,ta,ma
  vse8.v	v1,0(a5)
  sub	a4,a4,a3
  add	a5,a5,a3
  bne	a4,zero,.L_state_29
  ret
.size	core_init_state, .-core_init_state
.align	2
.globl	core_state_transition
.type	core_state_transition, @function
core_state_transition:
  lw	a4,0(a0)
  mv	a2,a0
  lbu	a5,0(a4)
  beq	a5,zero,.L_state_66
  li	a3,44
  li	a0,0
  beq	a5,a3,.L_state_48
  li	a6,46
  beq	a5,a6,.L_state_44
  bgtu	a5,a6,.L_state_45
  addi	a5,a5,-43
  andi	a5,a5,253
  beq	a5,zero,.L_state_91
.L_state_47:
  lw	a3,4(a1)
  lw	a5,0(a1)
  addi	a4,a4,1
  addi	a3,a3,1
  addi	a5,a5,1
  sw	a3,4(a1)
  sw	a5,0(a1)
  li	a0,1
.L_state_42:
  sw	a4,0(a2)
  ret
.L_state_45:
  addi	a5,a5,-48
  andi	a5,a5,0xff
  li	a0,9
  bgtu	a5,a0,.L_state_47
  lw	a0,0(a1)
  addi	a5,a4,1
  addi	a0,a0,1
  sw	a0,0(a1)
  lbu	a4,1(a4)
  beq	a4,zero,.L_state_73
.L_state_88:
  beq	a4,a3,.L_state_92
  li	a3,46
  beq	a4,a3,.L_state_93
  addi	a4,a4,-48
  andi	a4,a4,0xff
  li	a3,9
  bleu	a4,a3,.L_state_56
  lw	a3,16(a1)
  li	a0,1
  addi	a4,a5,1
  add	a5,a3,a0
  sw	a5,16(a1)
  sw	a4,0(a2)
  ret
.L_state_51:
  sw	a0,8(a1)
.L_state_56:
  lbu	a4,1(a5)
  addi	a5,a5,1
  beq	a4,zero,.L_state_73
  li	a3,44
  j	.L_state_88
.L_state_93:
  lw	a4,16(a1)
  addi	a4,a4,1
  sw	a4,16(a1)
.L_state_54:
  lbu	a4,1(a5)
  addi	a5,a5,1
  beq	a4,zero,.L_state_77
  li	a3,44
.L_state_90:
  beq	a4,a3,.L_state_94
  andi	a3,a4,223
  li	a0,69
  bne	a3,a0,.L_state_57
  lw	a3,20(a1)
  addi	a4,a5,1
  addi	a3,a3,1
  sw	a3,20(a1)
  lbu	a3,1(a5)
  beq	a3,zero,.L_state_75
  li	a0,44
  beq	a3,a0,.L_state_95
  lw	a4,12(a1)
  addi	a3,a3,-43
  andi	a3,a3,253
  addi	a4,a4,1
  sw	a4,12(a1)
  bne	a3,zero,.L_state_60
  lbu	a3,2(a5)
  addi	a4,a5,2
  beq	a3,zero,.L_state_79
  beq	a3,a0,.L_state_96
  lw	a0,24(a1)
  addi	a3,a3,-48
  andi	a3,a3,0xff
  addi	a0,a0,1
  li	a6,9
  sw	a0,24(a1)
  bleu	a3,a6,.L_state_63
  addi	a4,a5,3
  li	a0,1
  sw	a4,0(a2)
  ret
.L_state_63:
  mv	a0,a4
  lbu	a3,1(a4)
  addi	a4,a4,1
  li	a7,44
  addi	a5,a3,-48
  andi	a5,a5,0xff
  beq	a3,zero,.L_state_81
  beq	a3,a7,.L_state_97
  bleu	a5,a6,.L_state_63
  lw	a5,4(a1)
  addi	a4,a0,2
  li	a0,1
  add	a5,a5,a0
  sw	a5,4(a1)
  sw	a4,0(a2)
  ret
.L_state_60:
  addi	a4,a5,2
  li	a0,1
  j	.L_state_42
.L_state_57:
  addi	a4,a4,-48
  andi	a4,a4,0xff
  li	a3,9
  bleu	a4,a3,.L_state_54
  lw	a3,20(a1)
  li	a0,1
  addi	a4,a5,1
  add	a5,a3,a0
  sw	a5,20(a1)
  j	.L_state_42
.L_state_94:
  mv	a4,a5
  li	a0,5
.L_state_48:
  addi	a4,a4,1
  j	.L_state_42
.L_state_44:
  lw	a0,0(a1)
  addi	a5,a4,1
  addi	a0,a0,1
  sw	a0,0(a1)
  lbu	a4,1(a4)
  bne	a4,zero,.L_state_90
.L_state_77:
  mv	a4,a5
  li	a0,5
  j	.L_state_42
.L_state_91:
  lw	a0,0(a1)
  addi	a5,a4,1
  addi	a0,a0,1
  sw	a0,0(a1)
  lbu	t1,1(a4)
  beq	t1,zero,.L_state_71
  beq	t1,a3,.L_state_72
  lw	a0,8(a1)
  addi	a3,t1,-48
  andi	a3,a3,0xff
  li	a7,9
  addi	a0,a0,1
  bleu	a3,a7,.L_state_51
  beq	t1,a6,.L_state_98
  sw	a0,8(a1)
  addi	a4,a4,2
  li	a0,1
  j	.L_state_42
.L_state_98:
  sw	a0,8(a1)
  j	.L_state_54
.L_state_73:
  mv	a4,a5
  li	a0,4
  j	.L_state_42
.L_state_66:
  li	a0,0
  j	.L_state_42
.L_state_92:
  mv	a4,a5
  li	a0,4
  addi	a4,a4,1
  j	.L_state_42
.L_state_95:
  li	a0,3
  addi	a4,a4,1
  j	.L_state_42
.L_state_79:
  li	a0,6
  j	.L_state_42
.L_state_96:
  li	a0,6
  addi	a4,a4,1
  j	.L_state_42
.L_state_81:
  li	a0,7
  j	.L_state_42
.L_state_97:
  li	a0,7
  addi	a4,a4,1
  j	.L_state_42
.L_state_75:
  li	a0,3
  j	.L_state_42
.L_state_71:
  mv	a4,a5
  li	a0,2
  j	.L_state_42
.L_state_72:
  mv	a4,a5
  li	a0,2
  addi	a4,a4,1
  j	.L_state_42
.size	core_state_transition, .-core_state_transition
.align	2
.globl	core_bench_state
.type	core_bench_state, @function
core_bench_state:
  vsetvli	a6,zero,e32,m1,ta,ma
  addi	sp,sp,-128
  vmv.v.i	v1,0
  sw	s0,120(sp)
  sw	s3,108(sp)
  addi	s0,sp,16
  addi	s3,sp,48
  sw	s1,116(sp)
  sw	s2,112(sp)
  sw	s5,100(sp)
  sw	s6,96(sp)
  sw	s7,92(sp)
  sw	s8,88(sp)
  mv	s7,a2
  mv	s5,a3
  mv	s6,a4
  sw	ra,124(sp)
  sw	s4,104(sp)
  mv	s1,a1
  sw	a1,12(sp)
  mv	s2,a5
  mv	s8,a0
  mv	a2,s0
  mv	a3,s3
  li	a4,8
.L_state_100:
  vsetvli	a5,a4,e8,mf4,ta,ma
  vse32.v	v1,0(a3)
  slli	a1,a5,2
  sub	a4,a4,a5
  add	a3,a3,a1
  vse32.v	v1,0(a2)
  add	a2,a2,a1
  bne	a4,zero,.L_state_100
  lbu	a5,0(s1)
  addi	s4,sp,12
  beq	a5,zero,.L_state_154
.L_state_101:
  mv	a1,s3
  mv	a0,s4
  call	core_state_transition
  slli	a4,a0,2
  add	a4,a4,s0
  lw	a3,12(sp)
  lw	a5,0(a4)
  lbu	a3,0(a3)
  addi	a5,a5,1
  sw	a5,0(a4)
  bne	a3,zero,.L_state_101
  sw	s1,12(sp)
  add	s8,s1,s8
  bgeu	s1,s8,.L_state_143
.L_state_102:
  li	a4,1
  mv	a5,s1
  li	a2,44
  bne	s6,a4,.L_state_108
.L_state_111:
  lbu	a4,0(a5)
  xor	a3,a4,s7
  beq	a4,a2,.L_state_110
  sb	a3,0(a5)
.L_state_110:
  addi	a5,a5,1
  bgtu	s8,a5,.L_state_111
  lbu	a5,0(s1)
  addi	s4,sp,12
  beq	a5,zero,.L_state_155
.L_state_143:
  mv	a1,s3
  mv	a0,s4
  call	core_state_transition
  slli	a4,a0,2
  add	a4,a4,s0
  lw	a3,12(sp)
  lw	a5,0(a4)
  lbu	a3,0(a3)
  addi	a5,a5,1
  sw	a5,0(a4)
  bne	a3,zero,.L_state_143
  bgeu	s1,s8,.L_state_119
  li	a5,1
  bne	s6,a5,.L_state_114
.L_state_115:
  li	a3,44
.L_state_123:
  lbu	a5,0(s1)
  xor	a4,a5,s5
  beq	a5,a3,.L_state_122
  sb	a4,0(s1)
.L_state_122:
  addi	s1,s1,1
  bgtu	s8,s1,.L_state_123
.L_state_119:
  mv	s1,s3
.L_state_124:
  lw	a0,0(s0)
  mv	a1,s2
  addi	s0,s0,4
  call	crcu32
  mv	a1,a0
  lw	a0,0(s1)
  addi	s1,s1,4
  call	crcu32
  mv	s2,a0
  bne	s3,s0,.L_state_124
  lw	ra,124(sp)
  lw	s0,120(sp)
  lw	s1,116(sp)
  lw	s2,112(sp)
  lw	s3,108(sp)
  lw	s4,104(sp)
  lw	s5,100(sp)
  lw	s6,96(sp)
  lw	s7,92(sp)
  lw	s8,88(sp)
  addi	sp,sp,128
  jr	ra
.L_state_108:
  lbu	a4,0(a5)
  xor	a3,a4,s7
  beq	a4,a2,.L_state_107
  sb	a3,0(a5)
.L_state_107:
  add	a5,a5,s6
  bgtu	s8,a5,.L_state_108
  lbu	a5,0(s1)
  addi	s4,sp,12
  bne	a5,zero,.L_state_143
  bleu	s8,s1,.L_state_119
.L_state_114:
  li	a3,44
.L_state_121:
  lbu	a5,0(s1)
  xor	a4,a5,s5
  beq	a5,a3,.L_state_120
  sb	a4,0(s1)
.L_state_120:
  add	s1,s1,s6
  bgtu	s8,s1,.L_state_121
  j	.L_state_119
.L_state_155:
  bgtu	s8,s1,.L_state_115
  j	.L_state_119
.L_state_154:
  sw	s1,12(sp)
  add	s8,s1,s8
  bltu	s1,s8,.L_state_102
  j	.L_state_119
.size	core_bench_state, .-core_bench_state
.section	.rodata.str1.4,"aMS",@progbits,1
.align	2
.LC_state_0:
.string	"T0.3e-1F"
.align	2
.LC_state_1:
.string	"-T.T++Tq"
.align	2
.LC_state_2:
.string	"1T3.4e4z"
.align	2
.LC_state_3:
.string	"34.0e-T^"
.align	2
.LC_state_4:
.string	"5.500e+3"
.align	2
.LC_state_5:
.string	"-.123e-2"
.align	2
.LC_state_6:
.string	"-87e+832"
.align	2
.LC_state_7:
.string	"+0.6e-12"
.align	2
.LC_state_8:
.string	"35.54400"
.align	2
.LC_state_9:
.string	".1234500"
.align	2
.LC_state_10:
.string	"-110.700"
.align	2
.LC_state_11:
.string	"+0.64400"
.align	2
.LC_state_12:
.string	"5012"
.align	2
.LC_state_13:
.string	"1234"
.align	2
.LC_state_14:
.string	"-874"
.align	2
.LC_state_15:
.string	"+122"
.section	.rodata
.align	2
.set	.LANCHOR_state_0,. + 0
.type	intpat, @object
.size	intpat, 16
intpat:
.word	.LC_state_12
.word	.LC_state_13
.word	.LC_state_14
.word	.LC_state_15
.type	floatpat, @object
.size	floatpat, 16
floatpat:
.word	.LC_state_8
.word	.LC_state_9
.word	.LC_state_10
.word	.LC_state_11
.type	scipat, @object
.size	scipat, 16
scipat:
.word	.LC_state_4
.word	.LC_state_5
.word	.LC_state_6
.word	.LC_state_7
.type	errpat, @object
.size	errpat, 16
errpat:
.word	.LC_state_0
.word	.LC_state_1
.word	.LC_state_2
.word	.LC_state_3
.ident	"GCC: (Alpine Linux) 15.2.0"
.section	.note.GNU-stack,"",@progbits
.file	"core_main.c"
.option nopic
.text
.align	2
.globl	iterate
.type	iterate, @function
iterate:
  vsetivli	zero,2,e16,m1,ta,ma
  vmv.v.i	v1,0
  addi	sp,sp,-32
  sw	s2,16(sp)
  sw	ra,28(sp)
  addi	a5,a0,56
  lw	s2,28(a0)
  addi	a4,a0,60
  vse16.v	v1,0(a5)
  vse16.v	v1,0(a4)
  beq	s2,zero,.L_main_8
  li	a1,1
  sw	a5,12(sp)
  sw	s0,24(sp)
  sw	s1,20(sp)
  mv	s0,a0
  call	core_bench_list
  lhu	a1,56(s0)
  li	s1,1
  call	crcu16
  sh	a0,56(s0)
  li	a1,-1
  mv	a0,s0
  call	core_bench_list
  lhu	a1,56(s0)
  call	crcu16
  vsetivli	zero,2,e16,m1,ta,ma
  vmv.v.x	v1,a0
  lw	a5,12(sp)
  vse16.v	v1,0(a5)
  beq	s2,s1,.L_main_10
.L_main_3:
  li	a1,1
  mv	a0,s0
  call	core_bench_list
  lhu	a1,56(s0)
  addi	s1,s1,1
  call	crcu16
  sh	a0,56(s0)
  li	a1,-1
  mv	a0,s0
  call	core_bench_list
  lhu	a1,56(s0)
  call	crcu16
  sh	a0,56(s0)
  bne	s2,s1,.L_main_3
.L_main_10:
  lw	s0,24(sp)
  lw	s1,20(sp)
.L_main_8:
  lw	ra,28(sp)
  lw	s2,16(sp)
  li	a0,0
  addi	sp,sp,32
  jr	ra
.size	iterate, .-iterate
.section	.rodata.str1.4,"aMS",@progbits,1
.align	2
.LC_main_1:
.string	"6k performance run parameters for coremark.\n"
.align	2
.LC_main_2:
.string	"6k validation run parameters for coremark.\n"
.align	2
.LC_main_3:
.string	"Profile generation run parameters for coremark.\n"
.align	2
.LC_main_4:
.string	"2K performance run parameters for coremark.\n"
.align	2
.LC_main_5:
.string	"2K validation run parameters for coremark.\n"
.align	2
.LC_main_6:
.string	"[%u]ERROR! list crc 0x%04x - should be 0x%04x\n"
.align	2
.LC_main_7:
.string	"[%u]ERROR! matrix crc 0x%04x - should be 0x%04x\n"
.align	2
.LC_main_8:
.string	"[%u]ERROR! state crc 0x%04x - should be 0x%04x\n"
.align	2
.LC_main_9:
.string	"CoreMark Size    : %lu\n"
.align	2
.LC_main_10:
.string	"Total ticks      : %lu\n"
.align	2
.LC_main_11:
.string	"Total time (secs): %f\n"
.align	2
.LC_main_12:
.string	"Iterations/Sec   : %f\n"
.align	2
.LC_main_14:
.string	"ERROR! Must execute for at least 10 secs for a valid result!\n"
.align	2
.LC_main_15:
.string	"Iterations       : %lu\n"
.align	2
.LC_main_16:
.string	"GCC"
.align	2
.LC_main_17:
.string	"Compiler version : %s\n"
.align	2
.LC_main_18:
.string	"-march=rv32imf_zve32f_zicsr_zifencei_zbb"
.align	2
.LC_main_19:
.string	"Compiler flags   : %s\n"
.align	2
.LC_main_20:
.string	"STACK"
.align	2
.LC_main_21:
.string	"Memory location  : %s\n"
.align	2
.LC_main_22:
.string	"seedcrc          : 0x%04x\n"
.align	2
.LC_main_23:
.string	"[%d]crclist       : 0x%04x\n"
.align	2
.LC_main_24:
.string	"[%d]crcmatrix     : 0x%04x\n"
.align	2
.LC_main_25:
.string	"[%d]crcstate      : 0x%04x\n"
.align	2
.LC_main_26:
.string	"[%d]crcfinal      : 0x%04x\n"
.align	2
.LC_main_27:
.string	"Correct operation validated. See readme.txt for run and reporting rules.\n"
.align	2
.LC_main_28:
.string	"CoreMark 1.0 : %f / %s %s"
.align	2
.LC_main_29:
.string	" / %s"
.align	2
.LC_main_30:
.string	"\n"
.align	2
.LC_main_31:
.string	"Cannot validate operation for these seed values, please compare with results on a known platform.\n"
.align	2
.LC_main_32:
.string	"Errors detected\n"
.section	.text.startup,"ax",@progbits
.align	2
.globl	main
.type	main, @function
main:
  addi	sp,sp,-176
  addi	a2,sp,40
  addi	a1,sp,36
  addi	a0,sp,110
  sw	ra,172(sp)
  sw	s0,168(sp)
  sw	s1,164(sp)
  sw	s2,160(sp)
  sw	s3,156(sp)
  sw	s4,152(sp)
  sw	s5,148(sp)
  sw	s6,144(sp)
  sw	s7,140(sp)
  sw	s8,136(sp)
  sw	s9,132(sp)
  sw	s10,128(sp)
  sw	zero,36(sp)
  call	portable_init
  li	a0,1
  call	get_seed_32
  sh	a0,44(sp)
  li	a0,2
  call	get_seed_32
  sh	a0,46(sp)
  li	a0,3
  call	get_seed_32
  sh	a0,48(sp)
  li	a0,4
  call	get_seed_32
  sw	a0,72(sp)
  li	a0,5
  call	get_seed_32
  bne	a0,zero,.L_main_14
  li	a0,7
.L_main_14:
  lw	a5,44(sp)
  sw	a0,76(sp)
  beq	a5,zero,.L_main_121
  li	a4,1
  addi	s1,sp,44
  beq	a5,a4,.L_main_122
.L_main_16:
  vsetvli	a4,zero,e32,m1,ta,ma
  vmv.v.x	v5,a0
  vsetvli	zero,zero,e16,mf2,ta,ma
  vmv.v.i	v1,0
  vsetvli	zero,zero,e32,m1,ta,ma
  vmv.v.i	v4,1
  lui	a1,%hi(.LANCHOR_main_0)
  vsetvli	zero,zero,e16,mf2,ta,ma
  addi	a1,a1,%lo(.LANCHOR_main_0)
  vid.v	v2
  sh	zero,108(sp)
  sw	a1,52(sp)
  li	a4,3
.L_main_17:
  vsetvli	a5,a4,e32,m1,ta,ma
  vzext.vf2	v0,v2
  sub	a4,a4,a5
  vsll.vv	v0,v4,v0
  vsetvli	a3,zero,e16,mf2,ta,ma
  vmv.v.x	v3,a5
  vsetvli	zero,a5,e32,m1,ta,ma
  vand.vv	v0,v0,v5
  vsetvli	a3,zero,e16,mf2,ta,ma
  vadd.vv	v2,v2,v3
  vsetvli	zero,a5,e32,m1,ta,ma
  vmsne.vi	v0,v0,0
  vsetvli	zero,zero,e16,mf2,tu,mu
  vadd.vi	v1,v1,1,v0.t
  bne	a4,zero,.L_main_17
  vsetvli	a5,zero,e16,mf2,ta,ma
  vmv.s.x	v2,zero
  li	a5,2000
  vredsum.vs	v1,v1,v2
  andi	a4,a0,1
  li	a2,0
  vmv.x.s	a3,v1
  zext.h	a3,a3
  divu	a5,a5,a3
  sw	a5,68(sp)
  bne	a4,zero,.L_main_123
.L_main_18:
  andi	a4,a0,2
  bne	a4,zero,.L_main_124
.L_main_19:
  andi	a0,a0,4
  bne	a0,zero,.L_main_125
.L_main_20:
  lw	a5,76(sp)
  andi	a4,a5,1
  beq	a4,zero,.L_main_21
  lh	a2,44(sp)
  lw	a1,56(sp)
  lw	a0,68(sp)
  call	core_list_init
  lw	a5,76(sp)
  sw	a0,80(sp)
.L_main_21:
  andi	a4,a5,2
  bne	a4,zero,.L_main_126
  andi	a5,a5,4
  beq	a5,zero,.L_main_25
.L_main_23:
  lw	a2,64(sp)
  lh	a1,44(sp)
  lw	a0,68(sp)
  call	core_init_state
.L_main_25:
  lw	a5,72(sp)
  bne	a5,zero,.L_main_26
  vsetivli	zero,2,e16,m1,ta,ma
  vmv.v.i	v1,0
  lui	a5,%hi(.LC_main_0)
  lw	s4,%lo(.LC_main_0)(a5)
  lw	s5,%lo(.LC_main_0+4)(a5)
  addi	a5,sp,12
  vse16.v	v1,0(a5)
  li	s9,1
  sw	s9,72(sp)
  addi	s3,sp,100
  addi	s8,sp,104
.L_main_28:
  lw	a4,72(sp)
  slli	a5,a4,2
  add	a5,a5,a4
  slli	a5,a5,1
  sw	a5,72(sp)
  call	start_time
  vsetivli	zero,2,e16,m1,ta,ma
  addi	a5,sp,12
  vle16.v	v1,0(a5)
  lw	s2,72(sp)
  vse16.v	v1,0(s3)
  vse16.v	v1,0(s8)
  beq	s2,zero,.L_main_31
  li	a1,1
  mv	a0,s1
  call	core_bench_list
  lhu	a1,100(sp)
  li	s0,1
  call	crcu16
  sh	a0,100(sp)
  li	a1,-1
  mv	a0,s1
  call	core_bench_list
  lhu	a1,100(sp)
  call	crcu16
  vsetivli	zero,2,e16,m1,ta,ma
  vmv.v.x	v1,a0
  vse16.v	v1,0(s3)
  beq	s2,s9,.L_main_31
.L_main_30:
  li	a1,1
  mv	a0,s1
  call	core_bench_list
  lhu	a1,100(sp)
  addi	s0,s0,1
  call	crcu16
  sh	a0,100(sp)
  li	a1,-1
  mv	a0,s1
  call	core_bench_list
  lhu	a1,100(sp)
  call	crcu16
  sh	a0,100(sp)
  bne	s2,s0,.L_main_30
.L_main_31:
  call	stop_time
  call	get_time
  call	time_in_secs
  mv	a2,s4
  mv	a3,s5
  mv	s6,a0
  mv	s7,a1
  call	__ltdf2
  blt	a0,zero,.L_main_28
  mv	a0,s6
  mv	a1,s7
  call	__fixunsdfsi
  li	a5,1
  maxu	a4,a0,a5
  li	a5,10
  divu	a5,a5,a4
  lw	a4,72(sp)
  addi	a5,a5,1
  mul	a5,a4,a5
  sw	a5,72(sp)
.L_main_26:
  call	start_time
  mv	a0,s1
  call	iterate
  call	stop_time
  call	get_time
  mv	s0,a0
  lh	a0,44(sp)
  li	a1,0
  call	crc16
  mv	a1,a0
  lh	a0,46(sp)
  call	crc16
  mv	a1,a0
  lh	a0,48(sp)
  call	crc16
  mv	a1,a0
  lh	a0,68(sp)
  call	crc16
  li	a5,32768
  addi	a5,a5,-1275
  mv	s1,a0
  beq	a0,a5,.L_main_33
  bleu	a0,a5,.L_main_127
  li	a5,36864
  addi	a5,a5,-1534
  beq	a0,a5,.L_main_38
  li	a5,61440
  addi	a5,a5,-1547
  bne	a0,a5,.L_main_111
  lui	a0,%hi(.LC_main_4)
  addi	a0,a0,%lo(.LC_main_4)
  call	ee_printf
  li	s3,28672
  li	s2,12288
  addi	s3,s3,-517
  addi	s2,s2,-11
  li	a5,3
  mv	s6,s3
  mv	s5,s2
  sw	zero,12(sp)
  li	s7,0
  sw	a5,28(sp)
  j	.L_main_40
.L_main_126:
  lh	a5,46(sp)
  lh	a2,44(sp)
  lw	a1,60(sp)
  lw	a0,68(sp)
  slli	a5,a5,16
  or	a2,a5,a2
  addi	a3,sp,84
  call	core_init_matrix
  lw	a5,76(sp)
  andi	a5,a5,4
  bne	a5,zero,.L_main_23
  j	.L_main_25
.L_main_127:
  li	a5,8192
  addi	a5,a5,-1806
  beq	a0,a5,.L_main_35
  li	a5,20480
  addi	a5,a5,-337
  bne	a0,a5,.L_main_111
  lui	a0,%hi(.LC_main_3)
  addi	a0,a0,%lo(.LC_main_3)
  call	ee_printf
  li	s7,57344
  li	s3,20480
  li	s2,28672
  addi	s7,s7,1444
  addi	s3,s3,1544
  addi	s2,s2,-1415
  li	a5,2
  sw	s7,12(sp)
  mv	s6,s3
  mv	s5,s2
  sw	a5,28(sp)
.L_main_40:
  lui	s10,%hi(default_num_contexts)
  lw	a5,%lo(default_num_contexts)(s10)
  beq	a5,zero,.L_main_69
  lui	a3,%hi(.LC_main_6)
  lui	a4,%hi(.LC_main_7)
  lui	a5,%hi(.LC_main_8)
  addi	a3,a3,%lo(.LC_main_6)
  addi	a4,a4,%lo(.LC_main_7)
  addi	a5,a5,%lo(.LC_main_8)
  sw	s11,124(sp)
  sw	a3,16(sp)
  sw	a4,20(sp)
  sw	a5,24(sp)
  li	s4,0
  li	s11,0
  j	.L_main_46
.L_main_129:
  add	s8,s8,sp
  lhu	a5,108(s8)
.L_main_44:
  add	s8,a5,s4
  lw	a5,%lo(default_num_contexts)(s10)
  addi	s11,s11,1
  zext.h	s11,s11
  mv	s4,s8
  bgeu	s11,a5,.L_main_128
.L_main_46:
  slli	s8,s11,4
  add	s9,s8,s11
  slli	s9,s9,2
  addi	a5,s9,80
  addi	a4,sp,32
  add	s9,a5,a4
  lw	a5,-36(s9)
  sh	zero,-4(s9)
  andi	a3,a5,1
  beq	a3,zero,.L_main_41
  lhu	a2,-10(s9)
  beq	a2,s5,.L_main_41
  lw	a0,16(sp)
  mv	a3,s2
  mv	a1,s11
  call	ee_printf
  lhu	a3,-4(s9)
  lw	a5,-36(s9)
  addi	a3,a3,1
  sh	a3,-4(s9)
.L_main_41:
  andi	a3,a5,2
  beq	a3,zero,.L_main_42
  add	s9,s8,s11
  slli	s9,s9,2
  addi	a4,s9,80
  addi	a3,sp,32
  add	s9,a4,a3
  lhu	a2,-8(s9)
  beq	a2,s6,.L_main_42
  lw	a0,20(sp)
  mv	a3,s3
  mv	a1,s11
  call	ee_printf
  lhu	a3,-4(s9)
  lw	a5,-36(s9)
  addi	a3,a3,1
  sh	a3,-4(s9)
.L_main_42:
  add	s8,s8,s11
  andi	a5,a5,4
  slli	s8,s8,2
  beq	a5,zero,.L_main_129
  addi	a5,s8,80
  addi	a4,sp,32
  add	s8,a5,a4
  lhu	a2,-6(s8)
  bne	a2,s7,.L_main_45
  lhu	a5,-4(s8)
  addi	s11,s11,1
  zext.h	s11,s11
  add	s8,a5,s4
  lw	a5,%lo(default_num_contexts)(s10)
  mv	s4,s8
  bltu	s11,a5,.L_main_46
.L_main_128:
  lw	s11,124(sp)
  j	.L_main_37
.L_main_125:
  mul	a2,a2,a5
  add	a1,a1,a2
  sw	a1,64(sp)
  j	.L_main_20
.L_main_124:
  mul	a4,a2,a5
  addi	a2,a2,1
  zext.h	a2,a2
  add	a4,a1,a4
  sw	a4,60(sp)
  j	.L_main_19
.L_main_123:
  sw	a1,56(sp)
  li	a2,1
  j	.L_main_18
.L_main_121:
  lh	a5,48(sp)
  addi	s1,sp,44
  bne	a5,zero,.L_main_16
  vsetivli	zero,2,e16,m1,ta,ma
  vmv.v.i	v1,0
  li	a5,102
  sh	a5,48(sp)
  vse16.v	v1,0(s1)
  j	.L_main_16
.L_main_111:
  li	a5,65536
  addi	s8,a5,-1
  li	a5,-1
  sw	a5,28(sp)
  lui	s10,%hi(default_num_contexts)
.L_main_37:
  call	check_data_types
  lw	a1,68(sp)
  add	s2,a0,s8
  lui	a0,%hi(.LC_main_9)
  addi	a0,a0,%lo(.LC_main_9)
  call	ee_printf
  lui	a0,%hi(.LC_main_10)
  mv	a1,s0
  addi	a0,a0,%lo(.LC_main_10)
  call	ee_printf
  mv	a0,s0
  call	time_in_secs
  mv	a2,a0
  lui	a0,%hi(.LC_main_11)
  mv	a3,a1
  addi	a0,a0,%lo(.LC_main_11)
  call	ee_printf
  mv	a0,s0
  call	time_in_secs
  li	a2,0
  li	a3,0
  call	__gtdf2
  ble	a0,zero,.L_main_47
  lw	a5,%lo(default_num_contexts)(s10)
  lw	a0,72(sp)
  mul	a0,a0,a5
  call	__floatunsidf
  mv	s4,a0
  mv	a0,s0
  mv	s5,a1
  call	time_in_secs
  mv	a2,a0
  mv	a3,a1
  mv	a0,s4
  mv	a1,s5
  call	__divdf3
  mv	a2,a0
  lui	a0,%hi(.LC_main_12)
  mv	a3,a1
  addi	a0,a0,%lo(.LC_main_12)
  call	ee_printf
.L_main_47:
  mv	a0,s0
  call	time_in_secs
  lui	a5,%hi(.LC_main_13)
  lw	a2,%lo(.LC_main_13)(a5)
  lw	a3,%lo(.LC_main_13+4)(a5)
  call	__ltdf2
  blt	a0,zero,.L_main_130
.L_main_49:
  lw	a5,%lo(default_num_contexts)(s10)
  lw	a1,72(sp)
  lui	a0,%hi(.LC_main_15)
  addi	a0,a0,%lo(.LC_main_15)
  mul	a1,a1,a5
  lui	s4,%hi(.LC_main_16)
  lui	s5,%hi(.LC_main_18)
  lui	s3,%hi(.LC_main_20)
  sext.h	s2,s2
  call	ee_printf
  lui	a0,%hi(.LC_main_17)
  addi	a1,s4,%lo(.LC_main_16)
  addi	a0,a0,%lo(.LC_main_17)
  call	ee_printf
  lui	a0,%hi(.LC_main_19)
  addi	a1,s5,%lo(.LC_main_18)
  addi	a0,a0,%lo(.LC_main_19)
  call	ee_printf
  lui	a0,%hi(.LC_main_21)
  addi	a1,s3,%lo(.LC_main_20)
  addi	a0,a0,%lo(.LC_main_21)
  call	ee_printf
  lui	a0,%hi(.LC_main_22)
  mv	a1,s1
  addi	a0,a0,%lo(.LC_main_22)
  call	ee_printf
  lw	a5,76(sp)
  andi	a4,a5,1
  beq	a4,zero,.L_main_54
  lw	a4,%lo(default_num_contexts)(s10)
  beq	a4,zero,.L_main_54
  lui	s6,%hi(.LC_main_23)
  addi	s6,s6,%lo(.LC_main_23)
  li	s1,0
.L_main_55:
  slli	a5,s1,4
  add	a5,a5,s1
  slli	a5,a5,2
  add	a5,a5,sp
  lhu	a2,102(a5)
  mv	a1,s1
  mv	a0,s6
  call	ee_printf
  lw	a5,%lo(default_num_contexts)(s10)
  addi	s1,s1,1
  zext.h	s1,s1
  bltu	s1,a5,.L_main_55
  lw	a5,76(sp)
.L_main_54:
  andi	a4,a5,2
  beq	a4,zero,.L_main_53
  lw	a4,%lo(default_num_contexts)(s10)
  beq	a4,zero,.L_main_58
  lui	s6,%hi(.LC_main_24)
  addi	s6,s6,%lo(.LC_main_24)
  li	s1,0
.L_main_59:
  slli	a5,s1,4
  add	a5,a5,s1
  slli	a5,a5,2
  add	a5,a5,sp
  lhu	a2,104(a5)
  mv	a1,s1
  mv	a0,s6
  call	ee_printf
  lw	a5,%lo(default_num_contexts)(s10)
  addi	s1,s1,1
  zext.h	s1,s1
  bltu	s1,a5,.L_main_59
  lw	a5,76(sp)
.L_main_53:
  andi	a5,a5,4
  beq	a5,zero,.L_main_57
  lw	a5,%lo(default_num_contexts)(s10)
  beq	a5,zero,.L_main_61
  lui	s6,%hi(.LC_main_25)
  addi	s6,s6,%lo(.LC_main_25)
  li	s1,0
.L_main_62:
  slli	a5,s1,4
  add	a5,a5,s1
  slli	a5,a5,2
  add	a5,a5,sp
  lhu	a2,106(a5)
  mv	a1,s1
  mv	a0,s6
  call	ee_printf
  lw	a5,%lo(default_num_contexts)(s10)
  addi	s1,s1,1
  zext.h	s1,s1
  bltu	s1,a5,.L_main_62
.L_main_57:
  lw	a5,%lo(default_num_contexts)(s10)
  lui	s6,%hi(.LC_main_26)
  addi	s6,s6,%lo(.LC_main_26)
  li	s1,0
  beq	a5,zero,.L_main_61
.L_main_60:
  slli	a5,s1,4
  add	a5,a5,s1
  slli	a5,a5,2
  add	a5,a5,sp
  lhu	a2,100(a5)
  mv	a1,s1
  mv	a0,s6
  call	ee_printf
  lw	a5,%lo(default_num_contexts)(s10)
  addi	s1,s1,1
  zext.h	s1,s1
  bltu	s1,a5,.L_main_60
.L_main_61:
  beq	s2,zero,.L_main_131
  ble	s2,zero,.L_main_132
  lui	a0,%hi(.LC_main_32)
  addi	a0,a0,%lo(.LC_main_32)
  call	ee_printf
.L_main_65:
  addi	a0,sp,110
  call	portable_fini
  lw	ra,172(sp)
  lw	s0,168(sp)
  lw	s1,164(sp)
  lw	s2,160(sp)
  lw	s3,156(sp)
  lw	s4,152(sp)
  lw	s5,148(sp)
  lw	s6,144(sp)
  lw	s7,140(sp)
  lw	s8,136(sp)
  lw	s9,132(sp)
  lw	s10,128(sp)
  li	a0,0
  addi	sp,sp,176
  jr	ra
.L_main_45:
  lw	a3,12(sp)
  lw	a0,24(sp)
  mv	a1,s11
  call	ee_printf
  lhu	a5,-4(s8)
  addi	a5,a5,1
  zext.h	a5,a5
  sh	a5,-4(s8)
  j	.L_main_44
.L_main_130:
  lui	a0,%hi(.LC_main_14)
  addi	a0,a0,%lo(.LC_main_14)
  call	ee_printf
  addi	s2,s2,1
  j	.L_main_49
.L_main_131:
  lui	a0,%hi(.LC_main_27)
  addi	a0,a0,%lo(.LC_main_27)
  call	ee_printf
  lw	a4,28(sp)
  li	a5,3
  bne	a4,a5,.L_main_65
  lw	a5,%lo(default_num_contexts)(s10)
  lw	a0,72(sp)
  mul	a0,a0,a5
  call	__floatunsidf
  mv	s6,a0
  mv	a0,s0
  mv	s7,a1
  call	time_in_secs
  mv	a2,a0
  mv	a3,a1
  mv	a0,s6
  mv	a1,s7
  call	__divdf3
  mv	a2,a0
  lui	a0,%hi(.LC_main_28)
  mv	a3,a1
  addi	a5,s5,%lo(.LC_main_18)
  addi	a4,s4,%lo(.LC_main_16)
  addi	a0,a0,%lo(.LC_main_28)
  call	ee_printf
  lui	a0,%hi(.LC_main_29)
  addi	a1,s3,%lo(.LC_main_20)
  addi	a0,a0,%lo(.LC_main_29)
  call	ee_printf
  lui	a0,%hi(.LC_main_30)
  addi	a0,a0,%lo(.LC_main_30)
  call	ee_printf
  j	.L_main_65
.L_main_132:
  lui	a0,%hi(.LC_main_31)
  addi	a0,a0,%lo(.LC_main_31)
  call	ee_printf
  j	.L_main_65
.L_main_35:
  lui	a0,%hi(.LC_main_5)
  addi	a0,a0,%lo(.LC_main_5)
  call	ee_printf
  li	s7,36864
  li	s2,57344
  addi	s7,s7,-636
  addi	s2,s2,961
  li	s3,1863
  li	a5,4
  sw	s7,12(sp)
  mv	s5,s2
  mv	s6,s3
  sw	a5,28(sp)
  j	.L_main_40
.L_main_33:
  lui	a0,%hi(.LC_main_2)
  addi	a0,a0,%lo(.LC_main_2)
  call	ee_printf
  li	s7,16384
  li	s3,4096
  li	s2,12288
  addi	s7,s7,-1601
  addi	s3,s3,409
  addi	s2,s2,832
  li	a5,1
  sw	s7,12(sp)
  mv	s6,s3
  mv	s5,s2
  sw	a5,28(sp)
  j	.L_main_40
.L_main_38:
  lui	a0,%hi(.LC_main_1)
  li	s7,24576
  li	s3,49152
  li	s2,53248
  addi	a0,a0,%lo(.LC_main_1)
  addi	s7,s7,-441
  addi	s3,s3,-430
  addi	s2,s2,1200
  call	ee_printf
  mv	s6,s3
  sw	s7,12(sp)
  mv	s5,s2
  sw	zero,28(sp)
  j	.L_main_40
.L_main_122:
  lh	a5,48(sp)
  bne	a5,zero,.L_main_16
  li	a5,12288
  addi	a5,a5,1045
  vsetivli	zero,2,e16,m1,ta,ma
  vmv.v.x	v1,a5
  li	a5,102
  sh	a5,48(sp)
  vse16.v	v1,0(s1)
  j	.L_main_16
.L_main_69:
  li	s8,0
  j	.L_main_37
.L_main_58:
  andi	a5,a5,4
  beq	a5,zero,.L_main_57
  j	.L_main_61
.size	main, .-main
.globl	mem_name
.section	.rodata.str1.4
.align	2
.LC_main_34:
.string	"Static"
.align	2
.LC_main_35:
.string	"Heap"
.align	2
.LC_main_36:
.string	"Stack"
.globl	static_memblk
.section	.srodata.cst8,"aM",@progbits,8
.align	3
.LC_main_0:
.word	0
.word	1072693248
.align	3
.LC_main_13:
.word	0
.word	1076101120
.data
.align	2
.type	mem_name, @object
.size	mem_name, 12
mem_name:
.word	.LC_main_34
.word	.LC_main_35
.word	.LC_main_36
.bss
.align	2
.set	.LANCHOR_main_0,. + 0
.type	static_memblk, @object
.size	static_memblk, 2000
static_memblk:
.zero	2000
.globl	__divdf3
.globl	__floatunsidf
.globl	__gtdf2
.globl	__fixunsdfsi
.globl	__ltdf2
.ident	"GCC: (Alpine Linux) 15.2.0"
.section	.note.GNU-stack,"",@progbits
.file	"core_util.c"
.option nopic
.text
.align	2
.globl	get_seed_32
.type	get_seed_32, @function
get_seed_32:
  li	a5,5
  bgtu	a0,a5,.L_util_9
  lui	a5,%hi(.L_util_4)
  addi	a5,a5,%lo(.L_util_4)
  slli	a0,a0,2
  add	a0,a0,a5
  lw	a5,0(a0)
  jr	a5
.section	.rodata
.align	2
.align	2
.L_util_4:
.word	.L_util_9
.word	.L_util_8
.word	.L_util_7
.word	.L_util_6
.word	.L_util_5
.word	.L_util_3
.text
.L_util_3:
  lui	a5,%hi(seed5_volatile)
  lw	a0,%lo(seed5_volatile)(a5)
  ret
.L_util_8:
  lui	a5,%hi(seed1_volatile)
  lw	a0,%lo(seed1_volatile)(a5)
  ret
.L_util_7:
  lui	a5,%hi(seed2_volatile)
  lw	a0,%lo(seed2_volatile)(a5)
  ret
.L_util_6:
  lui	a5,%hi(seed3_volatile)
  lw	a0,%lo(seed3_volatile)(a5)
  ret
.L_util_5:
  lui	a5,%hi(seed4_volatile)
  lw	a0,%lo(seed4_volatile)(a5)
  ret
.L_util_9:
  li	a0,0
  ret
.size	get_seed_32, .-get_seed_32
.align	2
.globl	crcu8
.type	crcu8, @function
crcu8:
  andi	a4,a1,255
  slli	a4,a4,8
  srli	a1,a1,8
  li	a5,4096
  li	t1,-4096
  or	a4,a4,a1
  addi	a5,a5,-241
  addi	t1,t1,240
  and	a2,a4,a5
  and	a4,a4,t1
  srli	a4,a4,4
  slli	a2,a2,4
  or	a2,a2,a4
  li	a7,-12288
  li	a4,12288
  addi	a4,a4,819
  addi	a7,a7,-820
  andi	a3,a0,15
  and	a1,a2,a4
  srli	a0,a0,4
  and	a2,a2,a7
  slli	a3,a3,4
  or	a3,a3,a0
  srli	a2,a2,2
  slli	a1,a1,2
  or	a1,a1,a2
  li	a0,20480
  andi	a2,a3,51
  li	a6,-20480
  andi	a3,a3,204
  addi	a6,a6,-1366
  srli	t3,a3,2
  addi	a0,a0,1365
  slli	a2,a2,2
  and	a3,a1,a0
  or	a2,a2,t3
  and	a1,a1,a6
  srli	a1,a1,1
  slli	a3,a3,1
  andi	t4,a2,85
  andi	a2,a2,170
  or	t3,a3,a1
  slli	t4,t4,1
  srli	a2,a2,1
  srli	t3,t3,8
  or	a2,t4,a2
  xor	a2,t3,a2
  lui	t3,%hi(.LC_util_0)
  addi	t3,t3,%lo(.LC_util_0)
  slli	a2,a2,1
  add	a2,a2,t3
  lhu	a2,0(a2)
  or	a3,a3,a1
  slli	a3,a3,8
  xor	a3,a2,a3
  slli	a1,a3,16
  andi	a2,a2,0xff
  srli	a3,a1,24
  slli	a2,a2,8
  or	a2,a2,a3
  and	a3,a2,a5
  and	a2,a2,t1
  srli	a2,a2,4
  slli	a3,a3,4
  or	a3,a3,a2
  and	a5,a3,a4
  and	a3,a3,a7
  srli	a3,a3,2
  slli	a5,a5,2
  or	a5,a5,a3
  and	a0,a5,a0
  and	a5,a5,a6
  srli	a5,a5,1
  slli	a0,a0,1
  or	a0,a0,a5
  ret
.size	crcu8, .-crcu8
.align	2
.globl	crcu16
.type	crcu16, @function
crcu16:
  andi	a5,a1,255
  slli	a5,a5,8
  srli	a1,a1,8
  li	a4,4096
  li	t1,-4096
  addi	t1,t1,240
  addi	a4,a4,-241
  or	a5,a5,a1
  and	a1,a5,a4
  and	a5,a5,t1
  srli	a5,a5,4
  slli	a1,a1,4
  or	a1,a1,a5
  li	a7,-12288
  li	a5,12288
  addi	a7,a7,-820
  addi	a5,a5,819
  andi	a3,a0,15
  andi	a6,a0,240
  and	a2,a1,a5
  srli	a6,a6,4
  and	a1,a1,a7
  slli	a3,a3,4
  or	a3,a3,a6
  srli	a1,a1,2
  slli	a2,a2,2
  or	a2,a2,a1
  andi	t3,a3,51
  li	a1,20480
  andi	a3,a3,204
  li	a6,-20480
  addi	a1,a1,1365
  addi	a6,a6,-1366
  srli	t4,a3,2
  slli	t3,t3,2
  and	a3,a2,a1
  or	t3,t3,t4
  and	a2,a2,a6
  slli	a3,a3,1
  srli	a2,a2,1
  andi	t5,t3,85
  andi	t3,t3,170
  or	t4,a3,a2
  slli	t5,t5,1
  srli	t3,t3,1
  or	t3,t5,t3
  srli	t4,t4,8
  xor	t4,t4,t3
  lui	t3,%hi(.LC_util_0)
  addi	t3,t3,%lo(.LC_util_0)
  or	a2,a3,a2
  slli	a3,t4,1
  add	a3,a3,t3
  lhu	a3,0(a3)
  slli	a2,a2,8
  srli	a0,a0,8
  xor	a2,a3,a2
  slli	t4,a2,16
  andi	a3,a3,0xff
  srli	a2,t4,24
  slli	a3,a3,8
  or	a3,a3,a2
  and	a2,a3,a4
  and	a3,a3,t1
  srli	a3,a3,4
  slli	a2,a2,4
  or	a2,a2,a3
  and	a3,a2,a5
  and	a2,a2,a7
  srli	a2,a2,2
  slli	a3,a3,2
  or	a3,a3,a2
  and	a2,a3,a1
  and	a3,a3,a6
  slli	a2,a2,1
  srli	a3,a3,1
  or	a3,a2,a3
  andi	a2,a3,255
  slli	a2,a2,8
  srli	a3,a3,8
  or	a2,a2,a3
  and	a3,a2,a4
  and	t4,a2,t1
  srli	t4,t4,4
  andi	a2,a0,15
  slli	a3,a3,4
  or	a3,a3,t4
  srli	a0,a0,4
  slli	a2,a2,4
  or	a2,a2,a0
  and	t4,a3,a7
  and	a0,a3,a5
  srli	t4,t4,2
  andi	a3,a2,51
  slli	a0,a0,2
  andi	a2,a2,204
  or	a0,a0,t4
  srli	a2,a2,2
  slli	a3,a3,2
  or	a3,a3,a2
  and	a2,a0,a1
  and	a0,a0,a6
  andi	t5,a3,85
  slli	a2,a2,1
  srli	a0,a0,1
  andi	a3,a3,170
  or	t4,a2,a0
  srli	a3,a3,1
  slli	t5,t5,1
  or	t5,t5,a3
  srli	a3,t4,8
  xor	a3,a3,t5
  slli	a3,a3,1
  add	a3,a3,t3
  lhu	a3,0(a3)
  slli	a2,t4,8
  xor	a2,a3,a2
  slli	a0,a2,16
  andi	a3,a3,0xff
  srli	a2,a0,24
  slli	a3,a3,8
  or	a3,a3,a2
  and	a4,a3,a4
  and	a3,a3,t1
  srli	a3,a3,4
  slli	a4,a4,4
  or	a4,a4,a3
  and	a5,a4,a5
  and	a4,a4,a7
  srli	a4,a4,2
  slli	a5,a5,2
  or	a5,a5,a4
  and	a0,a5,a1
  and	a5,a5,a6
  srli	a5,a5,1
  slli	a0,a0,1
  or	a0,a0,a5
  ret
.size	crcu16, .-crcu16
.align	2
.globl	crcu32
.type	crcu32, @function
crcu32:
  andi	a5,a1,255
  slli	a5,a5,8
  srli	a1,a1,8
  li	a4,4096
  li	a7,-4096
  addi	a7,a7,240
  addi	a4,a4,-241
  or	a5,a5,a1
  and	a3,a5,a4
  and	a5,a5,a7
  srli	a5,a5,4
  slli	a3,a3,4
  or	a3,a3,a5
  li	a6,-12288
  li	a5,12288
  addi	a6,a6,-820
  addi	a5,a5,819
  andi	a2,a0,15
  andi	a1,a0,240
  and	t1,a3,a5
  srli	a1,a1,4
  and	a3,a3,a6
  slli	a2,a2,4
  or	a2,a2,a1
  srli	a3,a3,2
  slli	t1,t1,2
  or	t1,t1,a3
  andi	t3,a2,51
  li	a3,20480
  andi	a2,a2,204
  li	a1,-20480
  addi	a3,a3,1365
  addi	a1,a1,-1366
  srli	t4,a2,2
  slli	t3,t3,2
  and	a2,t1,a3
  or	t3,t3,t4
  and	t1,t1,a1
  slli	a2,a2,1
  srli	t1,t1,1
  andi	t5,t3,85
  andi	t3,t3,170
  or	t4,a2,t1
  slli	t5,t5,1
  srli	t3,t3,1
  or	t3,t5,t3
  srli	t4,t4,8
  xor	t4,t4,t3
  or	t3,a2,t1
  lui	t1,%hi(.LC_util_0)
  addi	t1,t1,%lo(.LC_util_0)
  slli	a2,t4,1
  add	a2,a2,t1
  lhu	a2,0(a2)
  slli	t3,t3,8
  srli	t5,a0,8
  xor	t3,a2,t3
  slli	t4,t3,16
  andi	a2,a2,0xff
  srli	t3,t4,24
  slli	a2,a2,8
  or	a2,a2,t3
  and	t3,a2,a4
  and	a2,a2,a7
  srli	a2,a2,4
  slli	t3,t3,4
  or	t3,t3,a2
  and	a2,t3,a5
  and	t3,t3,a6
  srli	t3,t3,2
  slli	a2,a2,2
  or	a2,a2,t3
  and	t3,a2,a3
  and	a2,a2,a1
  srli	a2,a2,1
  slli	t3,t3,1
  or	t3,t3,a2
  andi	a2,t3,255
  slli	a2,a2,8
  srli	t3,t3,8
  or	a2,a2,t3
  and	t4,a2,a4
  and	a2,a2,a7
  srli	a2,a2,4
  slli	t4,t4,4
  or	t4,t4,a2
  andi	a2,t5,15
  andi	t5,t5,240
  and	t3,t4,a5
  srli	t5,t5,4
  and	t4,t4,a6
  slli	a2,a2,4
  or	a2,a2,t5
  srli	t4,t4,2
  slli	t3,t3,2
  or	t3,t3,t4
  andi	t4,a2,51
  andi	a2,a2,204
  srli	t5,a2,2
  slli	t4,t4,2
  and	a2,t3,a3
  or	t4,t4,t5
  and	t3,t3,a1
  slli	a2,a2,1
  srli	t3,t3,1
  andi	t6,t4,85
  andi	t4,t4,170
  or	t5,a2,t3
  slli	t6,t6,1
  srli	t4,t4,1
  srli	t5,t5,8
  or	t4,t6,t4
  xor	t4,t5,t4
  or	t3,a2,t3
  slli	a2,t4,1
  add	a2,a2,t1
  lhu	a2,0(a2)
  slli	t3,t3,8
  srli	a0,a0,16
  xor	t3,a2,t3
  slli	t4,t3,16
  andi	a2,a2,0xff
  srli	t3,t4,24
  slli	a2,a2,8
  or	a2,a2,t3
  and	t3,a2,a4
  and	a2,a2,a7
  srli	a2,a2,4
  slli	t3,t3,4
  or	t3,t3,a2
  and	a2,t3,a5
  and	t3,t3,a6
  srli	t3,t3,2
  slli	a2,a2,2
  or	a2,a2,t3
  and	t3,a2,a3
  and	a2,a2,a1
  slli	t3,t3,1
  srli	a2,a2,1
  or	a2,t3,a2
  andi	t3,a2,255
  slli	t3,t3,8
  srli	a2,a2,8
  or	t3,t3,a2
  and	a2,t3,a4
  and	t3,t3,a7
  srli	t3,t3,4
  slli	a2,a2,4
  or	a2,a2,t3
  andi	t5,a0,240
  andi	t3,a0,15
  and	t4,a2,a5
  srli	t5,t5,4
  and	a2,a2,a6
  slli	t3,t3,4
  or	t3,t3,t5
  srli	a2,a2,2
  slli	t4,t4,2
  or	t4,t4,a2
  andi	a2,t3,51
  andi	t3,t3,204
  srli	t5,t3,2
  slli	a2,a2,2
  and	t3,t4,a3
  or	a2,a2,t5
  and	t4,t4,a1
  srli	t4,t4,1
  slli	t3,t3,1
  andi	t6,a2,85
  andi	a2,a2,170
  or	t5,t3,t4
  slli	t6,t6,1
  srli	a2,a2,1
  srli	t5,t5,8
  or	a2,t6,a2
  xor	a2,t5,a2
  slli	a2,a2,1
  add	a2,a2,t1
  lhu	a2,0(a2)
  or	t3,t3,t4
  slli	t3,t3,8
  xor	t3,a2,t3
  slli	t4,t3,16
  andi	a2,a2,0xff
  srli	t3,t4,24
  slli	a2,a2,8
  or	a2,a2,t3
  and	t3,a2,a4
  and	a2,a2,a7
  srli	a2,a2,4
  slli	t3,t3,4
  or	t3,t3,a2
  and	a2,t3,a5
  and	t3,t3,a6
  srli	t3,t3,2
  slli	a2,a2,2
  or	a2,a2,t3
  and	t3,a2,a3
  and	a2,a2,a1
  srli	a2,a2,1
  slli	t3,t3,1
  or	t3,t3,a2
  andi	a2,t3,255
  slli	a2,a2,8
  srli	t3,t3,8
  or	a2,a2,t3
  and	t4,a2,a4
  and	t3,a2,a7
  srli	a0,a0,8
  andi	a2,a0,15
  srli	t3,t3,4
  slli	t4,t4,4
  slli	a2,a2,4
  or	t4,t4,t3
  srli	a0,a0,4
  and	t3,t4,a5
  or	a0,a2,a0
  and	t4,t4,a6
  andi	a2,a0,51
  srli	t4,t4,2
  andi	a0,a0,204
  slli	t3,t3,2
  or	t3,t3,t4
  srli	a0,a0,2
  slli	a2,a2,2
  or	a2,a2,a0
  and	a0,t3,a3
  and	t3,t3,a1
  andi	t5,a2,85
  slli	a0,a0,1
  srli	t3,t3,1
  andi	a2,a2,170
  or	t4,a0,t3
  srli	a2,a2,1
  slli	t5,t5,1
  or	t5,t5,a2
  srli	a2,t4,8
  xor	a2,a2,t5
  slli	a2,a2,1
  add	a2,a2,t1
  lhu	a2,0(a2)
  slli	a0,t4,8
  xor	a0,a2,a0
  slli	t1,a0,16
  andi	a2,a2,0xff
  srli	a0,t1,24
  slli	a2,a2,8
  or	a2,a2,a0
  and	a4,a2,a4
  and	a2,a2,a7
  srli	a2,a2,4
  slli	a4,a4,4
  or	a4,a4,a2
  and	a5,a4,a5
  and	a4,a4,a6
  srli	a4,a4,2
  slli	a5,a5,2
  or	a5,a5,a4
  and	a0,a5,a3
  and	a5,a5,a1
  srli	a5,a5,1
  slli	a0,a0,1
  or	a0,a0,a5
  ret
.size	crcu32, .-crcu32
.align	2
.globl	crc16
.type	crc16, @function
crc16:
  andi	a5,a1,255
  slli	a5,a5,8
  srli	a1,a1,8
  li	a4,4096
  li	t1,-4096
  addi	t1,t1,240
  addi	a4,a4,-241
  or	a5,a5,a1
  and	a1,a5,a4
  and	a5,a5,t1
  srli	a5,a5,4
  slli	a1,a1,4
  or	a1,a1,a5
  li	a7,-12288
  li	a5,12288
  addi	a7,a7,-820
  addi	a5,a5,819
  andi	a3,a0,15
  andi	a6,a0,240
  and	a2,a1,a5
  srli	a6,a6,4
  and	a1,a1,a7
  slli	a3,a3,4
  or	a3,a3,a6
  srli	a1,a1,2
  slli	a2,a2,2
  or	a2,a2,a1
  andi	t3,a3,51
  li	a1,20480
  andi	a3,a3,204
  li	a6,-20480
  addi	a1,a1,1365
  addi	a6,a6,-1366
  srli	t4,a3,2
  slli	t3,t3,2
  and	a3,a2,a1
  or	t3,t3,t4
  and	a2,a2,a6
  slli	a3,a3,1
  srli	a2,a2,1
  andi	t5,t3,85
  andi	t3,t3,170
  or	t4,a3,a2
  slli	t5,t5,1
  srli	t3,t3,1
  or	t3,t5,t3
  srli	t4,t4,8
  xor	t4,t4,t3
  lui	t3,%hi(.LC_util_0)
  addi	t3,t3,%lo(.LC_util_0)
  or	a2,a3,a2
  slli	a3,t4,1
  add	a3,a3,t3
  lhu	a3,0(a3)
  slli	a2,a2,8
  srli	a0,a0,8
  xor	a2,a3,a2
  slli	t4,a2,16
  andi	a3,a3,0xff
  srli	a2,t4,24
  slli	a3,a3,8
  or	a3,a3,a2
  and	a2,a3,a4
  and	a3,a3,t1
  srli	a3,a3,4
  slli	a2,a2,4
  or	a2,a2,a3
  and	a3,a2,a5
  and	a2,a2,a7
  srli	a2,a2,2
  slli	a3,a3,2
  or	a3,a3,a2
  and	a2,a3,a1
  and	a3,a3,a6
  slli	a2,a2,1
  srli	a3,a3,1
  or	a3,a2,a3
  andi	a2,a3,255
  slli	a2,a2,8
  srli	a3,a3,8
  or	a2,a2,a3
  and	a3,a2,a4
  and	t4,a2,t1
  srli	t4,t4,4
  andi	a2,a0,15
  slli	a3,a3,4
  andi	a0,a0,240
  or	a3,a3,t4
  srli	a0,a0,4
  slli	a2,a2,4
  or	a2,a2,a0
  and	t4,a3,a7
  and	a0,a3,a5
  srli	t4,t4,2
  andi	a3,a2,51
  slli	a0,a0,2
  andi	a2,a2,204
  or	a0,a0,t4
  srli	a2,a2,2
  slli	a3,a3,2
  or	a3,a3,a2
  and	a2,a0,a1
  and	a0,a0,a6
  andi	t5,a3,85
  slli	a2,a2,1
  srli	a0,a0,1
  andi	a3,a3,170
  or	t4,a2,a0
  srli	a3,a3,1
  slli	t5,t5,1
  or	t5,t5,a3
  srli	a3,t4,8
  xor	a3,a3,t5
  slli	a3,a3,1
  add	a3,a3,t3
  lhu	a3,0(a3)
  slli	a2,t4,8
  xor	a2,a3,a2
  slli	a0,a2,16
  andi	a3,a3,0xff
  srli	a2,a0,24
  slli	a3,a3,8
  or	a3,a3,a2
  and	a4,a3,a4
  and	a3,a3,t1
  srli	a3,a3,4
  slli	a4,a4,4
  or	a4,a4,a3
  and	a5,a4,a5
  and	a4,a4,a7
  srli	a4,a4,2
  slli	a5,a5,2
  or	a5,a5,a4
  and	a0,a5,a1
  and	a5,a5,a6
  srli	a5,a5,1
  slli	a0,a0,1
  or	a0,a0,a5
  ret
.size	crc16, .-crc16
.align	2
.globl	check_data_types
.type	check_data_types, @function
check_data_types:
  li	a0,0
  ret
.size	check_data_types, .-check_data_types
.section	.rodata
.align	2
.LC_util_0:
.half	0
.half	-32763
.half	-32753
.half	10
.half	-32741
.half	30
.half	20
.half	-32751
.half	-32717
.half	54
.half	60
.half	-32711
.half	40
.half	-32723
.half	-32729
.half	34
.half	-32669
.half	102
.half	108
.half	-32663
.half	120
.half	-32643
.half	-32649
.half	114
.half	80
.half	-32683
.half	-32673
.half	90
.half	-32693
.half	78
.half	68
.half	-32703
.half	-32573
.half	198
.half	204
.half	-32567
.half	216
.half	-32547
.half	-32553
.half	210
.half	240
.half	-32523
.half	-32513
.half	250
.half	-32533
.half	238
.half	228
.half	-32543
.half	160
.half	-32603
.half	-32593
.half	170
.half	-32581
.half	190
.half	180
.half	-32591
.half	-32621
.half	150
.half	156
.half	-32615
.half	136
.half	-32627
.half	-32633
.half	130
.half	-32381
.half	390
.half	396
.half	-32375
.half	408
.half	-32355
.half	-32361
.half	402
.half	432
.half	-32331
.half	-32321
.half	442
.half	-32341
.half	430
.half	420
.half	-32351
.half	480
.half	-32283
.half	-32273
.half	490
.half	-32261
.half	510
.half	500
.half	-32271
.half	-32301
.half	470
.half	476
.half	-32295
.half	456
.half	-32307
.half	-32313
.half	450
.half	320
.half	-32443
.half	-32433
.half	330
.half	-32421
.half	350
.half	340
.half	-32431
.half	-32397
.half	374
.half	380
.half	-32391
.half	360
.half	-32403
.half	-32409
.half	354
.half	-32477
.half	294
.half	300
.half	-32471
.half	312
.half	-32451
.half	-32457
.half	306
.half	272
.half	-32491
.half	-32481
.half	282
.half	-32501
.half	270
.half	260
.half	-32511
.half	-31997
.half	774
.half	780
.half	-31991
.half	792
.half	-31971
.half	-31977
.half	786
.half	816
.half	-31947
.half	-31937
.half	826
.half	-31957
.half	814
.half	804
.half	-31967
.half	864
.half	-31899
.half	-31889
.half	874
.half	-31877
.half	894
.half	884
.half	-31887
.half	-31917
.half	854
.half	860
.half	-31911
.half	840
.half	-31923
.half	-31929
.half	834
.half	960
.half	-31803
.half	-31793
.half	970
.half	-31781
.half	990
.half	980
.half	-31791
.half	-31757
.half	1014
.half	1020
.half	-31751
.half	1000
.half	-31763
.half	-31769
.half	994
.half	-31837
.half	934
.half	940
.half	-31831
.half	952
.half	-31811
.half	-31817
.half	946
.half	912
.half	-31851
.half	-31841
.half	922
.half	-31861
.half	910
.half	900
.half	-31871
.half	640
.half	-32123
.half	-32113
.half	650
.half	-32101
.half	670
.half	660
.half	-32111
.half	-32077
.half	694
.half	700
.half	-32071
.half	680
.half	-32083
.half	-32089
.half	674
.half	-32029
.half	742
.half	748
.half	-32023
.half	760
.half	-32003
.half	-32009
.half	754
.half	720
.half	-32043
.half	-32033
.half	730
.half	-32053
.half	718
.half	708
.half	-32063
.half	-32189
.half	582
.half	588
.half	-32183
.half	600
.half	-32163
.half	-32169
.half	594
.half	624
.half	-32139
.half	-32129
.half	634
.half	-32149
.half	622
.half	612
.half	-32159
.half	544
.half	-32219
.half	-32209
.half	554
.half	-32197
.half	574
.half	564
.half	-32207
.half	-32237
.half	534
.half	540
.half	-32231
.half	520
.half	-32243
.half	-32249
.half	514
.ident	"GCC: (Alpine Linux) 15.2.0"
.section	.note.GNU-stack,"",@progbits
.file	"core_matrix.c"
.option nopic
.text
.align	2
.globl	core_init_matrix
.type	core_init_matrix, @function
core_init_matrix:
  addi	sp,sp,-16
  sw	s1,8(sp)
  sw	s2,4(sp)
  mv	a6,a0
  bne	a2,zero,.L_matrix_2
  li	a2,1
.L_matrix_2:
  addi	t0,a1,-1
  andi	t0,t0,-4
  addi	t2,t0,4
  li	a5,0
  beq	a6,zero,.L_matrix_24
.L_matrix_4:
  mv	a0,a5
  addi	a5,a5,1
  mul	a4,a5,a5
  slli	a4,a4,3
  bgtu	a6,a4,.L_matrix_4
  mul	s1,a0,a0
  slli	s1,s1,2
  add	t0,t2,s1
  beq	a0,zero,.L_matrix_16
  sw	s0,12(sp)
  mv	s2,a0
.L_matrix_6:
  lui	a5,%hi(.LC_matrix_0)
  flw	ft2,%lo(.LC_matrix_0)(a5)
  lui	a5,%hi(.LC_matrix_1)
  flw	fa1,%lo(.LC_matrix_1)(a5)
  lui	a5,%hi(.LC_matrix_2)
  flw	ft1,%lo(.LC_matrix_2)(a5)
  lui	a5,%hi(.LC_matrix_3)
  flw	ft0,%lo(.LC_matrix_3)(a5)
  lui	a5,%hi(.LC_matrix_4)
  flw	fa0,%lo(.LC_matrix_4)(a5)
  addi	t1,a0,1
  slli	s0,a0,2
  mv	t5,t0
  li	t4,0
  li	a5,1
  sub	t3,t2,t0
.L_matrix_8:
  mv	t6,a5
  mv	a1,t5
.L_matrix_13:
  mul	a2,a2,a5
  fcvt.s.w	fa5,a5
  add	a6,t3,a1
  srai	a4,a2,31
  srli	a4,a4,16
  add	a2,a2,a4
  zext.h	a2,a2
  sub	a2,a2,a4
  add	a4,a2,a5
  fcvt.s.w	fa3,a4
  addi	a5,a5,1
  fmul.s	fa4,fa3,ft2
  fabs.s	fa2,fa4
  fcvt.w.s a7,fa4,rtz
  flt.s	a4,fa2,fa1
  fcvt.s.w	fa2,a7
  beq	a4,zero,.L_matrix_10
  fsgnj.s	fa4,fa2,fa4
.L_matrix_10:
  fmul.s	fa4,fa4,ft1
  addi	a1,a1,4
  fsub.s	fa4,fa3,fa4
  fadd.s	fa5,fa5,fa4
  fsw	fa4,-4(a1)
  fmul.s	fa4,fa5,ft0
  fabs.s	fa3,fa4
  fcvt.w.s a7,fa4,rtz
  flt.s	a4,fa3,fa1
  fcvt.s.w	fa3,a7
  beq	a4,zero,.L_matrix_12
  fsgnj.s	fa4,fa3,fa4
.L_matrix_12:
  fmul.s	fa4,fa4,fa0
  fsub.s	fa5,fa5,fa4
  fsw	fa5,0(a6)
  bne	a5,t1,.L_matrix_13
  addi	t4,t4,1
  add	a5,a0,t6
  add	t1,t1,a0
  add	t5,t5,s0
  bne	t4,a0,.L_matrix_8
  lw	s0,12(sp)
.L_matrix_7:
  add	s1,t0,s1
  sw	s1,12(a3)
  sw	s2,0(a3)
  sw	t2,4(a3)
  sw	t0,8(a3)
  lw	s1,8(sp)
  lw	s2,4(sp)
  addi	sp,sp,16
  jr	ra
.L_matrix_24:
  li	s2,-1
  sw	s0,12(sp)
  addi	t0,t0,8
  mv	a0,s2
  li	s1,4
  j	.L_matrix_6
.L_matrix_16:
  li	s1,0
  li	s2,0
  j	.L_matrix_7
.size	core_init_matrix, .-core_init_matrix
.align	2
.globl	matrix_sum
.type	matrix_sum, @function
matrix_sum:
  mv	a7,a0
  beq	a0,zero,.L_matrix_32
  fmv.s.x	fa5,zero
  slli	a6,a0,2
  add	a2,a1,a6
  fmv.s	fa4,fa5
  li	a1,0
  li	a0,0
.L_matrix_31:
  fmv.s	fa3,fa5
  sub	a5,a2,a6
.L_matrix_30:
  flw	fa5,0(a5)
  addi	a3,a0,10
  fadd.s	fa4,fa4,fa5
  flt.s	a4,fa3,fa5
  fmv.s	fa3,fa5
  add	a0,a0,a4
  flt.s	a4,fa0,fa4
  bne	a4,zero,.L_matrix_27
  addi	a5,a5,4
  sext.h	a0,a0
  bne	a2,a5,.L_matrix_30
  addi	a1,a1,1
  add	a2,a2,a6
  bne	a7,a1,.L_matrix_31
  ret
.L_matrix_27:
  addi	a5,a5,4
  fmv.s.x	fa4,zero
  sext.h	a0,a3
  bne	a2,a5,.L_matrix_30
  addi	a1,a1,1
  add	a2,a2,a6
  bne	a7,a1,.L_matrix_31
  ret
.L_matrix_32:
  li	a0,0
  ret
.size	matrix_sum, .-matrix_sum
.align	2
.globl	matrix_mul_const
.type	matrix_mul_const, @function
matrix_mul_const:
  beq	a0,zero,.L_matrix_50
  vsetvli	a5,zero,e32,m1,ta,ma
  vfmv.v.f	v2,fa0
  csrr	t4,vlenb
  addi	t4,t4,-8
  slli	t3,a0,2
  li	t1,0
.L_matrix_38:
  addi	a5,a2,4
  sub	a5,a1,a5
  bleu	a5,t4,.L_matrix_53
  mv	a6,a2
  mv	a3,a1
  mv	a4,a0
.L_matrix_41:
  vsetvli	a5,a4,e32,m1,ta,ma
  vle32.v	v1,0(a6)
  slli	a7,a5,2
  sub	a4,a4,a5
  add	a6,a6,a7
  vfmul.vv	v1,v1,v2
  vse32.v	v1,0(a3)
  add	a3,a3,a7
  bne	a4,zero,.L_matrix_41
  add	a4,t3,a2
.L_matrix_40:
  addi	t1,t1,1
  add	a1,a1,t3
  mv	a2,a4
  bne	a0,t1,.L_matrix_38
.L_matrix_50:
  ret
.L_matrix_53:
  add	a4,t3,a2
  mv	a5,a1
.L_matrix_39:
  flw	fa5,0(a2)
  addi	a2,a2,4
  addi	a5,a5,4
  fmul.s	fa5,fa5,fa0
  fsw	fa5,-4(a5)
  bne	a2,a4,.L_matrix_39
  j	.L_matrix_40
.size	matrix_mul_const, .-matrix_mul_const
.align	2
.globl	matrix_add_const
.type	matrix_add_const, @function
matrix_add_const:
  beq	a0,zero,.L_matrix_62
  vsetvli	a5,zero,e32,m1,ta,ma
  vfmv.v.f	v2,fa0
  slli	t1,a0,2
  li	a7,0
.L_matrix_56:
  mv	a2,a1
  mv	a3,a0
  mv	a4,a1
.L_matrix_57:
  vsetvli	a5,a3,e32,m1,ta,ma
  vle32.v	v1,0(a2)
  slli	a6,a5,2
  sub	a3,a3,a5
  add	a2,a2,a6
  vfadd.vv	v1,v1,v2
  vse32.v	v1,0(a4)
  add	a4,a4,a6
  bne	a3,zero,.L_matrix_57
  addi	a7,a7,1
  add	a1,a1,t1
  bne	a0,a7,.L_matrix_56
.L_matrix_62:
  ret
.size	matrix_add_const, .-matrix_add_const
.align	2
.globl	matrix_mul_vect
.type	matrix_mul_vect, @function
matrix_mul_vect:
  beq	a0,zero,.L_matrix_71
  slli	a6,a0,2
  add	t1,a1,a6
  li	a7,0
  add	a6,a3,a6
.L_matrix_67:
  fmv.s.x	fa4,zero
  slli	a4,a7,2
  add	a4,a2,a4
  fsw	fa4,0(a1)
  mv	a5,a3
.L_matrix_66:
  flw	fa5,0(a4)
  flw	fa3,0(a5)
  addi	a5,a5,4
  addi	a4,a4,4
  fmul.s	fa5,fa5,fa3
  fadd.s	fa4,fa4,fa5
  fsw	fa4,0(a1)
  bne	a6,a5,.L_matrix_66
  addi	a1,a1,4
  add	a7,a7,a0
  bne	t1,a1,.L_matrix_67
.L_matrix_71:
  ret
.size	matrix_mul_vect, .-matrix_mul_vect
.align	2
.globl	matrix_mul_matrix
.type	matrix_mul_matrix, @function
matrix_mul_matrix:
  beq	a0,zero,.L_matrix_82
  slli	a6,a0,2
  mv	t4,a1
  mv	t3,a2
  add	a1,a2,a6
  li	t5,0
.L_matrix_74:
  mv	a7,a3
  mv	a2,t4
  li	t1,0
.L_matrix_76:
  fmv.s.x	fa4,zero
  mv	a4,a7
  mv	a5,t3
  fsw	fa4,0(a2)
.L_matrix_75:
  flw	fa5,0(a5)
  flw	fa3,0(a4)
  addi	a5,a5,4
  add	a4,a4,a6
  fmul.s	fa5,fa5,fa3
  fadd.s	fa4,fa4,fa5
  fsw	fa4,0(a2)
  bne	a1,a5,.L_matrix_75
  addi	a5,t1,1
  addi	a2,a2,4
  addi	a7,a7,4
  beq	a0,a5,.L_matrix_83
  mv	t1,a5
  j	.L_matrix_76
.L_matrix_83:
  add	t3,t3,a6
  add	a1,a1,a6
  add	t4,t4,a6
  beq	t5,t1,.L_matrix_82
  addi	t5,t5,1
  j	.L_matrix_74
.L_matrix_82:
  ret
.size	matrix_mul_matrix, .-matrix_mul_matrix
.align	2
.globl	matrix_mul_matrix_bitextract
.type	matrix_mul_matrix_bitextract, @function
matrix_mul_matrix_bitextract:
  beq	a0,zero,.L_matrix_98
  mv	t6,a2
  lui	t5,%hi(.LC_matrix_5)
  lui	t4,%hi(.LC_matrix_6)
  lui	a7,%hi(.LC_matrix_1)
  lui	a6,%hi(.LC_matrix_7)
  lui	a2,%hi(.LC_matrix_8)
  lui	a4,%hi(.LC_matrix_9)
  lui	a5,%hi(.LC_matrix_10)
  flw	ft7,%lo(.LC_matrix_5)(t5)
  flw	ft6,%lo(.LC_matrix_6)(t4)
  flw	ft1,%lo(.LC_matrix_1)(a7)
  flw	ft5,%lo(.LC_matrix_7)(a6)
  flw	ft4,%lo(.LC_matrix_8)(a2)
  flw	ft3,%lo(.LC_matrix_9)(a4)
  flw	ft2,%lo(.LC_matrix_10)(a5)
  slli	t1,a0,2
  mv	t0,a1
  add	t3,t6,t1
  li	t2,0
.L_matrix_86:
  mv	t4,a3
  mv	a7,t0
  li	t5,0
.L_matrix_92:
  fmv.s.x	ft0,zero
  mv	a4,t4
  mv	a5,t6
  fsw	ft0,0(a7)
.L_matrix_91:
  flw	fa4,0(a4)
  flw	fa5,0(a5)
  add	a4,a4,t1
  addi	a5,a5,4
  fmul.s	fa5,fa5,fa4
  fmul.s	fa3,fa5,ft6
  fmul.s	fa4,fa5,ft3
  fmul.s	fa2,fa5,ft7
  fmul.s	fa5,fa5,ft4
  fabs.s	fa0,fa3
  fcvt.w.s a2,fa3,rtz
  fabs.s	fa1,fa4
  fcvt.w.s a6,fa4,rtz
  flt.s	a1,fa0,ft1
  fcvt.s.w	fa0,a2
  flt.s	a2,fa1,ft1
  fcvt.s.w	fa1,a6
  beq	a1,zero,.L_matrix_88
  fsgnj.s	fa3,fa0,fa3
.L_matrix_88:
  fmul.s	fa3,fa3,ft5
  fsub.s	fa3,fa2,fa3
  beq	a2,zero,.L_matrix_90
  fsgnj.s	fa4,fa1,fa4
.L_matrix_90:
  fmul.s	fa4,fa4,ft2
  fsub.s	fa5,fa5,fa4
  fmul.s	fa5,fa5,fa3
  fadd.s	ft0,ft0,fa5
  fsw	ft0,0(a7)
  bne	t3,a5,.L_matrix_91
  addi	a5,t5,1
  addi	a7,a7,4
  addi	t4,t4,4
  beq	a0,a5,.L_matrix_99
  mv	t5,a5
  j	.L_matrix_92
.L_matrix_99:
  add	t6,t6,t1
  add	t3,t3,t1
  add	t0,t0,t1
  beq	t2,t5,.L_matrix_98
  addi	t2,t2,1
  j	.L_matrix_86
.L_matrix_98:
  ret
.size	matrix_mul_matrix_bitextract, .-matrix_mul_matrix_bitextract
.align	2
.globl	matrix_test
.type	matrix_test, @function
matrix_test:
  addi	sp,sp,-64
  csrr	t0,vlenb
  sw	s4,40(sp)
  sw	s5,36(sp)
  sw	s6,32(sp)
  sw	ra,60(sp)
  sw	s0,56(sp)
  sw	s1,52(sp)
  sw	s2,48(sp)
  sw	s3,44(sp)
  sw	s7,28(sp)
  sw	s8,24(sp)
  sw	s9,20(sp)
  sw	s10,16(sp)
  fsw	fs0,12(sp)
  slli	t1,t0,2
  sub	sp,sp,t1
  mv	s5,a1
  mv	s4,a2
  mv	s6,a3
  beq	a0,zero,.L_matrix_101
  lui	a5,%hi(.LC_matrix_11)
  flw	fs0,%lo(.LC_matrix_11)(a5)
  vsetvli	a5,zero,e32,m1,ta,ma
  vfmv.v.f	v2,fa0
  fadd.s	fs0,fa0,fs0
  mv	s3,a0
  slli	s1,a0,2
  mv	s7,a2
  mv	a1,a2
  li	s8,0
.L_matrix_102:
  mv	a3,a1
  mv	s2,s3
  mv	a4,a1
.L_matrix_103:
  vsetvli	a5,s2,e32,m1,ta,ma
  vle32.v	v1,0(a3)
  slli	a2,a5,2
  sub	s2,s2,a5
  add	a3,a3,a2
  vfadd.vv	v1,v1,v2
  vse32.v	v1,0(a4)
  add	a4,a4,a2
  bne	s2,zero,.L_matrix_103
  addi	s0,s8,1
  add	a1,a1,s1
  beq	s3,s0,.L_matrix_104
  mv	s8,s0
  j	.L_matrix_102
.L_matrix_104:
  mv	a0,s4
  csrr	t1,vlenb
  mv	a6,s5
  addi	a5,a0,4
  addi	t1,t1,-8
  sub	a5,a6,a5
  li	a7,0
  bleu	a5,t1,.L_matrix_178
.L_matrix_110:
  mv	a3,a6
  mv	a2,a0
  mv	a4,s0
.L_matrix_107:
  vsetvli	a5,a4,e32,m1,ta,ma
  vle32.v	v1,0(a2)
  slli	a1,a5,2
  sub	a4,a4,a5
  add	a2,a2,a1
  vfmul.vv	v1,v1,v2
  vse32.v	v1,0(a3)
  add	a3,a3,a1
  bne	a4,zero,.L_matrix_107
  add	a3,a0,s1
  add	a6,a6,s1
  mv	a0,a3
  beq	a7,s8,.L_matrix_179
.L_matrix_141:
  addi	a5,a0,4
  sub	a5,a6,a5
  addi	a7,a7,1
  bgtu	a5,t1,.L_matrix_110
.L_matrix_178:
  add	a3,a0,s1
  mv	a5,a0
  mv	a4,a6
.L_matrix_105:
  flw	fa5,0(a5)
  addi	a5,a5,4
  addi	a4,a4,4
  fmul.s	fa5,fa5,fa0
  fsw	fa5,-4(a4)
  bne	a5,a3,.L_matrix_105
  add	a6,a6,s1
  mv	a0,a3
  bne	a7,s8,.L_matrix_141
.L_matrix_179:
  fmv.s.x	fa5,zero
  add	s9,s5,s1
  mv	a2,s9
  fmv.s	fa4,fa5
  li	a1,0
  li	a0,0
.L_matrix_109:
  fmv.s	fa3,fa5
  sub	a5,a2,s1
.L_matrix_168:
  flw	fa5,0(a5)
  addi	a3,a0,10
  fadd.s	fa4,fa4,fa5
  fgt.s	a4,fa5,fa3
  fmv.s	fa3,fa5
  add	a0,a0,a4
  flt.s	a4,fs0,fa4
  bne	a4,zero,.L_matrix_112
  addi	a5,a5,4
  sext.h	a0,a0
  bne	a5,a2,.L_matrix_168
  add	a2,a2,s1
  beq	a1,s8,.L_matrix_180
.L_matrix_143:
  addi	a1,a1,1
  j	.L_matrix_109
.L_matrix_112:
  addi	a5,a5,4
  fmv.s.x	fa4,zero
  sext.h	a0,a3
  bne	a2,a5,.L_matrix_168
  add	a2,a2,s1
  bne	a1,s8,.L_matrix_143
.L_matrix_180:
  csrr	s10,vlenb
  slli	a5,s10,2
  sub	a5,a5,s10
  add	a5,a5,sp
  li	a1,0
  vs1r.v	v2,0(a5)
  call	crc16
  slli	a5,s10,2
  sub	a5,a5,s10
  add	a5,a5,sp
  vl1re32.v	v2,0(a5)
  mv	a1,a0
  add	a2,s6,s1
  mv	a3,s5
  li	a6,0
.L_matrix_119:
  fmv.s.x	fa4,zero
  slli	a4,a6,2
  add	a4,s4,a4
  fsw	fa4,0(a3)
  mv	a5,s6
.L_matrix_118:
  flw	fa5,0(a4)
  flw	fa3,0(a5)
  addi	a5,a5,4
  addi	a4,a4,4
  fmul.s	fa5,fa5,fa3
  fadd.s	fa4,fa4,fa5
  fsw	fa4,0(a3)
  bne	a2,a5,.L_matrix_118
  addi	a3,a3,4
  add	a6,a6,s0
  bne	s9,a3,.L_matrix_119
  fmv.s.x	fa5,zero
  li	a6,0
  li	a5,0
  fmv.s	fa4,fa5
.L_matrix_124:
  fmv.s	fa3,fa5
  sub	a4,s9,s1
.L_matrix_167:
  flw	fa5,0(a4)
  addi	a2,a5,10
  fadd.s	fa4,fa4,fa5
  fgt.s	a3,fa5,fa3
  fmv.s	fa3,fa5
  add	a5,a5,a3
  flt.s	a3,fs0,fa4
  bne	a3,zero,.L_matrix_120
  addi	a4,a4,4
  sext.h	a5,a5
  bne	a4,s9,.L_matrix_167
  add	s9,s9,s1
  beq	a6,s8,.L_matrix_181
.L_matrix_145:
  addi	a6,a6,1
  j	.L_matrix_124
.L_matrix_120:
  addi	a4,a4,4
  fmv.s.x	fa4,zero
  sext.h	a5,a2
  bne	a4,s9,.L_matrix_167
  add	s9,s9,s1
  bne	a6,s8,.L_matrix_145
.L_matrix_181:
  csrr	s8,vlenb
  slli	a4,s8,2
  sub	a4,a4,s8
  add	a4,a4,sp
  mv	a0,a5
  vs1r.v	v2,0(a4)
  call	crc16
  slli	a5,s8,2
  sub	a5,a5,s8
  add	a5,a5,sp
  vl1re32.v	v2,0(a5)
  mv	a1,a0
  mv	t3,s5
  li	t1,0
.L_matrix_125:
  mv	a2,s6
  mv	a7,t3
  li	a3,0
.L_matrix_127:
  fmv.s.x	fa4,zero
  mv	a6,a2
  mv	a4,s7
  fsw	fa4,0(a7)
  li	a5,0
.L_matrix_126:
  flw	fa5,0(a4)
  flw	fa3,0(a6)
  addi	a5,a5,1
  addi	a4,a4,4
  fmul.s	fa5,fa5,fa3
  add	a6,a6,s1
  fadd.s	fa4,fa4,fa5
  fsw	fa4,0(a7)
  bgtu	s0,a5,.L_matrix_126
  addi	a3,a3,1
  addi	a7,a7,4
  addi	a2,a2,4
  bgtu	s0,a3,.L_matrix_127
  addi	t1,t1,1
  add	s7,s7,s1
  add	t3,t3,s1
  bgtu	s0,t1,.L_matrix_125
  fmv.s.x	fa5,zero
  li	t1,0
  li	a7,0
  fmv.s	fa4,fa5
  li	a5,0
.L_matrix_128:
  slli	a3,t1,2
  fmv.s	fa3,fa5
  add	a3,s5,a3
  li	a4,0
  j	.L_matrix_132
.L_matrix_182:
  addi	a4,a4,1
  sext.h	a5,a5
  bleu	s0,a4,.L_matrix_130
.L_matrix_165:
  fmv.s	fa3,fa5
  addi	a3,a3,4
.L_matrix_132:
  flw	fa5,0(a3)
  addi	a6,a5,10
  fadd.s	fa4,fa4,fa5
  fgt.s	a2,fa5,fa3
  add	a5,a5,a2
  flt.s	a2,fs0,fa4
  beq	a2,zero,.L_matrix_182
  addi	a4,a4,1
  fmv.s.x	fa4,zero
  sext.h	a5,a6
  bgtu	s0,a4,.L_matrix_165
  fmv.s.x	fa4,zero
.L_matrix_130:
  addi	a7,a7,1
  add	t1,t1,s0
  bgtu	s0,a7,.L_matrix_128
  csrr	s8,vlenb
  slli	a4,s8,2
  sub	a4,a4,s8
  add	a4,a4,sp
  mv	a0,a5
  vs1r.v	v2,0(a4)
  call	crc16
  mv	a1,s5
  mv	s0,a0
  mv	a3,s6
  mv	a0,s3
  mv	a2,s4
  call	matrix_mul_matrix_bitextract
  slli	a5,s8,2
  sub	a5,a5,s8
  add	a5,a5,sp
  fmv.s.x	fa5,zero
  vl1re32.v	v2,0(a5)
  add	s5,s5,s1
  fmv.s	fa4,fa5
  li	s7,0
  li	a0,0
.L_matrix_138:
  fmv.s	fa3,fa5
  sub	a5,s5,s1
.L_matrix_166:
  flw	fa5,0(a5)
  addi	a3,a0,10
  fadd.s	fa4,fa4,fa5
  fgt.s	a4,fa5,fa3
  fmv.s	fa3,fa5
  add	a0,a0,a4
  flt.s	a4,fs0,fa4
  bne	a4,zero,.L_matrix_134
  addi	a5,a5,4
  sext.h	a0,a0
  bne	a5,s5,.L_matrix_166
  addi	s6,s7,1
  add	s5,s5,s1
  beq	s3,s6,.L_matrix_183
.L_matrix_149:
  mv	s7,s6
  j	.L_matrix_138
.L_matrix_134:
  addi	a5,a5,4
  fmv.s.x	fa4,zero
  sext.h	a0,a3
  bne	s5,a5,.L_matrix_166
  addi	s6,s7,1
  add	s5,s5,s1
  bne	s3,s6,.L_matrix_149
.L_matrix_183:
  csrr	s3,vlenb
  slli	a5,s3,2
  sub	a5,a5,s3
  add	a5,a5,sp
  mv	a1,s0
  vs1r.v	v2,0(a5)
  call	crc16
  slli	a5,s3,2
  sub	a5,a5,s3
  add	a5,a5,sp
  vl1re32.v	v2,0(a5)
.L_matrix_139:
  mv	a2,s4
  mv	a3,s6
  mv	a4,s4
.L_matrix_140:
  vsetvli	a5,a3,e32,m1,ta,ma
  vle32.v	v1,0(a2)
  slli	a1,a5,2
  sub	a3,a3,a5
  add	a2,a2,a1
  vfsub.vv	v1,v1,v2
  vse32.v	v1,0(a4)
  add	a4,a4,a1
  bne	a3,zero,.L_matrix_140
  add	s4,s4,s1
  beq	s2,s7,.L_matrix_117
  addi	s2,s2,1
  j	.L_matrix_139
.L_matrix_101:
  li	a1,0
  call	crc16
  mv	a1,a0
  li	a0,0
  call	crc16
  mv	a1,a0
  li	a0,0
  call	crc16
  mv	s0,a0
  mv	a1,s5
  li	a0,0
  mv	a3,s6
  mv	a2,s4
  call	matrix_mul_matrix_bitextract
  mv	a1,s0
  li	a0,0
  call	crc16
.L_matrix_117:
  csrr	t0,vlenb
  slli	t1,t0,2
  add	sp,sp,t1
  lw	ra,60(sp)
  lw	s0,56(sp)
  lw	s1,52(sp)
  lw	s2,48(sp)
  lw	s3,44(sp)
  lw	s4,40(sp)
  lw	s5,36(sp)
  lw	s6,32(sp)
  lw	s7,28(sp)
  lw	s8,24(sp)
  lw	s9,20(sp)
  lw	s10,16(sp)
  flw	fs0,12(sp)
  sext.h	a0,a0
  addi	sp,sp,64
  jr	ra
.size	matrix_test, .-matrix_test
.align	2
.globl	core_bench_matrix
.type	core_bench_matrix, @function
core_bench_matrix:
  addi	sp,sp,-16
  sw	s0,8(sp)
  fcvt.s.w	fa0,a1
  mv	s0,a2
  lw	a1,12(a0)
  lw	a3,8(a0)
  lw	a2,4(a0)
  lw	a0,0(a0)
  sw	ra,12(sp)
  call	matrix_test
  mv	a1,s0
  lw	s0,8(sp)
  lw	ra,12(sp)
  addi	sp,sp,16
  tail	crc16
.size	core_bench_matrix, .-core_bench_matrix
.section	.srodata.cst4,"aM",@progbits,4
.align	2
.LC_matrix_0:
.word	931135488
.align	2
.LC_matrix_1:
.word	1258291200
.align	2
.LC_matrix_2:
.word	1199570944
.align	2
.LC_matrix_3:
.word	998244352
.align	2
.LC_matrix_4:
.word	1132462080
.align	2
.LC_matrix_5:
.word	1048576000
.align	2
.LC_matrix_6:
.word	1015021568
.align	2
.LC_matrix_7:
.word	1098907648
.align	2
.LC_matrix_8:
.word	1023410176
.align	2
.LC_matrix_9:
.word	964689920
.align	2
.LC_matrix_10:
.word	1124073472
.align	2
.LC_matrix_11:
.word	1198522368
.ident	"GCC: (Alpine Linux) 15.2.0"
.section	.note.GNU-stack,"",@progbits
