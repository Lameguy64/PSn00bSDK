.set noreorder

.include "gtereg.h"
.include "inline_s.h"

.section .text


.global ApplyMatrixLV
.type ApplyMatrixLV, @function
ApplyMatrixLV:

	# Load matrix to GTE
	lw		$t0, 0($a0)
	lw		$t1, 4($a0)
	ctc2	$t0, $0
	ctc2	$t1, $1
	lw		$t0, 8($a0)
	lw		$t1, 12($a0)
	lhu		$t2, 16($a0)
	ctc2	$t0, $2
	ctc2	$t1, $3
	ctc2	$t2, $4

	lw		$t0, 0($a1)
	lw		$t1, 4($a1)
	mtc2	$t0, C2_IR1
	lw		$t0, 8($a1)
	mtc2	$t1, C2_IR2
	mtc2	$t0, C2_IR3

	nMVMVA(1, 0, 3, 3, 0)

	swc2	C2_IR1, 0($a2)
	swc2	C2_IR2, 4($a2)
	swc2	C2_IR3, 8($a2)

	jr		$ra
	move	$v0, $a2
	