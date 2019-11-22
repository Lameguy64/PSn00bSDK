.set noreorder

.section .text

.global AddDummyTty
.type AddDummyTty, @function
AddDummyTty:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x99
