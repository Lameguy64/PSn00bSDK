.set noreorder
.section .text

.global _card_read
.type _card_read, @function
_card_read:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x4f
	