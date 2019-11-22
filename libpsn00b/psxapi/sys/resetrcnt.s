.set noreorder

.section .text

.global ResetRCnt
.type ResetRCnt, @function
ResetRCnt:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x06