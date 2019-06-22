# High speed ASM memset implementation by Lameguy64
#
# Part of PSn00bSDK

.set noreorder

.section .text

# Arguments:
#	a0 - address to buffer
#	a1 - value to set
#	a2 - bytes to set
.global memset
.type memset,@function
memset:
	move	$v0, $a0
	blez	$a2, .Lexit
	addi	$a2, -1
	sb		$a1, 0($a0)
	b		memset
	addiu	$a0, 1
.Lexit:
	jr		$ra
	nop
	