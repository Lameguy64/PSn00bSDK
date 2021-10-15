.set noreorder

.include "hwregs_a.h"

.section .data


.global SpuSetKey
.type SpuSetKey, @function
SpuSetKey:
	# a0 - 0: key off, 1: key on
	# a1 - Voice bit mask
	
	lui		$a2, IOBASE
	
	beqz	$a0, .Lkey_off
	nop
	
	jr		$ra
	sw		$a1, SPU_KEY_ON($a2)
	
.Lkey_off:

	jr		$ra
	sw		$a1, SPU_KEY_OFF($a2)
	