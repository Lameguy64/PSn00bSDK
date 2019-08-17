.set noreorder
.section .text

.global _card_wait
.type _card_wait, @function
_card_wait:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x5d
	