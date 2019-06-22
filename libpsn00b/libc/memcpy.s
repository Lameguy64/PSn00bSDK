# High speed ASM memcpy implementation by Lameguy64
#
# Part of PSn00bSDK

.set noreorder

.section .text

# Arguments: 
#	a0 - destination address
#	a1 - source adress
#	a2 - bytes to copy
.global memcpy
.type memcpy, @function
memcpy:
	move	$v0, $a0
.Lloop:
	blez	$a2, .exit
	addi	$a2, -1
	lbu		$a3, 0($a1)
	addiu	$a1, 1
	sb		$a3, 0($a0)
	b		.Lloop
	addiu	$a0, 1
.Lexit:
	jr		$ra
	nop
	