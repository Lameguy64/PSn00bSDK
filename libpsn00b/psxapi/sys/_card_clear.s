.set noreorder
.section .text

.global _card_clear
.type _card_clear, @function
_card_clear:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0xaf
	