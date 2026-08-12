.file	"core_list_join.c"
.option nopic
.text
.align	2
.globl	calc_func
.type	calc_func, @function
calc_func:
  addi	sp,sp,-32
  sw	s0,24(sp)
  lh	s0,0(a0)
  sw	ra,28(sp)
  srai	a5,s0,7
  andi	a5,a5,1
  beq	a5,zero,.L_list_join_2
  lw	ra,28(sp)
  andi	a0,s0,127
  lw	s0,24(sp)
  addi	sp,sp,32
  jr	ra
.L_list_join_2:
  srai	a4,s0,3
  andi	a4,a4,15
  slli	a5,a4,4
  add	a2,a4,a5
  sw	s1,20(sp)
  sw	s3,12(sp)
  sw	s2,16(sp)
  andi	a3,s0,7
  lhu	a5,56(a1)
  mv	s3,a0
  mv	s1,a1
  mv	a4,a2
  beq	a3,zero,.L_list_join_4
  li	a2,1
  bne	a3,a2,.L_list_join_17
  mv	a2,a5
  mv	a1,a4
  addi	a0,s1,40
  call	core_bench_matrix
  lhu	a5,60(s1)
  sext.h	s2,a0
  bne	a5,zero,.L_list_join_15
  lhu	a5,56(s1)
  sh	a0,60(s1)
.L_list_join_6:
  mv	a1,a5
  call	crcu16
  mv	a5,a0
  andi	s0,s0,-256
  andi	a0,s2,127
  or	s0,a0,s0
  sh	a5,56(s1)
  ori	s0,s0,128
  sh	s0,0(s3)
  lw	ra,28(sp)
  lw	s0,24(sp)
  lw	s1,20(sp)
  lw	s2,16(sp)
  lw	s3,12(sp)
  addi	sp,sp,32
  jr	ra
.L_list_join_17:
  zext.h	a0,s0
  mv	s2,s0
  j	.L_list_join_6
.L_list_join_4:
  li	a3,33
  bgtu	a2,a3,.L_list_join_7
  li	a4,34
.L_list_join_7:
  lh	a3,2(s1)
  lh	a2,0(s1)
  lw	a1,20(s1)
  lw	a0,24(s1)
  call	core_bench_state
  lhu	a5,62(s1)
  sext.h	s2,a0
  bne	a5,zero,.L_list_join_15
  sh	a0,62(s1)
.L_list_join_15:
  lhu	a5,56(s1)
  j	.L_list_join_6
.size	calc_func, .-calc_func
.align	2
.globl	cmp_complex
.type	cmp_complex, @function
cmp_complex:
  addi	sp,sp,-32
  sw	s1,20(sp)
  lh	s1,0(a0)
  sw	s0,24(sp)
  sw	s2,16(sp)
  srai	a5,s1,7
  sw	s4,8(sp)
  sw	ra,28(sp)
  sw	s3,12(sp)
  andi	a5,a5,1
  mv	s2,a1
  mv	s0,a2
  andi	s4,s1,127
  bne	a5,zero,.L_list_join_20
  srai	a4,s1,3
  andi	a4,a4,15
  slli	a5,a4,4
  add	a2,a4,a5
  andi	a3,s1,7
  lhu	a5,56(s0)
  mv	s3,a0
  mv	a4,a2
  beq	a3,zero,.L_list_join_21
  li	a2,1
  bne	a3,a2,.L_list_join_49
  mv	a2,a5
  mv	a1,a4
  addi	a0,s0,40
  call	core_bench_matrix
  lhu	a5,60(s0)
  sext.h	s4,a0
  bne	a5,zero,.L_list_join_47
  lhu	a5,56(s0)
  sh	a0,60(s0)
.L_list_join_23:
  mv	a1,a5
  andi	s4,s4,127
  andi	s1,s1,-256
  call	crcu16
  or	s1,s4,s1
  ori	s1,s1,128
  sh	a0,56(s0)
  sh	s1,0(s3)
.L_list_join_20:
  lh	s1,0(s2)
  srai	a5,s1,7
  andi	a5,a5,1
  andi	s3,s1,127
  bne	a5,zero,.L_list_join_45
  srai	a4,s1,3
  andi	a4,a4,15
  slli	a5,a4,4
  add	a2,a4,a5
  andi	a3,s1,7
  lhu	a5,56(s0)
  mv	a4,a2
  beq	a3,zero,.L_list_join_29
  li	a2,1
  bne	a3,a2,.L_list_join_50
  mv	a2,a5
  mv	a1,a4
  addi	a0,s0,40
  call	core_bench_matrix
  lhu	a5,60(s0)
  sext.h	s3,a0
  bne	a5,zero,.L_list_join_48
  lhu	a5,56(s0)
  sh	a0,60(s0)
.L_list_join_31:
  mv	a1,a5
  andi	s3,s3,127
  andi	s1,s1,-256
  call	crcu16
  or	s1,s3,s1
  ori	s1,s1,128
  sh	a0,56(s0)
  sh	s1,0(s2)
.L_list_join_45:
  lw	ra,28(sp)
  lw	s0,24(sp)
  sub	a0,s4,s3
  lw	s1,20(sp)
  lw	s2,16(sp)
  lw	s3,12(sp)
  lw	s4,8(sp)
  addi	sp,sp,32
  jr	ra
.L_list_join_49:
  zext.h	a0,s1
  mv	s4,s1
  j	.L_list_join_23
.L_list_join_50:
  zext.h	a0,s1
  mv	s3,s1
  j	.L_list_join_31
.L_list_join_29:
  li	a3,33
  bgtu	a2,a3,.L_list_join_32
  li	a4,34
.L_list_join_32:
  lh	a3,2(s0)
  lh	a2,0(s0)
  lw	a1,20(s0)
  lw	a0,24(s0)
  call	core_bench_state
  lhu	a5,62(s0)
  sext.h	s3,a0
  bne	a5,zero,.L_list_join_48
  sh	a0,62(s0)
.L_list_join_48:
  lhu	a5,56(s0)
  j	.L_list_join_31
.L_list_join_21:
  li	a3,33
  bgtu	a2,a3,.L_list_join_24
  li	a4,34
.L_list_join_24:
  lh	a3,2(s0)
  lh	a2,0(s0)
  lw	a1,20(s0)
  lw	a0,24(s0)
  call	core_bench_state
  lhu	a5,62(s0)
  sext.h	s4,a0
  bne	a5,zero,.L_list_join_47
  sh	a0,62(s0)
.L_list_join_47:
  lhu	a5,56(s0)
  j	.L_list_join_23
.size	cmp_complex, .-cmp_complex
.align	2
.globl	cmp_idx
.type	cmp_idx, @function
cmp_idx:
  beq	a2,zero,.L_list_join_53
  lh	a0,2(a0)
  lh	a5,2(a1)
  sub	a0,a0,a5
  ret
.L_list_join_53:
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
  lh	a4,0(a1)
  lh	a5,2(a1)
  sh	a4,0(a0)
  sh	a5,2(a0)
  ret
.size	copy_info, .-copy_info
.align	2
.globl	core_bench_list
.type	core_bench_list, @function
core_bench_list:
  lh	t1,4(a0)
  addi	sp,sp,-64
  sw	s0,56(sp)
  sw	s7,28(sp)
  sw	ra,60(sp)
  sw	s1,52(sp)
  sw	s2,48(sp)
  sw	s3,44(sp)
  sw	s9,20(sp)
  sw	s10,16(sp)
  lw	s0,36(a0)
  mv	s7,a0
  mv	a7,a1
  ble	t1,zero,.L_list_join_144
  mv	s9,a1
  li	a2,0
  li	a1,0
  li	a6,0
  li	a0,0
.L_list_join_73:
  andi	s10,a2,0xff
  blt	s9,zero,.L_list_join_219
  mv	a5,s0
  bne	s0,zero,.L_list_join_60
  j	.L_list_join_220
.L_list_join_64:
  lw	a5,0(a5)
  beq	a5,zero,.L_list_join_63
.L_list_join_60:
  lw	a4,4(a5)
  lh	a4,2(a4)
  bne	s9,a4,.L_list_join_64
.L_list_join_63:
  li	a3,0
  j	.L_list_join_66
.L_list_join_147:
  mv	s0,a4
.L_list_join_66:
  lw	a4,0(s0)
  sw	a3,0(s0)
  mv	a3,s0
  bne	a4,zero,.L_list_join_147
  beq	a5,zero,.L_list_join_221
  lw	a4,4(a5)
  lh	a4,0(a4)
  andi	a3,a4,1
  beq	a3,zero,.L_list_join_69
  srai	a4,a4,9
  andi	a4,a4,1
  add	a4,a0,a4
  zext.h	a0,a4
.L_list_join_69:
  lw	a4,0(a5)
  beq	a4,zero,.L_list_join_70
  lw	a3,0(a4)
  sw	a3,0(a5)
  lw	a5,0(s0)
  sw	a5,0(a4)
  sw	a4,0(s0)
.L_list_join_70:
  addi	a6,a6,1
  zext.h	a6,a6
.L_list_join_68:
  blt	s9,zero,.L_list_join_62
.L_list_join_61:
  addi	a2,a2,1
  addi	s9,s9,1
  sext.h	a2,a2
  sext.h	s9,s9
  bne	t1,a2,.L_list_join_73
.L_list_join_72:
  slli	a5,a6,2
  sub	a5,a5,a1
  add	a0,a0,a5
  zext.h	s2,a0
.L_list_join_56:
  bgt	a7,zero,.L_list_join_222
.L_list_join_83:
  lw	a5,0(s0)
  mv	s1,s0
  lw	s3,0(a5)
  lw	a3,4(a5)
  lw	a2,4(s3)
  lw	a4,0(s3)
  sw	a2,4(a5)
  sw	a3,4(s3)
  sw	a4,0(a5)
  sw	zero,0(s3)
  blt	s9,zero,.L_list_join_223
  bne	s0,zero,.L_list_join_111
  j	.L_list_join_224
.L_list_join_113:
  lw	s1,0(s1)
  beq	s1,zero,.L_list_join_112
.L_list_join_111:
  lw	a5,4(s1)
  lh	a5,2(a5)
  bne	a5,s9,.L_list_join_113
.L_list_join_116:
  lw	a5,4(s0)
  mv	a1,s2
  lh	a0,0(a5)
  call	crc16
  lw	s1,0(s1)
  mv	s2,a0
  bne	s1,zero,.L_list_join_116
.L_list_join_243:
  lw	s1,0(s0)
.L_list_join_105:
  lw	a3,4(s1)
  lw	a4,4(s3)
  lw	a5,0(s1)
  sw	a3,4(s3)
  sw	a4,4(s1)
  sw	a5,0(s3)
  li	t4,1
  sw	s3,0(s1)
  mv	t6,t4
  beq	s0,zero,.L_list_join_117
.L_list_join_231:
  li	s1,0
  li	a3,0
  li	t5,0
.L_list_join_137:
  addi	t5,t5,1
  mv	a2,s0
  li	a5,0
.L_list_join_118:
  bge	a5,t4,.L_list_join_121
  lw	a2,0(a2)
  addi	a5,a5,1
  bne	a2,zero,.L_list_join_118
.L_list_join_121:
  mv	a4,a3
  mv	a1,t4
  mv	a3,s0
  mv	s0,a2
  ble	a5,zero,.L_list_join_225
.L_list_join_132:
  beq	a1,zero,.L_list_join_226
  bne	s0,zero,.L_list_join_122
.L_list_join_218:
  mv	a2,a3
  addi	a5,a5,-1
  lw	a3,0(a3)
  beq	a4,zero,.L_list_join_161
.L_list_join_227:
  sw	a2,0(a4)
  mv	a4,a2
.L_list_join_228:
  bgt	a5,zero,.L_list_join_132
.L_list_join_225:
  ble	a1,zero,.L_list_join_164
  beq	s0,zero,.L_list_join_135
  beq	a5,zero,.L_list_join_134
.L_list_join_122:
  lw	a6,4(a3)
  lw	a0,4(s0)
  lh	a2,0(a6)
  lh	t1,2(a0)
  lh	t3,2(a6)
  zext.h	a7,a2
  srli	a7,a7,8
  andi	a2,a2,-256
  or	a2,a2,a7
  sh	a2,0(a6)
  lh	a2,0(a0)
  zext.h	a6,a2
  srli	a6,a6,8
  andi	a2,a2,-256
  or	a2,a2,a6
  sh	a2,0(a0)
  ble	t3,t1,.L_list_join_218
  mv	a2,s0
  addi	a1,a1,-1
  lw	s0,0(s0)
  bne	a4,zero,.L_list_join_227
.L_list_join_161:
  mv	s1,a2
  mv	a4,a2
  j	.L_list_join_228
.L_list_join_226:
  lw	a2,0(a3)
  addi	a5,a5,-1
  beq	a4,zero,.L_list_join_229
.L_list_join_126:
  sw	a3,0(a4)
  beq	a5,zero,.L_list_join_128
.L_list_join_127:
  mv	a4,a3
  mv	a3,a2
  lw	a2,0(a3)
  addi	a5,a5,-1
  bne	a4,zero,.L_list_join_126
.L_list_join_229:
  mv	s1,a3
  bne	a5,zero,.L_list_join_127
.L_list_join_128:
  bne	s0,zero,.L_list_join_137
.L_list_join_233:
  mv	a4,a3
.L_list_join_135:
  sw	zero,0(a4)
  beq	t5,t6,.L_list_join_230
  mv	s0,s1
  slli	t4,t4,1
  bne	s0,zero,.L_list_join_231
.L_list_join_117:
  sw	zero,0(zero)
.L_list_join_214:
  lw	ra,60(sp)
  lw	s0,56(sp)
  lw	s1,52(sp)
  lw	s3,44(sp)
  lw	s7,28(sp)
  lw	s9,20(sp)
  lw	s10,16(sp)
  mv	a0,s2
  lw	s2,48(sp)
  addi	sp,sp,64
  jr	ra
.L_list_join_159:
  mv	s1,s0
  beq	a1,zero,.L_list_join_162
.L_list_join_232:
  mv	a4,s0
  beq	a5,zero,.L_list_join_135
  mv	s0,a5
