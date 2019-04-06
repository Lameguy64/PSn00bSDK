.set noreorder
.section .text

.global b_InitHeap
.type b_InitHeap, @function
b_InitHeap:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x39
	