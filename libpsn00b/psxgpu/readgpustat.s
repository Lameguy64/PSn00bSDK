.set noreorder

.include "hwregs_a.h"

.section .text


.global ReadGPUstat
.type ReadGPUstat, @function
ReadGPUstat:
	lui     $v0, 0x1f80
	lw      $v0, GP1($v0)
	jr      $ra
	nop
