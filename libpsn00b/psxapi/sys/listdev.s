.set noreorder
.section .text

.global ListDev
.type ListDev, @function
ListDev:
	addiu	$t2, $0 , 0xb0
	jr		$t2
	addiu	$t1, $0 , 0x49
	