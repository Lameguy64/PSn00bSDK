.set noreorder
.section .text

.global open
.type open, @function
open:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x00
	