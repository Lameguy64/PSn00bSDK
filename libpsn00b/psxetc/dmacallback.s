.set noreorder

.include "hwregs_a.h"

.section .text

.global DMACallback
.type DMACallback, @function
DMACallback:

	# a0 - DMA channel
	# a1 - Callback function
	
	addiu	$sp, -8
	sw		$ra, 0($sp)

	beqz	$a1, .Lremove_cb		# Remove callback if function is NULL
	nop
	
	addiu	$sp, -8					# Install IRQ handler for DMA handler
	sw		$a0, 0($sp)				# if not set installed yet
	sw		$a1, 4($sp)
	
	jal		GetInterruptCallback
	li		$a0, 3
	
	bnez	$v0, .Lskip_install
	nop
	
	la		$a1, _dma_handler
	jal		InterruptCallback
	li		$a0, 3

.Lskip_install:
	
	lw		$a0, 0($sp)
	lw		$a1, 4($sp)
	addiu	$sp, 8
	
	la		$v0, _dma_func_table
	sll		$v1, $a0, 2
	addu	$v0, $v1
	lw		$v1, 0($v0)
	sw		$a1, 0($v0)
	sw		$v1, 4($sp)
	
	lui		$a2, IOBASE
	
	lw		$v0, DMA_DICR($a2)		# Enable DMA interrupt
	lui		$v1, 0x1
	sll		$v1, $a0
	or		$v0, $v1
	lui		$v1, 0x80
	or		$v0, $v1
	sw		$v0, DMA_DICR($a2)
	
	b		.Lskip_remove
	nop
	
.Lremove_cb:

	la		$v0, _dma_func_table	# Set callback address
	sll		$v1, $a0, 2
	addu	$v0, $v1
	lw		$v1, 0($v0)
	sw		$a1, 0($v0)
	sw		$v1, 4($sp)
	
	lui		$a2, IOBASE				# Disable DMA interrupt
	lw		$v0, DMA_DICR($a2)
	lui		$v1, 0x1
	sll		$v1, $a0
	.set noat
	addiu	$at, $0, -1
	xor		$v1, $at
	and		$v0, $v1
	lui		$v1, 0x7f00
	xor		$v1, $at
	and		$v0, $v1
	.set at
	sw		$v0, DMA_DICR($a2)
	
	jal		_dma_has_cb				# Check if callbacks are present
	nop
	bnez	$v0, .Lskip_remove
	nop
	sw		$0 , DMA_DICR($a2)
	
	jal		GetInterruptCallback	# Check if callback is the DMA handler
	li		$a0, 3
	la		$v1, _dma_handler
	bne		$v0, $v1, .Lskip_remove
	nop
	
	li		$a0, 3
	jal		InterruptCallback
	move	$a1, $0
	
.Lskip_remove:
	
	lw		$ra, 0($sp)
	lw		$v0, 4($sp)
	jr		$ra
	addiu	$sp, 8

	
.type _dma_has_cb, @function
_dma_has_cb:

	la		$v1, _dma_func_table
	li		$t0, 6
	
.Lscan_loop:

	lw		$v0, 0($v1)
	addiu	$v1, 4
	bnez	$v0, .Lhas_cb
	nop
	
	bgtz	$t0, .Lscan_loop
	addiu	$t0, -1
	
	jr		$ra
	move	$v0, $0
	
.Lhas_cb:

	jr		$ra
	li		$v0, 1
	
	
.type _dma_handler, @function
_dma_handler:

	addiu	$sp, -12
	sw		$ra, 0($sp)
	sw		$s0, 4($sp)
	sw		$s1, 8($sp)
	
	move	$s0, $0
	la		$s1, _dma_func_table
	
.Lhandler_loop:

	lui		$a0, IOBASE
	lw		$v0, DMA_DICR($a0)
	li		$v1, 24
	addu	$v1, $s0
	srl		$v0, $v1
	andi	$v0, 0x1
	
	lw		$v1, 0($s1)
	
	beqz	$v0, .Lno_irq
	addiu	$s1, 4
	
	beqz	$v1, .Lno_irq
	nop
	
	jalr	$v1
	nop
	
.Lno_irq:
	
	blt		$s0, 6, .Lhandler_loop
	addi	$s0, 1
	
	lui		$a0, IOBASE
	lw		$v0, DMA_DICR($a0)
	nop
	sw		$v0, DMA_DICR($a0)
	
	lw		$ra, 0($sp)
	lw		$s0, 4($sp)
	lw		$s1, 8($sp)
	
	jr		$ra
	addiu	$sp, -12
	
	
.section .data

_dma_func_table:
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	