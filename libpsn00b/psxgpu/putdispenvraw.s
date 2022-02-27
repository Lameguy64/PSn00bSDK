.set noreorder
.set noat

.include "hwregs_a.h"

.set DISP_mode,		0
.set DISP_vxpos,	4
.set DISP_vypos,	6
.set DISP_fbx,		8
.set DISP_fby,		10

.section .text


.global PutDispEnvRaw
.type PutDispEnvRaw, @function
PutDispEnvRaw:
	addiu	$sp, -8
	sw		$ra, 0($sp)
	sw		$s0, 4($sp)

	lui		$s0, IOBASE

	lh		$at, DISP_vxpos($a0)	# Set horizontal display range
	li		$v0, 608
	add		$v0, $at
	li		$v1, 3168
	add		$v1, $at
	sll		$v1, 12
	or		$v0, $v1
	lui		$v1, 0x600
	or		$v1, $v0
	sw		$v1, GPU_GP1($s0)

	lh		$at, DISP_vypos($a0)	# Set vertical display range (for NTSC)
	li		$v1, 120				# (values differet for PAL modes)
	sub		$v1, $at
	li		$v0, 136
	sub		$v0, $v1
	andi	$v0, 0x1ff
	li		$v1, 120
	add		$v1, $at
	li		$at, 136
	add		$at, $v1
	andi	$at, 0x1ff
	sll		$at, 10
	or		$v0, $at
	lui		$at, 0x700
	or		$v0, $at
	sw		$v0, GPU_GP1($s0)

	lw		$v0, DISP_mode($a0)		# Set video mode
	lui		$at, 0x800
	or		$v0, $at
	sw		$v0, GPU_GP1($s0)

	lhu		$v0, DISP_fbx($a0)		# Set VRAM XY offset
	lhu		$v1, DISP_fby($a0)
	andi	$v0, 0x3ff
	andi	$v1, 0x1ff
	sll		$v1, 10
	or		$v0, $v1
	lui		$v1, 0x500
	or		$v0, $v1
	sw		$v0, GPU_GP1($s0)

	lw		$ra, 0($sp)
	lw		$s0, 4($sp)
	jr		$ra
	addiu	$sp, 8

