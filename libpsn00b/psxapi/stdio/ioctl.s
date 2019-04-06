.set noreorder
.section .text

.global ioctl
.type ioctl, @function
ioctl:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x05
	