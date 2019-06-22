.set noreorder

.include "hwregs_a.h"

.set DISP_dx,		0
.set DISP_dy,		2
.set DISP_dw,		4
.set DISP_dh,		6
.set DISP_sx,		8
.set DISP_sy,		10
.set DISP_sw,		12
.set DISP_sh,		14
.set DISP_inter,	16
.set DISP_isrgb24,	17
.set DISP_reverse,	18

.section .text


.global PutDispEnv
.type PutDispEnv, @function
PutDispEnv:

	lui		$a3, IOBASE

	# Horizontal resolution stuff

	lh		$a2, DISP_dw($a0)			# Get X resolution

	lh		$v0, DISP_sx($a0)
	lh		$v1, DISP_sw($a0)			# Get X screen width

	move	$a1, $0						# To use as mode value

	bgt		$a2, 560, .Lmode_640
	nop
	bgt		$a2, 400, .Lmode_512
	nop
	bgt		$a2, 352, .Lmode_384
	nop
	bgt		$a2, 280, .Lmode_320
	nop

	.set noat

.Lmode_256:
	li		$at, 10
	mult	$at, $v1
	li		$a2, 0x24e
	sll		$v0, 2
	add		$a2, $v0
	b		.Lmode_end
	li		$v1, 0xa00
.Lmode_320:
	li		$at, 8
	mult	$at, $v1
	li		$a2, 0x258
	ori		$a1, 0x01
	sll		$v0, 2
	add		$a2, $v0
	b		.Lmode_end
	li		$v1, 0xa00
.Lmode_384:
	li		$at, 7
	mult	$at, $v1
	li		$a2, 0x21b
	ori		$a1, 0x64
	sll		$v0, 2
	add		$a2, $v0
	b		.Lmode_end
	li		$v1, 0xa80
.Lmode_512:
	li		$at, 5
	mult	$at, $v1
	li		$a2, 0x267
	ori		$a1, 0x02
	sll		$v0, 2
	add		$a2, $v0
	b		.Lmode_end
	li		$v1, 0xa00
.Lmode_640:
	li		$at, 4
	mult	$at, $v1
	li		$a2, 0x26c
	ori		$a1, 0x03
	sll		$v0, 2
	add		$a2, $v0
	li		$v1, 0xa00
.Lmode_end:

	.set at

	mflo	$v0
	bnez	$v0, .Lno_default			# Check if screen with is non zero
	nop
	move	$v0, $v1					# Use default if screen width is 0
.Lno_default:

	addu	$v0, $a2					# Apply horizontal display coordinates
	sll		$v0, 12
	andi	$a2, 0xfff
	or		$a2, $v0
	lui		$v0, 0x0600
	or		$v0, $a2
	sw		$v0, GP1($a3)

	# Vertical resolution

	lh		$v0, DISP_dh($a0)
	li		$a2, 0x10
	ble		$v0, 256, .Lmode_low
	nop

.Lmode_high:
	ori		$a1, 0x04
.Lmode_low:
	lh		$v0, DISP_sy($a0)
	lh		$v1, DISP_sh($a0)
	add		$a2, $v0
	bnez	$v1, .Lno_default_vert
	nop
	li		$v1, 0xf0
.Lno_default_vert:
	add		$v1, $a2
	and		$a2, 0x3ff
	sll		$v1, 10
	or		$v1, $a2
	lui		$v0, 0x0700
	or		$v1, $v0
	sw		$v1, GP1($a3)

	# Video mode

	la		$v0, _gpu_standard
	lbu		$v0, 0($v0)
	nop
	beqz	$v0, .Lconfig_ntsc
	nop
.Lconfig_pal:
	ori		$a1, 0x08
.Lconfig_ntsc:

	lbu		$v0, DISP_inter($a0)
	lbu		$v1, DISP_isrgb24($a0)
	beqz	$v0, .Lno_inter
	nop
	or		$a1, 0x20
.Lno_inter:
	beqz	$v1, .Lno_rgb24
	nop
	or		$a1, 0x10
.Lno_rgb24:
	lbu		$v0, DISP_inter($a0)
	nop
	beqz	$v0, .Lno_reverse
	nop
	or		$a1, 0x80
.Lno_reverse:

	lui		$v0, 0x800					# Apply mode
	or		$a1, $v0
	sw		$a1, GP1($a3)

	lhu		$v0, DISP_dx($a0)			# Set VRAM XY offset
	lhu		$v1, DISP_dy($a0)
	andi	$v0, 0x3ff
	andi	$v1, 0x1ff
	sll		$v1, 10
	or		$v0, $v1
	lui		$v1, 0x500
	or		$v0, $v1

	jr		$ra
	sw		$v0, GP1($a3)
