.set noreorder
.set noat

.section .text


.global AddPrim
.type AddPrim, @function
AddPrim:

	lw		$v0, 0($a0)				# Load OT entry
	lw		$v1, 0($a1)				# Set packet length value (in words)
	
	lui 	$at, 0xff00
	and		$v1, $at				# Clear address in primitive, keep length
	
	lui		$at, 0x00ff
	or		$at, 0xffff
	and		$v0, $at				# Mask off the upper 8 bits of OT entry
	or		$v1, $v0				# OR values together
	sw		$v1, 0($a1)				# Store new address to primitive tag
	lw		$v0, 0($a0)				# Load OT entry
	and		$a1, $at				# Mask off the upper 8 bits of primitive tag
	lui		$at, 0xff00
	and		$v0, $at				# Mask off the first 24 bits of OT entry
	or		$v0, $a1				# OR values together

	jr		$ra
	sw		$v0, 0($a0)				# Store result to OT

