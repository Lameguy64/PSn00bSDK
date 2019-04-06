.set noreorder
.section .text

.global nextfile
.type nextfile, @function
nextfile:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x43
	