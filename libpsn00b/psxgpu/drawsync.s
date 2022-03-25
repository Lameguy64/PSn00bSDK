.set noreorder

.include "hwregs_a.h"

.section .text


.global DrawSync
.type DrawSync, @function
DrawSync:

	bnez	$a0, .Lgetwords
	lui		$a0, IOBASE

	addiu	$sp, -4
	sw		$ra, 0($sp)

	jal		ReadGPUstat			# Check if DMA enabled
	nop
	srl		$v0, 29
	andi	$v0, 0x3
	
	beqz	$v0, .Lsimple_wait
	nop
	
.Ldma_wait:
	lw		$v0, DMA2_CHCR($a0)
	nop
	srl		$v0, 24
	andi	$v0, 0x1
	bnez	$v0, .Ldma_wait
	nop

.Lgpu_wait:
	jal		ReadGPUstat
	nop
	srl		$v0, 26
	andi	$v0, 0x5
	bne		$v0, 5, .Lgpu_wait
	nop
	
	b		.Lexit
	nop
	
.Lsimple_wait:					# Wait for GPU to be ready for next DMA
	jal		ReadGPUstat
	nop
	srl		$v0, 28
	andi	$v0, 0x1
	beqz	$v0, .Lsimple_wait
	nop

.Lexit:

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	
.Lgetwords:

	lw		$v0, DMA2_BCR($a0)
	nop

	jr		$ra
	srl		$v0, 16
	