.L_list_join_134:
  lw	a5,0(s0)
  addi	a1,a1,-1
  beq	a4,zero,.L_list_join_159
  sw	s0,0(a4)
  bne	a1,zero,.L_list_join_232
.L_list_join_162:
  mv	a3,s0
  mv	s0,a5
  bne	s0,zero,.L_list_join_137
  j	.L_list_join_233
.L_list_join_235:
  lw	a5,0(zero)
  addi	a1,a1,1
  zext.h	a1,a1
  lw	a5,4(a5)
  lb	a5,1(a5)
  andi	a5,a5,1
  add	a5,a0,a5
  zext.h	a0,a5
.L_list_join_62:
  addi	a2,a2,1
  sext.h	a2,a2
  beq	t1,a2,.L_list_join_72
.L_list_join_71:
  andi	s10,a2,0xff
  beq	s0,zero,.L_list_join_74
.L_list_join_79:
  mv	a3,s0
  j	.L_list_join_76
.L_list_join_234:
  lw	a3,0(a3)
  beq	a3,zero,.L_list_join_75
.L_list_join_76:
  lw	a5,4(a3)
  lbu	a5,0(a5)
  bne	a5,s10,.L_list_join_234
.L_list_join_75:
  li	a4,0
  j	.L_list_join_81
.L_list_join_148:
  mv	s0,a5
.L_list_join_81:
  lw	a5,0(s0)
  sw	a4,0(s0)
  mv	a4,s0
  bne	a5,zero,.L_list_join_148
  beq	a3,zero,.L_list_join_74
  lw	a5,4(a3)
  lh	a5,0(a5)
  andi	a4,a5,1
  beq	a4,zero,.L_list_join_77
  srai	a5,a5,9
  andi	a5,a5,1
  add	a5,a0,a5
  zext.h	a0,a5
.L_list_join_77:
  lw	a5,0(a3)
  beq	a5,zero,.L_list_join_80
  lw	a4,0(a5)
  sw	a4,0(a3)
  lw	a4,0(s0)
  sw	a4,0(a5)
  sw	a5,0(s0)
.L_list_join_80:
  addi	a5,a2,1
  addi	a6,a6,1
  sext.h	a2,a5
  zext.h	a6,a6
  zext.h	a5,a5
  ble	t1,a2,.L_list_join_72
  andi	s10,a5,0xff
  j	.L_list_join_79
.L_list_join_74:
  lw	a5,0(s0)
  addi	a2,a2,1
  addi	a1,a1,1
  lw	a5,4(a5)
  sext.h	a2,a2
  zext.h	a1,a1
  lb	a5,1(a5)
  andi	a5,a5,1
  add	a5,a0,a5
  zext.h	a0,a5
  bgt	t1,a2,.L_list_join_71
  slli	a5,a6,2
  sub	a5,a5,a1
  add	a0,a0,a5
  zext.h	s2,a0
  j	.L_list_join_56
.L_list_join_219:
  beq	s0,zero,.L_list_join_235
  mv	a5,s0
  j	.L_list_join_58
.L_list_join_65:
  lw	a5,0(a5)
  beq	a5,zero,.L_list_join_63
.L_list_join_58:
  lw	a4,4(a5)
  lbu	a4,0(a4)
  bne	s10,a4,.L_list_join_65
  j	.L_list_join_63
.L_list_join_221:
  lw	a5,0(s0)
  addi	a1,a1,1
  zext.h	a1,a1
  lw	a5,4(a5)
  lb	a5,1(a5)
  andi	a5,a5,1
  add	a5,a0,a5
  zext.h	a0,a5
  j	.L_list_join_68
.L_list_join_230:
  lw	s0,0(s1)
  beq	s0,zero,.L_list_join_214
.L_list_join_140:
  lw	a5,4(s1)
  mv	a1,s2
  lh	a0,0(a5)
  call	crc16
  lw	s0,0(s0)
  mv	s2,a0
  bne	s0,zero,.L_list_join_140
  j	.L_list_join_214
.L_list_join_222:
  sw	s5,36(sp)
  sw	s4,40(sp)
  sw	s6,32(sp)
  sw	s8,24(sp)
  sw	s11,12(sp)
  li	s5,1
  beq	s0,zero,.L_list_join_84
.L_list_join_241:
  li	s6,0
  li	s4,0
  li	s8,0
.L_list_join_104:
  addi	s8,s8,1
  mv	a4,s0
  li	s1,0
.L_list_join_85:
  bge	s1,s5,.L_list_join_88
  lw	a4,0(a4)
  addi	s1,s1,1
  bne	a4,zero,.L_list_join_85
.L_list_join_88:
  mv	s3,s4
  mv	s11,s5
  mv	s4,s0
  mv	s0,a4
  ble	s1,zero,.L_list_join_236
.L_list_join_99:
  beq	s11,zero,.L_list_join_237
  bne	s0,zero,.L_list_join_89
.L_list_join_217:
  mv	a4,s4
  addi	s1,s1,-1
  lw	s4,0(s4)
  beq	s3,zero,.L_list_join_151
.L_list_join_238:
  sw	a4,0(s3)
  mv	s3,a4
.L_list_join_239:
  bgt	s1,zero,.L_list_join_99
.L_list_join_236:
  ble	s11,zero,.L_list_join_154
  beq	s0,zero,.L_list_join_102
  beq	s1,zero,.L_list_join_101
.L_list_join_89:
  lw	a1,4(s0)
  lw	a0,4(s4)
  mv	a2,s7
  call	cmp_complex
  ble	a0,zero,.L_list_join_217
  mv	a4,s0
  addi	s11,s11,-1
  lw	s0,0(s0)
  bne	s3,zero,.L_list_join_238
.L_list_join_151:
  mv	s6,a4
  mv	s3,a4
  j	.L_list_join_239
.L_list_join_237:
  lw	a5,0(s4)
  addi	s1,s1,-1
  beq	s3,zero,.L_list_join_240
.L_list_join_93:
  sw	s4,0(s3)
  beq	s1,zero,.L_list_join_95
.L_list_join_94:
  mv	s3,s4
  mv	s4,a5
  lw	a5,0(s4)
  addi	s1,s1,-1
  bne	s3,zero,.L_list_join_93
.L_list_join_240:
  mv	s6,s4
  bne	s1,zero,.L_list_join_94
.L_list_join_95:
  bne	s0,zero,.L_list_join_104
.L_list_join_245:
  mv	s3,s4
.L_list_join_102:
  sw	zero,0(s3)
  li	a5,1
  beq	s8,a5,.L_list_join_156
  mv	s0,s6
  slli	s5,s5,1
  bne	s0,zero,.L_list_join_241
.L_list_join_84:
  sw	zero,0(zero)
  bge	s9,zero,.L_list_join_242
  lw	s4,40(sp)
  lw	s5,36(sp)
  lw	s6,32(sp)
  lw	s8,24(sp)
  lw	s11,12(sp)
.L_list_join_106:
  lw	s1,0(zero)
  mv	s3,s0
  li	s0,0
  beq	s1,zero,.L_list_join_105
.L_list_join_246:
  lw	a5,4(s0)
  mv	a1,s2
  lh	a0,0(a5)
  call	crc16
  lw	s1,0(s1)
  mv	s2,a0
  bne	s1,zero,.L_list_join_116
  j	.L_list_join_243
.L_list_join_149:
  mv	s6,s0
  beq	s11,zero,.L_list_join_152
.L_list_join_244:
  mv	s3,s0
  beq	a4,zero,.L_list_join_102
  mv	s0,a4
.L_list_join_101:
  lw	a4,0(s0)
  addi	s11,s11,-1
  beq	s3,zero,.L_list_join_149
  sw	s0,0(s3)
  bne	s11,zero,.L_list_join_244
.L_list_join_152:
  mv	s4,s0
  mv	s0,a4
  bne	s0,zero,.L_list_join_104
  j	.L_list_join_245
.L_list_join_164:
  mv	a3,a4
  bne	s0,zero,.L_list_join_137
  j	.L_list_join_233
.L_list_join_112:
  lw	s1,0(s0)
  bne	s1,zero,.L_list_join_246
  j	.L_list_join_105
.L_list_join_154:
  mv	s4,s3
  bne	s0,zero,.L_list_join_104
  j	.L_list_join_245
.L_list_join_144:
  li	s10,0
  mv	s9,a1
  li	s2,0
  j	.L_list_join_56
.L_list_join_220:
  lw	a5,0(zero)
  addi	a1,a1,1
  zext.h	a1,a1
  lw	a5,4(a5)
  lb	a5,1(a5)
  andi	a5,a5,1
  add	a5,a0,a5
  zext.h	a0,a5
  j	.L_list_join_61
.L_list_join_156:
  mv	s0,s6
  lw	s4,40(sp)
  lw	s5,36(sp)
  lw	s6,32(sp)
  lw	s8,24(sp)
  lw	s11,12(sp)
  j	.L_list_join_83
.L_list_join_224:
  lw	s1,0(zero)
  bne	s1,zero,.L_list_join_246
  j	.L_list_join_105
.L_list_join_223:
  bne	s0,zero,.L_list_join_109
  j	.L_list_join_247
.L_list_join_115:
  lw	s1,0(s1)
  beq	s1,zero,.L_list_join_112
.L_list_join_109:
  lw	a5,4(s1)
  lbu	a5,0(a5)
  bne	s10,a5,.L_list_join_115
  lw	a5,4(s0)
  mv	a1,s2
  lh	a0,0(a5)
  call	crc16
  lw	s1,0(s1)
  mv	s2,a0
  bne	s1,zero,.L_list_join_116
  j	.L_list_join_243
.L_list_join_242:
  lw	s4,40(sp)
  lw	s5,36(sp)
  lw	s6,32(sp)
  lw	s8,24(sp)
  lw	s11,12(sp)
  li	s1,0
  li	s3,0
  j	.L_list_join_105
.L_list_join_247:
  mv	s0,s3
  j	.L_list_join_106
.size	core_bench_list, .-core_bench_list
.align	2
.globl	core_list_init
.type	core_list_init, @function
core_list_init:
  li	a4,-858992640
  addi	a4,a4,-819
  mulhu	a5,a0,a4
  mv	a0,a1
  li	a1,-32768
  sw	zero,0(a0)
  addi	a1,a1,128
  addi	a6,a0,16
  mv	t3,a2
  addi	a3,a0,8
  srli	a5,a5,4
  addi	a4,a5,-2
  slli	a7,a4,3
  add	a7,a0,a7
  sw	a7,4(a0)
  sh	a1,0(a7)
  slli	t5,a4,2
  sh	zero,2(a7)
  add	t5,a7,t5
  addi	a1,a7,4
  li	a5,-32768
  bleu	a7,a6,.L_list_join_284
  addi	a2,a7,8
  bleu	t5,a2,.L_list_join_284
  sw	a3,0(a0)
  sw	a1,12(a0)
  sw	zero,8(a0)
  not	a5,a5
  li	t1,-1
  mv	t4,a3
  sh	a5,6(a7)
  mv	a1,a2
  mv	a3,a6
  sh	t1,4(a7)
.L_list_join_249:
  beq	a4,zero,.L_list_join_250
  li	t6,-32768
  xori	t6,t6,-1
  zext.h	t0,t3
  li	a6,0
.L_list_join_253:
  zext.h	a2,a6
  xor	a5,t0,a2
  slli	a5,a5,3
  andi	a2,a2,7
  andi	a5,a5,120
  or	a5,a5,a2
  slli	a2,a5,8
  add	a5,a5,a2
  addi	a2,a3,8
  addi	t1,a1,4
  bleu	a7,a2,.L_list_join_250
  bgeu	t1,t5,.L_list_join_305
  sw	t4,0(a3)
  sw	a3,0(a0)
  sw	a1,4(a3)
  sh	a5,0(a1)
  sh	t6,2(a1)
  addi	a6,a6,1
  beq	a4,a6,.L_list_join_306
.L_list_join_252:
  mv	t4,a3
  mv	a1,t1
  mv	a3,a2
  j	.L_list_join_253
.L_list_join_306:
  mv	t4,a3
.L_list_join_250:
  lw	a1,0(t4)
  beq	a1,zero,.L_list_join_258
  li	a5,-858992640
  addi	a5,a5,-819
  mulhu	a4,a4,a5
  li	t1,16384
  addi	t1,t1,-1
  li	a5,512
  li	a3,1
  srli	a2,a4,2
  j	.L_list_join_259
.L_list_join_307:
  sh	a3,2(a7)
  zext.h	a5,t5
  addi	a3,a3,1
  beq	a6,zero,.L_list_join_258
.L_list_join_257:
  mv	t4,a1
  mv	a1,a6
.L_list_join_259:
  xor	a6,t3,a3
  andi	a4,a5,1792
  or	a4,a4,a6
  lw	a7,4(t4)
  lw	a6,0(a1)
  and	a4,a4,t1
  addi	t5,a5,256
  bgtu	a2,a3,.L_list_join_307
  sh	a4,2(a7)
  addi	a3,a3,1
  zext.h	a5,t5
  bne	a6,zero,.L_list_join_257
.L_list_join_258:
  li	t5,1
  mv	t2,t5
  beq	a0,zero,.L_list_join_260
.L_list_join_314:
  li	t6,0
  li	a3,0
  li	t0,0
.L_list_join_280:
  addi	t0,t0,1
  mv	a2,a0
  li	a5,0
.L_list_join_261:
  bge	a5,t5,.L_list_join_264
  lw	a2,0(a2)
  addi	a5,a5,1
  bne	a2,zero,.L_list_join_261
.L_list_join_264:
  mv	a4,a3
  mv	a1,t5
  mv	a3,a0
  mv	a0,a2
  ble	a5,zero,.L_list_join_308
.L_list_join_275:
  beq	a1,zero,.L_list_join_309
  bne	a0,zero,.L_list_join_265
.L_list_join_304:
  mv	a2,a3
  addi	a5,a5,-1
  lw	a3,0(a3)
  beq	a4,zero,.L_list_join_289
.L_list_join_310:
  sw	a2,0(a4)
  mv	a4,a2
.L_list_join_311:
  bgt	a5,zero,.L_list_join_275
.L_list_join_308:
  ble	a1,zero,.L_list_join_292
  beq	a0,zero,.L_list_join_278
  beq	a5,zero,.L_list_join_277
