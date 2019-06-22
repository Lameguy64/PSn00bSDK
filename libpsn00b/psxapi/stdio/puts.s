.set noreorder
.section .text

.global puts
.type puts, @function
puts:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x3e
	