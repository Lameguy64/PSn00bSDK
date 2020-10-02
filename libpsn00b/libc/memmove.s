.set noreorder

.section .text

# Arguments
#	a0 - destination address
#	a1 - source address
#	a2 - bytes to move
.global memmove
.type memmove, @function
memmove:
	move	$v0, $a0
	sltu	$v1, $a0, $a1
	blez	$v1, .Linit_backward
.Lloop_forward:
	blez	$a2, .Lexit
	addi	$a2, -1
	lbu		$v1, 0($a1)
	addiu	$a1, 1
	sb		$v1, 0($a0)
	addiu	$a0, 1
	b		.Lloop_forward
	nop
.Linit_backward:
	addu	$a0, $a2
	addu	$a1, $a2
	addiu	$a0, -1
	addiu	$a1, -1
	b		.Lloop_backward
	nop
.Lloop_backward:
	blez	$a2, .Lexit
	addi	$a2, -1
	lbu		$v1, 0($a1)
	addiu	$a1, -1
	sb		$v1, 0($a0)
	addiu	$a0, -1
	b		.Lloop_backward
	nop
.Lexit:
	jr		$ra
	nop