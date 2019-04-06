.set noreorder

.include "gtereg.h"
.include "inline_s.h"

.section .text


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


.section .data


.type matrix_stack, @object
_matrix_stack:
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0

