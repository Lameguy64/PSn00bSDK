.set noreorder
.section .text

.global ExitCriticalSection
.type ExitCriticalSection, @function
ExitCriticalSection:
	addiu	$a0, $0, 2
	syscall	0
	jr		$ra
	nop
	