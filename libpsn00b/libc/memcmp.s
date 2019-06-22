# High speed ASM memcmp implementation by Lameguy64
#
# Part of PSn00bSDK

.set noreorder

.section .text

# Arguments:
#	a0 - buffer 1 address
#	a1 - buffer 2 address
#	a2 - bytes to compare
.global memcmp
.type memcmp, @function
memcmp:
	blez	$a2, .Lexit
	addi	$a2, -1
	lbu		$v0, 0($a0)
	lbu		$v1, 0($a1)
	addiu	$a0, 1
	bne		$v0, $v1, .Lmismatch
	addiu	$a1, 1
	b		memcmp
	nop
.Lmismatch:
	jr		$ra
	sub		$v0, $v1
.Lexit:
	jr		$ra
	move	$v0, $0
	