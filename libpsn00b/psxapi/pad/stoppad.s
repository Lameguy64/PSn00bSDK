.set noreorder
.section .text

.global _StopPad
.type _StopPad, @function
_StopPad:
	addiu	$t2, $0 , 0xb0
	jr		$t2
	addiu	$t1, $0 , 0x14
	