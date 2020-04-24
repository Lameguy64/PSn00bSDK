.set noreorder

.include "gtereg.h"
.include "inline_s.h"
.include "smd_s.h"


.section .text

.global smdSortModelFlat
.type smdSortModelFlat, @function
smdSortModelFlat:
	# a0 - Pointer SC_OT structure
	# a1 - Pointer to next primitive
	# a2 - Pointer to SMD data address
	# v0 - New pointer of primitive buffer (return)
	
	addiu	$sp, -16
	sw		$s0, 0( $sp )
	sw		$s1, 4( $sp )
	sw		$s2, 8( $sp )
	sw		$s3, 12( $sp )
	
	la		$v0, _sc_clip
	lw		$t8, 0($v0)
	lw		$t9, 4($v0)
	
	lw		$t1, SMD_HEAD_PVERTS( $a2 )
	lw		$t2, SMD_HEAD_PNORMS( $a2 )
	lw		$t3, SMD_HEAD_PPRIMS( $a2 )
	
.sort_loop:

	nop
	lw		$a3, 0($t3)				# Get primitive ID word
	move	$t4, $t3
	
	beqz	$a3, .exit				# Check if terminator (just zero)
	addiu	$t4, 4
	
	lhu		$t5, 0( $t4 )			# Load vertices
	lhu		$t6, 2( $t4 )
	lhu		$t7, 4( $t4 )
	sll		$t5, 3
	sll		$t6, 3
	sll		$t7, 3
	addu	$t5, $t1
	addu	$t6, $t1
	addu	$t7, $t1
	lwc2	C2_VXY0, 0( $t5 )
	lwc2	C2_VZ0 , 4( $t5 )
	lwc2	C2_VXY1, 0( $t6 )
	lwc2	C2_VZ1 , 4( $t6 )
	lwc2	C2_VXY2, 0( $t7 )
	lwc2	C2_VZ2 , 4( $t7 )
	
	srl		$v1, $a3, 24			# Get primitive size
	addu	$t3, $v1				# Step main pointer to next primitive
	
	RTPT
	
	cfc2	$v0, C2_FLAG			# Get GTE flag value
	nop
	
	bltz	$v0, .skip_prim			# Skip primitive if Z overflow
	nop
	
	#NCLIP							# Backface culling
	
	#mfc2	$v1, C2_MAC0
	
	andi	$v0, $a3, 0x3
	
	#bltz	$v1, .skip_prim
	#nop
	
	beq		$v0, 0x1, .prim_tri		# If primitive is a triangle
	nop
	beq		$v0, 0x2, .prim_quad	# If primitive is a quad
	nop
	
	b		.skip_prim
	nop

## Triangles

.prim_tri:								# Triangle processing

	addiu	$t4, 8						# Advance from indices

	#AVSZ3								# Calculate average Z
	
	ClipTestTri
	
	and		$v0, $s0, $s1				# v0 & v1
	beqz	$v0, .do_draw
	and		$v0, $s1, $s2				# v1 & v2
	beqz	$v0, .do_draw
	and		$v0, $s2, $s0				# v2 & v0
	beqz	$v0, .do_draw
	nop
	b		.skip_prim
	nop
	
.do_draw:
	
	#srl		$v0, $a3, 2					# Lighting enabled?
	#andi	$v0, 0x3
	#bnez	$v0, .F3_light
	#nop

	andi	$v0, $a3, 0x20				# Textured triangle
	bnez	$v0, .F3_textured
	nop
	
	andi	$v0, $a3, 0x10				# Gouraud shaded
	bnez	$v0, .F3_gouraud
	nop
	
	lw		$v0, 0( $t4 )				# Flat color, no lighting
	lui		$v1, 0x2000
	or		$v0, $v1
	
	b		.sort_F3_pri
	sw		$v0, POLYF3_rgbc( $a1 )
	
.F3_textured:

	lw		$v0, 0( $t4 )				# Flat color, no lighting
	lui		$v1, 0x2400
	or		$v0, $v1
	sw		$v0, POLYFT3_rgbc( $a1 )
	addiu	$t4, 4
	
	lhu		$v0, 0( $t4 )				# Load texture coordinates
	lhu		$v1, 2( $t4 )
	sh		$v0, POLYFT3_uv0( $a1 )
	lhu		$v0, 4( $t4 )
	sh		$v1, POLYFT3_uv1( $a1 )
	sh		$v0, POLYFT3_uv2( $a1 )
	
	lw		$v0, 8( $t4 )				# Tpage + CLUT
	nop
	andi	$v1, $v0, 0xffff
	sh		$v1, POLYFT3_tpage( $a1 )
	srl		$v0, 16
	
	b		.sort_FT3_pri
	sh		$v0, POLYFT3_clut( $a1 )
	
