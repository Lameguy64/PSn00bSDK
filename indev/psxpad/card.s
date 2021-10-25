.set noreorder

.include "hwregs_a.h"

.section .text
	
_CardCSum:
	
	# a0 - base csum
	# a1 - data pointer
	# a2 - length
	
	lbu		$v0, 0($a1)
	addi	$a2, -1
	xor		$a0, $v0
	bgtz	$a2, _CardCSum
	addiu	$a1, 1
	
	jr		$ra
	move	$v0, $a0
	
	
.global _CardWrite
.type _CardWrite, @function
_CardWrite:

	# a0 - port number
	# a1 - pointer to 128 byte buffer
	# a2 - sector number

	# return values:
	#   0 - ok
	#   1 - no device
	#   2 - timeout
	#   3 - bad checksum
	#   4 - bad sector
	
	# note: you must wait at least two vsyncs between each sector write
	
	addiu	$sp, -4
	sw		$ra, 0($sp)

	lui		$t0, IOBASE
	
	li		$v0, 0x1003				# TX Enable, Joypad port select
	andi	$a0, 1
	sll		$a0, 13
	or		$v0, $a0				# Select port 2 if a0 is 1
	
	sh		$v0, JOY_CTRL($t0)		# Set to Joypad control interface
	
	jal		_wait					# Delay for analog pads
	li		$v0, 310				# (needs optimization testing)
	
# May cause issues with third party adapters such as Brook wireless
#.Lread_empty_fifo_write:			# Flush the RX FIFO just in case
#	lbu		$v1, JOY_TXRX($t0)
#	lhu		$v0, JOY_STAT($t0)
#	nop
#	andi	$v0, 0x2
#	bnez	$v0, .Lread_empty_fifo_write
#	nop
	
	lhu		$v1, JOY_CTRL($t0)
	nop
	or		$v1, 0x10
	sh		$v1, JOY_CTRL($t0)
	
	jal		_CardExchng				# Send device check byte
	li		$a0, 0x81
	andi	$v1, $v0, 0x100			# No card if exchange timed out
	bnez	$v1, .Lno_device_write
	addiu	$v0, $0 , 1
	
	jal     _wait              		# 1st exchange needs 27microsec after ACK
    li      $v0, 190                # (e.g. 7 as usual + an extra 20)
	
	jal		_CardExchng				# Send write command
	li		$a0, 0x57
	andi	$v1, $v0, 0x100			# No card if exchange timed out
	bnez	$v1, .Lno_device_write
	addiu	$v0, $0 , 1
	
	jal		_CardExchng				# Receive card ID bytes
	move	$a0, $0
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lwrite_timeout
	addiu	$v0, $0 , 2
	jal		_CardExchng
	move	$a0, $0
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lwrite_timeout
	addiu	$v0, $0 , 2
	
	jal		_CardExchng				# Send address bytes
	srl		$a0, $a2, 8
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lwrite_timeout
	addiu	$v0, $0 , 2
	jal		_CardExchng
	andi	$a0, $a2, 0xFF
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lwrite_timeout
	addiu	$v0, $0 , 2
	
	srl		$t1, $a2, 8				# Checksum address by MSB xor LSB
	andi	$v0, $a2, 0xFF
	xor		$t1, $v0
	
	move	$a2, $0					# Send data and compute checksum
.Lwrite_loop:
	lbu		$a0, 0($a1)
	addiu	$a1, 1
	jal		_CardExchng
	xor		$t1, $a0
	addiu	$a2, 1
	blt		$a2, 128, .Lwrite_loop
	nop
	
	jal		_CardExchng				# Send checksum byte
	move	$a0, $t1
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lwrite_timeout
	addiu	$v0, $0 , 2
	
	jal		_CardExchng				# Receive card acknowledge bytes
	move	$a0, $0
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lwrite_timeout
	addiu	$v0, $0 , 2
	jal		_CardExchng
	move	$a0, $0
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lwrite_timeout
	addiu	$v0, $0 , 2
	
	sb		$0 , JOY_TXRX($t0)		# Gets end byte
	nop
.Lsend_wait_end_write:
	lhu		$v0, JOY_STAT($t0)
	nop
	andi	$v0, 0x6
	bne		$v0, 0x6, .Lsend_wait_end_write
	nop
	lbu		$v0, JOY_TXRX($t0)
	nop
	
	beq		$v0, 0x4e, .Lwrite_timeout	# Bad checksum
	addiu	$v0, $0 , 3
	beq		$v0, 0xff, .Lwrite_timeout	# Bad sector
	addiu	$v0, $0 , 4
	
	move	$v0, $0
	
.Lwrite_timeout:
.Lno_device_write:

	sh		$0 , JOY_CTRL($t0)		# Apparently required

	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	

