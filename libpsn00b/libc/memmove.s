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
	addu	$a0, $a2
	addu	$a1, $a2
	addiu	$a0, -1
	addiu	$a1, -1
.loop:
	blez	$a2, .exit
	addi	$a2, -1
	lbu		$v1, 0($a1)
	addiu	$a1, -1
	sb		$v1, 0($a0)
	addiu	$a0, -1
	b		.loop
	nop
.exit:
	jr		$ra
	nop
	