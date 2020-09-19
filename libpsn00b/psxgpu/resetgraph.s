.set noreorder

.include "hwregs_a.h"

.section .text

.set ISR_STACK_SIZE, 4096

.global ResetGraph						# Resets the GPU and installs a
.type ResetGraph, @function				# VSync event handler
ResetGraph:
	addiu	$sp, -8						# C style stack allocation (required if
	sw		$ra, 0($sp)					# you call BIOS functions from asm)
	sw		$a0, 4($sp)

	la		$a0, resetgraph_msg
	move	$a1, $0
	move	$a2, $0
	la		$a1, _irq_func_table
	la		$a2, _custom_exit
	jal		printf
	addiu	$sp, -16
	addiu	$sp, 16
	
	la		$a0, sr_msg
	mfc0	$a1, $12
	jal		printf
	addiu	$sp, -16
	addiu	$sp, 16
	
	la		$v0, _hooks_installed		# Skip installing hooks if this function
	lbu		$v0, 0($v0)					# has already been called before once
	nop
	bnez	$v0, .Lskip_hook_init
	nop
	
	jal		EnterCriticalSection		# Disable interrupts as LoadExec() keeps
	nop									# interrupts enabled when transferring 
										# execution to the loaded program

	lui		$a3, 0x1f80					# Base address for I/O

	lui		$v0, 0x3b33					# Enables DMA channel 6 (for ClearOTag)
	ori		$v0, 0x3b33					# Enables DMA channel 2
	sw		$v0, DPCR($a3)
	sw		$0 , DICR($a3)				# Clear DICR (not needed)

	sw		$0 , IMASK($a3)				# Clear IRQ settings
	
	la		$v0, _hooks_installed		# Set installed flag
	li		$v1, 0x1
	sb		$v1, 0($v0)

	la		$v0, _vsync_cb_func			# Clear VSync callback function
	sw		$0 , 0($v0)

	la		$a1, _vsync_irq_callback	# Install VSync interrupt callback
	jal		InterruptCallback
	li		$a0, 0

	jal		RestartCallback
	nop

	la		$a0, cbhooks_msg
	jal		printf
	addiu	$sp, -16
	addiu	$sp, 16
	
	jal		_96_remove					# Remove CD handling left by the BIOS
	nop
	
	la		$a0, abouttoen_msg
	jal		printf
	addiu	$sp, -16
	addiu	$sp, 16
	
	jal		ExitCriticalSection			# Re-enable interrupts
	nop
	
	la		$a0, enableint_msg
	jal		printf
	addiu	$sp, -16
	addiu	$sp, 16
	
.Lskip_hook_init:
	
	lui		$a3, 0x1f80

	lw		$v0, GP1($a3)				# Get video standard
	lui		$v1, 0x0010
	and		$v0, $v1
	la		$v1, _gpu_standard
	beqz	$v0, .Lnot_pal
	sw		$0 , 0($v1)
	li		$v0, 1
	sw		$v0, 0($v1)
.Lnot_pal:

	lw		$a0, 4($sp)					# Get argument value

	lui		$a3, 0x1f80					# Set base I/O again (likely destroyed
										# by previous calls)

	li		$v0, 0x1d00					# Configure timer 1 as Hblank counter
	sw		$v0, T1_MODE($a3)			# Set timer 1 value
	
	beq		$a0, 1, .Lgpu_init_1
	nop
	beq		$a0, 3, .Lgpu_init_3
	nop

	sw		$0 , GP1($a3)				# Reset the GPU

	b		.Linit_done
	nop

.Lgpu_init_1:

	sw		$0 , D2_CHCR($a3)			# Stop any DMA

.Lgpu_init_3:

	li		$v0, 0x1					# Reset the command buffer
	sw		$v0, GP1($a3)

.Linit_done:

	lw		$ra, 0($sp)
	lw		$a0, 4($sp)					# Return
	jr		$ra
	addiu	$sp, 8


.global VSync							# VSync function
.type VSync, @function
VSync:
	
	addiu	$sp, -12
	sw		$ra, 0($sp)
	sw		$s0, 4($sp)
	
	lui		$a3, IOBASE					# Get GPU status (for interlace sync)
	lw		$s0, GP1($a3)
	
.Lhwait_loop:							# Get Hblank time
	lw		$v0, T1_CNT($a3)
	nop
	lw		$v1, T1_CNT($a3)
	nop
	bne		$v0, $v1, .Lhwait_loop
	nop
	
	la		$a3, _vsync_lasthblank		# Calculate Hblank time since last
	lw		$v1, 0($a3)
	nop
	subu	$v0, $v1
	andi	$v0, 0xffff
	
	beq		$a0, 1, .Lhblank_exit		# Return Hblank time only, no VSync
	sw		$v0, 8($sp)					# Stored as return value
	
	bgez	$a0, .Lvsync				# Vsync if argument is 0 and up
	nop
	
	la		$v0, _vsync_rcnt			# Return VSync count only
	lw		$v0, 0($v0)
	nop
	b		.Lvsync_exit
	sw		$v0, 8($sp)
	
