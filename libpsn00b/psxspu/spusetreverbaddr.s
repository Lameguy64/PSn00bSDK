.set noreorder

.include "hwregs_a.h"


.section .text

.global SpuSetReverbAddr
.type SpuSetReverbAddr, @function
SpuSetReverbAddr:
	lui		$a3, 0x1f80
	srl		$a0, 3
	sh		$a0, SPU_REVERB_ADDR($a3)
	jr		$ra
	nop
	
	
.global SpuSetReverbVolume
.type SpuSetReverbVolume, @function
SpuSetReverbVolume:
	lui		$a3, 0x1f80
	sh		$a0, SPU_REVERB_VOL($a3)
	sh		$a1, SPU_REVERB_VOL+2($a3)
	jr		$ra
	nop