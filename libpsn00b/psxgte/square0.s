.set noreorder

.include "gtereg.h"
.include "inline_s.h"

.section .text


.global Square0
.type Square0, @function
Square0:

	# a0 - Pointer to input vector (v0)
	# a1 - Pointer to output vector (v1)

	lwc2	C2_IR1, 0($a0)
	lwc2	C2_IR2, 4($a0)
	lwc2	C2_IR3, 8($a0)

	nSQR(0)

	swc2	C2_IR1, 0($a1)
	swc2	C2_IR2, 4($a1)
	swc2	C2_IR3, 8($a1)

	jr		$ra
	nop
