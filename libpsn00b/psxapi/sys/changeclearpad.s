.set noreorder
.section .text

.global ChangeClearPAD
.type ChangeClearPAD, @function
ChangeClearPAD:
	addiu	$t2, $0 , 0xb0
	jr		$t2
	addiu	$t1, $0 , 0x5b
	