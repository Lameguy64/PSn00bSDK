.set noreorder
.section .text

.global _card_load
.type _card_load, @function
_card_load:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0xac
	