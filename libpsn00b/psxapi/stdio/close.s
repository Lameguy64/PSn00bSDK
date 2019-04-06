.set noreorder
.section .text

.global close
.type close, @function
close:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x04
	