.L_list_join_265:
  lw	a7,4(a3)
  lw	a6,4(a0)
  lh	a2,0(a7)
  lh	t3,2(a6)
  lh	t4,2(a7)
  zext.h	t1,a2
  srli	t1,t1,8
  andi	a2,a2,-256
  or	a2,a2,t1
  sh	a2,0(a7)
  lh	a2,0(a6)
  zext.h	a7,a2
  srli	a7,a7,8
  andi	a2,a2,-256
  or	a2,a2,a7
  sh	a2,0(a6)
  ble	t4,t3,.L_list_join_304
  mv	a2,a0
  addi	a1,a1,-1
  lw	a0,0(a0)
  bne	a4,zero,.L_list_join_310
.L_list_join_289:
  mv	t6,a2
  mv	a4,a2
  j	.L_list_join_311
.L_list_join_309:
  lw	a2,0(a3)
  addi	a5,a5,-1
  beq	a4,zero,.L_list_join_312
.L_list_join_269:
  sw	a3,0(a4)
  beq	a5,zero,.L_list_join_271
.L_list_join_270:
  mv	a4,a3
  mv	a3,a2
  lw	a2,0(a3)
  addi	a5,a5,-1
  bne	a4,zero,.L_list_join_269
.L_list_join_312:
  mv	t6,a3
  bne	a5,zero,.L_list_join_270
.L_list_join_271:
  bne	a0,zero,.L_list_join_280
.L_list_join_316:
  mv	a4,a3
.L_list_join_278:
  sw	zero,0(a4)
  beq	t0,t2,.L_list_join_313
  mv	a0,t6
  slli	t5,t5,1
  bne	a0,zero,.L_list_join_314
.L_list_join_260:
  sw	zero,0(zero)
  ret
.L_list_join_287:
  mv	t6,a0
  beq	a1,zero,.L_list_join_290
.L_list_join_315:
  mv	a4,a0
  beq	a5,zero,.L_list_join_278
  mv	a0,a5
.L_list_join_277:
  lw	a5,0(a0)
  addi	a1,a1,-1
  beq	a4,zero,.L_list_join_287
  sw	a0,0(a4)
  bne	a1,zero,.L_list_join_315
.L_list_join_290:
  mv	a3,a0
  mv	a0,a5
  bne	a0,zero,.L_list_join_280
  j	.L_list_join_316
.L_list_join_292:
  mv	a3,a4
  bne	a0,zero,.L_list_join_280
  j	.L_list_join_316
.L_list_join_284:
  li	t4,0
  j	.L_list_join_249
.L_list_join_313:
  mv	a0,t6
  ret
.L_list_join_305:
  addi	a6,a6,1
  beq	a6,a4,.L_list_join_250
  mv	a2,a3
  mv	t1,a1
  mv	a3,t4
  j	.L_list_join_252
.size	core_list_init, .-core_list_init
.align	2
.globl	core_list_insert_new
.type	core_list_insert_new, @function
core_list_insert_new:
  lw	a6,0(a2)
  addi	a7,a6,8
  bgeu	a7,a4,.L_list_join_320
  lw	a4,0(a3)
  addi	t1,a4,4
  bleu	a5,t1,.L_list_join_320
  sw	a7,0(a2)
  lw	a5,0(a0)
  lh	a7,0(a1)
  lh	a2,2(a1)
  sw	a5,0(a6)
  sw	a6,0(a0)
  sw	a4,4(a6)
  lw	a5,0(a3)
  mv	a0,a6
  addi	a5,a5,4
  sw	a5,0(a3)
  lw	a5,4(a6)
  sh	a7,0(a5)
  sh	a2,2(a5)
  ret
.L_list_join_320:
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
  lw	a2,4(a1)
  lw	a3,4(a0)
  lw	a4,0(a1)
  sw	a2,4(a0)
  sw	a3,4(a1)
  sw	a4,0(a0)
  sw	a0,0(a1)
  ret
.size	core_list_undo_remove, .-core_list_undo_remove
.align	2
.globl	core_list_find
.type	core_list_find, @function
core_list_find:
  lh	a4,2(a1)
  blt	a4,zero,.L_list_join_338
  bne	a0,zero,.L_list_join_328
  ret
.L_list_join_329:
  lw	a0,0(a0)
  beq	a0,zero,.L_list_join_332
.L_list_join_328:
  lw	a5,4(a0)
  lh	a5,2(a5)
  bne	a5,a4,.L_list_join_329
  ret
.L_list_join_338:
  beq	a0,zero,.L_list_join_332
  lh	a4,0(a1)
  j	.L_list_join_327
.L_list_join_330:
  lw	a0,0(a0)
  beq	a0,zero,.L_list_join_337
.L_list_join_327:
  lw	a5,4(a0)
  lbu	a5,0(a5)
  bne	a5,a4,.L_list_join_330
  ret
.L_list_join_332:
  li	a0,0
.L_list_join_337:
  ret
.size	core_list_find, .-core_list_find
.align	2
.globl	core_list_reverse
.type	core_list_reverse, @function
core_list_reverse:
  beq	a0,zero,.L_list_join_346
  li	a4,0
  j	.L_list_join_341
.L_list_join_342:
  mv	a0,a5
.L_list_join_341:
  lw	a5,0(a0)
  sw	a4,0(a0)
  mv	a4,a0
  bne	a5,zero,.L_list_join_342
.L_list_join_346:
  ret
.size	core_list_reverse, .-core_list_reverse
.align	2
.globl	core_list_mergesort
.type	core_list_mergesort, @function
core_list_mergesort:
  addi	sp,sp,-48
  sw	s3,28(sp)
  sw	s5,20(sp)
  sw	s7,12(sp)
  sw	s8,8(sp)
  sw	s10,0(sp)
  li	s5,1
  sw	ra,44(sp)
  sw	s0,40(sp)
  sw	s1,36(sp)
  sw	s2,32(sp)
  sw	s4,24(sp)
  sw	s6,16(sp)
  sw	s9,4(sp)
  mv	s3,a0
  mv	s8,a1
  mv	s7,a2
  mv	s10,s5
  beq	s3,zero,.L_list_join_348
.L_list_join_391:
  li	s9,0
  li	s2,0
  li	s6,0
.L_list_join_368:
  addi	s9,s9,1
  mv	a5,s3
  li	s0,0
.L_list_join_349:
  ble	s5,s0,.L_list_join_352
  lw	a5,0(a5)
  addi	s0,s0,1
  bne	a5,zero,.L_list_join_349
.L_list_join_352:
  mv	s1,s2
  mv	s4,s5
  mv	s2,s3
  mv	s3,a5
  ble	s0,zero,.L_list_join_385
.L_list_join_363:
  beq	s4,zero,.L_list_join_386
  bne	s3,zero,.L_list_join_353
.L_list_join_384:
  mv	a5,s2
  addi	s0,s0,-1
  lw	s2,0(s2)
  beq	s1,zero,.L_list_join_374
.L_list_join_387:
  sw	a5,0(s1)
  mv	s1,a5
.L_list_join_388:
  bgt	s0,zero,.L_list_join_363
.L_list_join_385:
  ble	s4,zero,.L_list_join_377
  beq	s3,zero,.L_list_join_366
  beq	s0,zero,.L_list_join_365
.L_list_join_353:
  lw	a1,4(s3)
  lw	a0,4(s2)
  mv	a2,s7
  jalr	s8
  ble	a0,zero,.L_list_join_384
  mv	a5,s3
  addi	s4,s4,-1
  lw	s3,0(s3)
  bne	s1,zero,.L_list_join_387
.L_list_join_374:
  mv	s6,a5
  mv	s1,a5
  j	.L_list_join_388
.L_list_join_386:
  lw	a5,0(s2)
  addi	s0,s0,-1
  beq	s1,zero,.L_list_join_357
.L_list_join_389:
  sw	s2,0(s1)
  beq	s0,zero,.L_list_join_359
.L_list_join_358:
  mv	s1,s2
  mv	s2,a5
  lw	a5,0(s2)
  addi	s0,s0,-1
  bne	s1,zero,.L_list_join_389
.L_list_join_357:
  mv	s6,s2
  bne	s0,zero,.L_list_join_358
.L_list_join_359:
  bne	s3,zero,.L_list_join_368
.L_list_join_393:
  mv	s1,s2
.L_list_join_366:
  sw	zero,0(s1)
  beq	s9,s10,.L_list_join_390
  mv	s3,s6
  slli	s5,s5,1
  bne	s3,zero,.L_list_join_391
.L_list_join_348:
  sw	zero,0(zero)
.L_list_join_382:
  lw	ra,44(sp)
  lw	s0,40(sp)
  lw	s1,36(sp)
  lw	s2,32(sp)
  lw	s4,24(sp)
  lw	s5,20(sp)
  lw	s6,16(sp)
  lw	s7,12(sp)
  lw	s8,8(sp)
  lw	s9,4(sp)
  lw	s10,0(sp)
  mv	a0,s3
  lw	s3,28(sp)
  addi	sp,sp,48
  jr	ra
.L_list_join_372:
  mv	s6,s3
  beq	s4,zero,.L_list_join_375
.L_list_join_392:
  mv	s1,s3
  beq	a5,zero,.L_list_join_366
  mv	s3,a5
.L_list_join_365:
  lw	a5,0(s3)
  addi	s4,s4,-1
  beq	s1,zero,.L_list_join_372
  sw	s3,0(s1)
  bne	s4,zero,.L_list_join_392
.L_list_join_375:
  mv	s2,s3
  mv	s3,a5
  bne	s3,zero,.L_list_join_368
  j	.L_list_join_393
.L_list_join_377:
  mv	s2,s1
  bne	s3,zero,.L_list_join_368
  j	.L_list_join_393
.L_list_join_390:
  mv	s3,s6
  j	.L_list_join_382
.size	core_list_mergesort, .-core_list_mergesort
.ident	"GCC: (Alpine Linux) 14.1.0"
.section	.note.GNU-stack,"",@progbits
.file	"core_state.c"
.option nopic
.text
.align	2
.globl	core_init_state
.type	core_init_state, @function
core_init_state:
  addi	t1,a0,-1
  li	t4,1
  bleu	t1,t4,.L_state_13
  add	a1,a1,t4
  zext.h	a1,a1
  lui	a7,%hi(.LANCHOR_state_0)
  srli	a5,a1,3
  li	t3,7
  andi	a4,a1,7
  addi	a7,a7,%lo(.LANCHOR_state_0)
  li	a3,0
  li	a6,4
  li	t5,44
  andi	a5,a5,3
  beq	a4,t3,.L_state_4
.L_state_18:
  bgtu	a4,a6,.L_state_5
  addi	a4,a4,-3
  slli	a5,a5,2
  zext.h	a4,a4
  add	a5,a7,a5
  bgtu	a4,t4,.L_state_17
  lw	a4,16(a5)
  li	t6,8
.L_state_7:
  addi	a5,a3,1
  add	t0,t6,a5
  bgeu	t0,t1,.L_state_8
.L_state_19:
  lbu	t2,0(a4)
  add	a3,a2,a3
  add	a5,a2,a5
  sb	t2,0(a3)
  lbu	t2,1(a4)
  sb	t2,0(a5)
  lbu	a5,2(a4)
  sb	a5,2(a3)
  lbu	a5,3(a4)
  sb	a5,3(a3)
  beq	t6,a6,.L_state_10
  lbu	a5,4(a4)
  sb	a5,4(a3)
  lbu	a5,5(a4)
  sb	a5,5(a3)
  lbu	a5,6(a4)
  sb	a5,6(a3)
  lbu	a5,7(a4)
  sb	a5,7(a3)
.L_state_10:
  addi	a1,a1,1
  add	a3,a3,t6
  zext.h	a1,a1
  sb	t5,0(a3)
  srli	a5,a1,3
  andi	a4,a1,7
  mv	a3,t0
  andi	a5,a5,3
  bne	a4,t3,.L_state_18
.L_state_4:
  slli	a5,a5,2
  add	a5,a7,a5
  lw	a4,48(a5)
  li	t6,8
  addi	a5,a3,1
  add	t0,t6,a5
  bltu	t0,t1,.L_state_19
.L_state_8:
  bltu	a3,a0,.L_state_11
  ret
.L_state_20:
  addi	a5,a5,1
.L_state_11:
  add	a3,a2,a3
  sb	zero,0(a3)
  mv	a3,a5
  bgtu	a0,a5,.L_state_20
  ret
.L_state_17:
  lw	a4,0(a5)
  li	t6,4
  j	.L_state_7
.L_state_5:
  slli	a5,a5,2
  add	a5,a7,a5
  lw	a4,32(a5)
  li	t6,8
  j	.L_state_7
.L_state_13:
  mv	a5,t4
  li	a3,0
  j	.L_state_11
.size	core_init_state, .-core_init_state
.align	2
.globl	core_state_transition
.type	core_state_transition, @function
core_state_transition:
  lw	a3,0(a0)
  mv	a2,a0
  lbu	a5,0(a3)
  beq	a5,zero,.L_state_46
  li	a4,44
  li	a0,0
  beq	a5,a4,.L_state_28
  li	a6,46
  beq	a5,a6,.L_state_24
  bgtu	a5,a6,.L_state_25
  addi	a5,a5,-43
  andi	a5,a5,253
  beq	a5,zero,.L_state_71
.L_state_27:
  lw	a4,4(a1)
  lw	a5,0(a1)
  addi	a3,a3,1
  addi	a4,a4,1
  addi	a5,a5,1
  sw	a4,4(a1)
  sw	a5,0(a1)
  li	a0,1
.L_state_22:
  sw	a3,0(a2)
  ret
.L_state_71:
  lw	a0,0(a1)
  addi	a5,a3,1
  addi	a0,a0,1
  sw	a0,0(a1)
  lbu	a7,1(a3)
  beq	a7,zero,.L_state_51
  beq	a7,a4,.L_state_52
  lw	a0,8(a1)
  addi	a4,a7,-48
  andi	a4,a4,0xff
  li	t1,9
  addi	a0,a0,1
  bleu	a4,t1,.L_state_31
  beq	a7,a6,.L_state_72
  sw	a0,8(a1)
  addi	a3,a3,2
  li	a0,1
  sw	a3,0(a2)
  ret
.L_state_31:
  sw	a0,8(a1)
