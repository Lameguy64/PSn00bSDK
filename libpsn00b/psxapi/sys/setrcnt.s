.set noreorder

.section .text

.global SetRCnt
.type SetRCnt, @function
SetRCnt:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x02