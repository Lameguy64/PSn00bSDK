.set noreorder
.section .text

.global StartPad
.type StartPad, @function
StartPad:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x13
	