.L_state_36:
  lbu	a3,1(a5)
  addi	a5,a5,1
  beq	a3,zero,.L_state_53
  li	a4,44
.L_state_68:
  beq	a3,a4,.L_state_73
  li	a4,46
  beq	a3,a4,.L_state_74
  addi	a3,a3,-48
  andi	a3,a3,0xff
  li	a4,9
  bleu	a3,a4,.L_state_36
  lw	a4,16(a1)
  li	a0,1
  addi	a3,a5,1
  add	a5,a4,a0
  sw	a5,16(a1)
  sw	a3,0(a2)
  ret
.L_state_74:
  lw	a4,16(a1)
  addi	a4,a4,1
  sw	a4,16(a1)
.L_state_34:
  lbu	a3,1(a5)
  addi	a5,a5,1
  beq	a3,zero,.L_state_57
  li	a4,44
.L_state_70:
  beq	a3,a4,.L_state_75
  andi	a4,a3,223
  li	a0,69
  beq	a4,a0,.L_state_76
  addi	a3,a3,-48
  andi	a3,a3,0xff
  li	a4,9
  bleu	a3,a4,.L_state_34
  lw	a4,20(a1)
  li	a0,1
  addi	a3,a5,1
  add	a5,a4,a0
  sw	a5,20(a1)
  sw	a3,0(a2)
  ret
.L_state_76:
  lw	a4,20(a1)
  addi	a3,a5,1
  addi	a4,a4,1
  sw	a4,20(a1)
  lbu	a4,1(a5)
  beq	a4,zero,.L_state_55
  li	a0,44
  beq	a4,a0,.L_state_77
  lw	a3,12(a1)
  addi	a4,a4,-43
  andi	a4,a4,253
  addi	a3,a3,1
  sw	a3,12(a1)
  beq	a4,zero,.L_state_78
  addi	a3,a5,2
  li	a0,1
  sw	a3,0(a2)
  ret
.L_state_78:
  lbu	a4,2(a5)
  addi	a3,a5,2
  beq	a4,zero,.L_state_59
  beq	a4,a0,.L_state_79
  lw	a6,24(a1)
  addi	a4,a4,-48
  andi	a4,a4,0xff
  addi	a6,a6,1
  li	a0,9
  sw	a6,24(a1)
  bleu	a4,a0,.L_state_43
  addi	a3,a5,3
  li	a0,1
  j	.L_state_22
.L_state_43:
  mv	a6,a3
  lbu	a4,1(a3)
  addi	a3,a3,1
  li	a7,44
  addi	a5,a4,-48
  andi	a5,a5,0xff
  beq	a4,zero,.L_state_61
  beq	a4,a7,.L_state_80
  bleu	a5,a0,.L_state_43
  lw	a5,4(a1)
  li	a0,1
  addi	a3,a6,2
  add	a5,a5,a0
  sw	a5,4(a1)
  j	.L_state_22
.L_state_75:
  mv	a3,a5
  li	a0,5
.L_state_28:
  addi	a3,a3,1
  j	.L_state_22
.L_state_25:
  addi	a5,a5,-48
  andi	a5,a5,0xff
  li	a0,9
  bgtu	a5,a0,.L_state_27
  lw	a0,0(a1)
  addi	a5,a3,1
  addi	a0,a0,1
  sw	a0,0(a1)
  lbu	a3,1(a3)
  bne	a3,zero,.L_state_68
.L_state_53:
  mv	a3,a5
  li	a0,4
  j	.L_state_22
.L_state_24:
  lw	a0,0(a1)
  addi	a5,a3,1
  addi	a0,a0,1
  sw	a0,0(a1)
  lbu	a3,1(a3)
  bne	a3,zero,.L_state_70
.L_state_57:
  mv	a3,a5
  li	a0,5
  j	.L_state_22
.L_state_72:
  sw	a0,8(a1)
  j	.L_state_34
.L_state_52:
  mv	a3,a5
  li	a0,2
  addi	a3,a3,1
  j	.L_state_22
.L_state_46:
  li	a0,0
  j	.L_state_22
.L_state_59:
  li	a0,6
  j	.L_state_22
.L_state_55:
  li	a0,3
  j	.L_state_22
.L_state_79:
  li	a0,6
  addi	a3,a3,1
  j	.L_state_22
.L_state_77:
  li	a0,3
  addi	a3,a3,1
  j	.L_state_22
.L_state_73:
  mv	a3,a5
  li	a0,4
  addi	a3,a3,1
  j	.L_state_22
.L_state_80:
  li	a0,7
  addi	a3,a3,1
  j	.L_state_22
.L_state_61:
  li	a0,7
  j	.L_state_22
.L_state_51:
  mv	a3,a5
  li	a0,2
  j	.L_state_22
.size	core_state_transition, .-core_state_transition
.align	2
.globl	core_bench_state
.type	core_bench_state, @function
core_bench_state:
  addi	sp,sp,-128
  sw	s0,120(sp)
  sw	s1,116(sp)
  sw	s2,112(sp)
  sw	s3,108(sp)
  sw	s4,104(sp)
  sw	s5,100(sp)
  sw	s6,96(sp)
  sw	s7,92(sp)
  sw	s8,88(sp)
  sw	ra,124(sp)
  sw	s9,84(sp)
  lbu	s9,0(a1)
  sw	a1,12(sp)
  sw	zero,48(sp)
  sw	zero,16(sp)
  sw	zero,52(sp)
  sw	zero,20(sp)
  sw	zero,56(sp)
  sw	zero,24(sp)
  sw	zero,60(sp)
  sw	zero,28(sp)
  sw	zero,64(sp)
  sw	zero,32(sp)
  sw	zero,68(sp)
  sw	zero,36(sp)
  sw	zero,72(sp)
  sw	zero,40(sp)
  sw	zero,76(sp)
  sw	zero,44(sp)
  mv	s0,a1
  mv	s7,a0
  mv	s8,a2
  mv	s5,a3
  mv	s6,a4
  mv	s2,a5
  addi	s3,sp,48
  addi	s1,sp,16
  addi	s4,sp,12
  beq	s9,zero,.L_state_121
.L_state_82:
  mv	a1,s3
  mv	a0,s4
  call	core_state_transition
  slli	a6,a0,2
  add	a6,a6,s1
  lw	a4,12(sp)
  lw	a5,0(a6)
  lbu	a4,0(a4)
  addi	a5,a5,1
  sw	a5,0(a6)
  bne	a4,zero,.L_state_82
  sw	s0,12(sp)
  add	s7,s0,s7
  bgeu	s0,s7,.L_state_98
.L_state_83:
  li	a4,1
  mv	a5,s0
  li	a3,44
  beq	s6,a4,.L_state_93
  j	.L_state_90
.L_state_122:
  lbu	s9,0(a5)
.L_state_93:
  xor	a4,s9,s8
  beq	s9,a3,.L_state_91
  sb	a4,0(a5)
.L_state_91:
  addi	a5,a5,1
  bgtu	s7,a5,.L_state_122
  lbu	a5,0(s0)
  addi	s4,sp,12
  beq	a5,zero,.L_state_123
.L_state_98:
  mv	a1,s3
  mv	a0,s4
  call	core_state_transition
  slli	a2,a0,2
  add	a2,a2,s1
  lw	a4,12(sp)
  lw	a5,0(a2)
  lbu	a4,0(a4)
  addi	a5,a5,1
  sw	a5,0(a2)
  bne	a4,zero,.L_state_98
  sw	s0,12(sp)
  bgeu	s0,s7,.L_state_100
  li	a5,1
  bne	s6,a5,.L_state_95
.L_state_96:
  li	a3,44
.L_state_104:
  lbu	a5,0(s0)
  xor	a4,a5,s5
  beq	a5,a3,.L_state_103
  sb	a4,0(s0)
.L_state_103:
  addi	s0,s0,1
  bgtu	s7,s0,.L_state_104
.L_state_100:
  mv	s0,s3
.L_state_105:
  lw	a0,0(s1)
  mv	a1,s2
  addi	s1,s1,4
  call	crcu32
  mv	a1,a0
  lw	a0,0(s0)
  addi	s0,s0,4
  call	crcu32
  mv	s2,a0
  bne	s1,s3,.L_state_105
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
  lw	s9,84(sp)
  addi	sp,sp,128
  jr	ra
.L_state_124:
  lbu	s9,0(a5)
.L_state_90:
  xor	a4,s9,s8
  beq	s9,a3,.L_state_88
  sb	a4,0(a5)
.L_state_88:
  add	a5,a5,s6
  bltu	a5,s7,.L_state_124
  lbu	a5,0(s0)
  addi	s4,sp,12
  bne	a5,zero,.L_state_98
  sw	s0,12(sp)
  bgeu	s0,s7,.L_state_100
.L_state_95:
  li	a3,44
.L_state_102:
  lbu	a5,0(s0)
  xor	a4,a5,s5
  beq	a5,a3,.L_state_101
  sb	a4,0(s0)
.L_state_101:
  add	s0,s0,s6
  bgtu	s7,s0,.L_state_102
  j	.L_state_100
.L_state_123:
  sw	s0,12(sp)
  bltu	s0,s7,.L_state_96
  j	.L_state_100
.L_state_121:
  add	s7,a1,a0
  bltu	a1,s7,.L_state_83
  j	.L_state_100
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
.ident	"GCC: (Alpine Linux) 14.1.0"
.section	.note.GNU-stack,"",@progbits
.file	"core_main.c"
.option nopic
.text
.align	2
.globl	iterate
.type	iterate, @function
iterate:
  addi	sp,sp,-16
  sw	s2,0(sp)
  lw	s2,28(a0)
  sw	ra,12(sp)
  sw	zero,56(a0)
  sw	zero,60(a0)
  beq	s2,zero,.L_main_8
  li	a1,1
  sw	s0,8(sp)
  sw	s1,4(sp)
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
  sh	a0,56(s0)
  sh	a0,58(s0)
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
  lw	s0,8(sp)
  lw	s1,4(sp)
.L_main_8:
  lw	ra,12(sp)
  lw	s2,0(sp)
  li	a0,0
  addi	sp,sp,16
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
  sw	s11,124(sp)
  sw	zero,36(sp)
  call	portable_init
  li	a0,1
  call	get_seed_32
  mv	a5,a0
  li	a0,2
  sh	a5,44(sp)
  call	get_seed_32
  mv	a5,a0
  li	a0,3
  sh	a5,46(sp)
  call	get_seed_32
  mv	a5,a0
  li	a0,4
  sh	a5,48(sp)
  call	get_seed_32
  mv	a5,a0
  li	a0,5
  sw	a5,72(sp)
  call	get_seed_32
  bne	a0,zero,.L_main_13
  li	a0,7
.L_main_13:
  lw	a5,44(sp)
  sw	a0,76(sp)
  bne	a5,zero,.L_main_14
  lh	a5,48(sp)
  beq	a5,zero,.L_main_116
.L_main_15:
  lui	a5,%hi(.LANCHOR_main_0)
  addi	a5,a5,%lo(.LANCHOR_main_0)
  andi	a1,a0,1
  andi	a4,a0,2
  sh	zero,108(sp)
  sw	a5,52(sp)
  mv	a3,a1
  andi	a0,a0,4
  bne	a4,zero,.L_main_16
  beq	a0,zero,.L_main_117
  addi	a2,a1,1
  zext.h	a2,a2
  li	a4,2000
  divu	a4,a4,a2
  sw	a4,68(sp)
  bne	a1,zero,.L_main_118
.L_main_24:
  mul	a3,a3,a4
  add	a5,a5,a3
  sw	a5,64(sp)
.L_main_19:
  lw	a5,76(sp)
  andi	a4,a5,1
  beq	a4,zero,.L_main_27
  lh	a2,44(sp)
  lw	a1,56(sp)
  lw	a0,68(sp)
  call	core_list_init
  lw	a5,76(sp)
  sw	a0,80(sp)
.L_main_27:
  andi	a4,a5,2
  bne	a4,zero,.L_main_119
  andi	a5,a5,4
  beq	a5,zero,.L_main_31
.L_main_29:
  lw	a2,64(sp)
  lh	a1,44(sp)
  lw	a0,68(sp)
  call	core_init_state
.L_main_31:
  lw	a5,72(sp)
  bne	a5,zero,.L_main_32
  lui	a5,%hi(.LC_main_0)
  lw	s0,%lo(.LC_main_0)(a5)
  lw	s1,%lo(.LC_main_0+4)(a5)
  li	s6,1
  sw	s6,72(sp)
.L_main_34:
  lw	a4,72(sp)
  slli	a5,a4,2
  add	a5,a5,a4
  slli	a5,a5,1
  sw	a5,72(sp)
  call	start_time
  lw	s5,72(sp)
  sw	zero,100(sp)
  sw	zero,104(sp)
  beq	s5,zero,.L_main_37
  li	a1,1
  addi	a0,sp,44
  call	core_bench_list
  lhu	a1,100(sp)
  li	s4,1
  call	crcu16
  mv	a5,a0
  li	a1,-1
  addi	a0,sp,44
  sh	a5,100(sp)
  call	core_bench_list
  lhu	a1,100(sp)
  call	crcu16
  sh	a0,100(sp)
  sh	a0,102(sp)
  beq	s5,s6,.L_main_37
.L_main_36:
  li	a1,1
  addi	a0,sp,44
  call	core_bench_list
  lhu	a1,100(sp)
  addi	s4,s4,1
  call	crcu16
  mv	a5,a0
  li	a1,-1
  addi	a0,sp,44
  sh	a5,100(sp)
  call	core_bench_list
  lhu	a1,100(sp)
  call	crcu16
  sh	a0,100(sp)
  bne	s5,s4,.L_main_36
.L_main_37:
  call	stop_time
  call	get_time
  call	time_in_secs
  mv	a2,s0
  mv	a3,s1
  mv	s5,a0
  mv	s4,a1
  call	__ltdf2
  blt	a0,zero,.L_main_34
  mv	a0,s5
  mv	a1,s4
  call	__fixunsdfsi
  li	a5,1
  maxu	a4,a0,a5
  li	a5,10
  divu	a5,a5,a4
  lw	a4,72(sp)
  addi	a5,a5,1
  mul	a5,a4,a5
  sw	a5,72(sp)
