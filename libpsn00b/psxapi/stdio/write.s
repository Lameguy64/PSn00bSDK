.set noreorder
.section .text

.global write
.type write, @function
write:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x03
	