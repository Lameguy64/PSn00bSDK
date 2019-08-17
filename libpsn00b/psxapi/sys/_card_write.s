.set noreorder
.section .text

.global _card_write
.type _card_write, @function
_card_write:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x4e
	