.set noreorder

.include "hwregs_a.h"

.section .text

.global _cd_init
.type _cd_init, @function
_cd_init:

	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	jal		EnterCriticalSection
	nop
	
	lui		$a3, IOBASE				# Acknowledge all CD IRQs
	li		$v0, 1
	sb		$v0, CD_REG0($a3)
	li		$v0, 0x1f
	sb		$v0, CD_REG3($a3)
	sb		$v0, CD_REG2($a3)		# Enable all IRQs
	
	sb		$0 , CD_REG0($a3)
	sb		$0 , CD_REG3($a3)
	
	li		$v0, 0x1325
	sw		$v0, COM_DELAY($a3)
	
	la		$a1, _cd_handler
	jal		InterruptCallback
	li		$a0, 2
	
	li		$v0, 2					# Set CD left volume
	sb		$v0, CD_REG0($a3)
	li		$v0, 0x80
	sb		$v0, CD_REG2($a3)
	
	li		$v0, 3					# Set CD right volume
	sb		$v0, CD_REG0($a3)
	li		$v0, 0x80
	sb		$v0, CD_REG1($a3)
	
	li		$v0, 0x20				# Apply volume
	sb		$v0, CD_REG3($a3)
	
	# Clear a bunch of stats
	la		$v0, _cd_ack_wait
	sb		$0 , 0($v0)
	la		$v0, _cd_complt_wait
	sb		$0 , 0($v0)
	la		$v0, _cd_last_cmd
	sb		$0 , 0($v0)
	la		$v0, _cd_last_mode
	sb		$0 , 0($v0)
	la		$v0, _cd_last_int
	sb		$0 , 0($v0)
	
	la		$v0, _cd_result_ptr
	sw		$0 , 0($v0)
	
	# Clear callback hooks
	la		$v0, _cd_callback_int1_data
	sw		$0 , 0($v0)
	la		$v0, _cd_callback_int4
	sw		$0 , 0($v0)
	
	la		$v0, _cd_read_cb
	sw		$0 , 0($v0)
	
	lw		$v0, DPCR($a3)
	li		$v1, 0xB000
	or		$v0, $v1
	sw		$v0, DPCR($a3)
	
	jal		ExitCriticalSection
	nop
	
	#li		$a0, 0x01				# GetStat
	#jal		_cd_control
	#move	$a2, $0
	#jal		_cd_wait
	#nop
	
	#li		$a0, 0x0a				# Init
	#jal		_cd_control
	#move	$a2, $0
	#jal		_cd_wait
	#nop
	
	#li		$a0, 0x0c				# Demute
	#jal		_cd_control
	#move	$a2, $0
	#jal		_cd_wait
	#nop
	
	#la		$a0, _cd_init_msg
	#jal		printf
	#addiu	$sp, -16
	#addiu	$sp, 16
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	
	
.global _cd_wait
.type _cd_wait, @function
_cd_wait:
	la		$v0, _cd_ack_wait
	lbu		$v0, 0($v0)
	nop
	bnez	$v0, _cd_wait
	nop
.Lcomplete:
	la		$v0, _cd_complt_wait
	lbu		$v0, 0($v0)
	nop
	bnez	$v0, .Lcomplete
	nop
	jr		$ra
	nop

	
.global _cd_wait_ack
.type _cd_wait_ack, @function
_cd_wait_ack:
	la		$v0, _cd_ack_wait
	lbu		$v0, 0($v0)
	nop
	bnez	$v0, _cd_wait_ack
	nop
	jr		$ra
	nop
	
	
.global _cd_wait_complt
.type _cd_wait_complt, @function
_cd_wait_complt:
	la		$v0, _cd_complt_wait
	lbu		$v0, 0($v0)
	nop
	bnez	$v0, _cd_wait_complt
	nop
	jr		$ra
	nop
	
	
.type _cd_handler, @function
_cd_handler:

	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	lui		$a3, IOBASE				# Print out IRQ number
	li		$v0, 1
	sb		$v0, CD_REG0($a3)

	lbu		$v0, CD_REG3($a3)
	nop
	andi	$v0, 0x7
	
	la		$v1, _cd_last_int		# Save last IRQ value
	sb		$v0, 0($v1)
	
	bne		$v0, 0x1, .Lno_data
	nop
	
	sb		$0 , CD_REG0($a3)		# Load data FIFO on INT1
	li		$v1, 0x80
	sb		$v1, CD_REG3($a3)
	
