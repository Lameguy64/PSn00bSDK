.set noreorder
.section .text

.global _InitPad
.type _InitPad, @function
_InitPad:
	addiu	$t2, $0 , 0xb0
	jr		$t2
	addiu	$t1, $0 , 0x12
	