.set noreorder

.section .text

.global GetSystemInfo
.type GetSystemInfo, @function
GetSystemInfo:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0xb4