.set noreorder

.include "hwregs_a.h"

.text

.global DrawPrim
.type DrawPrim, @function
DrawPrim:

	addiu	$sp, -8
	sw		$ra, 0($sp)
	sw		$s0, 4($sp)
	
	move	$s0, $a0			# Wait for GPU to complete
	jal		DrawSync
	move	$a0, $0
	
	lui		$a3, IOBASE
	lui		$v0, 0x0400			# Set transfer direction to off
	sw		$v0, GPU_GP1($a3)
	
	move	$a0, $s0
	lbu		$a1, 3($a0)			# Get length of primitive packet
	addiu	$a0, 4
	addiu	$a1, -1
	
.Ltransfer_loop:
	lw		$v0, 0($a0)
	addiu	$a0, 4
	sw		$v0, GPU_GP0($a3)
	bgtz	$a1, .Ltransfer_loop
	addiu	$a1, -1

	jal		DrawSync
	move	$a0, $0
	
	lw		$ra, 0($sp)
	lw		$s0, 4($sp)
	jr		$ra
	addiu	$sp, 8