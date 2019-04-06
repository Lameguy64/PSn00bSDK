.set noreorder
.section .text

.global InitCard
.type InitCard, @function
InitCard:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x4a
	