.Lno_data:
	
	li		$v1, 1
	sb		$v1, CD_REG0($a3)		# Clear CD interrupt and parameter FIFO
	li		$v1, 0x5f
	sb		$v1, CD_REG3($a3)
	li		$v1, 0x40
	sb		$v1, CD_REG3($a3)
	
	li		$v1, 0					# Delay when clearing parameter FIFO
	sw		$v1, 0($0)
	li		$v1, 1
	sw		$v1, 0($0)
	li		$v1, 2
	sw		$v1, 0($0)
	li		$v1, 3
	sw		$v1, 0($0)
	
	beq		$v0, 0x1, .Lirq_1		# Data ready
	nop
	beq		$v0, 0x2, .Lirq_2		# Command finish
	nop
	beq		$v0, 0x3, .Lirq_3		# Acknowledge
	nop
	beq		$v0, 0x4, .Lirq_4		# Data/track end
	nop
	beq		$v0, 0x5, .Lirq_5		# Error
	nop
	
	b		.Lirq_misc
	nop
	
.Lirq_1:	# Data ready
	
	jal		_cd_fetch_result
	nop
	
	la		$v0, _cd_callback_int1_data
	lw		$v0, 0($v0)
	nop
	beqz	$v0, .Lirq_misc
	nop
	
	la		$a0, _cd_last_int
	lbu		$a0, 0($a0)
	la		$a1, _cd_result_ptr
	lw		$a1, 0($a1)
	
	jalr	$v0
	addiu	$sp, -16
	addiu	$sp, 16
	
	b		.Lirq_misc
	nop
	
.Lirq_2:	# Command complete

	jal		_cd_fetch_result
	nop
	
	la		$v0, _cd_complt_wait
	sb		$0 , 0($v0)
	
	la		$v0, _cd_sync_cb
	lw		$v0, 0($v0)
	nop
	beqz	$v0, .Lirq_misc
	nop
	
	la		$a0, _cd_last_int
	lbu		$a0, 0($a0)
	la		$a1, _cd_result_ptr
	lw		$a1, 0($a1)
	jalr	$v0
	addiu	$sp, -16
	addiu	$sp, 16
	
	b		.Lirq_misc
	nop
	
.Lirq_3:	# Command acknowledge
	
	jal		_cd_fetch_result
	nop
	
	la		$v0, _cd_ack_wait
	sb		$0 , 0($v0)
	
	b		.Lirq_misc
	nop
	
.Lirq_4:

	jal		_cd_fetch_result
	nop
	
	la		$v0, _cd_callback_int4
	lw		$v0, 0($v0)
	nop
	beqz	$v0, .Lirq_misc
	nop
	
	jalr	$v0
	addiu	$sp, -16
	addiu	$sp, 16
	
	b		.Lirq_misc
	nop
	
.Lirq_5:	# Error
	
	jal		_cd_fetch_result
	nop
	
	la		$v0, _cd_complt_wait
	lbu		$v0, 0($v0)
	nop
	beqz	$v0, .Lno_complete
	nop
	
	la		$v0, _cd_sync_cb
	lw		$v0, 0($v0)
	nop
	beqz	$v0, .Lno_complete
	nop
	
	li		$a0, 0x05				# CdlDiskError
	la		$a1, _cd_result_ptr
	lw		$a1, 0($a1)
	jalr	$v0
	addiu	$sp, -16
	addiu	$sp, 16
	
.Lno_complete:

	la		$v0, _cd_complt_wait
	sb		$0 , 0($v0)
	la		$v0, _cd_ack_wait
	sb		$0 , 0($v0)
	
	la		$v0, _cd_callback_int1_data
	lw		$v0, 0($v0)
	nop
	beqz	$v0, .Lirq_misc
	nop
	
	li		$a0, 0x05				# CdlDiskError
	la		$a1, _cd_result_ptr
	lw		$a1, 0($a1)
	
	jalr	$v0
	addiu	$sp, -16
	addiu	$sp, 16
	
.Lirq_misc:
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	

_cd_fetch_result:

	lui		$a3, IOBASE
	
	la		$a0, _cd_status
	lbu		$v0, CD_REG1($a3)
	
	la		$v1, _cd_last_int
	lbu		$v1, 0($v1)
	nop
	beq		$v1, 0x2, .Lirq2_checks
	nop
	
	# IRQ3 checks
	
	la		$v1, _cd_last_cmd
	lbu		$v1, 0($v1)
	nop
	beq		$v1, 0x10, .Lskip_status
	nop
	beq		$v1, 0x11, .Lskip_status
	nop

	b		.Lwrite_status
	nop
	
	# IRQ2 checks
	
