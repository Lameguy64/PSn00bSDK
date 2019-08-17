.set noreorder
.section .text

.global StartCARD
.type StartCARD, @function
StartCARD:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x4b
	