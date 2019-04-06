.set noreorder

.include "hwregs_a.h"

.section .data


.global SpuKeyOn
.type SpuKeyOn, @function
SpuKeyOn:
	lui		$v1, IOBASE
	li		$v0, 1
	sll		$v0, $a0
	sh		$v0, SPU_KEY_ON($v1)
	jr		$ra
	nop