.F3_gouraud:
	lw		$v0, 0($t4)
	lw		$v1, 4($t4)
	.set noat
	lui		$at, 0x3000
	or		$v0, $at
	.set at
	sw		$v0, POLYG3_rgbc0($a1)
	lw		$v0, 8($t4)
	sw		$v1, POLYG3_rgbc1($a1)
	b		.sort_G3_pri
	sw		$v0, POLYG3_rgbc2($a1)
	
#.F3_light:
	
	#lhu		$v0, 0( $t4 )				# Load normal 0
	
	#srl		$v1, $a3, 2
	#andi	$v1, $v1, 0x3
	
	#sll		$v0, 3
	#addu	$v0, $t2
	#lwc2	C2_VXY0, 0( $v0 )
	#lwc2	C2_VZ0 , 4( $v0 )

	#beq		$v1, 0x2, .F3_light_smt
	#nop
	
	#lw		$v0, 4( $t4 )
	#lui		$v1, 0x2000
	#or		$v0, $v1
	#mtc2	$v0, C2_RGB
	
	#addiu	$t4, 8
	#nop
	
	#NCCS

	#andi	$v0, $a3, 0x20				# Textured triangle
	#bnez	$v0, .F3_light_tex
	#nop
	
	#swc2	C2_RGB2, POLYF3_rgbc( $a1 )
	
	#b		.sort_F3_pri
	#nop
	
#.F3_light_tex:

	#lhu		$v0, 0( $t4 )				# Load texture coordinates
	#lhu		$v1, 2( $t4 )
	#sh		$v0, POLYFT3_uv0( $a1 )
	#lhu		$v0, 4( $t4 )
	#sh		$v1, POLYFT3_uv1( $a1 )
	#sh		$v0, POLYFT3_uv2( $a1 )
	
	#lw		$v1, 8( $t4 )
	#nop
	#andi	$v0, $v1, 0xffff
	#sh		$v0, POLYFT3_tpage( $a1 )
	#srl		$v0, $v1, 16
	#sh		$v0, POLYFT3_clut( $a1 )
	
	#mfc2	$v0, C2_RGB2
	#lui		$v1, 0x2400
	#or		$v0, $v1
	
	#b		.sort_FT3_pri
	#sw		$v0, POLYFT3_rgbc( $a1 )
	
#.F3_light_smt:
	
	#lhu		$v0, 2( $t4 )			# Load normals 1 and 2
	#lhu		$v1, 4( $t4 )
	#sll		$v0, 3
	#sll		$v1, 3
	#addu	$v0, $t2
	#addu	$v1, $t2
	#lwc2	C2_VXY1, 0( $v0 )
	#lwc2	C2_VZ1 , 4( $v0 )
	#lw		$v0, 8( $t4 )
	#lwc2	C2_VXY2, 0( $v1 )
	#lwc2	C2_VZ2 , 4( $v1 )
	#lui		$v1, 0x3000				# Load color
	#or		$v0, $v1
	#mtc2	$v0, C2_RGB
	
	#addiu	$t4, 12
	#nop
	
	#NCCT
	
	#andi	$v0, $a3, 0x20				# Textured triangle
	#bnez	$v0, .F3_light_tex_smt
	#nop
	
	#swc2	C2_RGB0, POLYG3_rgbc0( $a1 )
	#swc2	C2_RGB1, POLYG3_rgbc1( $a1 )
	#swc2	C2_RGB2, POLYG3_rgbc2( $a1 )
	
	#b		.sort_G3_pri
	#nop

# .F3_light_tex_smt:

	# lhu		$v0, 0( $t4 )				# Load texture coordinates
	# lhu		$v1, 2( $t4 )
	# sh		$v0, POLYGT3_uv0( $a1 )
	# lhu		$v0, 4( $t4 )
	# sh		$v1, POLYGT3_uv1( $a1 )
	# sh		$v0, POLYGT3_uv2( $a1 )
	
	# lw		$v1, 8( $t4 )
	# nop
	# andi	$v0, $v1, 0xffff
	# sh		$v0, POLYGT3_tpage( $a1 )
	# srl		$v0, $v1, 16
	# sh		$v0, POLYGT3_clut( $a1 )
	
	# mfc2	$v0, C2_RGB0
	# lui		$v1, 0x3400
	# or		$v0, $v1
	
	# swc2	C2_RGB1, POLYGT3_rgbc1( $a1 )
	# swc2	C2_RGB2, POLYGT3_rgbc2( $a1 )
	
	# b		.sort_GT3_pri
	# sw		$v0, POLYGT3_rgbc0( $a1 )

