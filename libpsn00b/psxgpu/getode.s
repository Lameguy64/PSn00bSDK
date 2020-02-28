.set noreorder

.include "hwregs_a.h"

.section .text


.global GetODE
.type GetODE, @function
GetODE:
	addiu	$sp, -4
	sw		$ra, 0($sp)
	jal		ReadGPUstat
	nop
	srl		$v0, 31
	andi	$v0, 1
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
