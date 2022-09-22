.section .text
.set noreorder

.include "hwregs_a.inc"

.global _InitCd
.type _InitCd, @function
_InitCd:
	addiu	$sp, -8
	sw		$ra, 0($sp)
	
	lui		$a0, IOBASE			# Load IOBASE value
	
	lw		$v0, DMA_DPCR($a0)	# Get current DMA settings
	nop
	sw		$v0, 4($sp)			# Save to stack
	
	jal		_96_init			# Init CD subsystem
	nop
	
	lui		$a0, IOBASE			# Load IOBASE again
	
	lw		$v0, 4($sp)			# Get old DMA control settings
	lw		$v1, DMA_DPCR($a0)	# Get DMA settings by _96_init()
	
	lui		$a1, 0xffff			# Mask out settings for CD DMA
	ori		$a1, 0x0f00
	and		$v0, $a1
	
	or		$v0, $v1			# Merge and set new DMA settings
	sw		$v0, DMA_DPCR($a0)
	
	lw		$ra, 0($sp)
	addiu	$sp, 8
	jr		$ra
	nop
	