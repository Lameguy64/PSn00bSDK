.set noreorder
.section .text

.global _card_info
.type _card_info, @function
_card_info:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0xab
	