.sort_F3_pri:

	swc2	C2_SXY0, POLYF3_xy0($a1)
	swc2	C2_SXY1, POLYF3_xy1($a1)
	swc2	C2_SXY2, POLYF3_xy2($a1)
	
	la		$v0, _smd_tpage_base
	lhu		$v0, 0($v0)
	srl		$v1, $a3, 6				# Get blend mode
	andi	$v1, 0x3
	sll		$v1, 5
	or		$v0, $v1
	lui		$v1, 0xe100
	or		$v0, $v1
	sw		$v0, POLYF3_tpage($a1)	# Store TPage
	
	.set noat

	lui		$v1, 0x0500
	lw		$v0, 0($a0)
	lui		$at, 0xff00
	and		$v1, $at
	lui		$at, 0x00ff
	or		$at, 0xffff
	and		$v0, $at
	or		$v1, $v0
	sw		$v1, 0($a1)
	lw		$v0, 0($a0)
	and		$a1, $at
	lui		$at, 0xff00
	and		$v0, $at
	or		$v0, $a1
	sw		$v0, 0($a0)
	
	.set at

	lui		$v0, 0x8000
	or		$a1, $v0
	
	b		.sort_loop
	addiu	$a1, POLYF3_len
	
.sort_FT3_pri:

	swc2	C2_SXY0, POLYFT3_xy0( $a1 )
	swc2	C2_SXY1, POLYFT3_xy1( $a1 )
	swc2	C2_SXY2, POLYFT3_xy2( $a1 )
	
	.set noat

	lui		$v1, 0x0700
	lw		$v0, 0($a0)
	lui		$at, 0xff00
	and		$v1, $at
	lui		$at, 0x00ff
	or		$at, 0xffff
	and		$v0, $at
	or		$v1, $v0
	sw		$v1, 0($a1)
	lw		$v0, 0($a0)
	and		$a1, $at
	lui		$at, 0xff00
	and		$v0, $at
	or		$v0, $a1
	sw		$v0, 0($a0)
	
	.set at

	lui		$v0, 0x8000
	or		$a1, $v0
	
	b		.sort_loop
	addiu	$a1, POLYFT3_len
	
.sort_G3_pri:

	swc2	C2_SXY0, POLYG3_xy0( $a1 )
	swc2	C2_SXY1, POLYG3_xy1( $a1 )
	swc2	C2_SXY2, POLYG3_xy2( $a1 )
	
	la		$v0, _smd_tpage_base
	lhu		$v0, 0($v0)
	srl		$v1, $a3, 6				# Get blend mode
	andi	$v1, 0x3
	sll		$v1, 5
	or		$v0, $v1
	lui		$v1, 0xe100
	or		$v0, $v1
	sw		$v0, POLYG3_tpage($a1)	# Store TPage
	
	.set noat

	lui		$v1, 0x0700
	lw		$v0, 0($a0)
	lui		$at, 0xff00
	and		$v1, $at
	lui		$at, 0x00ff
	or		$at, 0xffff
	and		$v0, $at
	or		$v1, $v0
	sw		$v1, 0($a1)
	lw		$v0, 0($a0)
	and		$a1, $at
	lui		$at, 0xff00
	and		$v0, $at
	or		$v0, $a1
	sw		$v0, 0($a0)
	
	.set at

	lui		$v0, 0x8000
	or		$a1, $v0
	
	b		.sort_loop
	addiu	$a1, POLYG3_len
	
.sort_GT3_pri:

	swc2	C2_SXY0, POLYGT3_xy0( $a1 )
	swc2	C2_SXY1, POLYGT3_xy1( $a1 )
	swc2	C2_SXY2, POLYGT3_xy2( $a1 )
	
	.set noat

	lui		$v1, 0x0900
	lw		$v0, 0($a0)
	lui		$at, 0xff00
	and		$v1, $at
	lui		$at, 0x00ff
	or		$at, 0xffff
	and		$v0, $at
	or		$v1, $v0
	sw		$v1, 0($a1)
	lw		$v0, 0($a0)
	and		$a1, $at
	lui		$at, 0xff00
	and		$v0, $at
	or		$v0, $a1
	sw		$v0, 0($a0)
	
	.set at

	lui		$v0, 0x8000
	or		$a1, $v0
	
	b		.sort_loop
	addiu	$a1, POLYGT3_len

