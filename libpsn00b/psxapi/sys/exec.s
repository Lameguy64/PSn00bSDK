.set noreorder

.section .text

.global Exec
.type Exec, @function
Exec:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x43