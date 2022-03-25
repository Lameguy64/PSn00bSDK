.set noreorder

.include "hwregs_a.h"

.section .text


.global SetDispMask
.type SetDispMask, @function
SetDispMask:
	lui		$v1, IOBASE
	andi	$a0, 0x1
	lui		$v0, 0x300
	ori		$v0, 0x1
	sub		$v0, $a0
	sw		$v0, GPU_GP1($v1)
	jr		$ra
	nop

