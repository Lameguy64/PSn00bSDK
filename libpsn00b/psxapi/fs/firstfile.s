.set noreorder
.section .text

.global firstfile
.type firstfile, @function
firstfile:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x42
	