.L_main_32:
  call	start_time
  addi	a0,sp,44
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
  beq	a0,a5,.L_main_39
  bgtu	a0,a5,.L_main_40
  li	a5,8192
  addi	a5,a5,-1806
  beq	a0,a5,.L_main_41
  li	a5,20480
  addi	a5,a5,-337
  bne	a0,a5,.L_main_114
  lui	a0,%hi(.LC_main_3)
  addi	a0,a0,%lo(.LC_main_3)
  call	ee_printf
  li	s4,57344
  li	s3,20480
  li	s2,28672
  addi	s7,s4,1444
  addi	s3,s3,1544
  addi	s2,s2,-1415
  li	a5,2
  sw	s7,8(sp)
  mv	s6,s3
  mv	s5,s2
  sw	a5,12(sp)
  j	.L_main_46
.L_main_14:
  li	a4,1
  bne	a5,a4,.L_main_15
  lh	a5,48(sp)
  bne	a5,zero,.L_main_15
  li	a5,873803776
  addi	a5,a5,1045
  li	a4,102
  sw	a5,44(sp)
  sh	a4,48(sp)
  j	.L_main_15
.L_main_16:
  addi	a2,a1,1
  zext.h	a2,a2
  beq	a0,zero,.L_main_120
  addi	a0,a1,2
  zext.h	a0,a0
  li	a4,2000
  divu	a4,a4,a0
  sw	a4,68(sp)
  bne	a1,zero,.L_main_121
  li	a0,4
.L_main_26:
  mul	a1,a3,a4
  mv	a3,a2
  add	a2,a5,a1
  sw	a2,60(sp)
  beq	a0,zero,.L_main_19
  j	.L_main_24
.L_main_120:
  li	a4,2000
  divu	a4,a4,a2
  sw	a4,68(sp)
  beq	a1,zero,.L_main_26
  sw	a5,56(sp)
  j	.L_main_26
.L_main_117:
  li	a4,2000
  sw	a4,68(sp)
  beq	a1,zero,.L_main_19
  sw	a5,56(sp)
  j	.L_main_19
.L_main_116:
  li	a5,102
  sh	a5,48(sp)
  j	.L_main_15
.L_main_119:
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
  bne	a5,zero,.L_main_29
  j	.L_main_31
.L_main_40:
  li	a5,36864
  addi	a5,a5,-1534
  beq	a0,a5,.L_main_44
  li	a5,61440
  addi	a5,a5,-1547
  bne	a0,a5,.L_main_114
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
  sw	zero,8(sp)
  li	s7,0
  sw	a5,12(sp)
.L_main_46:
  lui	s11,%hi(default_num_contexts)
  lw	a5,%lo(default_num_contexts)(s11)
  beq	a5,zero,.L_main_74
  sw	s8,136(sp)
  sw	s9,132(sp)
  sw	s10,128(sp)
  li	s8,0
  li	s4,0
  j	.L_main_52
.L_main_123:
  add	a5,a5,a4
  lhu	a5,-4(a5)
.L_main_50:
  lw	a3,%lo(default_num_contexts)(s11)
  addi	a1,s4,1
  add	a5,a5,s8
  zext.h	s4,a1
  zext.h	a2,a5
  sext.h	s8,a5
  bgeu	s4,a3,.L_main_122
.L_main_52:
  slli	s9,s4,4
  add	a6,s9,s4
  slli	a6,a6,2
  addi	a5,a6,80
  addi	a4,sp,32
  add	s10,a5,a4
  lw	a3,-36(s10)
  sh	zero,-4(s10)
  andi	a2,a3,1
  beq	a2,zero,.L_main_47
  lhu	a2,-10(s10)
  beq	a2,s5,.L_main_47
  lui	a5,%hi(.LC_main_6)
  mv	a3,s2
  mv	a1,s4
  addi	a0,a5,%lo(.LC_main_6)
  call	ee_printf
  lhu	a2,-4(s10)
  lw	a3,-36(s10)
  addi	a2,a2,1
  sh	a2,-4(s10)
.L_main_47:
  andi	a2,a3,2
  beq	a2,zero,.L_main_48
  add	a6,s9,s4
  slli	a6,a6,2
  addi	a5,a6,80
  addi	a4,sp,32
  add	s10,a5,a4
  lhu	a2,-8(s10)
  beq	a2,s6,.L_main_48
  lui	a5,%hi(.LC_main_7)
  mv	a3,s3
  mv	a1,s4
  addi	a0,a5,%lo(.LC_main_7)
  call	ee_printf
  lhu	a2,-4(s10)
  lw	a3,-36(s10)
  addi	a2,a2,1
  sh	a2,-4(s10)
.L_main_48:
  add	a5,s9,s4
  slli	a5,a5,2
  andi	a3,a3,4
  addi	a5,a5,80
  addi	a4,sp,32
  beq	a3,zero,.L_main_123
  add	s9,a5,a4
  lhu	a2,-6(s9)
  bne	a2,s7,.L_main_51
  lhu	a5,-4(s9)
  lw	a3,%lo(default_num_contexts)(s11)
  addi	a1,s4,1
  add	a5,a5,s8
  zext.h	s4,a1
  zext.h	a2,a5
  sext.h	s8,a5
  bltu	s4,a3,.L_main_52
.L_main_122:
  lw	s8,136(sp)
  lw	s9,132(sp)
  lw	s10,128(sp)
  j	.L_main_43
.L_main_114:
  li	a2,65536
  li	a5,-1
  addi	a2,a2,-1
  sw	a5,12(sp)
  lui	s11,%hi(default_num_contexts)
.L_main_43:
  sw	a2,8(sp)
  call	check_data_types
  lw	a2,8(sp)
  lw	a1,68(sp)
  lui	a5,%hi(.LC_main_9)
  add	s2,a0,a2
  addi	a0,a5,%lo(.LC_main_9)
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
  zext.h	s2,s2
  call	__gtdf2
  ble	a0,zero,.L_main_53
  lw	a5,%lo(default_num_contexts)(s11)
  lw	a0,72(sp)
  mul	a0,a0,a5
  call	__floatunsidf
  mv	s4,a0
  mv	s3,a1
  mv	a0,s0
  call	time_in_secs
  sw	s4,16(sp)
  sw	s3,20(sp)
  mv	a2,a0
  mv	a3,a1
  lw	a0,16(sp)
  lw	a1,20(sp)
  call	__divdf3
  mv	a2,a0
  lui	a0,%hi(.LC_main_12)
  mv	a3,a1
  addi	a0,a0,%lo(.LC_main_12)
  call	ee_printf
.L_main_53:
  mv	a0,s0
  call	time_in_secs
  lui	a5,%hi(.LC_main_13)
  lw	a2,%lo(.LC_main_13)(a5)
  lw	a3,%lo(.LC_main_13+4)(a5)
  call	__ltdf2
  blt	a0,zero,.L_main_55
.L_main_115:
  lw	a5,%lo(default_num_contexts)(s11)
  lw	a1,72(sp)
  lui	a0,%hi(.LC_main_15)
  addi	a0,a0,%lo(.LC_main_15)
  mul	a1,a1,a5
  lui	s3,%hi(.LC_main_16)
  lui	s4,%hi(.LC_main_18)
  lui	s5,%hi(.LC_main_20)
  sext.h	s2,s2
  call	ee_printf
  lui	a0,%hi(.LC_main_17)
  addi	a1,s3,%lo(.LC_main_16)
  addi	a0,a0,%lo(.LC_main_17)
  call	ee_printf
  lui	a0,%hi(.LC_main_19)
  addi	a1,s4,%lo(.LC_main_18)
  addi	a0,a0,%lo(.LC_main_19)
  call	ee_printf
  lui	a0,%hi(.LC_main_21)
  addi	a1,s5,%lo(.LC_main_20)
  addi	a0,a0,%lo(.LC_main_21)
  call	ee_printf
  lui	a0,%hi(.LC_main_22)
  mv	a1,s1
  addi	a0,a0,%lo(.LC_main_22)
  call	ee_printf
  lw	a5,76(sp)
  andi	a3,a5,1
  beq	a3,zero,.L_main_60
  lw	a3,%lo(default_num_contexts)(s11)
  beq	a3,zero,.L_main_60
  li	s1,0
  lui	s6,%hi(.LC_main_23)
.L_main_61:
  slli	a5,s1,4
  add	a5,a5,s1
  slli	a5,a5,2
  add	a5,a5,sp
  lhu	a2,102(a5)
  mv	a1,s1
  addi	a0,s6,%lo(.LC_main_23)
  call	ee_printf
  lw	a5,%lo(default_num_contexts)(s11)
  addi	s1,s1,1
  zext.h	s1,s1
  bltu	s1,a5,.L_main_61
  lw	a5,76(sp)
.L_main_60:
  andi	a3,a5,2
  beq	a3,zero,.L_main_59
  lw	a3,%lo(default_num_contexts)(s11)
  beq	a3,zero,.L_main_64
  li	s1,0
  lui	s6,%hi(.LC_main_24)
.L_main_65:
  slli	a5,s1,4
  add	a5,a5,s1
  slli	a5,a5,2
  add	a5,a5,sp
  lhu	a2,104(a5)
  mv	a1,s1
  addi	a0,s6,%lo(.LC_main_24)
  call	ee_printf
  lw	a5,%lo(default_num_contexts)(s11)
  addi	s1,s1,1
  zext.h	s1,s1
  bltu	s1,a5,.L_main_65
  lw	a5,76(sp)
.L_main_59:
  andi	a5,a5,4
  beq	a5,zero,.L_main_63
  lw	a5,%lo(default_num_contexts)(s11)
  beq	a5,zero,.L_main_67
  li	s1,0
  lui	s6,%hi(.LC_main_25)
.L_main_68:
  slli	a5,s1,4
  add	a5,a5,s1
  slli	a5,a5,2
  add	a5,a5,sp
  lhu	a2,106(a5)
  mv	a1,s1
  addi	a0,s6,%lo(.LC_main_25)
  call	ee_printf
  lw	a5,%lo(default_num_contexts)(s11)
  addi	s1,s1,1
  zext.h	s1,s1
  bltu	s1,a5,.L_main_68
.L_main_63:
  lw	a5,%lo(default_num_contexts)(s11)
  li	s1,0
  lui	s6,%hi(.LC_main_26)
  beq	a5,zero,.L_main_67
.L_main_66:
  slli	a5,s1,4
  add	a5,a5,s1
  slli	a5,a5,2
  add	a5,a5,sp
  lhu	a2,100(a5)
  mv	a1,s1
  addi	a0,s6,%lo(.LC_main_26)
  call	ee_printf
  lw	a5,%lo(default_num_contexts)(s11)
  addi	s1,s1,1
  zext.h	s1,s1
  bltu	s1,a5,.L_main_66
.L_main_67:
  beq	s2,zero,.L_main_124
  bgt	s2,zero,.L_main_72
  lui	a0,%hi(.LC_main_31)
  addi	a0,a0,%lo(.LC_main_31)
  call	ee_printf
.L_main_71:
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
  lw	s11,124(sp)
  li	a0,0
  addi	sp,sp,176
  jr	ra
.L_main_121:
  sw	a5,56(sp)
  li	a0,4
  j	.L_main_26
.L_main_118:
  sw	a5,56(sp)
  j	.L_main_24
.L_main_51:
  lw	a3,8(sp)
  lui	a5,%hi(.LC_main_8)
  addi	a0,a5,%lo(.LC_main_8)
  mv	a1,s4
  call	ee_printf
  lhu	a5,-4(s9)
  addi	a5,a5,1
  zext.h	a5,a5
  sh	a5,-4(s9)
  j	.L_main_50
.L_main_72:
  lui	a0,%hi(.LC_main_32)
  addi	a0,a0,%lo(.LC_main_32)
  call	ee_printf
  j	.L_main_71
.L_main_124:
  lui	a0,%hi(.LC_main_27)
  addi	a0,a0,%lo(.LC_main_27)
  call	ee_printf
  lw	a4,12(sp)
  li	a5,3
  bne	a4,a5,.L_main_71
  lw	a5,%lo(default_num_contexts)(s11)
  lw	a0,72(sp)
  mul	a0,a0,a5
  call	__floatunsidf
  mv	s1,a0
  mv	a0,s0
  mv	s0,a1
  call	time_in_secs
  sw	s1,24(sp)
  sw	s0,28(sp)
  mv	a2,a0
  mv	a3,a1
  lw	a0,24(sp)
  lw	a1,28(sp)
  call	__divdf3
  mv	a2,a0
  lui	a0,%hi(.LC_main_28)
  mv	a3,a1
  addi	a5,s4,%lo(.LC_main_18)
  addi	a4,s3,%lo(.LC_main_16)
  addi	a0,a0,%lo(.LC_main_28)
  call	ee_printf
  lui	a0,%hi(.LC_main_29)
  addi	a1,s5,%lo(.LC_main_20)
  addi	a0,a0,%lo(.LC_main_29)
  call	ee_printf
  lui	a0,%hi(.LC_main_30)
  addi	a0,a0,%lo(.LC_main_30)
  call	ee_printf
  j	.L_main_71
.L_main_55:
  lui	a0,%hi(.LC_main_14)
  addi	a0,a0,%lo(.LC_main_14)
  call	ee_printf
  addi	s2,s2,1
  j	.L_main_115
.L_main_44:
  lui	a0,%hi(.LC_main_1)
  li	s4,24576
  li	s3,49152
  li	s2,53248
  addi	a0,a0,%lo(.LC_main_1)
  addi	s7,s4,-441
  addi	s3,s3,-430
  addi	s2,s2,1200
  call	ee_printf
  mv	s6,s3
  sw	s7,8(sp)
  mv	s5,s2
  sw	zero,12(sp)
  j	.L_main_46
.L_main_41:
  lui	a0,%hi(.LC_main_5)
  addi	a0,a0,%lo(.LC_main_5)
  call	ee_printf
  li	s4,36864
  li	s2,57344
  addi	s7,s4,-636
  addi	s2,s2,961
  li	s3,1863
  li	a5,4
  sw	s7,8(sp)
  mv	s5,s2
  mv	s6,s3
  sw	a5,12(sp)
  j	.L_main_46
