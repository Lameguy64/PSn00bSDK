.set noreorder

.include "hwregs_a.h"

.section .text

.global InterruptCallback
.type InterruptCallback, @function
InterruptCallback:

	# a0 - Interrupt number
	# a1 - Callback function

	lui		$a2, IOBASE
	
	beqz	$a1, .Ldisable_irq
	nop

	lw		$v0, IMASK($a2)				# Enable interrupt mask
	li		$v1, 1
	sll		$v1, $a0
	or		$v0, $v1

	b		.Lcont
	sw		$v0, IMASK($a2)
	
.Ldisable_irq:

.set noat
	lw		$v0, IMASK($a2)				# Disable interrupt mask
	li		$v1, 1
	sll		$v1, $a0
	addiu	$at, $0 , -1
	xor		$v1, $at
.set at
	and		$v0, $v1
	sw		$v0, IMASK($a2)
	
.Lcont:

	la		$a2, _irq_func_table		# Get address to IRQ function table
	
	sll		$v1, $a0, 2					# Compute the slot
	addu	$v1, $a2, $v1
	lw		$v0, 0($v1)					# Get old handler address
	
	jr		$ra							# Return and set new IRQ handler
	sw		$a1, 0($v1)
