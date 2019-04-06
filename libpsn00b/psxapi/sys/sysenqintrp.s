.set noreorder
.section .text

.global SysEnqIntRP
.type SysEnqIntRP, @function
SysEnqIntRP:
	addiu	$t2, $0, 0xc0
	jr		$t2
	addiu	$t1, $0, 0x02
	