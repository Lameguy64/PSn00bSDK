.set noreorder
.section .text

.global atoi
.type atoi, @function
atoi:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x10
