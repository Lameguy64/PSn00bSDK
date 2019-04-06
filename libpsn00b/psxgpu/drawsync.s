.set noreorder

.include "hwregs_a.h"

.section .text


.global DrawSync
.type DrawSync, @function
DrawSync:
	addiu	$sp, -4
	sw		$ra, 0($sp)

.gpu_wait:						# Wait for GPU to be ready for commands and DMA
	jal		ReadGPUstat
	nop
	srl		$v0, 0x1a
	andi	$v0, 0x5
	li		$v1, 5
	bne		$v0, $v1, .gpu_wait
	nop

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
