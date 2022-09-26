.set noreorder

.include "gtereg.inc"
.include "inline_s.inc"

.set MATRIX_r11r12,	0
.set MATRIX_r13r21,	4
.set MATRIX_r22r23,	8
.set MATRIX_r31r32,	12
.set MATRIX_r33,	16
.set MATRIX_trx,	20
.set MATRIX_try,	24
.set MATRIX_trz,	28


.global CompMatrixLV
.type CompMatrixLV, @function
CompMatrixLV:

	# Load matrix v0 to GTE
	lw		$t0, MATRIX_r11r12($a0)
	lw		$t1, MATRIX_r13r21($a0)
	ctc2	$t0, C2_R11R12
	ctc2	$t1, C2_R13R21
	lw		$t0, MATRIX_r22r23($a0)
	lw		$t1, MATRIX_r31r32($a0)
	lhu		$t2, MATRIX_r33($a0)
	ctc2	$t0, C2_R22R23
	lw		$t0, MATRIX_trx($a0)
	ctc2	$t1, C2_R31R32
	lw		$t1, MATRIX_try($a0)
	ctc2	$t2, C2_R33
	lw		$t2, MATRIX_trz($a0)
	ctc2	$t0, C2_TRX
	ctc2	$t1, C2_TRY
	ctc2	$t2, C2_TRZ
	
	lw		$t0, MATRIX_trx($a1)
	lw		$t1, MATRIX_try($a1)
	mtc2	$t0, C2_IR1
	lw		$t0, MATRIX_trz($a1)
	mtc2	$t1, C2_IR2
	mtc2	$t0, C2_IR3

	nMVMVA(1, 0, 3, 0, 0)

	swc2	C2_IR1, MATRIX_trx($a2)
	swc2	C2_IR2, MATRIX_try($a2)
	swc2	C2_IR3, MATRIX_trz($a2)
	
	lhu		$t1, 2*(0+(3*1))($a1)		# Load values for first
	lhu		$t0, 2*(0+(3*0))($a1)		# R11 R21 R31
	sll		$t1, 16
	or		$t0, $t1
	lhu		$t1, 2*(0+(3*2))($a1)
	mtc2	$t0, C2_VXY0
	mtc2	$t1, C2_VZ0

	lhu		$t1, 2*(1+(3*1))($a1)		# Load values for second
	lhu		$t0, 2*(1+(3*0))($a1)		# R12 R22 R32
	MVMVA(1, 0, 0, 3, 0)				# First multiply
	sll		$t1, 16
	or		$t0, $t1
	lhu		$t1, 2*(1+(3*2))($a1)
	mtc2	$t0, C2_VXY0
	mtc2	$t1, C2_VZ0

	mfc2	$t0, C2_IR1					# Store results of first
	mfc2	$t1, C2_IR2
	sh		$t0, 2*(0+(3*0))($a2)
	mfc2	$t0, C2_IR3
	sh		$t1, 2*(0+(3*1))($a2)
	sh		$t0, 2*(0+(3*2))($a2)

	lhu		$t1, 2*(2+(3*1))($a1)		# Load values for third
	lhu		$t0, 2*(2+(3*0))($a1)		# R13 R23 R33
	MVMVA(1, 0, 0, 3, 0)				# Second multiply
	sll		$t1, 16
	or		$t0, $t1
	lhu		$t1, 2*(2+(3*2))($a1)
	mtc2	$t0, C2_VXY0
	mtc2	$t1, C2_VZ0

	mfc2	$t0, C2_IR1					# Store results of second
	mfc2	$t1, C2_IR2
	sh		$t0, 2*(1+(3*0))($a2)
	mfc2	$t0, C2_IR3
	sh		$t1, 2*(1+(3*1))($a2)
	sh		$t0, 2*(1+(3*2))($a2)
	MVMVA(1, 0, 0, 3, 0)				# Third multiply

	mfc2	$t0, C2_IR1					# Store results of third
	mfc2	$t1, C2_IR2
	sh		$t0, 2*(2+(3*0))($a2)
	mfc2	$t0, C2_IR3
	sh		$t1, 2*(2+(3*1))($a2)
	sh		$t0, 2*(2+(3*2))($a2)

	jr		$ra
	move	$v0, $a2