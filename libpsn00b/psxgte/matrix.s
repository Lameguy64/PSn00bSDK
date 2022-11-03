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

.section .text.ApplyMatrixLV
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

.section .text.CompMatrixLV
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

.section .text.MulMatrix
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

.section .text.MulMatrix0
.global MulMatrix0
.type MulMatrix0, @function
MulMatrix0:
	# Load m1 to GTE
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

.section .text.ScaleMatrix
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

.section .text.ScaleMatrixL
.global ScaleMatrixL
.type ScaleMatrixL, @function
ScaleMatrixL:
	lwc2	C2_IR0,	0($a1)			# X

	lh		$v0, 2*(0+(3*0))($a0)
	lh		$v1, 2*(1+(3*0))($a0)
	mtc2	$v0, C2_IR1
	lh		$v0, 2*(2+(3*0))($a0)
	mtc2	$v1, C2_IR2
	mtc2	$v0, C2_IR3

	nGPF(1)

	mfc2	$v0, C2_IR1
	mfc2	$v1, C2_IR2
	sh		$v0, 2*(0+(3*0))($a0)
	mfc2	$v0, C2_IR3
	sh		$v1, 2*(1+(3*0))($a0)
	sh		$v0, 2*(2+(3*0))($a0)

	lwc2	C2_IR0,	4($a1)			# Y

	lh		$v0, 2*(0+(3*1))($a0)
	lh		$v1, 2*(1+(3*1))($a0)
	mtc2	$v0, C2_IR1
	lh		$v0, 2*(2+(3*1))($a0)
	mtc2	$v1, C2_IR2
	mtc2	$v0, C2_IR3

	nGPF(1)

	mfc2	$v0, C2_IR1
	mfc2	$v1, C2_IR2
	sh		$v0, 2*(0+(3*1))($a0)
	mfc2	$v0, C2_IR3
	sh		$v1, 2*(1+(3*1))($a0)
	sh		$v0, 2*(2+(3*1))($a0)

	lwc2	C2_IR0,	8($a1)			# Z

	lh		$v0, 2*(0+(3*2))($a0)
	lh		$v1, 2*(1+(3*2))($a0)
	mtc2	$v0, C2_IR1
	lh		$v0, 2*(2+(3*2))($a0)
	mtc2	$v1, C2_IR2
	mtc2	$v0, C2_IR3

	nGPF(1)

	mfc2	$v0, C2_IR1
	mfc2	$v1, C2_IR2
	sh		$v0, 2*(0+(3*2))($a0)
	mfc2	$v0, C2_IR3
	sh		$v1, 2*(1+(3*2))($a0)
	sh		$v0, 2*(2+(3*2))($a0)

	jr		$ra
	move	$v0, $a0

.section .text.PushMatrix
.global PushMatrix
.type PushMatrix, @function
PushMatrix:
	la		$a0, _matrix_stack
	cfc2	$v0, C2_R11R12
	cfc2	$v1, C2_R13R21
	sw		$v0, 0($a0)
	cfc2	$v0, C2_R22R23
	sw		$v1, 4($a0)
	sw		$v0, 8($a0)
	cfc2	$v0, C2_R31R32
	cfc2	$v1, C2_R33
	sw		$v0, 12($a0)
	sw		$v1, 16($a0)
	cfc2	$v0, C2_TRX
	cfc2	$v1, C2_TRY
	sw		$v0, 20($a0)
	cfc2	$v0, C2_TRZ
	sw		$v1, 24($a0)
	jr		$ra
	sw		$v0, 28($a0)

.section .text.PopMatrix
.global PopMatrix
.type PopMatrix, @function
PopMatrix:
	la		$a0, _matrix_stack
	lw		$v0, 0($a0)
	lw		$v1, 4($a0)
	ctc2	$v0, C2_R11R12
	ctc2	$v1, C2_R13R21
	lw		$v0, 8($a0)
	lw		$v1, 12($a0)
	ctc2	$v0, C2_R22R23
	lw		$v0, 16($a0)
	ctc2	$v1, C2_R31R32
	ctc2	$v0, C2_R33
	lw		$v0, 20($a0)
	lw		$v1, 24($a0)
	ctc2	$v0, C2_TRX
	lw		$v0, 28($a0)
	ctc2	$v1, C2_TRY
	ctc2	$v0, C2_TRZ
	jr		$ra
	nop

.section .data._matrix_stack
.type _matrix_stack, @object
_matrix_stack:
	.word 0, 0, 0, 0, 0, 0, 0, 0
