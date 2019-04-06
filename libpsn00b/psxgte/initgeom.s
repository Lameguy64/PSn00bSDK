.set noreorder

.include "gtereg.h"

.section .text


.global InitGeom
.type InitGeom, @function
InitGeom:
	addiu	$sp, -4
	sw		$ra, 0($sp)

	jal		EnterCriticalSection
	nop

	mfc0	$v0, $12				# Get SR
	lui		$v1, 0x4000				# Set bit to enable cop2
	or		$v0, $v1
	mtc0	$v0, $12				# Set new SR

	jal		ExitCriticalSection
	nop

	ctc2	$0 , $24				# Reset GTE offset
	ctc2	$0 , $25

	li		$v0, 320				# Set default projection plane
	ctc2	$v0, $26

	li		$v0, 0x155				# Set ZSF3 and ZSF4 defaults
	ctc2	$v0, $29
	li		$v0, 0x100
	ctc2	$v0, $30

	li		$v0, 0xef9e				# DQA and DQB defaults
	lui		$v1, 0x0140
	ctc2	$v0, C2_DQA
	ctc2	$v1, C2_DQB

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop

