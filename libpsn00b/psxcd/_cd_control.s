.set noreorder

.include "hwregs_a.h"

.section .text

#
# Issues command and parameter bytes to CD controller directly
#
.global _cd_control
.type _cd_control, @function
_cd_control:

	# a0 - command value
	# a1 - pointer to parameters
	# a2 - length of parameters
	
	addiu	$sp, -16
	sw		$ra, 0($sp)
	sw		$a0, 4($sp)
	sw		$a1, 8($sp)
	sw		$a2, 12($sp)
	
#	move	$a3, $a2					# Debug
#	move	$a2, $a1
#	move	$a1, $a0
#	la		$a0, _cd_control_msg
#	jal		printf
#	addiu	$sp, -16
#	addiu	$sp, 16

	lw		$a0, 4($sp)
	lw		$a1, 8($sp)
	lw		$a2, 12($sp)
	
	li		$v0, 1					# Set acknowledge wait flag
	la		$v1, _cd_ack_wait
	sb		$v0, 0($v1)
	
	# Commands that have a 'completion' interrupt (CDIRQ2)
	
	beq		$a0, 0x07, .Lset_complete	# CdlStandby
	nop
	beq		$a0, 0x08, .Lset_complete	# CdlStop
	nop
	beq		$a0, 0x09, .Lset_complete	# CdlPause
	nop
	beq		$a0, 0x0A, .Lset_complete	# CdlInit
	nop
	beq		$a0, 0x12, .Lset_complete	# CdlSetsession
	nop
	beq		$a0, 0x15, .Lset_complete	# CdlSeekL
	nop
	beq		$a0, 0x16, .Lset_complete	# CdlSeekP
	nop
	beq		$a0, 0x1A, .Lset_complete	# GetID
	nop
	beq		$a0, 0x1D, .Lset_complete	# GetQ
	nop
	
	la		$v1, _cd_complt_wait	# Set wait complete flag
	sb		$0 , 0($v1)
	
	b		.Lno_complete
	nop
	
.Lset_complete:

	la		$v1, _cd_complt_wait	# Set wait complete flag
	sb		$v0, 0($v1)
	
.Lno_complete:
	
	bne		$a0, 0x0E, .Lnot_mode
	lbu		$v0, 0($a1)
	la		$v1, _cd_last_mode
	sb		$v0, 0($v1)
	
.Lnot_mode:
	
	la		$v1, _cd_last_int		# Clear last IRQ value
	sb		$0 , 0($v1)

	la		$v1, _cd_last_cmd		# Save command as last command
	sb		$a0, 0($v1)
	
	lui		$v1, IOBASE
	
.Lbusy_wait:
	lbu		$v0, CD_REG0($v1)
	nop
	andi	$v0, 0x80
	bnez	$v0, .Lbusy_wait
	nop
	
	li		$v0, 1					# Clear parameter FIFO (in case it wasn't cleared)
	sb		$v0, CD_REG0($v1)
	li		$v0, 0x40
	sb		$v0, CD_REG3($v1)
	
#.Lflush_response:					# Flush response FIFO
#	lbu		$v0, CD_REG1($v1)
#	nop
#	lbu		$v0, CD_REG0($v1)
#	nop
#	andi	$v0, 0x20
#	beqz	$v0, .Lflush_response
#	nop
	
.Lcmd_wait:							# Wait for CD to become ready for commands
	lbu		$v0, CD_REG0($v1)
	nop
	andi	$v0, 0x80
	bnez	$v0, .Lcmd_wait
	nop
	
	sb		$0 , CD_REG0($v1)
	
	beqz	$a2, .Lno_params
	nop
	
.Lfeed_params:						# Feed parameters to parameter FIFO
	lbu		$v0, 0($a1)
	addi	$a2, -1
	sb		$v0, CD_REG2($v1)
	bgtz	$a2, .Lfeed_params
	addiu	$a1, 1
	
.Lno_params:

	sb		$0 , CD_REG0($v1)		# Feed command value
	sb		$a0, CD_REG1($v1)
	
	lw		$ra, 0($sp)
	addiu	$sp, 16
	jr		$ra
	nop
	

#.section .data

#_cd_control_msg:
#	.asciiz "CdControl(%d, %x, %d)\n"
	