.L_main_39:
  lui	a0,%hi(.LC_main_2)
  addi	a0,a0,%lo(.LC_main_2)
  call	ee_printf
  li	s4,16384
  li	s3,4096
  li	s2,12288
  addi	s7,s4,-1601
  addi	s3,s3,409
  addi	s2,s2,832
  li	a5,1
  sw	s7,8(sp)
  mv	s6,s3
  mv	s5,s2
  sw	a5,12(sp)
  j	.L_main_46
.L_main_74:
  li	a2,0
  j	.L_main_43
.L_main_64:
  andi	a5,a5,4
  beq	a5,zero,.L_main_63
  j	.L_main_67
.size	main, .-main
.globl	mem_name
.section	.rodata.str1.4
.align	2
.LC_main_33:
.string	"Static"
.align	2
.LC_main_34:
.string	"Heap"
.align	2
.LC_main_35:
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
.word	.LC_main_33
.word	.LC_main_34
.word	.LC_main_35
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
.ident	"GCC: (Alpine Linux) 14.1.0"
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
  xor	a5,a0,a1
  andi	a5,a5,1
  li	a3,-24576
  addi	a3,a3,1
  neg	a5,a5
  srli	a1,a1,1
  and	a5,a5,a3
  xor	a5,a5,a1
  zext.h	a5,a5
  srli	a4,a0,1
  xor	a4,a4,a5
  andi	a4,a4,1
  neg	a4,a4
  and	a4,a4,a3
  srli	a5,a5,1
  xor	a4,a4,a5
  zext.h	a4,a4
  srli	a5,a0,2
  xor	a5,a5,a4
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a3
  srli	a4,a4,1
  xor	a5,a5,a4
  zext.h	a5,a5
  srli	a4,a0,3
  xor	a4,a4,a5
  andi	a4,a4,1
  neg	a4,a4
  and	a4,a4,a3
  srli	a5,a5,1
  xor	a4,a4,a5
  zext.h	a4,a4
  srli	a5,a0,4
  xor	a5,a5,a4
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a3
  srli	a4,a4,1
  xor	a5,a5,a4
  zext.h	a5,a5
  srli	a4,a0,5
  xor	a4,a4,a5
  andi	a4,a4,1
  neg	a4,a4
  and	a4,a4,a3
  srli	a5,a5,1
  xor	a4,a4,a5
  zext.h	a4,a4
  srli	a5,a0,6
  xor	a5,a5,a4
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a3
  srli	a4,a4,1
  xor	a5,a5,a4
  zext.h	a5,a5
  srli	a0,a0,7
  xor	a0,a0,a5
  andi	a0,a0,1
  neg	a0,a0
  and	a0,a0,a3
  srli	a5,a5,1
  xor	a0,a0,a5
  zext.h	a0,a0
  ret
.size	crcu8, .-crcu8
.align	2
.globl	crcu16
.type	crcu16, @function
crcu16:
  andi	a5,a0,0xff
  xor	a3,a5,a1
  andi	a3,a3,1
  li	a4,-24576
  addi	a4,a4,1
  neg	a3,a3
  srli	a1,a1,1
  and	a3,a3,a4
  xor	a3,a3,a1
  zext.h	a3,a3
  srli	a2,a5,1
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a4
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a5,2
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a5,3
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a4
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a5,4
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a5,5
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a4
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a5,6
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a5,a5,7
  xor	a5,a5,a3
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a4
  srli	a3,a3,1
  xor	a5,a5,a3
  srli	a0,a0,8
  zext.h	a5,a5
  xor	a3,a0,a5
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a5,a5,1
  xor	a5,a3,a5
  zext.h	a5,a5
  srli	a3,a0,1
  xor	a3,a3,a5
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a5,a5,1
  xor	a3,a3,a5
  zext.h	a3,a3
  srli	a5,a0,2
  xor	a5,a5,a3
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a4
  srli	a3,a3,1
  xor	a5,a5,a3
  zext.h	a5,a5
  srli	a3,a0,3
  xor	a3,a3,a5
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a5,a5,1
  xor	a3,a3,a5
  zext.h	a3,a3
  srli	a5,a0,4
  xor	a5,a5,a3
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a4
  srli	a3,a3,1
  xor	a5,a5,a3
  zext.h	a5,a5
  srli	a3,a0,5
  xor	a3,a3,a5
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a5,a5,1
  xor	a3,a3,a5
  zext.h	a3,a3
  srli	a5,a0,6
  xor	a5,a5,a3
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a4
  srli	a3,a3,1
  xor	a5,a5,a3
  zext.h	a5,a5
  srli	a0,a0,7
  xor	a0,a0,a5
  andi	a0,a0,1
  neg	a0,a0
  and	a0,a0,a4
  srli	a5,a5,1
  xor	a0,a0,a5
  zext.h	a0,a0
  ret
.size	crcu16, .-crcu16
.align	2
.globl	crcu32
.type	crcu32, @function
crcu32:
  andi	a4,a0,0xff
  xor	a2,a4,a1
  andi	a2,a2,1
  li	a5,-24576
  addi	a5,a5,1
  neg	a2,a2
  srli	a1,a1,1
  and	a2,a2,a5
  xor	a2,a2,a1
  zext.h	a2,a2
  srli	a3,a4,1
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a4,2
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,3
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a4,4
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,5
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a4,6
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,7
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a4,a0
  srli	a4,a4,8
  zext.h	a3,a3
  xor	a2,a4,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,1
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a4,2
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,3
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a4,4
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,5
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a4,6
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,7
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  srli	a0,a0,16
  xor	a3,a3,a2
  andi	a4,a0,0xff
  zext.h	a3,a3
  xor	a2,a4,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a3,a2,a3
  zext.h	a3,a3
  srli	a2,a4,1
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,2
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a4,3
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,4
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a4,5
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a5
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a4,6
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a4,a4,7
  xor	a4,a4,a3
  andi	a4,a4,1
  neg	a4,a4
  and	a4,a4,a5
  srli	a3,a3,1
  xor	a4,a4,a3
  srli	a0,a0,8
  zext.h	a4,a4
  xor	a3,a0,a4
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a4,a4,1
  xor	a4,a3,a4
  zext.h	a4,a4
  srli	a3,a0,1
  xor	a3,a3,a4
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a4,a4,1
  xor	a3,a3,a4
  zext.h	a3,a3
  srli	a4,a0,2
  xor	a4,a4,a3
  andi	a4,a4,1
  neg	a4,a4
  and	a4,a4,a5
  srli	a3,a3,1
  xor	a4,a4,a3
  zext.h	a4,a4
  srli	a3,a0,3
  xor	a3,a3,a4
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a4,a4,1
  xor	a3,a3,a4
  zext.h	a3,a3
  srli	a4,a0,4
  xor	a4,a4,a3
  andi	a4,a4,1
  neg	a4,a4
  and	a4,a4,a5
  srli	a3,a3,1
  xor	a4,a4,a3
  zext.h	a4,a4
  srli	a3,a0,5
  xor	a3,a3,a4
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a5
  srli	a4,a4,1
  xor	a3,a3,a4
  zext.h	a3,a3
  srli	a4,a0,6
  xor	a4,a4,a3
  andi	a4,a4,1
  neg	a4,a4
  and	a4,a4,a5
  srli	a3,a3,1
  xor	a4,a4,a3
  zext.h	a4,a4
  srli	a0,a0,7
  xor	a0,a0,a4
  andi	a0,a0,1
  neg	a0,a0
  and	a0,a0,a5
  srli	a4,a4,1
  xor	a0,a0,a4
  zext.h	a0,a0
  ret
.size	crcu32, .-crcu32
.align	2
.globl	crc16
.type	crc16, @function
crc16:
  andi	a5,a0,0xff
  xor	a3,a5,a1
  andi	a3,a3,1
  li	a4,-24576
  addi	a4,a4,1
  neg	a3,a3
  srli	a1,a1,1
  and	a3,a3,a4
  xor	a3,a3,a1
  zext.h	a3,a3
  srli	a2,a5,1
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a4
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a5,2
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a5,3
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a4
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a5,4
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a2,a5,5
  xor	a2,a2,a3
  andi	a2,a2,1
  neg	a2,a2
  and	a2,a2,a4
  srli	a3,a3,1
  xor	a2,a2,a3
  zext.h	a2,a2
  srli	a3,a5,6
  xor	a3,a3,a2
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a2,a2,1
  xor	a3,a3,a2
  zext.h	a3,a3
  srli	a5,a5,7
  xor	a5,a5,a3
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a4
  srli	a3,a3,1
  xor	a5,a5,a3
  zext.h	a0,a0
  srli	a0,a0,8
  zext.h	a5,a5
  xor	a3,a0,a5
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a5,a5,1
  xor	a5,a3,a5
  zext.h	a5,a5
  srli	a3,a0,1
  xor	a3,a3,a5
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a5,a5,1
  xor	a3,a3,a5
  zext.h	a3,a3
  srli	a5,a0,2
  xor	a5,a5,a3
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a4
  srli	a3,a3,1
  xor	a5,a5,a3
  zext.h	a5,a5
  srli	a3,a0,3
  xor	a3,a3,a5
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a5,a5,1
  xor	a3,a3,a5
  zext.h	a3,a3
  srli	a5,a0,4
  xor	a5,a5,a3
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a4
  srli	a3,a3,1
  xor	a5,a5,a3
  zext.h	a5,a5
  srli	a3,a0,5
  xor	a3,a3,a5
  andi	a3,a3,1
  neg	a3,a3
  and	a3,a3,a4
  srli	a5,a5,1
  xor	a3,a3,a5
  zext.h	a3,a3
  srli	a5,a0,6
  xor	a5,a5,a3
  andi	a5,a5,1
  neg	a5,a5
  and	a5,a5,a4
  srli	a3,a3,1
  xor	a5,a5,a3
  zext.h	a5,a5
  srli	a0,a0,7
  xor	a0,a0,a5
  andi	a0,a0,1
  neg	a0,a0
  and	a0,a0,a4
  srli	a5,a5,1
  xor	a0,a0,a5
  zext.h	a0,a0
  ret
.size	crc16, .-crc16
.align	2
.globl	check_data_types
.type	check_data_types, @function
check_data_types:
  li	a0,0
  ret
.size	check_data_types, .-check_data_types
.ident	"GCC: (Alpine Linux) 14.1.0"
.section	.note.GNU-stack,"",@progbits
.file	"core_matrix.c"
.option nopic
.text
.align	2
.globl	core_init_matrix
.type	core_init_matrix, @function
core_init_matrix:
  addi	sp,sp,-16
  sw	s0,12(sp)
  sw	s1,8(sp)
  mv	a6,a0
  bne	a2,zero,.L_matrix_2
  li	a2,1
.L_matrix_2:
  addi	a1,a1,-1
  andi	t6,a1,-4
  addi	t2,t6,4
  li	a5,0
  beq	a6,zero,.L_matrix_19
.L_matrix_4:
  mv	a0,a5
  addi	a5,a5,1
  mul	a4,a5,a5
  slli	a4,a4,3
  bgtu	a6,a4,.L_matrix_4
  mul	s0,a0,a0
  slli	s0,s0,2
  add	t6,t2,s0
  beq	a0,zero,.L_matrix_12
  mv	s1,a0
.L_matrix_6:
  lui	a6,%hi(.LC_matrix_0)
  lui	a1,%hi(.LC_matrix_1)
  lui	a4,%hi(.LC_matrix_2)
  flw	ft0,%lo(.LC_matrix_0)(a6)
  flw	fa0,%lo(.LC_matrix_1)(a1)
  flw	fa1,%lo(.LC_matrix_2)(a4)
  lui	a5,%hi(.LC_matrix_3)
  flw	fa2,%lo(.LC_matrix_3)(a5)
  addi	a7,a0,1
  slli	t0,a0,2
  mv	t4,t6
  li	t3,0
  li	a5,1
  sub	t1,t2,t6
.L_matrix_8:
  mv	t5,a5
  mv	a1,t4
.L_matrix_9:
  mul	a2,a2,a5
  fcvt.s.w	fa5,a5
  add	a6,t1,a1
  addi	a1,a1,4
  srai	a4,a2,31
  srli	a4,a4,16
  add	a2,a2,a4
  zext.h	a2,a2
  sub	a2,a2,a4
  add	a4,a2,a5
  fcvt.s.w	fa4,a4
  addi	a5,a5,1
  fmul.s	fa3,fa4,ft0
  fcvt.w.s a4,fa3,rtz
  fcvt.s.w	fa3,a4
  fmul.s	fa3,fa3,fa0
  fsub.s	fa4,fa4,fa3
  fadd.s	fa5,fa5,fa4
  fsw	fa4,-4(a1)
  fmul.s	fa4,fa5,fa1
  fcvt.w.s a4,fa4,rtz
  fcvt.s.w	fa4,a4
  fmul.s	fa4,fa4,fa2
  fsub.s	fa5,fa5,fa4
  fsw	fa5,0(a6)
  bne	a5,a7,.L_matrix_9
  addi	t3,t3,1
  add	a5,a0,t5
  add	a7,a7,a0
  add	t4,t4,t0
  bne	t3,a0,.L_matrix_8
.L_matrix_7:
  add	s0,t6,s0
  sw	s0,12(a3)
  lw	s0,12(sp)
  sw	s1,0(a3)
  sw	t2,4(a3)
  sw	t6,8(a3)
  lw	s1,8(sp)
  addi	sp,sp,16
  jr	ra
.L_matrix_19:
  li	s1,-1
  addi	t6,t6,8
  mv	a0,s1
  li	s0,4
  j	.L_matrix_6
.L_matrix_12:
  li	s0,0
  li	s1,0
  j	.L_matrix_7
.size	core_init_matrix, .-core_init_matrix
.align	2
.globl	matrix_sum
.type	matrix_sum, @function
matrix_sum:
  mv	a7,a0
  beq	a0,zero,.L_matrix_26
  fmv.s.x	fa5,zero
  slli	t3,a0,2
  neg	t1,a0
  fmv.s	fa4,fa5
  add	a2,a1,t3
  slli	t1,t1,3
  li	a1,0
  li	a0,0
.L_matrix_25:
  sub	a6,a2,t3
  mv	a4,a6
  j	.L_matrix_24
.L_matrix_31:
  addi	a4,a4,4
  sext.h	a0,a5
  beq	a2,a4,.L_matrix_30
