.set noreorder

.include "smd_s.h"

.set SMD_PRI_ID,		0
.set SMD_PRI_v0,		4
.set SMD_PRI_v1,		6
.set SMD_PRI_v2,		8
.set SMD_PRI_v3,		10
.set SMD_PRI_n0,		12
.set SMD_PRI_n1,		14
.set SMD_PRI_n2,		16
.set SMD_PRI_n3,		18
.set SMD_PRI_rgbc0,		20
.set SMD_PRI_rgbc1,		24
.set SMD_PRI_rgbc2,		28
.set SMD_PRI_rgbc3,		32
.set SMD_PRI_tuv0,		36
.set SMD_PRI_tuv1,		38
.set SMD_PRI_tuv2,		40
.set SMD_PRI_tuv3,		42
.set SMD_PRI_tpage,		44
.set SMD_PRI_clut,		46

.section .text


.comm _smd_parse_addr, 4, 4


.global OpenSMD
.type OpenSMD, @function
OpenSMD:
	lw		$v0, SMD_HEAD_ID($a0)
	li		$v1, 0x01444d53
	
	bne		$v0, $v1, .not_smd
	nop
	
	lw		$v0, SMD_HEAD_PPRIMS($a0)
	la		$v1, _smd_parse_addr
	sw		$v0, 0($v1)
	
	jr		$ra
	lhu		$v0, SMD_HEAD_NPRIMS($a0)
	
.not_smd:
	jr		$ra
	move	$v0, $0
	
	
.global ReadSMD
.type ReadSMD, @function
ReadSMD:

	la		$v0, _smd_parse_addr
	lw		$v0, 0($v0)
	nop
	
	lw		$a2, 0($v0)				# Load primitive ID
	addiu	$a1, $v0, 4
	
	sw		$a2, SMD_PRI_ID($a0)
	
	beqz	$a2, $end_prim
	nop
	
	srl		$v1, $a2, 24			# Get primitive size
	addu	$v0, $v1
	la		$v1, _smd_parse_addr
	sw		$v0, 0($v1)
	
	lw		$v0, 0($a1)				# Copy vertex coords
	lw		$v1, 4($a1)
	sw		$v0, SMD_PRI_v0($a0)
	sw		$v1, SMD_PRI_v2($a0)
	addiu	$a1, 8
	
	srl		$v0, $a2, 2				# Lighting enabled?
	andi	$v0, 0x3
	bnez	$v0, $light
	nop
	
	b		$no_light
	nop
	
$light:
	srl		$v1, $a2, 2
	lw		$v0, 0($a1)				# Copy vertex coords
	andi	$v1, 0x3
	sw		$v0, SMD_PRI_n0($a0)
	
	bne		$v1, 0x2, $light_flat
	addiu	$a1, 4
	
	lw		$v1, 0($a1)
	addiu	$a1, 4
	sw		$v1, SMD_PRI_n2($a0)

$light_flat:
$no_light:
	
	lw		$v0, 0($a1)
	nop
	sw		$v0, SMD_PRI_rgbc0($a0)
	addiu	$a1, 4
	
	srl		$v0, $a2, 5
	andi	$v0, 0x1
	beqz	$v0, $not_textured
	nop
	
	lw		$v0, 0($a1)
	lw		$v1, 4($a1)
	sw		$v0, SMD_PRI_tuv0($a0)
	lw		$v0, 8($a1)
	sw		$v1, SMD_PRI_tuv2($a0)
	sw		$v0, SMD_PRI_tpage($a0)
	
$not_textured:
	
	jr		$ra
	move	$v0, $a0
	
$end_prim:

	jr		$ra
	move	$v0, $0
	