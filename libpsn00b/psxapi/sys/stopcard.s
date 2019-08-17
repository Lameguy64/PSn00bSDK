.set noreorder
.section .text

.global StopCARD
.type StopCARD, @function
StopCARD:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x4c
	