.set noreorder
.section .text

.global EnterCriticalSection
.type EnterCriticalSection, @function
EnterCriticalSection:
	addiu	$a0, $0, 1
	syscall	0
	jr		$ra
	nop
	