.global _CardRead
.type _CardRead, @function
_CardRead:
	
	# a0 - port number
	# a1 - pointer to 128 byte buffer
	# a2 - sector number
	
	addiu	$sp, -12
	sw		$ra, 0($sp)
	sw		$a1, 4($sp)
	sw		$a2, 8($sp)
	
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
	
	lhu		$v1, JOY_CTRL($t0)
	nop
	or		$v1, 0x10
	sh		$v1, JOY_CTRL($t0)
	
	jal		_CardExchng				# Send device check byte
	li		$a0, 0x81
	andi	$v1, $v0, 0x100			# No card if exchange timed out
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 1
	
	jal     _wait              		# 1st exchange needs 27microsec after ACK
    li      $v0, 190                # (e.g. 7 as usual + an extra 20)
	
	jal		_CardExchng				# Send read command
	li		$a0, 0x52
	andi	$v1, $v0, 0x100			# No card if exchange timed out
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 2
	
	jal		_CardExchng				# Receive card ID bytes
	move	$a0, $0
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 3
	jal		_CardExchng
	move	$a0, $0
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 4
	
	jal		_CardExchng				# Send address
	srl		$a0, $a2, 8
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 5
	jal		_CardExchng
	andi	$a0, $a2, 0xFF
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 6
	
	sb		$0 , JOY_TXRX($t0)		# Receive command acknowledge 1
	nop
.Lsend_wait:
	lhu		$v0, JOY_STAT($t0)
	nop
	andi	$v0, 0x4
	beqz	$v0, .Lsend_wait
	nop
	move	$v1, $0
	lui		$a0, 0xBFC0
.Lwait_ack:
	bgt		$v1, 30000, .Lread_timeout
	addiu	$v0, $0, 10
	lhu		$v0, JOY_STAT($t0)
	lw		$0 , 4($a0)
	lw		$0 , 0($a0)
	andi	$v0, 0x202
	bne		$v0, 0x202, .Lwait_ack
	addiu	$v1, 1
	lhu		$v1, JOY_CTRL($t0)
	lbu		$v0, JOY_TXRX($t0)
	or		$v1, 0x10
	sh		$v1, JOY_CTRL($t0)
	
	jal		_CardExchng				# Receive command acknowledge 2
	move	$a0, $0
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 7
		
	jal		_CardExchng				# Receive confirmed address MSB
	move	$a0, $0
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 8
	
	jal		_CardExchng				# Receive confirmed address LSB
	move	$a0, $0
	andi	$v1, $v0, 0x100
	bnez	$v1, .Lno_device
	addiu	$v0, $0 , 9
	
	move	$a2, $0
.Ltransfer_loop:
	jal		_CardExchng
	move	$a0, $0
	sb		$v0, 0($a1)
	addiu	$a2, 1
	blt		$a2, 128, .Ltransfer_loop
	addiu	$a1, 1
	
	jal		_CardExchng				# Gets checksum byte
	move	$a0, $0
	move	$a3, $v0
	
	sb		$0 , JOY_TXRX($t0)		# Gets end byte
	nop
.Lsend_wait_end:
	lhu		$v1, JOY_STAT($t0)
	nop
	andi	$v1, 0x6
	bne		$v1, 0x6, .Lsend_wait_end
	nop
	lbu		$v1, JOY_TXRX($t0)
	nop
	
	lw		$a2, 8($sp)
	lw		$a1, 4($sp)
	andi	$v1, $a2, 0xff
	srl		$a2, 8
	xor		$a0, $a2, $v1
	jal		_CardCSum
	li		$a2, 128
	
	bne		$v0, $a3, .Lno_device
	addiu	$v0, $0 , -1
	
	move	$v0, $0
.Lread_timeout:
.Lno_device:

	sh		$0 , JOY_CTRL($t0)		# Apparently required

	lw		$ra, 0($sp)
	addiu	$sp, 12
	jr		$ra
	nop


.global _CardExchng
.type _CardExchng, @function
_CardExchng:
	
	lui		$t0, IOBASE
	
	sb		$a0, JOY_TXRX($t0)
	nop
.Lsend_wait_exchg:
	lhu		$v0, JOY_STAT($t0)
	nop
	andi	$v0, 0x4
	beqz	$v0, .Lsend_wait_exchg
	nop
	
	lui		$a0, 0xBFC0
	move	$v1, $0
.Lwait_ack_exchg:
	bgt		$v1, 500, .Ltimeout
	lhu		$v0, JOY_STAT($t0)
	lw		$0 , 4($a0)
	lw		$0 , 0($a0)
	andi	$v0, 0x202
	bne		$v0, 0x202, .Lwait_ack_exchg
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
