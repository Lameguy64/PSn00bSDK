.set noreorder

.include "hwregs_a.inc"

.section .text

# Currently implemented serial control functions:
#
#	cmd(a0)		sub(a1)
#	0			0			Get serial status
#	0			1			Get control line status
#	0			2			Get serial mode
#	0			3			Get baud rate
#	0			4			Read 1 byte
#	1			1			Set serial control
#	1			2			Set serial mode
#	1			3			Set baud rate
#	1			4			Write 1 byte
#	2			0			Reset serial
#	2			1			Acknowledge serial

.global _sio_control
.type _sio_control, @function
_sio_control:

	# a0 - command
	# a1 - subcommand
	# a2 - argument
	
	lui		$a3, IOBASE
	
	beq		$a0, $0, .Lcmd0
	nop
	beq		$a0,  1, .Lcmd1
	nop
	beq		$a0,  2, .Lcmd2
	nop
	jr		$ra
	nop
	
	
.Lcmd0:

	beq		$a1, $0, .Lcmd0arg0
	nop
	beq		$a1,  1, .Lcmd0arg1
	nop
	beq		$a1,  2, .Lcmd0arg2
	nop
	beq		$a1,  3, .Lcmd0arg3
	nop
	beq		$a1,  4, .Lcmd0arg4
	nop
	jr		$ra
	nop

.Lcmd0arg0:		# Get SIO status

	lhu		$v0, SIO_STAT($a3)
	nop
	
	jr		$ra
	andi	$v0, 0x3FF

.Lcmd0arg1:		# Get control line status

	lhu		$v1, SIO_CTRL($a3)
	nop
	srl		$v0, $v1, 1
	andi	$v0, 0x1
	srl		$v1, 4
	andi	$v1, 0x2
	
	jr		$ra
	or		$v0, $v1
	
	
.Lcmd0arg2:		# Get serial mode

	lhu		$v0, SIO_MODE($a3)
	nop
	jr		$ra
	andi	$v0, 0xFF
	
.Lcmd0arg3:
	
	lui		$v1, 0x1f
	lhu		$v0, SIO_BAUD($a3)
	ori		$v1, 0xa400
	div		$v1, $v0
	nop
	nop
	mflo	$v0
	jr		$ra
	nop
	
.Lcmd0arg4:		# Serial RX read

	lbu		$v0, SIO_TXRX($a3)
	nop
	jr		$ra
	nop
	
	
.Lcmd1:

	beq		$a1,  1, .Lcmd1arg1
	nop
	beq		$a1,  2, .Lcmd1arg2
	nop
	beq		$a1,  3, .Lcmd1arg3
	nop
	beq		$a1,  4, .Lcmd1arg4
	nop
	jr		$ra
	nop
	
.Lcmd1arg1:

	andi	$v0, $a2, 0x1CFF
	sh		$a2, SIO_CTRL($a3)
	jr		$ra
	nop

.Lcmd1arg2:
	
	sh		$a2, SIO_MODE($a3)
	jr		$ra
	nop
	
.Lcmd1arg3:

	lui		$v0, 0x1f
	ori		$v0, 0xa400
	divu	$v0, $a2
	bnez	$a2, .Lgood_baud
	nop
	jr		$ra
	nop
	
.Lgood_baud:

	mflo	$v0
	sh		$v0, SIO_BAUD($a3)
	nop
	jr		$ra
	nop
	
.Lcmd1arg4:

	sb		$a2, SIO_TXRX($a3)
	nop
	jr		$ra
	nop

.Lcmd2:

	beq		$a1, $0 , .Lcmd2arg0
	li		$v0, 1
	beq		$a1, $v0, .Lcmd2arg1
	nop
	jr		$ra
	nop

.Lcmd2arg0:

	li		$v0, 0x40
	sh		$v0, SIO_CTRL($a3)
	sh		$0 , SIO_MODE($a3)
	sh		$0 , SIO_BAUD($a3)
	nop
	jr		$ra
	nop
	
.Lcmd2arg1:

	lhu		$v0, SIO_CTRL($a3)
	nop
	ori		$v0, 0x10
	sh		$v0, SIO_CTRL($a3)
	jr		$ra
	nop
	
	