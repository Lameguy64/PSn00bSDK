.set noreorder

.include "hwregs_a.h"

.section .data


.global SpuSetKey
.type SpuSetKey, @function
SpuSetKey:
	# a0 - 0: key off, 1: key on
	# a1 - Voice bit mask
	
	lui		$a2, IOBASE
	
	beqz	$a0, .key_off
	nop
	
	jr		$ra
	sh		$a1, SPU_KEY_ON($v1)
	
.key_off:

	jr		$ra
	sh		$a1, SPU_KEY_OFF($v1)
	