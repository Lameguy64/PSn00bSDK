.set noreorder

.section .text

.global GetRCnt
.type GetRCnt, @function
GetRCnt:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x03