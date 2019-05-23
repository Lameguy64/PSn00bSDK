.set noreorder
.set noat

.include "hwregs_a.h"

.section .text


.global ResetGraph						# Resets the GPU and installs a
.type ResetGraph, @function				# VSync event handler
ResetGraph:
	addiu	$sp, -8						# C style stack allocation (required if
	sw		$ra, 0($sp)					# you call BIOS functions from asm)
	sw		$a0, 4($sp)

	la		$v0, _hooks_installed		# Skip installing hooks if this function
	lbu		$v0, 0($v0)					# has already been called before once
	nop
	bnez	$v0, .skip_hook_init
	nop

	# Temporary, may help improve compatibility?
	#jal		SetDefaultExitFromException
	#nop

	jal		ChangeClearPAD				# Remove pad handler left by the BIOS
	move	$a0, $0

	li		$a0, 1
	jal		ChangeClearRCnt				# Remove RCnt handler
	move	$a1, $0

	jal		_96_remove					# Remove CD handling left by the BIOS
	nop

	lui		$a3, 0x1f80					# Base address for I/O

	lui		$v0, 0x3b33					# Enables DMA channel 6 (for ClearOTag)
	ori		$v0, 0x3b33					# Enables DMA channel 2
	sw		$v0, DPCR($a3)
	sw		$0 , DICR($a3)				# Clear DICR (not needed)

	li		$v0, 0x9					# Enable IRQ0 (vblank)
	sw		$v0, IMASK($a3)

	# Set an event handler

	li		$a0, 0xf2000003				# RCntCNT3 (vsync class)
	li		$a1, 0x2
	li		$a2, 0x1000
	la		$a3, _vsync_func			# VSync event handler

	jal		OpenEvent					# Open a VSync event handler
										# (PSXSDK style vsync handler)
	addiu	$sp, -16
	addiu	$sp, 16

	la		$v1, _vsync_event_desc		# Save event descriptor
	sw		$v0, 0($v1)

	move	$a0, $v0
	jal		EnableEvent					# Enable the opened event
	addiu	$sp, -4
	addiu	$sp, 4

	la		$v0, _hooks_installed		# Set installed flag
	li		$v1, 0x1
	sb		$v1, 0($v0)

	la		$v0, _vsync_counter			# Clear VSync counter
	sw		$0 , 0($v0)

	la		$v0, _vsync_callback_func	# Clear callback function
	sw		$0 , 0($v0)

	la		$a0, _custom_exit
	jal		SetCustomExitFromException
	addiu	$sp, -4
	addiu	$sp, 4
	
	jal		ExitCriticalSection			# Re-enable interrupts
	nop

.skip_hook_init:
	
	lui		$a3, 0x1f80

	lw		$v0, GP1($a3)				# Get video standard
	lui		$v1, 0x0010
	and		$v0, $v1
	la		$v1, _gpu_standard
	beqz	$v0, .not_pal
	sw		$0 , 0($v1)
	li		$v0, 1
	sw		$v0, 0($v1)
.not_pal:

	lw		$a0, 4($sp)				# Get argument value

	lui		$a3, 0x1f80					# Set base I/O again (likely destroyed
										# by previous calls)

	li		$v0, 0x1d00					# Configure timer 1 as Hblank counter
	sw		$v0, T1_MODE($a3)			# Set timer 1 value

	li		$at, 1
	beq		$a0, $at, .gpu_init_1
	nop
	li		$at, 3
	beq		$a0, $at, .gpu_init_3
	nop

	sw		$0 , GP1($a3)				# Reset the GPU

	b		.init_done
	nop

.gpu_init_1:

	sw		$0 , D2_CHCR($a3)			# Stop any DMA

.gpu_init_3:

	li		$v0, 0x1					# Reset the command buffer
	sw		$v0, GP1($a3)

.init_done:

	lw		$ra, 0($sp)
	lw		$a0, 4($sp)					# Return
	jr		$ra
	addiu	$sp, 8


.global _vsync_func						# VSync event handler, executed on
.type _vsync_func, @function			# every VBlank
_vsync_func:

	la		$gp, _gp

	lui		$at, 0x1f80					# Check if there's a VSync IRQ
	lw		$v0, IMASK($at)
	nop
	andi	$v0, $v0, 0x1
	beqz	$v0, .exit
	nop

	lw		$v1, ISTAT($at)
	nop
	andi	$v0, $v1, 0x1
	beqz	$v0, .exit
	nop
	
	#xori	$v1, $v1, 0x1				# Acknowledge the IRQ
	#sw		$v1, ISTAT($at)				# Commented out as it breaks BIOS pads

	la		$v1, _vsync_counter			# Increment VSync counter
	lw		$v0, 0($v1)
	nop
	addiu	$v0, 1
	sw		$v0, 0($v1)

	la		$v0, _vsync_callback_func	# Check if a callback function is set
	lw		$v0, 0($v0)
	nop
	beqz	$v0, .exit
	nop

	addiu	$sp, -0x20					# Save return address
	sw		$ra, 28($sp)

	jalr	$v0							# Execute user function
	nop

	lw		$ra, 28($sp)				# Restore previous return address
	addiu	$sp, 0x20

.exit:
	jr      $ra
	nop


.global _vsync_func_2
.type _vsync_func_2, @function
_vsync_func_2:
	
	lui		$at, 0x1f80					# Check if there's a VSync IRQ
	lw		$v0, IMASK($at)
	nop
	andi	$v0, $v0, 0x1
	beqz	$v0, .exit_2
	nop

	lw		$v1, ISTAT($at)
	nop
	andi	$v0, $v1, 0x1
	beqz	$v0, .exit_2
	nop
	
	xori	$v1, $v1, 0x1				# Acknowledge the IRQ
	sw		$v1, ISTAT($at)

.exit_2:
	
	j		ReturnFromException
	nop


.global VSync							# VSync function
.type VSync, @function
VSync:
	addiu	$sp, -4
	sw		$ra, 0($sp)

	la		$a1, _vsync_counter
	lw		$v0, 0($a1)
	nop
.loop:
	lw		$v1, 0($a1)
	nop
	beq		$v0, $v1, .loop
	nop

	la		$v0, _gpu_current_field		# Get last field value
	lbu		$v1, 0($v0)
.wait_field:							# Wait for field bit to change
	jal		ReadGPUstat
	nop
	srl		$v0, 31
	beq		$v0, $v1, .wait_field
	nop

	la		$v1, _gpu_current_field		# Store new field value
	sb		$v0, 0($v1)

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop


.section .data

.global library_credits
.type library_credits, @object
library_credits:
	.string "psxgpu programs by Lameguy64"


.type _custom_exit, @object
_custom_exit:
	.word _vsync_func_2		# pc
	.word _vsync_stack		# sp
	.word 0					# fp
	.word 0					# s0
	.word 0					# s1
	.word 0					# s2
	.word 0					# s3
	.word 0					# s4
	.word 0					# s5
	.word 0					# s6
	.word 0					# s7
	.word _gp				# gp

	.fill 60
_vsync_stack:
	.fill 4
	
.type _vsync_counter, @object
_vsync_counter:
	.word 0

.comm	_vsync_callback_func, 4, 4
.comm	_vsync_event_desc, 4, 4

.comm	_gpu_standard, 4, 4
.comm	_gpu_current_field, 4, 4
.comm	_hooks_installed, 4, 4
