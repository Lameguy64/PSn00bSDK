.set noreorder

.include "hwregs_a.h"

.section .text


.global DrawOTag
.type DrawOTag, @function
DrawOTag:
	addiu	$sp, -4
	sw		$ra, 0($sp)

	lui		$a3, 0x1f80			# I/O segment base

.gpu_wait:						# Wait for GPU to be ready for commands & DMA
	jal		ReadGPUstat
	nop
	srl		$v0, 26
	andi	$v0, 1
	beqz	$v0, .gpu_wait
	nop

	lui		$v0, 0x0400			# Set DMA direction to CPUtoGPU
	ori		$v0, 0x2
	sw		$v0, GP1($a3)

	sw		$a0, D2_MADR($a3)	# Set DMA base address to specified OT
	sw		$0, D2_BCR($a3)

	lui		$v0, 0x0100			# Begin OT transfer!
	ori		$v0, 0x0401
	sw		$v0, D2_CHCR($a3)

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
