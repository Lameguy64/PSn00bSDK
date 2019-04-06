.set noreorder
.set noat

.section .text


.global rand
.type rand, @function
rand:
	
	la		$at, _randseed
	lw		$v0, 0($at)
	li		$v1, 0x41c64e6d
	
	multu	$v0, $v1
	mflo	$v0
	nop
	addiu	$v0, 12345
	sw		$v0, 0($at)
	
	jr		$ra
	andi	$v0, 0x7fff
	

.global srand
.type srand, @function
srand:
	la		$at, _randseed
	jr		$ra
	sw		$a0, 0($at)
	
	
.section .data

.type _randseed, @object
_randseed:
	.word 1
	