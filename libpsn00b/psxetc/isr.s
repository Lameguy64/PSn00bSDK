.set noreorder

.include "hwregs_a.h"

.set ISR_STACK_SIZE, 4096


.section .text

# Global ISR handler of PSn00bSDK

.set at

.type _global_isr, @function
_global_isr:
	
.Lisr_loop:

	#la		$gp, _gp					# Keep restoring GP since it gets
										# changed elsewhere sometimes
	
	lui		$a0, IOBASE					# Get IRQ status
	lw		$v0, IRQ_MASK($a0)
	nop
	
	srl		$v0, $s1					# Check IRQ mask bit if set
	andi	$v0, 0x1
	
	beqz	$v0, .Lno_irq				# Don't execute callback if IRQ not enabled
	nop
	
	lw		$v0, IRQ_STAT($a0)
	nop
	srl		$v0, $s1					# Check IRQ status bit if set
	andi	$v0, 0x1
	beqz	$v0, .Lno_irq				# Don't execute callback if no IRQ
	nop
	
	lw		$v1, 0($s0)					# Load IRQ callback function
	nop
	
	lw		$v0, IRQ_STAT($a0)			# Acknowledge the IRQ (by writing a 0 bit)
	li		$a1, 1
	sll		$a1, $s1
	addiu	$a2, $0 , -1
	xor		$a1, $a2
	sw		$a1, IRQ_STAT($a0)
	
	beqz	$v1, .Lno_irq				# Don't execute if callback is not set
	nop

	jalr	$v1							# Call interrupt handler
	nop
	
.Lno_irq:

	addiu	$s0, 4
	
	blt		$s1, 11, .Lisr_loop
	addiu	$s1, 1

	j		ReturnFromException
	nop


.section .data

# Global ISR callback table

.global _irq_func_table
.type _irq_func_table, @object
_irq_func_table:
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0

# Global ISR hook structure
.global _custom_exit
.type _custom_exit, @object
_custom_exit:
	.word _global_isr			# pc
	.word _isr_stack+ISR_STACK_SIZE	# sp
	.word 0						# fp
	.word _irq_func_table		# s0
	.word 0						# s1
	.word 0						# s2
	.word 0						# s3
	.word 0						# s4
	.word 0						# s5
	.word 0						# s6
	.word 0						# s7
	.word _gp					# gp
	
# Global ISR stack
#	.fill 1024
#_custom_exit_stack:
#	.fill 4
.comm _isr_stack, ISR_STACK_SIZE+4
