# GTE register definitions for GNU assembler (as).
#
# Part of the PSn00bSDK Project by Lameguy64.
# 2019 Meido-Tek Productions

#
# GTE data registers (use mfc2, mtc2, lwc2, swc2)
#
.set C2_VXY0,	$0
.set C2_VZ0,	$1
.set C2_VXY1,	$2
.set C2_VZ1,	$3
.set C2_VXY2,	$4
.set C2_VZ2,	$5
.set C2_RGB,	$6
.set C2_OTZ,	$7

.set C2_IR0,	$8
.set C2_IR1,	$9
.set C2_IR2,	$10
.set C2_IR3,	$11
.set C2_SXY0,	$12
.set C2_SXY1,	$13
.set C2_SXY2,	$14
.set C2_SXYP,	$15

.set C2_SZ0,	$16
.set C2_SZ1,	$17
.set C2_SZ2,	$18
.set C2_SZ3,	$19
.set C2_RGB0,	$20
.set C2_RGB1,	$21
.set C2_RGB2,	$22

.set C2_MAC0,	$24
.set C2_MAC1,	$25
.set C2_MAC2,	$26
.set C2_MAC3,	$27
.set C2_IRGB,	$28
.set C2_ORGB,	$29
.set C2_LZCS,	$30
.set C2_LZCR,	$31

#
# GTE control registers (use cfc2/ctc2)
#
.set C2_R11R12,	$0
.set C2_R13R21,	$1
.set C2_R22R23,	$2
.set C2_R31R32,	$3
.set C2_R33,	$4
.set C2_TRX,	$5
.set C2_TRY,	$6
.set C2_TRZ,	$7

.set C2_L11L12,	$8
.set C2_L13L21,	$9
.set C2_L22L23,	$10
.set C2_L31L32,	$11
.set C2_L33,	$12
.set C2_RBK,	$13
.set C2_GBK,	$14
.set C2_BBK,	$15

.set C2_LR1LR2,	$16
.set C2_LR3LG1,	$17
.set C2_LG2LG3,	$18
.set C2_LB1LB2,	$19
.set C2_LB3,	$20
.set C2_RFC,	$21
.set C2_GFC,	$22
.set C2_BFC,	$23

.set C2_OFX,	$24
.set C2_OFY,	$25
.set C2_H,		$26
.set C2_DQA,	$27
.set C2_DQB,	$28
.set C2_ZSF3,	$29
.set C2_ZSF4,	$30
.set C2_FLAG,	$31
