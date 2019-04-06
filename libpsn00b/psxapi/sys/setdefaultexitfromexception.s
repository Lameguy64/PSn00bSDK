.set noreorder
.section .text

.global SetDefaultExitFromException
.type SetDefaultExitFromException, @function
SetDefaultExitFromException:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x18