## Quads
	
.prim_quad:							# Quad processing

	mfc2	$t6, C2_SXY0			# Retrieve first projected vertex

	lhu		$t5, 6( $t4 )			# Project the last vertex
	addiu	$t4, 8
	sll		$t5, 3
	addu	$t5, $t1
	lwc2	C2_VXY0, 0( $t5 )
	lwc2	C2_VZ0 , 4( $t5 )
	
	nRTPS
	
	cfc2	$v1, C2_FLAG			# Get GTE flag value
	
	nop
	
	bltz	$v1, .skip_prim
	nop
	
	ClipTestQuad
	
	and		$v0, $s0, $s1				# v0 & v1
	beqz	$v0, .do_draw_q
	and		$v0, $s1, $s2				# v1 & v2
	beqz	$v0, .do_draw_q
	and		$v0, $s2, $s3				# v2 & v3
	beqz	$v0, .do_draw_q
	and		$v0, $s3, $s0				# v3 & v0
	beqz	$v0, .do_draw_q
	and		$v0, $s0, $s2				# v0 & v2
	beqz	$v0, .do_draw_q
	and		$v0, $s1, $s3				# v1 & v3
	beqz	$v0, .do_draw_q
	nop
	b		.skip_prim
	nop
	
.do_draw_q:

	srl		$v0, $a3, 2					# Lighting enabled?
	andi	$v0, 0x3
	bnez	$v0, .F4_light
	nop
	
	andi	$v0, $a3, 0x10				# Gouraud quad
	bnez	$v0, .F4_gouraud
	nop
	
	andi	$v0, $a3, 0x20				# Textured quad
	bnez	$v0, .F4_textured
	nop
	
	lw		$v0, 0($t4)
	lui		$v1, 0x2800
	or		$v0, $v1
	
	b		.sort_F4_pri
	sw		$v0, POLYF4_rgbc($a1)

.F4_textured:
	
	lw		$v0, 0($t4)
	lui		$v1, 0x2c00
	or		$v0, $v1
	sw		$v0, POLYFT4_rgbc( $a1 )
	addiu	$t4, 4
	
	lhu		$v0, 0($t4)					# Load texture coordinates
	lhu		$v1, 2($t4)
	sh		$v0, POLYFT4_uv0( $a1 )
	lhu		$v0, 4( $t4 )
	sh		$v1, POLYFT4_uv1( $a1 )
	lhu		$v1, 6( $t4 )
	sh		$v0, POLYFT4_uv2( $a1 )
	sh		$v1, POLYFT4_uv3( $a1 )
	
	lw		$v1, 8( $t4 )
	nop
	andi	$v0, $v1, 0xffff
	sh		$v0, POLYFT4_tpage( $a1 )
	srl		$v0, $v1, 16
	
	b		.sort_FT4_pri
	sh		$v0, POLYFT4_clut($a1)
	
.F4_gouraud:

	lw		$v0, 0($t4)
	lw		$v1, 4($t4)
	.set noat
	lui		$at, 0x3800
	or		$v0, $at
	.set at
	sw		$v0, POLYG4_rgbc0($a1)
	lw		$v0, 8($t4)
	sw		$v1, POLYG4_rgbc1($a1)
	lw		$v1, 12($t4)
	sw		$v0, POLYG4_rgbc2($a1)
	b		.sort_G4_pri
	sw		$v1, POLYG4_rgbc3($a1)
	

.F4_light:

	lhu		$v0, 0( $t4 )				# Load normal 0
	
	srl		$v1, $a3, 2
	andi	$v1, $v1, 0x3
	
	sll		$v0, 3
	addu	$v0, $t2
	lwc2	C2_VXY0, 0( $v0 )
	lwc2	C2_VZ0 , 4( $v0 )

	beq		$v1, 0x2, .F4_light_smt
	nop
	
	lw		$v0, 4( $t4 )
	lui		$v1, 0x2800
	or		$v0, $v1
	mtc2	$v0, C2_RGB
	
	addiu	$t4, 8
	nop
	
	NCCS
	
	andi	$v0, $a3, 0x20				# Textured triangle
	bnez	$v0, .F4_light_tex
	nop

	swc2	C2_RGB2, POLYF4_rgbc( $a1 )
	
	b		.sort_F4_pri
	nop
	
