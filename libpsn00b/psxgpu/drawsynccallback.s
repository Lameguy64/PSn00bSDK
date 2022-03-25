.set noreorder

.include "hwregs_a.h"

.text

.global DrawSyncCallback
.type DrawSyncCallback, @function
DrawSyncCallback:

	addiu	$sp, -8
	sw		$ra, 0($sp)
	sw		$a0, 4($sp)
	
	jal		EnterCriticalSection
	nop
	
	beqz	$a0, .Luninstall
	nop
	
	la		$a1, _drawsync_handler
	lw		$a1, 4($sp)
	jal		DMACallback
	li		$a0, 2
	
	b		.Lcontinue
	nop
	
.Luninstall:

	move	$a1, $0
	jal		DMACallback
	li		$a0, 2

.Lcontinue:

	lw		$a0, 4($sp)
	la		$v1, _drawsync_func
	lw		$v0, 0($v1)
	sw		$a0, 0($v1)
	sw		$v0, 4($sp)
	
.Lexit:

	jal		ExitCriticalSection
	nop
	
	lw		$ra, 0($sp)
	lw		$v0, 4($sp)
	jr		$ra
	addiu	$sp, 8
	
	
.type _drawsync_handler, @function
_drawsync_handler:

.Ldma_wait:
	la		$v0, _drawsync_func
	lw		$v0, 0($v0)
	nop
	beqz	$v0, .Lskip
	nop

	addiu	$sp, -4
	sw		$ra, 0($sp)

	lw		$v0, DMA2_CHCR($a0)
	nop
	srl		$v0, 24
	andi	$v0, 0x1
	
	bnez	$v0, .Ldma_wait
	nop

.Lgpu_wait:
	jal		ReadGPUstat
	nop
	srl		$v0, 28
	andi	$v0, 0x1
	beqz	$v0, .Lgpu_wait
	nop
	
	la		$v1, _drawsync_func
	lw		$v1, 0($v1)
	
	lui		$v0, 0x0400			# Set DMA direction to off
	sw		$v0, GPU_GP1($a0)
	
	jalr	$v1
	nop
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	
.Lskip:

	jr		$ra
	nop
	

.data

_drawsync_func:
	.word 0
	