.L_matrix_24:
  fmv.s	fa3,fa5
  flw	fa5,0(a4)
  zext.h	a5,a0
  addi	a0,a5,10
  fadd.s	fa4,fa4,fa5
  flt.s	a3,fa3,fa5
  sext.h	a0,a0
  add	a5,a5,a3
  flt.s	a3,fa0,fa4
  beq	a3,zero,.L_matrix_31
  addi	a4,a4,4
  fmv.s.x	fa4,zero
  bne	a2,a4,.L_matrix_24
.L_matrix_30:
  addi	a1,a1,1
  sub	a2,a6,t1
  bne	a7,a1,.L_matrix_25
  ret
.L_matrix_26:
  li	a0,0
  ret
.size	matrix_sum, .-matrix_sum
.align	2
.globl	matrix_mul_const
.type	matrix_mul_const, @function
matrix_mul_const:
  beq	a0,zero,.L_matrix_46
  vsetvli	a5,zero,e32,m1,ta,ma
  vfmv.v.f	v2,fa0
  csrr	t4,vlenb
  addi	t4,t4,-8
  slli	t3,a0,2
  li	t1,0
.L_matrix_34:
  addi	a5,a2,4
  sub	a5,a1,a5
  mv	a3,a1
  mv	a6,a2
  mv	a4,a0
  bleu	a5,t4,.L_matrix_50
.L_matrix_37:
  vsetvli	a5,a4,e32,m1,ta,ma
  vle32.v	v1,0(a6)
  slli	a7,a5,2
  sub	a4,a4,a5
  add	a6,a6,a7
  vfmul.vv	v1,v1,v2
  vse32.v	v1,0(a3)
  add	a3,a3,a7
  bne	a4,zero,.L_matrix_37
  add	a3,t3,a2
.L_matrix_36:
  addi	t1,t1,1
  add	a1,a1,t3
  mv	a2,a3
  bne	a0,t1,.L_matrix_34
.L_matrix_46:
  ret
.L_matrix_50:
  add	a3,t3,a2
  mv	a5,a2
  mv	a4,a1
.L_matrix_35:
  flw	fa5,0(a5)
  addi	a5,a5,4
  addi	a4,a4,4
  fmul.s	fa5,fa5,fa0
  fsw	fa5,-4(a4)
  bne	a5,a3,.L_matrix_35
  j	.L_matrix_36
.size	matrix_mul_const, .-matrix_mul_const
.align	2
.globl	matrix_add_const
.type	matrix_add_const, @function
matrix_add_const:
  beq	a0,zero,.L_matrix_59
  vsetvli	a5,zero,e32,m1,ta,ma
  vfmv.v.f	v2,fa0
  slli	t1,a0,2
  li	a7,0
.L_matrix_53:
  mv	a2,a1
  mv	a3,a0
  mv	a4,a1
.L_matrix_54:
  vsetvli	a5,a3,e32,m1,ta,ma
  vle32.v	v1,0(a2)
  slli	a6,a5,2
  sub	a3,a3,a5
  add	a2,a2,a6
  vfadd.vv	v1,v2,v1
  vse32.v	v1,0(a4)
  add	a4,a4,a6
  bne	a3,zero,.L_matrix_54
  addi	a7,a7,1
  add	a1,a1,t1
  bne	a0,a7,.L_matrix_53
.L_matrix_59:
  ret
.size	matrix_add_const, .-matrix_add_const
.align	2
.globl	matrix_mul_vect
.type	matrix_mul_vect, @function
matrix_mul_vect:
  beq	a0,zero,.L_matrix_68
  slli	a6,a0,2
  add	t1,a1,a6
  li	a7,0
  add	a6,a3,a6
.L_matrix_64:
  fmv.s.x	fa4,zero
  slli	a4,a7,2
  add	a4,a2,a4
  fsw	fa4,0(a1)
  mv	a5,a3
.L_matrix_63:
  flw	fa5,0(a4)
  flw	fa3,0(a5)
  addi	a5,a5,4
  addi	a4,a4,4
  fmul.s	fa5,fa5,fa3
  fadd.s	fa4,fa4,fa5
  fsw	fa4,0(a1)
  bne	a6,a5,.L_matrix_63
  addi	a1,a1,4
  add	a7,a7,a0
  bne	a1,t1,.L_matrix_64
.L_matrix_68:
  ret
.size	matrix_mul_vect, .-matrix_mul_vect
.align	2
.globl	matrix_mul_matrix
.type	matrix_mul_matrix, @function
matrix_mul_matrix:
  mv	t1,a0
  mv	t5,a3
  beq	a0,zero,.L_matrix_79
  mv	a7,a2
  slli	a2,a0,2
  mv	t3,a1
  li	t4,0
  add	a1,a7,a2
.L_matrix_71:
  mv	a0,t5
  mv	a3,t3
  li	a6,0
.L_matrix_73:
  fmv.s.x	fa4,zero
  mv	a4,a0
  mv	a5,a7
  fsw	fa4,0(a3)
.L_matrix_72:
  flw	fa5,0(a5)
  flw	fa3,0(a4)
  addi	a5,a5,4
  add	a4,a4,a2
  fmul.s	fa5,fa5,fa3
  fadd.s	fa4,fa4,fa5
  fsw	fa4,0(a3)
  bne	a5,a1,.L_matrix_72
  addi	a5,a6,1
  addi	a3,a3,4
  addi	a0,a0,4
  beq	t1,a5,.L_matrix_80
  mv	a6,a5
  j	.L_matrix_73
.L_matrix_80:
  add	a7,a7,a2
  add	a1,a1,a2
  add	t3,t3,a2
  addi	a5,t4,1
  beq	t4,a6,.L_matrix_79
  mv	t4,a5
  j	.L_matrix_71
.L_matrix_79:
  ret
.size	matrix_mul_matrix, .-matrix_mul_matrix
.align	2
.globl	matrix_mul_matrix_bitextract
.type	matrix_mul_matrix_bitextract, @function
matrix_mul_matrix_bitextract:
  mv	t4,a0
  mv	t0,a3
  beq	a0,zero,.L_matrix_91
  mv	t3,a2
  lui	t1,%hi(.LC_matrix_4)
  lui	a7,%hi(.LC_matrix_5)
  lui	a2,%hi(.LC_matrix_6)
  lui	a3,%hi(.LC_matrix_7)
  lui	a4,%hi(.LC_matrix_8)
  lui	a5,%hi(.LC_matrix_9)
  flw	ft4,%lo(.LC_matrix_4)(t1)
  flw	ft3,%lo(.LC_matrix_5)(a7)
  flw	ft2,%lo(.LC_matrix_6)(a2)
  flw	ft1,%lo(.LC_matrix_7)(a3)
  flw	ft0,%lo(.LC_matrix_8)(a4)
  flw	fa0,%lo(.LC_matrix_9)(a5)
  slli	a0,a0,2
  mv	t5,a1
  add	a6,t3,a0
  li	t6,0
.L_matrix_83:
  mv	a7,t0
  mv	a1,t5
  li	t1,0
.L_matrix_85:
  fmv.s.x	fa1,zero
  mv	a4,a7
  mv	a5,t3
  fsw	fa1,0(a1)
.L_matrix_84:
  flw	fa4,0(a4)
  flw	fa5,0(a5)
  addi	a5,a5,4
  add	a4,a4,a0
  fmul.s	fa5,fa5,fa4
  fmul.s	fa2,fa5,ft3
  fmul.s	fa3,fa5,ft0
  fmul.s	fa4,fa5,ft4
  fmul.s	fa5,fa5,ft1
  fcvt.w.s a2,fa2,rtz
  fcvt.w.s a3,fa3,rtz
  fcvt.s.w	fa2,a2
  fcvt.s.w	fa3,a3
  fmul.s	fa2,fa2,ft2
  fmul.s	fa3,fa3,fa0
  fsub.s	fa4,fa4,fa2
  fsub.s	fa5,fa5,fa3
  fmul.s	fa5,fa4,fa5
  fadd.s	fa1,fa1,fa5
  fsw	fa1,0(a1)
  bne	a6,a5,.L_matrix_84
  addi	a5,t1,1
  addi	a1,a1,4
  addi	a7,a7,4
  beq	t4,a5,.L_matrix_92
  mv	t1,a5
  j	.L_matrix_85
.L_matrix_92:
  add	t3,t3,a0
  add	a6,a6,a0
  add	t5,t5,a0
  addi	a5,t6,1
  beq	t6,t1,.L_matrix_91
  mv	t6,a5
  j	.L_matrix_83
.L_matrix_91:
  ret
.size	matrix_mul_matrix_bitextract, .-matrix_mul_matrix_bitextract
.align	2
.globl	matrix_test
.type	matrix_test, @function
matrix_test:
  addi	sp,sp,-80
  lui	a5,%hi(.LC_matrix_10)
  fsw	fs0,12(sp)
  flw	fs0,%lo(.LC_matrix_10)(a5)
  csrr	t0,vlenb
  sw	s4,56(sp)
  sw	s5,52(sp)
  sw	s6,48(sp)
  sw	ra,76(sp)
  sw	s0,72(sp)
  sw	s1,68(sp)
  sw	s2,64(sp)
  sw	s3,60(sp)
  sw	s7,44(sp)
  sw	s8,40(sp)
  sw	s9,36(sp)
  sw	s10,32(sp)
  sw	s11,28(sp)
  slli	t1,t0,2
  fadd.s	fs0,fa0,fs0
  sub	sp,sp,t1
  mv	s5,a1
  mv	s4,a2
  mv	s6,a3
  beq	a0,zero,.L_matrix_94
  vsetvli	a5,zero,e32,m1,ta,ma
  fmv.s	fa4,fa0
  vfmv.v.f	v2,fa0
  mv	s3,a0
  slli	s1,a0,2
  mv	s7,a2
  mv	a1,a2
  li	s8,0
.L_matrix_95:
  mv	a3,a1
  mv	s2,s3
  mv	a4,a1
.L_matrix_96:
  vsetvli	a5,s2,e32,m1,ta,ma
  vle32.v	v1,0(a3)
  slli	a2,a5,2
  sub	s2,s2,a5
  add	a3,a3,a2
  vfadd.vv	v1,v1,v2
  vse32.v	v1,0(a4)
  add	a4,a4,a2
  bne	s2,zero,.L_matrix_96
  addi	s0,s8,1
  add	a1,a1,s1
  beq	s3,s0,.L_matrix_97
  mv	s8,s0
  j	.L_matrix_95
.L_matrix_97:
  mv	a0,s4
  mv	a6,s5
  csrr	t1,vlenb
  addi	a5,a0,4
  addi	t1,t1,-8
  sub	a5,a6,a5
  li	a7,0
  mv	a3,a6
  mv	a2,a0
  mv	a4,s0
  bleu	a5,t1,.L_matrix_169
.L_matrix_100:
  vsetvli	a5,a4,e32,m1,ta,ma
  vle32.v	v1,0(a2)
  slli	a1,a5,2
  sub	a4,a4,a5
  add	a2,a2,a1
  vfmul.vv	v1,v1,v2
  vse32.v	v1,0(a3)
  add	a3,a3,a1
  bne	a4,zero,.L_matrix_100
  add	a3,a0,s1
  add	a6,a6,s1
  mv	a0,a3
  addi	a5,a7,1
  beq	a7,s8,.L_matrix_170
.L_matrix_130:
  mv	a7,a5
  addi	a5,a0,4
  sub	a5,a6,a5
  mv	a3,a6
  mv	a2,a0
  mv	a4,s0
  bgtu	a5,t1,.L_matrix_100
.L_matrix_169:
  add	a3,a0,s1
  mv	a5,a0
  mv	a4,a6
.L_matrix_98:
  flw	fa5,0(a5)
  addi	a5,a5,4
  addi	a4,a4,4
  fmul.s	fa5,fa5,fa4
  fsw	fa5,-4(a4)
  bne	a3,a5,.L_matrix_98
  add	a6,a6,s1
  mv	a0,a3
  addi	a5,a7,1
  bne	a7,s8,.L_matrix_130
.L_matrix_170:
  fmv.s.x	fa5,zero
  add	s9,s5,s1
  neg	s10,s0
  fmv.s	fa4,fa5
  mv	a2,s9
  slli	s10,s10,3
  li	a1,0
  li	a0,0
.L_matrix_102:
  sub	a6,a2,s1
  mv	a4,a6
  j	.L_matrix_107
.L_matrix_172:
  addi	a4,a4,4
  sext.h	a0,a5
  beq	a2,a4,.L_matrix_171
.L_matrix_107:
  fmv.s	fa3,fa5
  flw	fa5,0(a4)
  zext.h	a5,a0
  addi	a0,a5,10
  fadd.s	fa4,fa4,fa5
  flt.s	a3,fa3,fa5
  sext.h	a0,a0
  add	a5,a5,a3
  flt.s	a3,fs0,fa4
  beq	a3,zero,.L_matrix_172
  addi	a4,a4,4
  fmv.s.x	fa4,zero
  bne	a2,a4,.L_matrix_107
.L_matrix_171:
  sub	a2,a6,s10
  addi	a5,a1,1
  beq	s8,a1,.L_matrix_173
  mv	a1,a5
  j	.L_matrix_102
.L_matrix_173:
  csrr	s11,vlenb
  slli	a5,s11,2
  sub	a5,a5,s11
  add	a5,a5,sp
  li	a1,0
  vs1r.v	v2,0(a5)
  call	crc16
  slli	a5,s11,2
  sub	a5,a5,s11
  add	a5,a5,sp
  vl1re32.v	v2,0(a5)
  mv	a1,a0
  add	a2,s6,s1
  mv	a3,s5
  li	a0,0
.L_matrix_111:
  fmv.s.x	fa4,zero
  slli	a4,a0,2
  add	a4,s4,a4
  fsw	fa4,0(a3)
  mv	a5,s6
.L_matrix_110:
  flw	fa5,0(a4)
  flw	fa3,0(a5)
  addi	a5,a5,4
  addi	a4,a4,4
  fmul.s	fa5,fa5,fa3
  fadd.s	fa4,fa4,fa5
  fsw	fa4,0(a3)
  bne	a2,a5,.L_matrix_110
  addi	a3,a3,4
  add	a0,a0,s0
  bne	s9,a3,.L_matrix_111
  fmv.s.x	fa5,zero
  li	a2,0
  li	a0,0
  fmv.s	fa4,fa5
