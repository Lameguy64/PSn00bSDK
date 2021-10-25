.set noreorder

.include "hwregs_a.h"

.section .text

.global mdec_reset						# Resets the MDEC
.type mdec_reset, @function
mdec_reset:

	lui		$a0, IOBASE
	
	lui		$v0, 0xE000					# Reset MDEC
	sw		$v0, MDEC1($a0)
	nop
	
	lui		$v1, 0x8004					# Wait until reset completes
.Lreset_wait:
	lw		$v0, MDEC1($a0)
	nop
	bne		$v0, $v1, .Lreset_wait
	nop
	
	jr		$ra
	nop
	

.global mdec_cmd						# Sends a MDEC command word
.type mdec_cmd, @function
mdec_cmd:

	lui		$a1, IOBASE
	
.Lcmd_ready:							# Wait until command busy is zero
	lw		$v0, MDEC1($a1)
	lui		$v1, 0x2000
	and		$v0, $v1
	bnez	$v0, .Lcmd_ready
	nop
	
	sw		$a0, MDEC0($a1)				# Write command word
	
	jr		$ra
	nop
	
.global mdec_in							# Transmit data to DMA0 (MDEC in)
.type mdec_in, @function
mdec_in:

	# a0 - data source address
	# a1 - blocks to transfer (64 bytes per block)

	lui		$a3, IOBASE
	
.Lmdec_in_wait:							# Wait until MDEC is ready for data
	lw		$v0, MDEC1($a3)
	lui		$v1, 0x1000
	and		$v0, $v1
	beqz	$v0, .Lmdec_in_wait
	nop
	
	sw		$a0, D0_MADR($a3)			# Set source address
	
	andi	$v0, $a1, 0xFFFF			# Set transfer length
	sll		$v0, 16
	ori		$v0, 0x20					# 32 word block size
	sw		$v0, D0_BCR($a3)
	
	lui		$v0, 0x0100					# Begin transfer
	ori		$v0, 0x0201
	sw		$v0, D0_CHCR($a3)

	jr		$ra
	nop
	
.global mdec_out						# Transmit data to DMA0 (MDEC in)
.type mdec_out, @function
mdec_out:

	# a0 - data source address
	# a1 - blocks to transfer (64 bytes per block)

	lui		$a3, IOBASE
	
.Lmdec_out_ready:						# Wait until MDEC is ready for data
	lw		$v0, MDEC1($a3)
	lui		$v1, 0x1000
	and		$v0, $v1
	beqz	$v0, .Lmdec_out_ready
	nop
	
	sw		$a0, D1_MADR($a3)			# Set source address
	
	andi	$v0, $a1, 0xFFFF			# Set transfer length
	sll		$v0, 16
	ori		$v0, 0x20					# 32 word block size
	sw		$v0, D1_BCR($a3)
	
	lui		$v0, 0x0100					# Begin transfer
	ori		$v0, 0x0200
	sw		$v0, D1_CHCR($a3)

.Lmdec_out_wait:
	lw		$v0, D1_CHCR($a3)
	lui		$v1, 0x0200
	and		$v0, $v1
	bnez	$v0, .Lmdec_out_wait
	nop
	
	jr		$ra
	nop
	
.global mdec_setscale					# Uploads the internal scaletable to MDEC
.type mdec_setscale, @function
mdec_setscale:

	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	lui		$a0, 0x6000					# Set scaletable command
	jal		mdec_cmd
	nop
	
	la		$a0, _mdec_scaletable		# Upload scaletable
	jal		mdec_in
	li		$a1, 1						# 64 halfwords
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	
	
.global mdec_setquants					# Uploads the internal quant tables
.type mdec_setquants, @function
mdec_setquants:

	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	lui		$a0, 0x4000					# Set quant tables command
	ori		$a0, 0x0001					# luma + chroma data
	jal		mdec_cmd
	nop
	
	la		$a0, _mdec_qtables			# Upload quant tables
	jal		mdec_in
	li		$a1, 2						# 128 halfwords
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	
	
.section .data

_mdec_qtables:
	# taken from CCITT Rec. T.81
	.hword	16, 11, 10, 16,  24,  40,  51,  61	# Luma table
	.hword	12, 12, 14, 19,  26,  58,  60,  55
	.hword	14, 13, 16, 24,  40,  57,  69,  56
	.hword	14, 17, 22, 29,  51,  87,  80,  62
	.hword	18, 22, 37, 56,  68, 109, 103,  77
	.hword	24, 35, 55, 64,  81, 104, 113,  92
	.hword	49, 64, 78, 87, 103, 121, 120, 101
	.hword	72, 92, 95, 98, 112, 100, 103,  99
	.hword	17, 18, 24, 47, 99, 99, 99, 99		# Chroma table
	.hword	18, 21, 26, 66, 99, 99, 99, 99
	.hword	24, 26, 56, 99, 99, 99, 99, 99
	.hword	47, 66, 99, 99, 99, 99, 99, 99
	.hword	99, 99, 99, 99, 99, 99, 99, 99
	.hword	99, 99, 99, 99, 99, 99, 99, 99
	.hword	99, 99, 99, 99, 99, 99, 99, 99
	.hword	99, 99, 99, 99, 99, 99, 99, 99
	
_mdec_scaletable:
	.hword	0x5A82,0x5A82,0x5A82,0x5A82,0x5A82,0x5A82,0x5A82,0x5A82
	.hword	0x7D8A,0x6A6D,0x471C,0x18F8,0xE707,0xB8E3,0x9592,0x8275
	.hword	0x7641,0x30FB,0xCF04,0x89BE,0x89BE,0xCF04,0x30FB,0x7641
	.hword	0x6A6D,0xE707,0x8275,0xB8E3,0x471C,0x7D8A,0x18F8,0x9592
	.hword	0x5A82,0xA57D,0xA57D,0x5A82,0x5A82,0xA57D,0xA57D,0x5A82
	.hword	0x471C,0x8275,0x18F8,0x6A6D,0x9592,0xE707,0x7D8A,0xB8E3
	.hword	0x30FB,0x89BE,0x7641,0xCF04,0xCF04,0x7641,0x89BE,0x30FB
	.hword	0x18F8,0xB8E3,0x6A6D,0x8275,0x7D8A,0x9592,0x471C,0xE707
