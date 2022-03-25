.set noreorder

.include "hwregs_a.h"

.section .text

.global CdGetSector
.type CdGetSector, @function
CdGetSector:
	
	lui		$a2, IOBASE
	
#.Lwait_fifo:					# Probably redundant as the BIOS CD-ROM
#	lbu		$v0, CD_REG0($a2)	# routines do not not wait for this
#	nop
#	andi	$v0, 0x40
#	beqz	$v0, .Lwait_fifo
#	nop

	lui		$v0, 0x1
#	srl		$a1, 2				# (the official implementation expects $a1/size
								# to be in 32-bit words rather than bytes)
	or		$v0, $a1
	sw		$a0, DMA3_MADR($a2)	# Set DMA base address and transfer length
	sw		$v0, DMA3_BCR($a2)

	lui		$v0, 0x1100			# Start DMA transfer
	sw		$v0, DMA3_CHCR($a2)
	nop
	nop
	
.Ldma_wait:						# Ensure DMA transfer has completed
	lw		$v0, DMA3_CHCR($a2)
	nop
	srl		$v0, 24
	andi	$v0, 0x1

	bnez	$v0, .Ldma_wait
	nop
	
# Not stable
#	sb		$0 , CD_REG0($a2)
#.Lflush_fifo:					# Read out any remaining bytes in the buffer
#	lbu		$v1, CD_REG0($a2)
#	li		$v0, 0x40
#	and		$v1, $v0
#	beqz	$v1, .Lend_flush
#	nop
#	lbu		$v0, CD_REG2($a2)
#	b		.Lflush_fifo
#	nop
#.Lend_flush:
	
	jr		$ra
	li		$v0, 1
	
