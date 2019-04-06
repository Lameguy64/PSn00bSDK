.set noreorder
.section .text

.global seek
.type seek, @function
seek:
	addiu	$t2, $0, 0xA0
	jr		$t2
	addiu	$t1, $0, 0x01
	