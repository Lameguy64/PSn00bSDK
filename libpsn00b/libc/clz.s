# PSn00bSDK leading zero count intrinsics
# (C) 2022-2023 spicyjpeg - MPL licensed
#
# libgcc provides two functions used internally by GCC to count the number of
# leading zeroes in a value, __clzsi2() (32-bit) and __clzdi2() (64-bit). This
# file overrides them with smaller implementations that make use of the GTE's
# LZCS/LZCR registers.

.set noreorder

.set LZCS, $30
.set LZCR, $31

.section .text.__clzsi2, "ax", @progbits
.global __clzsi2
.type __clzsi2, @function

__clzsi2:
	mtc2  $a0, LZCS
	bltz  $a0, .Lreturn # if (value & (1 << 31)) return 0
	li    $v0, 0
	mfc2  $v0, LZCR # else return GTE_CLZ(value)

.Lreturn:
	jr    $ra
	nop

.section .text.__clzdi2, "ax", @progbits
.global __clzdi2
.type __clzdi2, @function

__clzdi2:
	mtc2  $a1, LZCS
	bltz  $a1, .Lreturn2 # if (msb & (1 << 31)) return 0
	li    $v0, 0
	bnez  $a1, .LreturnMSB # else if (msb) return GTE_CLZ(msb)
	nop

.LnoMSB:
	mtc2  $a0, LZCS
	bltz  $a0, .Lreturn2 # else if (lsb & (1 << 31)) return 32
	li    $v0, 32
	mfc2  $v0, LZCR # else return 32 + GTE_CLZ(lsb)

	jr    $ra
	addiu $v0, 32

.LreturnMSB:
	mfc2  $v0, LZCR

.Lreturn2:
	jr    $ra
	nop
