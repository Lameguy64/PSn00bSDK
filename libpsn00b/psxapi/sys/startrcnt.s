.set noreorder

.section .text

.global StartRCnt
.type StartRCnt, @function
StartRCnt:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x04