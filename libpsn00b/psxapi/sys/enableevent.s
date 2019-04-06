.set noreorder
.section .text

.global EnableEvent
.type EnableEvent, @function
EnableEvent:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x0c
	