.L_matrix_115:
  sub	a6,s9,s1
  mv	a4,a6
  j	.L_matrix_114
.L_matrix_175:
  addi	a4,a4,4
  sext.h	a0,a5
  beq	s9,a4,.L_matrix_174
.L_matrix_114:
  fmv.s	fa3,fa5
  flw	fa5,0(a4)
  zext.h	a5,a0
  addi	a0,a5,10
  fadd.s	fa4,fa4,fa5
  flt.s	a3,fa3,fa5
  sext.h	a0,a0
  add	a5,a5,a3
  fgt.s	a3,fa4,fs0
  beq	a3,zero,.L_matrix_175
  addi	a4,a4,4
  fmv.s.x	fa4,zero
  bne	s9,a4,.L_matrix_114
.L_matrix_174:
  sub	s9,a6,s10
  addi	a5,a2,1
  beq	a2,s8,.L_matrix_176
  mv	a2,a5
  j	.L_matrix_115
.L_matrix_176:
  csrr	s8,vlenb
  slli	a5,s8,2
  sub	a5,a5,s8
  add	a5,a5,sp
  vs1r.v	v2,0(a5)
  call	crc16
  slli	a5,s8,2
  sub	a5,a5,s8
  add	a5,a5,sp
  vl1re32.v	v2,0(a5)
  mv	a1,a0
  mv	t1,s5
  li	a7,0
.L_matrix_116:
  mv	a2,s6
  mv	a6,t1
  li	a3,0
.L_matrix_118:
  fmv.s.x	fa4,zero
  mv	a0,a2
  mv	a4,s7
  fsw	fa4,0(a6)
  li	a5,0
.L_matrix_117:
  flw	fa5,0(a4)
  flw	fa3,0(a0)
  addi	a5,a5,1
  addi	a4,a4,4
  fmul.s	fa5,fa5,fa3
  add	a0,a0,s1
  fadd.s	fa4,fa4,fa5
  fsw	fa4,0(a6)
  bgtu	s0,a5,.L_matrix_117
  addi	a3,a3,1
  addi	a6,a6,4
  addi	a2,a2,4
  bgtu	s0,a3,.L_matrix_118
  addi	a7,a7,1
  add	s7,s7,s1
  add	t1,t1,s1
  bgtu	s0,a7,.L_matrix_116
  fmv.s.x	fa5,zero
  li	a7,0
  li	a6,0
  fmv.s	fa4,fa5
  li	a0,0
.L_matrix_119:
  slli	a4,a7,2
  add	a4,s5,a4
  li	a3,0
  j	.L_matrix_122
.L_matrix_178:
  addi	a3,a3,1
  sext.h	a0,a5
  addi	a4,a4,4
  bgeu	a3,s0,.L_matrix_177
.L_matrix_122:
  fmv.s	fa3,fa5
  flw	fa5,0(a4)
  zext.h	a5,a0
  addi	a0,a5,10
  fadd.s	fa4,fa4,fa5
  fgt.s	a2,fa5,fa3
  sext.h	a0,a0
  add	a5,a5,a2
  flt.s	a2,fs0,fa4
  beq	a2,zero,.L_matrix_178
  addi	a3,a3,1
  fmv.s.x	fa4,zero
  addi	a4,a4,4
  bltu	a3,s0,.L_matrix_122
.L_matrix_177:
  addi	a6,a6,1
  add	a7,a7,s0
  bgtu	s0,a6,.L_matrix_119
  csrr	s8,vlenb
  slli	a5,s8,2
  sub	a5,a5,s8
  add	a5,a5,sp
  vs1r.v	v2,0(a5)
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
  neg	a1,s3
  fmv.s	fa4,fa5
  add	s5,s5,s1
  slli	a1,a1,3
  li	s7,0
  li	a0,0
.L_matrix_127:
  sub	a2,s5,s1
  mv	a4,a2
  j	.L_matrix_126
.L_matrix_180:
  addi	a4,a4,4
  sext.h	a0,a5
  beq	s5,a4,.L_matrix_179
.L_matrix_126:
  fmv.s	fa3,fa5
  flw	fa5,0(a4)
  zext.h	a5,a0
  addi	a0,a5,10
  fadd.s	fa4,fa4,fa5
  fgt.s	a3,fa5,fa3
  sext.h	a0,a0
  add	a5,a5,a3
  flt.s	a3,fs0,fa4
  beq	a3,zero,.L_matrix_180
  addi	a4,a4,4
  fmv.s.x	fa4,zero
  bne	s5,a4,.L_matrix_126
.L_matrix_179:
  addi	s6,s7,1
  sub	s5,a2,a1
  beq	s3,s6,.L_matrix_181
  mv	s7,s6
  j	.L_matrix_127
.L_matrix_181:
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
.L_matrix_128:
  mv	a2,s4
  mv	a3,s6
  mv	a4,s4
.L_matrix_129:
  vsetvli	a5,a3,e32,m1,ta,ma
  vle32.v	v1,0(a2)
  slli	a1,a5,2
  sub	a3,a3,a5
  add	a2,a2,a1
  vfsub.vv	v1,v1,v2
  vse32.v	v1,0(a4)
  add	a4,a4,a1
  bne	a3,zero,.L_matrix_129
  add	s4,s4,s1
  addi	a5,s2,1
  beq	s2,s7,.L_matrix_109
  mv	s2,a5
  j	.L_matrix_128
.L_matrix_94:
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
.L_matrix_109:
  csrr	t0,vlenb
  slli	t1,t0,2
  add	sp,sp,t1
  lw	ra,76(sp)
  lw	s0,72(sp)
  lw	s1,68(sp)
  lw	s2,64(sp)
  lw	s3,60(sp)
  lw	s4,56(sp)
  lw	s5,52(sp)
  lw	s6,48(sp)
  lw	s7,44(sp)
  lw	s8,40(sp)
  lw	s9,36(sp)
  lw	s10,32(sp)
  lw	s11,28(sp)
  flw	fs0,12(sp)
  sext.h	a0,a0
  addi	sp,sp,80
  jr	ra
.size	matrix_test, .-matrix_test
.align	2
.globl	core_bench_matrix
.type	core_bench_matrix, @function
core_bench_matrix:
  addi	sp,sp,-16
  mv	a5,a1
  sw	s0,8(sp)
  lw	a1,12(a0)
  mv	s0,a2
  lw	a3,8(a0)
  lw	a2,4(a0)
  fcvt.s.w	fa0,a5
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
.word	1199570944
.align	2
.LC_matrix_2:
.word	998244352
.align	2
.LC_matrix_3:
.word	1132462080
.align	2
.LC_matrix_4:
.word	1023410176
.align	2
.LC_matrix_5:
.word	964689920
.align	2
.LC_matrix_6:
.word	1124073472
.align	2
.LC_matrix_7:
.word	1048576000
.align	2
.LC_matrix_8:
.word	1015021568
.align	2
.LC_matrix_9:
.word	1098907648
.align	2
.LC_matrix_10:
.word	1198522368
.ident	"GCC: (Alpine Linux) 14.1.0"
.section	.note.GNU-stack,"",@progbits
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
  addi	sp,sp,-144
  addi	a5,sp,63
  sw	s2,128(sp)
  andi	a5,a5,-64
  li	s2,64
  sw	s0,136(sp)
  sw	s1,132(sp)
  mv	s0,a2
  mv	s1,a1
  mv	a0,a5
  mv	a2,s2
  li	a1,0
  sw	ra,140(sp)
  call	memset
  sw	s2,0(a0)
  sw	zero,4(a0)
  li	a3,1
  sw	a3,8(a0)
  sw	zero,12(a0)
  sw	s1,16(a0)
  sw	zero,20(a0)
  sw	s0,24(a0)
  sw	zero,28(a0)
  #APP
  # 102 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  lui	a4,%hi(.LANCHOR_portme_0)
  addi	a4,a4,%lo(.LANCHOR_portme_0)
  sw	a0,0(a4)
  #APP
  # 115 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  sw	a3,64(a4)
  #APP
  # 117 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  li	a5,0
  sw	a5,128(a4)
  lw	ra,140(sp)
  mv	a0,s0
  li	a6,0
  lw	s0,136(sp)
  sw	a6,132(a4)
  sw	zero,64(a4)
  lw	s1,132(sp)
  lw	s2,128(sp)
  addi	sp,sp,144
  jr	ra
.size	_write, .-_write
.align	2
.globl	ee_printf
.type	ee_printf, @function
ee_printf:
  lui	t1,%hi(_impure_ptr)
  lw	t1,%lo(_impure_ptr)(t1)
  addi	sp,sp,-64
  sw	a1,36(sp)
  sw	a2,40(sp)
  sw	a3,44(sp)
  sw	a4,48(sp)
  sw	a5,52(sp)
  sw	a6,56(sp)
  sw	a7,60(sp)
  mv	a1,a0
  lw	a0,8(t1)
  addi	a2,sp,36
  sw	ra,28(sp)
  sw	a2,12(sp)
  call	vfprintf
  lw	ra,28(sp)
  addi	sp,sp,64
  jr	ra
.size	ee_printf, .-ee_printf
.align	2
.globl	_exit
.type	_exit, @function
_exit:
  #APP
  # 179 "coralnpu_port/core_portme.c" 1
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
  or	a4,a0,a4
  sw	a3,0(a5)
  sw	a4,4(a5)
  #APP
  # 194 "coralnpu_port/core_portme.c" 1
  fence rw, rw
  # 0 "" 2
  #NO_APP
  li	a4,1
  sw	a4,64(a5)
.L_portme_12:
  j	.L_portme_12
.size	_exit, .-_exit
.section	.rodata.str1.4,"aMS",@progbits,1
.align	2
.LC_portme_0:
.string	"ABORT CALLED!\n"
.text
.align	2
.globl	abort
.type	abort, @function
abort:
  lui	a0,%hi(.LC_portme_0)
  addi	sp,sp,-16
  addi	a0,a0,%lo(.LC_portme_0)
  sw	ra,12(sp)
  call	ee_printf
  li	a0,1
  call	_exit
.size	abort, .-abort
.align	2
.globl	barebones_clock
.type	barebones_clock, @function
barebones_clock:
  #APP
  # 212 "coralnpu_port/core_portme.c" 1
  csrr a0, mcycle
  # 0 "" 2
  #NO_APP
  ret
.size	barebones_clock, .-barebones_clock
.align	2
.globl	get_my_time
.type	get_my_time, @function
get_my_time:
  #APP
  # 212 "coralnpu_port/core_portme.c" 1
  csrr a0, mcycle
  # 0 "" 2
  #NO_APP
  ret
.size	get_my_time, .-get_my_time
.align	2
.globl	my_time_diff
.type	my_time_diff, @function
my_time_diff:
  sub	a0,a0,a1
  ret
.size	my_time_diff, .-my_time_diff
.align	2
.globl	start_time
.type	start_time, @function
start_time:
  #APP
  # 212 "coralnpu_port/core_portme.c" 1
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
  # 212 "coralnpu_port/core_portme.c" 1
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
  lui	a5,%hi(.LC_portme_1)
  flw	fa5,%lo(.LC_portme_1)(a5)
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
.globl	get_aligned_size
.type	get_aligned_size, @function
get_aligned_size:
  li	a5,-64
  bgtu	a0,a5,.L_portme_25
  addi	a0,a0,63
  and	a0,a0,a5
  ret
.L_portme_25:
  li	a0,0
  ret
.size	get_aligned_size, .-get_aligned_size
.align	2
.globl	portable_malloc
.type	portable_malloc, @function
portable_malloc:
  li	a5,-64
  bgtu	a0,a5,.L_portme_31
  addi	a0,a0,63
  and	a5,a0,a5
  beq	a5,zero,.L_portme_31
  lui	a3,%hi(current_alloc_offset)
  lw	a0,%lo(current_alloc_offset)(a3)
  li	a4,1998848
  addi	a4,a4,1152
  add	a5,a0,a5
  bgtu	a5,a4,.L_portme_31
  lui	a4,%hi(static_memblock)
  addi	a4,a4,%lo(static_memblock)
  sw	a5,%lo(current_alloc_offset)(a3)
  add	a0,a0,a4
  ret
.L_portme_31:
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
  beq	a5,zero,.L_portme_41
.L_portme_36:
  lui	a4,%hi(__heap_end)
  add	a0,a5,a0
  addi	a4,a4,%lo(__heap_end)
  bgtu	a0,a4,.L_portme_39
  lui	a4,%hi(__heap_start)
  addi	a4,a4,%lo(__heap_start)
  bltu	a0,a4,.L_portme_39
  sw	a0,%lo(heap_end.0)(a3)
  mv	a0,a5
  ret
.L_portme_39:
  li	a5,-1
  mv	a0,a5
  ret
.L_portme_41:
  lui	a5,%hi(__heap_start)
  addi	a4,a5,%lo(__heap_start)
  sw	a4,%lo(heap_end.0)(a3)
  addi	a5,a5,%lo(__heap_start)
  j	.L_portme_36
.size	_sbrk, .-_sbrk
.align	2
.globl	portable_init
.type	portable_init, @function
portable_init:
  lui	a5,%hi(_impure_ptr)
  lw	a5,%lo(_impure_ptr)(a5)
  addi	sp,sp,-16
  sw	s0,8(sp)
  mv	s0,a0
  lw	a0,8(a5)
  li	a3,0
  li	a2,2
  li	a1,0
  sw	ra,12(sp)
  call	setvbuf
  li	a5,1
  sb	a5,0(s0)
  lw	ra,12(sp)
  lw	s0,8(sp)
  addi	sp,sp,16
  jr	ra
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
.globl	stop_time_val
.globl	start_time_val
.globl	fromhost
.globl	fromhost_ready
.globl	tohost
.globl	tohost_ready
.globl	seed5_volatile
.globl	seed4_volatile
.globl	seed3_volatile
.globl	seed2_volatile
.globl	seed1_volatile
.section	.srodata.cst4,"aM",@progbits,4
.align	2
.LC_portme_1:
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
.word	2000
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
.ident	"GCC: (Alpine Linux) 14.1.0"
.section	.note.GNU-stack,"",@progbits
