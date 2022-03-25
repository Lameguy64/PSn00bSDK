.set noreorder

.include "hwregs_a.h"

.section .text


.global ReadGPUstat
.type ReadGPUstat, @function
ReadGPUstat:
	lui     $v0, IOBASE
	lw      $v0, GPU_GP1($v0)
	jr      $ra
	nop
