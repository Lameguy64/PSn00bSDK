.set noreorder

.section .text

.global AddDev
.type AddDev, @function
AddDev:
	addiu	$t2, $0, 0xb0
	jr		$t2
	addiu	$t1, $0, 0x47