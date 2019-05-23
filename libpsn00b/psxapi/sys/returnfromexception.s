.set noreorder
.section .text

.global ReturnFromException
.type ReturnFromException, @function
ReturnFromException:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x17
	