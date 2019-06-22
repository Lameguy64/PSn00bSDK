.set noreorder

.section .text

.global GetInterruptCallback
.type GetInterruptCallback, @function
GetInterruptCallback:
	
	# a0 - Interrupt number
	
	la		$a1, _irq_func_table
	sll		$a0, 2
	addu	$a1, $a0
	
	jr		$ra
	lw		$v0, 0($a1)
	