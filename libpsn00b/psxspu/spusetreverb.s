.set noreorder

.include "hwregs_a.h"

.section .data


.global SpuSetReverb
.type SpuSetReverb, @function
SpuSetReverb:
	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	lhu		$v0, SPUCNT($v1)
	nop
	ori		$v0, 0x80					# Enable reverb
	sh		$v0, SPUCNT($v1)
	jal		SpuCtrlSync
	move	$a0, $v0
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop