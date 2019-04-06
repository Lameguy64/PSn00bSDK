.set noreorder

.include "hwregs_a.h"

.section .data


.global SpuReverbOn
.type SpuReverbOn, @function
SpuReverbOn:
	lui		$v1, IOBASE
	li		$v0, 1
	sll		$v0, $a0
	sh		$v0, SPU_REVERB_ON($v1)
	jr		$ra
	nop