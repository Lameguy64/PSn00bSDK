.set noreorder
.section .text

.global SetCustomExitFromException
.type SetCustomExitFromException, @function
SetCustomExitFromException:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x19
	