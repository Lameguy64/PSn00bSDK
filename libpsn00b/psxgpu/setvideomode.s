.set noreorder

.include "hwregs_a.h"


.section .text

.global SetVideoMode
.type SetVideoMode, @function
SetVideoMode:
	addiu	$sp, -4
	sw		$ra, 0($sp)

	jal		ReadGPUstat
	nop

	srl		$a1, $v0, 17
	andi	$a1, 0x1f

	srl		$v1, $v0, 14			# Reverse flag
	andi	$v1, 1
	sll		$v1, 6
	or		$a1, $v1

	srl		$v1, $v0, 16			# Horizontal resolution 2
	andi	$v1, 1
	sll		$v1, 6
	or		$a1, $v1

	andi	$a1, 0xf7				# Mask off PAL bit

	la		$v0, _gpu_standard
	beqz	$a0, .Lset_done
	sw		$0 , 0($v0)
	li		$v1, 1
	sw		$v1, 0($v0)
	b		.Lset_done
	or		$a1, 0x8
.Lset_done:

	lui		$v0, 0x800				# Apply new mode
	or		$a1, $v0
	lui		$v0, IOBASE
	sw		$a1, GP1($v0)

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop

