.set noreorder
.section .text

.global getchar
.type getchar, @function
getchar:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x3b
	