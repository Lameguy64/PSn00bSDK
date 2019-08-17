.set noreorder
.section .text

.global StartPAD
.type StartPAD, @function
StartPAD:
	addiu	$t2, $0 , 0xb0
	jr		$t2
	addiu	$t1, $0 , 0x13
	