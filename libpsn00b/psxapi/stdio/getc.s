.set noreorder
.section .text

.global getc
.type getc, @function
getc:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x08
	