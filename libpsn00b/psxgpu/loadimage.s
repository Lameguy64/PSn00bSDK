.set noreorder

.include "hwregs_a.h"

.set RECT_x,	0
.set RECT_y,	2
.set RECT_w,	4
.set RECT_h,	6

.section .text


.global LoadImage
.type LoadImage, @function
LoadImage:
	addiu	$sp, -8
	sw		$ra, 0($sp)
	sw		$s0, 4($sp)

	lui		$s0, 0x1f80			# Set I/O segment base address

.Lgpu_wait:						# Wait for GPU to be ready for commands and DMA
	jal		ReadGPUstat
	nop
	srl		$v0, 0x1a
	andi	$v0, 0x5
	li		$v1, 5
	#srl		$v0, 28
	#andi	$v0, 1
	bne		$v0, $v1, .Lgpu_wait
	nop

	lui		$v0, 0x400			# Set DMA direction to off
	sw		$v0, GP1($s0)

	lui		$v0, 0x0100			# Clear GPU cache
	sw		$v0, GP0($s0)

	lui		$v1, 0xa000			# Load image to VRAM
	sw		$v1, GP0($s0)
	lw		$v0, RECT_x($a0)	# Set XY and dimensions of image
	lw		$v1, RECT_w($a0)
	sw		$v0, GP0($s0)
	sw		$v1, GP0($s0)

	lui		$v0, 0x400			# Set DMA direction to CPUtoVRAM
	ori		$v0, 0x2
	sw		$v0, GP1($s0)

	lhu		$v0, RECT_w($a0)	# Get rectangle size
	lhu		$v1, RECT_h($a0)
	nop
	mult	$v0, $v1			# Calculate BCR value
	mflo	$v1
	srl		$v1, 0x4
	sll		$v1, 0x10
	ori		$v1, 0x8

	sw		$a1, DMA2_MADR($s0)	# Set DMA base address and transfer length
	sw		$v1, DMA2_BCR($s0)

	lui		$v0, 0x100			# Start DMA transfer
	ori		$v0, 0x201
	sw		$v0, DMA2_CHCR($s0)

	lw		$ra, 0($sp)
	lw		$s0, 4($sp)
	jr		$ra
	addiu	$sp, 8

