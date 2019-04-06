.set noreorder


.section .text

.global GetVideoMode
.type GetVideoMode, @function
GetVideoMode:

	la		$v0, _gpu_standard
	lw		$v0, 0($v0)
	
	jr		$ra
	nop