.Lirq2_checks:

	la		$v1, _cd_last_cmd
	lbu		$v1, 0($v1)
	nop
	beq		$v1, 0x1D, .Lskip_status
	nop

.Lwrite_status:

	andi	$v1, $v0, 0x10
	
	beqz	$v1, .Lno_disc_change
	nop
	
	la		$v1, _cd_media_changed
	addiu	$a1, $0 , 1
	sw		$a1, 0($v1)
	
.Lno_disc_change:

	sb		$v0, 0($a0)
	
.Lskip_status:

	la		$a0, _cd_result_ptr
	lw		$a0, 0($a0)
	nop
	beqz	$a0, .Lno_result
	nop
	sb		$v0, 0($a0)
	addiu	$a0, 1
	
.Lread_futher_result:

	lbu		$v0, CD_REG0($a3)
	nop
	andi	$v0, 0x20
	beqz	$v0, .Lno_result
	nop

	lbu		$v0, CD_REG1($a3)
	lbu		$v1, CD_REG0($a3)
	sb		$v0, 0($a0)
	andi	$v1, 0x20
	bnez	$v1, .Lread_futher_result
	addiu	$a0, 1

.Lno_result:

	jr		$ra
	nop
	
	
.global CdAutoPauseCallback
.type CdAutoPauseCallback, @function
CdAutoPauseCallback:
	
	addiu	$sp, -8
	sw		$ra, 0($sp)
	sw		$a0, 4($sp)
	
	la		$v1, _cd_callback_int4
	lw		$v0, 0($v1)
	
	la		$v1, _cd_callback_int4
	
	jal		EnterCriticalSection
	nop
	
	lw		$a0, 4($sp)
	nop
	sw		$a0, 0($v1)
	
	jal		ExitCriticalSection
	nop
	
	lw		$ra, 0($sp)
	addiu	$sp, 8
	jr		$ra
	nop
	
	
.global CdReadyCallback
.type CdReadyCallback, @function
CdReadyCallback:
	
	addiu	$sp, -12
	sw		$ra, 0($sp)
	sw		$a0, 4($sp)
	
	la		$v1, _cd_callback_int1_data
	lw		$v0, 0($v1)
	
	la		$v1, _cd_callback_int1_data
	sw		$v0, 8($sp)
	
	jal		EnterCriticalSection
	nop
	
	lw		$a0, 4($sp)
	nop
	sw		$a0, 0($v1)
	
	jal		ExitCriticalSection
	nop
	
	lw		$ra, 0($sp)
	lw		$v0, 8($sp)
	jr		$ra
	addiu	$sp, 12
	
	
.global CdSyncCallback
.type CdSyncCallback, @function
CdSyncCallback:
	addiu	$sp, -12
	sw		$ra, 0($sp)
	sw		$a0, 4($sp)
	
	la		$v1, _cd_sync_cb
	lw		$v0, 0($v1)
	
	la		$v1, _cd_sync_cb
	sw		$v0, 8($sp)
	
	jal		EnterCriticalSection
	nop
	
	lw		$a0, 4($sp)
	nop
	sw		$a0, 0($v1)
	
	jal		ExitCriticalSection
	nop
	
	lw		$ra, 0($sp)
	lw		$v0, 8($sp)
	jr		$ra
	addiu	$sp, 12
	

.section .data
	
.global psxcd_credits
.type psxcd_credits, @object
psxgpu_credits:
	.ascii "psxcd library programs by Lameguy64\n"
	.asciiz "2019 PSn00bSDK Project / Meido-Tek Productions\n"
	
_cd_init_msg:
.asciiz "psxcd: Init OK\n"
.align 4
	
.comm	_cd_last_cmd, 1, 1
.comm	_cd_last_mode, 1, 1
.comm	_cd_ack_wait, 1, 1
.comm	_cd_complt_wait, 1, 1

.comm	_cd_status, 1, 1
.comm	_cd_last_int, 1, 1

.comm	_cd_media_changed, 1, 1

# Callback hooks
.comm	_cd_callback_int1_data, 4, 4		# Data IRQ callback
.comm	_cd_callback_int4, 4, 4		# Autopause callback
.comm	_cd_sync_cb, 4, 4
