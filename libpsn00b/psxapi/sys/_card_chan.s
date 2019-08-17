.set noreorder
.section .text

.global _card_chan
.type _card_chan, @function
_card_chan:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x58
	