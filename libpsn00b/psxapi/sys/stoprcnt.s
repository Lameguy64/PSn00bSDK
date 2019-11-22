.set noreorder

.section .text

.global StopRCnt
.type StopRCnt, @function
StopRCnt:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x05