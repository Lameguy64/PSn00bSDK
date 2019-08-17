.set noreorder
.section .text

.global StopPAD
.type StopPAD, @function
StopPAD:
	addiu	$t2, $0 , 0xb0
	jr		$t2
	addiu	$t1, $0 , 0x14
	