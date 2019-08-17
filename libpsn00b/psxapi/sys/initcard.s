.set noreorder
.section .text

.global InitCARD
.type InitCARD, @function
InitCARD:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x4a
	