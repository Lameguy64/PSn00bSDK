.set noreorder
.section .text

.global _96_init
.type _96_init, @function
_96_init:
	addiu	$t2, $0 , 0xa0
	jr		$t2
	addiu	$t1, $0 , 0x71
	