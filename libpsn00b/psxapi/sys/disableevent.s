.set noreorder
.section .text

.global DisableEvent
.type DisableEvent, @function
DisableEvent:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x0d
	