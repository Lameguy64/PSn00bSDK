.set noreorder
.section .text

.include "hwregs_a.h"

.global InitPad
.type InitPad, @function
InitPad:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x12
