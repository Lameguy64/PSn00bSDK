.set noreorder

.include "hwregs_a.h"

.section .text


.global ClearOTagR
.type ClearOTagR, @function
ClearOTagR:
	lui		$a2, IOBASE
	addi	$v0, $a1, -1
	sll		$v0, 2
	addu	$a0, $v0
	sw		$a0, DMA6_MADR($a2)
	andi	$a1, 0xffff
	sw		$a1, DMA6_BCR($a2)
	lui		$v0, 0x1100
	addiu	$v0, 2
	jr		$ra
	sw		$v0, DMA6_CHCR($a2)
