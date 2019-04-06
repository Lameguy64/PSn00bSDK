.set noreorder

.section .text

.global VSyncCallback
.type VSyncCallback, @function
VSyncCallback:
	addiu	$sp, -8
	sw		$ra, 0($sp)

	jal		EnterCriticalSection
	sw		$a0, 4($sp)

	lw		$a0, 4($sp)
	la		$v0, _vsync_callback_func
	sw		$a0, 0($v0)

	jal		ExitCriticalSection
	nop

	lw		$ra, 0($sp)
	addiu	$sp, 8
	jr		$ra
	nop

