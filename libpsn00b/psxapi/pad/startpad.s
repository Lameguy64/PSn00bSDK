.set noreorder
.section .text

.global _StartPad
.type _StartPad, @function
_StartPad:
	addiu	$t2, $0 , 0xb0
	jr		$t2
	addiu	$t1, $0 , 0x13
	