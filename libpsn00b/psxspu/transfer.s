.set noreorder

.include "hwregs_a.h"


.section .text

.global SpuSetTransferMode
.type SpuSetTransferMode, @function
SpuSetTransferMode:
	la		$v0, _spu_transfer_mode
	sb		$a0, 0($v0)
	jr		$ra
	move	$v0, $a0
	
	
.global SpuSetTransferStartAddr
.type SpuSetTransferStartAddr, @function
SpuSetTransferStartAddr:
	li		$v0, 0x1000				# Check if value is valid
	blt		$a0, $v0, .Lbad_value
	nop
	lui		$v0, 8					# 0x7ffff = (8<<16)-1
	addiu	$v0, -1
	bgt		$a0, $v0, .Lbad_value
	nop
	
	la		$v1, _spu_transfer_addr
	srl		$v0, $a0, 3					# Set transfer destination address
	sh		$v0, 0($v1)
	
	jr		$ra
	move	$v0, $a0
	
.Lbad_value:
	jr		$ra
	move	$v0, $0
	
	
.global SpuWrite
.type SpuWrite, @function
SpuWrite:
	addiu	$sp, -8
	sw		$ra, 0($sp)
	sw		$a0, 4($sp)
	
	lui		$a3, IOBASE	
	
	lhu		$v0, SPUCNT($a3)			# Set transfer mode to Stop
	nop
	andi	$v0, 0xffcf
	sh		$v0, SPUCNT($a3)
	jal		SpuCtrlSync
	move	$a0, $v0
	
	la		$v1, _spu_transfer_addr		# Set SPU write address
	lhu		$v1, 0($v1)
	nop
	sh		$v1, SPU_ADDR($a3)
	
	lhu		$v0, SPUCNT($a3)			# Set transfer mode to DMA write
	nop
	ori		$v0, 0x20
	sh		$v0, SPUCNT($a3)
	#jal		SpuCtrlSync				# Locks up on most emulators (bit 5 in
	#move	$a0, $v0					# SPUSTAT likely not updating, seems to
										# be okay to not wait for it on real HW)
	
	lw		$a0, 4($sp)
	
.Ldma_wait:								# Wait for SPU to be ready for DMA
	lhu		$v0, SPUSTAT($a3)
	nop
	andi	$v0, 0x400					# Bit 8 in SPUSTAT never changes to 1 on
	bnez	$v0, .Ldma_wait				# emulators so use bit 10 instead
	nop
	
	sw		$a0, D4_MADR($a3)			# Set DMA source address
	
	li		$v0, 0x10					# 16 words per block (64 bytes)
	addiu	$a1, 63						# Add by 63 to ensure all bytes get sent
	srl		$a1, 6						# Equivalent to divide by 64
	andi	$a1, 0xffff
	sll		$a1, 16
	or		$v0, $a1
	sw		$v0, D4_BCR($a3)
	
	lui		$v0, 0x0100					# Commence transfer
	ori		$v0, 0x0201
	sw		$v0, D4_CHCR($a3)
	
	lw		$ra, 0($sp)
	addiu	$sp, 8
	jr		$ra
	nop
	
	
.section .data

.global _spu_transfer_mode
.type _spu_transfer_mode, @object
_spu_transfer_mode:
	.word	0x0
	
.global _spu_transfer_addr
.type _spu_transfer_addr, @object
_spu_transfer_addr:
	.word	0x200