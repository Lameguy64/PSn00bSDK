.set noreorder
.section .text

.global ChangeClearRCnt
.type ChangeClearRCnt, @function
ChangeClearRCnt:
	addiu	$t2, $0 , 0xc0
	jr		$t2
	addiu	$t1, $0 , 0x0a
	