.F4_light_tex:

	lhu		$v0, 0( $t4 )				# Load texture coordinates
	lhu		$v1, 2( $t4 )
	sh		$v0, POLYFT4_uv0( $a1 )
	lhu		$v0, 4( $t4 )
	sh		$v1, POLYFT4_uv1( $a1 )
	lhu		$v1, 6( $t4 )
	sh		$v0, POLYFT4_uv2( $a1 )
	sh		$v1, POLYFT4_uv3( $a1 )
	
	lw		$v1, 8( $t4 )
	nop
	andi	$v0, $v1, 0xffff
	sh		$v0, POLYFT4_tpage( $a1 )
	srl		$v0, $v1, 16
	sh		$v0, POLYFT4_clut( $a1 )
	
	mfc2	$v0, C2_RGB2
	lui		$v1, 0x2c00
	or		$v0, $v1
	
	b		.sort_FT4_pri
	sw		$v0, POLYFT4_rgbc( $a1 )
	
.F4_light_smt:

	lhu		$v0, 2( $t4 )			# Load normals 1 and 2
	lhu		$v1, 4( $t4 )
	sll		$v0, 3
	sll		$v1, 3
	addu	$v0, $t2
	addu	$v1, $t2
	lwc2	C2_VXY1, 0( $v0 )
	lwc2	C2_VZ1 , 4( $v0 )
	lwc2	C2_VXY2, 0( $v1 )
	lwc2	C2_VZ2 , 4( $v1 )
	
	lw		$v0, 8( $t4 )
	lui		$v1, 0x3800				# Load color
	or		$v0, $v1
	mtc2	$v0, C2_RGB
	
	nNCCT
	
	lhu		$v0, 6( $t4 )			# Load normal 3
	
	addiu	$t4, 12
	
	sll		$v0, 3
	addu	$v0, $t2
	lwc2	C2_VXY0, 0( $v0 )
	lwc2	C2_VZ0 , 4( $v0 )
	
	andi	$v0, $a3, 0x20				# Textured triangle
	bnez	$v0, .F4_light_tex_smt
	nop
	
	swc2	C2_RGB0, POLYG4_rgbc0( $a1 )
	swc2	C2_RGB1, POLYG4_rgbc1( $a1 )
	swc2	C2_RGB2, POLYG4_rgbc2( $a1 )
	
	nNCCS
	
	swc2	C2_RGB2, POLYG4_rgbc3( $a1 )
	
	b		.sort_G4_pri
	nop
	
.F4_light_tex_smt:

	mfc2	$v0, C2_RGB0
	lui		$v1, 0x3400
	or		$v0, $v1
	sw		$v0, POLYGT4_rgbc0( $a1 )
	swc2	C2_RGB1, POLYGT4_rgbc1( $a1 )
	swc2	C2_RGB2, POLYGT4_rgbc2( $a1 )
	
	NCCS
	
	lhu		$v0, 0( $t4 )				# Load texture coordinates
	lhu		$v1, 2( $t4 )
	sh		$v0, POLYGT4_uv0( $a1 )
	lhu		$v0, 4( $t4 )
	sh		$v1, POLYGT4_uv1( $a1 )
	lhu		$v1, 6( $t4 )
	sh		$v0, POLYGT4_uv2( $a1 )
	sh		$v1, POLYGT4_uv3( $a1 )

	lw		$v1, 8( $t4 )
	swc2	C2_RGB2, POLYGT4_rgbc3( $a1 )
	
	andi	$v0, $v1, 0xffff
	sh		$v0, POLYGT4_tpage( $a1 )
	srl		$v0, $v1, 16
	
	b		.sort_GT4_pri
	sh		$v0, POLYGT4_clut( $a1 )
	
