.set noreorder
.section .text

.global putchar
.type putc, @function
putchar:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x09
	