# PSn00bSDK leading zero count intrinsics
# (C) 2022 spicyjpeg - MPL licensed
#
# libgcc provides two functions used internally by GCC to count the number of 
# leading zeroes or ones in a value, _clzsi2() (32-bit) and _clzdi2() (64-bit).
# This file overrides them with faster implementations that make use of the
# GTE's LZCS/LZCR registers.

.set noreorder

.section .text._clzsi2
.global _clzsi2
.type _clzsi2, @function
_clzsi2:
	mtc2 $a0, $30
	nop
	nop
	mfc2 $v0, $31

	jr   $ra
	nop

.section .text._clzdi2
.global _clzdi2
.type _clzdi2, @function
_clzdi2:
	bnez $a1, .Lhas_msb
	nop

	mtc2 $a0, $30 # if (!msb) return 32 + clz(lsb)
	b    .Lreturn
	li   $v1, 32

.Lhas_msb:
	mtc2 $a1, $30  # if (msb) return 0 + clz(msb)
	nop
	li   $v1, 0

.Lreturn:
	mfc2 $v0, $31

	jr   $ra
	addu $v0, $v1
