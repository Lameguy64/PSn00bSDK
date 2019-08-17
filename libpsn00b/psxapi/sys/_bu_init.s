.set noreorder
.section .text

.global _bu_init
.type _bu_init, @function
_bu_init:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x55
	