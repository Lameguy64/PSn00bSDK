.set noreorder
.section .text

.global erase
.type erase, @function
erase:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x45
	