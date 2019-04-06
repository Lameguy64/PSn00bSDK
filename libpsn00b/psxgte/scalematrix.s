.set noreorder

.include "gtereg.h"
.include "inline_s.h"

.section .text


.global ScaleMatrix
.type ScaleMatrix, @function
ScaleMatrix:

	lwc2	C2_IR0,	0($a1)			# X

	lh		$v0, 2*(0+(3*0))($a0)
	lh		$v1, 2*(0+(3*1))($a0)
	mtc2	$v0, C2_IR1
	lh		$v0, 2*(0+(3*2))($a0)
	mtc2	$v1, C2_IR2
	mtc2	$v0, C2_IR3

	nGPF(1)

	mfc2	$v0, C2_IR1
	mfc2	$v1, C2_IR2
	sh		$v0, 2*(0+(3*0))($a0)
	mfc2	$v0, C2_IR3
	sh		$v1, 2*(0+(3*1))($a0)
	sh		$v0, 2*(0+(3*2))($a0)

	lwc2	C2_IR0,	4($a1)			# Y

	lh		$v0, 2*(1+(3*0))($a0)
	lh		$v1, 2*(1+(3*1))($a0)
	mtc2	$v0, C2_IR1
	lh		$v0, 2*(1+(3*2))($a0)
	mtc2	$v1, C2_IR2
	mtc2	$v0, C2_IR3

	nGPF(1)

	mfc2	$v0, C2_IR1
	mfc2	$v1, C2_IR2
	sh		$v0, 2*(1+(3*0))($a0)
	mfc2	$v0, C2_IR3
	sh		$v1, 2*(1+(3*1))($a0)
	sh		$v0, 2*(1+(3*2))($a0)

	lwc2	C2_IR0,	8($a1)			# Z

	lh		$v0, 2*(2+(3*0))($a0)
	lh		$v1, 2*(2+(3*1))($a0)
	mtc2	$v0, C2_IR1
	lh		$v0, 2*(2+(3*2))($a0)
	mtc2	$v1, C2_IR2
	mtc2	$v0, C2_IR3

	nGPF(1)

	mfc2	$v0, C2_IR1
	mfc2	$v1, C2_IR2
	sh		$v0, 2*(2+(3*0))($a0)
	mfc2	$v0, C2_IR3
	sh		$v1, 2*(2+(3*1))($a0)
	sh		$v0, 2*(2+(3*2))($a0)

	jr		$ra
	move	$v0, $a0
