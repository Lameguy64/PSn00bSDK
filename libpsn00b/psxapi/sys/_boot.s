.set noreorder
.section .text

.global _boot
.type _boot, @function
_boot:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0xa0
	