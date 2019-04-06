.set noreorder
.section .text

.global printf
.type printf, @function
printf:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x3f
	