.sort_F4_pri:

	sw		$t6, POLYF4_xy0($a1)
	swc2	C2_SXY0, POLYF4_xy1($a1)
	swc2	C2_SXY1, POLYF4_xy2($a1)
	swc2	C2_SXY2, POLYF4_xy3($a1)
	
	la		$v0, _smd_tpage_base
	lhu		$v0, 0($v0)
	srl		$v1, $a3, 6				# Get blend mode
	andi	$v1, 0x3
	sll		$v1, 5
	or		$v0, $v1
	lui		$v1, 0xe100
	or		$v0, $v1
	sw		$v0, POLYF4_tpage($a1)	# Store TPage
	
	.set noat

	lui		$v1, 0x0600
	lw		$v0, 0($a0)
	lui		$at, 0xff00
	and		$v1, $at
	lui		$at, 0x00ff
	or		$at, 0xffff
	and		$v0, $at
	or		$v1, $v0
	sw		$v1, 0($a1)
	lw		$v0, 0($a0)
	and		$a1, $at
	lui		$at, 0xff00
	and		$v0, $at
	or		$v0, $a1
	sw		$v0, 0($a0)
	
	.set at

	lui		$v0, 0x8000
	or		$a1, $v0
	
	b		.sort_loop
	addiu	$a1, POLYF4_len
	
.sort_FT4_pri:

	sw		$t6, POLYFT4_xy0($a1)
	swc2	C2_SXY0, POLYFT4_xy1($a1)
	swc2	C2_SXY1, POLYFT4_xy2($a1)
	swc2	C2_SXY2, POLYFT4_xy3($a1)
	
	.set noat

	lui		$v1, 0x0900
	lw		$v0, 0($a0)
	lui		$at, 0xff00
	and		$v1, $at
	lui		$at, 0x00ff
	or		$at, 0xffff
	and		$v0, $at
	or		$v1, $v0
	sw		$v1, 0($a1)
	lw		$v0, 0($a0)
	and		$a1, $at
	lui		$at, 0xff00
	and		$v0, $at
	or		$v0, $a1
	sw		$v0, 0($a0)
	
	.set at

	lui		$v0, 0x8000
	or		$a1, $v0
	
	b		.sort_loop
	addiu	$a1, POLYFT4_len
	
.sort_G4_pri:

	sw		$t6, POLYG4_xy0($a1)
	swc2	C2_SXY0, POLYG4_xy1($a1)
	swc2	C2_SXY1, POLYG4_xy2($a1)
	swc2	C2_SXY2, POLYG4_xy3($a1)
	
	la		$v0, _smd_tpage_base
	lhu		$v0, 0($v0)
	srl		$v1, $a3, 6				# Get blend mode
	andi	$v1, 0x3
	sll		$v1, 5
	or		$v0, $v1
	lui		$v1, 0xe100
	or		$v0, $v1
	sw		$v0, POLYG4_tpage($a1)	# Store TPage
	
	.set noat

	lui		$v1, 0x0900
	lw		$v0, 0($a0)
	lui		$at, 0xff00
	and		$v1, $at
	lui		$at, 0x00ff
	or		$at, 0xffff
	and		$v0, $at
	or		$v1, $v0
	sw		$v1, 0($a1)
	lw		$v0, 0($a0)
	and		$a1, $at
	lui		$at, 0xff00
	and		$v0, $at
	or		$v0, $a1
	sw		$v0, 0($a0)
	
	.set at

	lui		$v0, 0x8000
	or		$a1, $v0
	
	b		.sort_loop
	addiu	$a1, POLYG4_len
	
.sort_GT4_pri:

	sw		$t6, POLYGT4_xy0($a1)
	swc2	C2_SXY0, POLYGT4_xy1($a1)
	swc2	C2_SXY1, POLYGT4_xy2($a1)
	swc2	C2_SXY2, POLYGT4_xy3($a1)
	
	.set noat

	lui		$v1, 0x0c00
	lw		$v0, 0($a0)
	lui		$at, 0xff00
	and		$v1, $at
	lui		$at, 0x00ff
	or		$at, 0xffff
	and		$v0, $at
	or		$v1, $v0
	sw		$v1, 0($a1)
	lw		$v0, 0($a0)
	and		$a1, $at
	lui		$at, 0xff00
	and		$v0, $at
	or		$v0, $a1
	sw		$v0, 0($a0)
	
	.set at

	lui		$v0, 0x8000
	or		$a1, $v0
	
	b		.sort_loop
	addiu	$a1, POLYGT4_len
	
.skip_prim:

	b		.sort_loop
	nop
	
.exit:

	lw		$s0, 0( $sp )
	lw		$s1, 4( $sp )
	lw		$s2, 8( $sp )
	lw		$s3, 12( $sp )
	addiu	$sp, 16
	jr		$ra
	move	$v0, $a1
	