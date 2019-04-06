.set noreorder
.section .text

.global _96_remove
.type _96_remove, @function
_96_remove:
	addiu	$t2, $0 , 0xa0
	jr		$t2
	addiu	$t1, $0 , 0x72
	