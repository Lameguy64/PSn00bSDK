.set noreorder

.include "gtereg.h"
.include "inline_s.h"

.section .text


.global MulMatrix
.type MulMatrix, @function
MulMatrix:

	# Load m1 to GTE
	lw		$t0, 0($a1)
	lw		$t1, 4($a1)
	ctc2	$t0, $0
	ctc2	$t1, $1
	lw		$t0, 8($a1)
	lw		$t1, 12($a1)
	lhu		$t2, 16($a1)
	ctc2	$t0, $2
	ctc2	$t1, $3
	ctc2	$t2, $4

	lhu		$t1, 2*(0+(3*1))($a0)		# Load values for first
	lhu		$t0, 2*(0+(3*0))($a0)		# R11 R21 R31
	sll		$t1, 16
	or		$t0, $t1
	lhu		$t1, 2*(0+(3*2))($a0)
	mtc2	$t0, C2_VXY0
	mtc2	$t1, C2_VZ0

	lhu		$t1, 2*(1+(3*1))($a0)		# Load values for second
	lhu		$t0, 2*(1+(3*0))($a0)		# R12 R22 R32
	MVMVA(1, 0, 0, 3, 0)				# First multiply
	sll		$t1, 16
	or		$t0, $t1
	lhu		$t1, 2*(1+(3*2))($a0)
	mtc2	$t0, C2_VXY0
	mtc2	$t1, C2_VZ0

	mfc2	$t0, C2_IR1					# Store results of first
	mfc2	$t1, C2_IR2
	sh		$t0, 2*(0+(3*0))($a0)
	mfc2	$t0, C2_IR3
	sh		$t1, 2*(0+(3*1))($a0)
	sh		$t0, 2*(0+(3*2))($a0)

	lhu		$t1, 2*(2+(3*1))($a0)		# Load values for third
	lhu		$t0, 2*(2+(3*0))($a0)		# R13 R23 R33
	MVMVA(1, 0, 0, 3, 0)				# Second multiply
	sll		$t1, 16
	or		$t0, $t1
	lhu		$t1, 2*(2+(3*2))($a0)
	mtc2	$t0, C2_VXY0
	mtc2	$t1, C2_VZ0

	mfc2	$t0, C2_IR1					# Store results of second
	mfc2	$t1, C2_IR2
	sh		$t0, 2*(1+(3*0))($a0)
	mfc2	$t0, C2_IR3
	sh		$t1, 2*(1+(3*1))($a0)
	sh		$t0, 2*(1+(3*2))($a0)
	MVMVA(1, 0, 0, 3, 0)				# Third multiply

	mfc2	$t0, C2_IR1					# Store results of third
	mfc2	$t1, C2_IR2
	sh		$t0, 2*(2+(3*0))($a0)
	mfc2	$t0, C2_IR3
	sh		$t1, 2*(2+(3*1))($a0)
	sh		$t0, 2*(2+(3*2))($a0)

	jr		$ra
	move	$v0, $a0
