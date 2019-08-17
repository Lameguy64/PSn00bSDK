.set noreorder
.section .text

.global _card_status
.type _card_status, @function
_card_status:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x5c
	