.Lvsync:

	bnez	$a0, .Lnot_zero
	nop
	li		$a0, 1
	
.Lnot_zero:
	
	la		$v0, _vsync_rcnt			# Call vsync sub function (with timeout)
	lw		$v0, 0($v0)
	addiu	$a1, $a0, 1
	jal		_vsync_sub
	addu	$a0, $v0, $a0
	
	lui		$v0, 0x40
	and		$v0, $s0, $v0
	beqz	$v0, .Lhblank_exit
	nop
	
	lui		$a3, IOBASE					# Interlace wait logic
	
	lw		$v0, GP1($a3)
	nop
	xor		$v0, $s0, $v0
	bltz	$v0, .Lhblank_exit
	lui		$a0, 0x8000
	
.Linterlace_wait:
	lw		$v0, GP1($a3)
	nop
	xor		$v0, $s0, $v0
	and		$v0, $a0
	beqz	$v0, .Linterlace_wait
	nop
	
.Lhblank_exit:							# Set current Hblank as last value

	la		$a2, _vsync_lasthblank
	
.Lhwait2_loop:
	lw		$v0, T1_CNT($a3)
	nop
	lw		$v1, T1_CNT($a3)
	sw		$v0, 0($a2)
	bne		$v0, $v1, .Lhwait2_loop
	nop

.Lvsync_exit:

	lw		$ra, 0($sp)
	lw		$s0, 4($sp)
	lw		$v0, 8($sp)
	jr		$ra
	addiu	$sp, 12

	
.type _vsync_sub, @function	
_vsync_sub:

	# a0 - VSync destination count
	# a1 - Timeout ratio (number of vsyncs to wait relative to vsync count)

	addiu	$sp, -4
	sw		$ra, 0($sp)

	sll		$a1, 15						# Timeout counter
	
	la		$v0, _vsync_rcnt
	lw		$v0, 0($v0)
	nop
	bge		$v0, $a0, .Lvsync_sub_exit
	nop
	
.Lvsync_wait:

	addiu	$a1, -1
	
	la		$v1, 0xffffffff
	bne		$a1, $v1, .Lnot_timeout
	nop

	la		$a0, vsynctimeout_msg
	jal		puts
	addiu	$sp, -8
	
	jal		ChangeClearPAD
	move	$a0, $0
	
	li		$a0, 3
	jal		ChangeClearRCnt
	move	$a1, $0
	
	addiu	$sp, 8
	b		.Lvsync_sub_exit
	li		$v0, -1
	
.Lnot_timeout:

	la		$v0, _vsync_rcnt
	lw		$v0, 0($v0)
	nop
	blt		$v0, $a0, .Lvsync_wait
	nop
	
.Lvsync_sub_exit:

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	move	$v0, $0


	
.type _vsync_irq_callback, @function
_vsync_irq_callback:
	
	lui		$a0, IOBASE
	
	la		$v1, _vsync_rcnt			# Increment VSync root counter
	lw		$v0, 0($v1)
	nop
	addiu	$v0, 1
	sw		$v0, 0($v1)

	la		$v0, _vsync_cb_func			# Check if a callback function is set
	lw		$v0, 0($v0)
	nop
	beqz	$v0, .Lno_callback
	nop
	
	addiu	$sp, -4						# Save return address
	sw		$ra, 0($sp)
	jalr	$v0							# Execute user callback function
	nop
	lw		$ra, 0($sp)					# Restore previous return address
	addiu	$sp, 4
	
	lui		$a0, IOBASE
	
.Lno_callback:

	jr		$ra
	nop
	

.section .data

# VSync root counter
.type _vsync_rcnt, @object
_vsync_rcnt:
	.word 0

.type _vsync_lasthblank, @object
_vsync_lasthblank:
	.word 0
	
.comm	_vsync_cb_func, 4, 4

.comm	_gpu_standard, 4, 4
.comm	_gpu_current_field, 4, 4
.comm	_hooks_installed, 4, 4

	
.type vsynctimeout_msg, @object
vsynctimeout_msg:
	.asciiz "VSync: timeout\n"
	
.type resetgraph_msg, @object
resetgraph_msg:
	.asciiz "ResetGraph:itb=%08x,ehk=%08x\n"
	
.type enableint_msg, @object
enableint_msg:
	.asciiz "ResetGraph:Interrupts enabled!\n"
	
.type cbhooks_msg, @object
cbhooks_msg:
	.asciiz "ResetGraph:Interrupt hooks enabled.\n"
	
.type abouttoen_msg, @object
abouttoen_msg:
	.asciiz "ResetGraph:About to init interrupts.\n"
	
.type sr_msg, @object
sr_msg:
	.asciiz "ResetGraph:SR=%x\n"
	
.global psxgpu_credits
.type psxgpu_credits, @object
psxgpu_credits:
	.ascii "psxgpu programs by Lameguy64\n"
	.asciiz "2020 PSn00bSDK Project / Meido-Tek Productions\n"
	