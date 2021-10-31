.set noreorder

.include "hwregs_a.h"

.section .text

.global _InitPadDirect
.type _InitPadDirect, @function
_InitPadDirect:
	
	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	lui		$t0, IOBASE
	
	# Interface setup
	li		$v0, 0x40				# Interface reset
	sh		$v0, JOY_CTRL($t0)
	li		$v0, 0x88				# 250kHz clock rate
	sh		$v0, JOY_BAUD($t0)
	li		$v0, 0x0d				# 8-bit, no parity, x1 multiplier
	sh		$v0, JOY_MODE($t0)
	li		$v0, 0x1003				# JOY1, TX enabled
	sh		$v0, JOY_CTRL($t0)

	jal		_wait
	li		$v0, 1000
	
	# Empty RX fifo
.Lempty_fifo:
	lbu		$v1, JOY_TXRX($t0)
	lhu		$v0, JOY_STAT($t0)
	nop
	andi	$v0, 0x2
	bnez	$v0, .Lempty_fifo
	nop
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	
	
.global _PadSetPort
.type _PadSetPort, @function
_PadSetPort:
	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	lui		$t0, IOBASE
	
	beq		$a0, 2, .Lstop_comms
	nop
	
	li		$v0, 0x1003				# TX Enable, Joypad port select
	andi	$a0, 1
	sll		$a0, 13
	or		$v0, $a0				# Select port 2 if a0 is 1
	
	sh		$v0, JOY_CTRL($t0)		# Set to Joypad control interface
	
	jal		_wait					# Delay for analog pads (needs testing)
	li		$v0, 500
	
.Lread_empty_fifo_set:				# Flush the RX FIFO just in case
	lbu		$v1, JOY_TXRX($t0)
	lhu		$v0, JOY_STAT($t0)
	nop
	andi	$v0, 0x2
	bnez	$v0, .Lread_empty_fifo_set
	nop
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	
.Lstop_comms:
	
	sh		$0 , JOY_CTRL($t0)

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	

.global _PadReadDirect
.type _PadReadDirect, @function
_PadReadDirect:

	# a0 - port number
	# a1 - device data buffer
	# a2 - data max length
	
	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	lui		$t0, IOBASE
	
	li		$v0, 0x1003				# TX Enable, Joypad port select
	andi	$a0, 1
	sll		$a0, 13
	or		$v0, $a0				# Select port 2 if a0 is 1
	
	sh		$v0, JOY_CTRL($t0)		# Set to Joypad control interface
	
	jal		_wait					# Delay for analog pads (needs testing)
	li		$v0, 310
	
# May cause issues with third party adapters such as Brook wireless
#.Lread_empty_fifo:					# Flush the RX FIFO just in case
#	lbu		$v1, JOY_TXRX($t0)
#	lhu		$v0, JOY_STAT($t0)
#	nop
#	andi	$v0, 0x2
#	bnez	$v0, .Lread_empty_fifo
#	nop
	
	jal		_PadExchng				# Send device check byte
	li		$a0, 0x01

	andi	$v1, $v0, 0x100			# No pad if exchange timed out
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 1
	
	sb		$v0, 0($a1)
	addiu	$a1, 1
	
    jal     _wait              		# 1st exchange needs 27microsec after ACK
    li      $v0, 190                # (e.g. 7 as usual + an extra 20)
	
	jal		_PadExchng				# Send command byte
	li		$a0, 0x42
	
	sb		$v0, 0($a1)
	addiu	$a1, 1
	addiu	$a2, -2
	
	jal		_PadExchng				# Send 0 for pads, 1 for multitap
	move	$a0, $0					# Read is usually 0x5A
	
	addi	$a3, $0 , 1
	
	la		$t1, _pad_mot_values
	
.Lread_loop:						# Read until buffer full, or no more data

	lbu		$a0, 0($t1)
	nop
	beqz	$a0, .Lskip_mot
	nop
	
	jal		_PadExchng
	nop
	
	b		.Ldone_exchg
	addiu	$t1, 1
	
.Lskip_mot:

	jal		_PadExchng				# when ACK is no longer triggered
	move	$a0, $0
	
.Ldone_exchg:

	sb		$v0, 0($a1)
	
	andi	$v0, 0x100
	bnez	$v0, .Lread_end
	addi	$a3, 1
	
	addiu	$a2, -1
	bgtz	$a2, .Lread_loop
	addiu	$a1, 1
	
.Lread_end:

	b		.Lexit
	move	$v0, $a3
	
.Lno_device:
	
	addiu	$v0, $0 , -1
	sb		$v0, 0($a1)
	
.Lexit:
	
	sh		$0 , JOY_CTRL($t0)
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	
	
.global _PadExchng
.type _PadExchng, @function
_PadExchng:
	
	lui		$t0, IOBASE
	
	sb		$a0, JOY_TXRX($t0)
	nop
.Lsend_wait:
	lhu		$v0, JOY_STAT($t0)
	nop
	andi	$v0, 0x4
	beqz	$v0, .Lsend_wait
	nop
	
	move	$v1, $0
.Lwait_ack:
	bgt		$v1, 100, .Ltimeout
	lhu		$v0, JOY_STAT($t0)
	nop
	andi	$v0, 0x202
	bne		$v0, 0x202, .Lwait_ack
	addiu	$v1, 1
	
	b		.Ldone
	nop
	
.Ltimeout:

	lbu		$v0, JOY_TXRX($t0)
	nop
	b		.Lexit_exchg
	ori		$v0, 0x100

.Ldone:

	lhu		$v1, JOY_CTRL($t0)
	lbu		$v0, JOY_TXRX($t0)
	or		$v1, 0x10
	sh		$v1, JOY_CTRL($t0)
	
.Lexit_exchg:

	jr		$ra
	nop
	
	
.global _wait
.type _wait, @function
_wait:
	addiu	$v0, -1
	bgtz	$v0, _wait
	nop
	jr		$ra
	nop
	

.section .data

.global _pad_mot_values
.type _pad_mot_values, @object
_pad_mot_values:
	.byte	0		# Small motor
	.byte	0		# Big motor
	.byte	0
	.byte	0
	
	
