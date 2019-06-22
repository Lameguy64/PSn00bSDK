.set noreorder
.section .text

.global gets
.type gets, @function
gets:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x3d
	