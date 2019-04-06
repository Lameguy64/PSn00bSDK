.set noreorder

.section .text

.global DelDev
.type DelDev, @function
DelDev:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x48