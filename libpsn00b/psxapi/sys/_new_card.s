.set noreorder
.section .text

.global _new_card
.type _new_card, @function
_new_card:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x50
	