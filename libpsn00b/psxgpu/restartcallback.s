.set noreorder

.include "hwregs_a.h"

.section .text

.global RestartCallback
.type RestartCallback, @function
RestartCallback:

	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	la		$a0, _custom_exit
	jal		SetCustomExitFromException
	addiu	$sp, -12
	
	jal		ChangeClearPAD
	move	$a0, $0
	
	li		$a0, 3
	jal		ChangeClearRCnt
	move	$a1, $0
	
	
	la		$a0, _irq_func_table
	move	$a1, $0
	move	$v0, $0
	
.Lcheck_cbs:
	lw		$v1, 0($a0)
	nop
	beqz	$v1, .Lno_cb
	addiu	$a0, 4
	li		$v1, 1
	sll		$v1, $a1
	or		$v0, $v1
.Lno_cb:
	blt		$a1, 10, .Lcheck_cbs
	addiu	$a1, 1
	
	lui		$a0, IOBASE
	sw		$0 , ISTAT($a0)
	sw		$v0, IMASK($a0)
	
	addiu	$sp, 12
	lw		$ra, 0($sp)
	nop
	jr		$ra
	nop