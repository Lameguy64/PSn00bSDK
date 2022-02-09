.set noreorder

.include "hwregs_a.h"

.section .data


.global SpuSetReverb
.type SpuSetReverb, @function
SpuSetReverb:
	addiu	$sp, -4
	sw		$ra, 0($sp)

	lui		$v1, IOBASE
	lhu		$v0, SPU_CTRL($v1)
	nop
	ori		$v0, 0x80					# Enable reverb
	sh		$v0, SPU_CTRL($v1)
	jal		SpuCtrlSync
	move	$a0, $v0
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop