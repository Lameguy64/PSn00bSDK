.set noreorder

.include "hwregs_a.inc"
.include "gtereg.inc"

.section .text.InitGeom
.global InitGeom
.type InitGeom, @function
InitGeom:
	# Disable interrupts and make sure the GTE is enabled in COP0.
	lui   $v0, IOBASE
	lhu   $v1, IRQ_MASK($v0)
	nop
	sh    $0,  IRQ_MASK($v0)

	mfc0  $a0, $12
	lui   $a1, 0x4000
	or    $a1, $a0
	mtc0  $a1, $12
	nop
	#nop

	# Re-enable interrupts, then load default values into some GTE registers.
	sh    $v1, IRQ_MASK($v0)

	ctc2  $0, C2_OFX
	nop
	ctc2  $0, C2_OFY

	li    $a0, 320
	ctc2  $a0, C2_H

	li    $a0, 0x155
	ctc2  $a0, C2_ZSF3
	li    $a0, 0x100
	ctc2  $a0, C2_ZSF4

	li    $a0, 0xef9e
	ctc2  $a0, C2_DQA
	lui   $a0, 0x0140
	ctc2  $a0, C2_DQB

	jr    $ra
	nop
