.set noreorder

.include "hwregs_a.inc"

.section .text


.global CdMix
.type CdMix, @function
CdMix:

	lui		$a3, IOBASE
	
	lbu		$t0, 0($a0)
	lbu		$t1, 1($a0)
	lbu		$t2, 2($a0)
	lbu		$t3, 3($a0)
	
	li		$v0, 2
	sb		$v0, CD_REG0($a3)
	sb		$t0, CD_REG2($a3)
	sb		$t1, CD_REG3($a3)
	
	li		$v0, 3
	sb		$v0, CD_REG0($a3)
	sb		$t2, CD_REG1($a3)
	sb		$t3, CD_REG2($a3)
	
	li		$v0, 0x20
	sb		$v0, CD_REG3($a3)
	
	jr		$ra
	li		$v0, 1
	