.set noreorder
.set noat

.include "hwregs_a.h"

.set DRAW_x,		0		# Drawing area
.set DRAW_y,		2
.set DRAW_w,		4
.set DRAW_h,		6
.set DRAW_ofx,		8		# Draw offset
.set DRAW_ofy,		10
.set DRAW_tx,		12		# Texture window
.set DRAW_ty,		14
.set DRAW_tw,		16
.set DRAW_th,		18
.set DRAW_tpage,	20		# TPage values
.set DRAW_dtd,		22
.set DRAW_dfe,		23
.set DRAW_isbg,		24		# Clear draw area
.set DRAW_r0,		25
.set DRAW_g0,		26
.set DRAW_b0,		27
.set DRAW_env,		28


.section .text

.global PutDrawEnv
.type PutDrawEnv, @function
PutDrawEnv:
	addiu	$sp, -4
	sw		$ra, 0($sp)

	addiu	$a1, $a0, DRAW_env

	li		$v0, 0x04ffffff				# Packet header (length+terminator)
	sw		$v0, 0($a1)

	lhu		$v0, DRAW_x($a0)			# Set draw area top-left
	lhu		$v1, DRAW_y($a0)
	andi	$v0, 0x3ff
	andi	$v1, 0x1ff
	sll		$v1, 10
	or		$v0, $v1
	lui		$v1, 0xe300
	or		$v0, $v1
	sw		$v0, 4($a1)					# 1

	.set noat

	lhu		$v0, DRAW_w($a0)			# Set draw area bottom-right
	lhu		$at, DRAW_x($a0)
	addiu	$v0, -1
	addu	$at, $v0
	andi	$at, 0x3ff
	lhu		$v1, DRAW_h($a0)
	lhu		$v0, DRAW_y($a0)
	addiu	$v1, -1
	addu	$v0, $v1
	andi	$v0, 0x1ff
	sll		$v0, 10
	or		$at, $v0
	lui		$v0, 0xe400
	or		$at, $v0
	sw		$at, 8($a1)					# 2

	lhu		$v0, DRAW_x($a0)			# Set drawing offset
	lhu		$v1, DRAW_ofx($a0)
	nop
	add		$v0, $v1
	andi	$at, $v0, 0x7ff
	lhu		$v0, DRAW_y($a0)
	lhu		$v1, DRAW_ofy($a0)
	nop
	add		$v0, $v1
	andi	$v0, 0x7ff
	sll		$v0, 11
	or		$at, $v0
	lui		$v0, 0xe500
	or		$at, $v0
	sw		$at, 12($a1)				# 3

	lhu		$at, DRAW_tpage($a0)		# Set tpage
	lbu		$v0, DRAW_dtd($a0)
	lbu		$v1, DRAW_dfe($a0)
	andi	$v0, 1
	and		$v1, 1
	sll		$v0, 9
	sll		$v1, 10
	or		$at, $v0
	or		$at, $v1
	lui		$v0, 0xe100
	or		$at, $v0
	sw		$at, 16($a1)				# 4

	.set at

	lbu		$v0, DRAW_isbg($a0)
	nop
	beqz	$v0, .no_fillVRAM
	nop

	lw		$v0, DRAW_isbg($a0)			# FillVRAM
	lui		$v1, 0x0200
	srl		$v0, 8
	or		$v0, $v1
	sw		$v0, 20($a1)				# 5
	lw		$v0, DRAW_x($a0)
	lw		$v1, DRAW_w($a0)
	sw		$v0, 24($a1)				# 6

	srl		$v0, $v1, 16				# Workaround as rectangle primitives
	blt		$v0, 511, .no_overflow		# don't accept a height of 512
	nop

	li		$v0, 511
	sll		$v0, 16
	andi	$v1, 0xffff
	or		$v1, $v0

.no_overflow:
	sw		$v1, 28($a1)				# 7
	li		$v0, 0x07ffffff				# Packet header (length+terminator)
	sw		$v0, 0($a1)

.no_fillVRAM:

.gpu_wait:								# Wait for GPU to become ready for commands and DMA
	jal		ReadGPUstat
	nop
	srl		$v0, 26
	andi	$v0, 1
	beqz	$v0, .gpu_wait
	nop

	jal		DrawOTag
	move	$a0, $a1

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
