.set noreorder
.section .text

.global OpenEvent
.type OpenEvent, @function
OpenEvent:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x08
	