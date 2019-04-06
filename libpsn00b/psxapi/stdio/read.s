.set noreorder
.section .text

.global read
.type read, @function
read:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x02
	