.set noreorder

.include "hwregs_a.h"


.section .text

.global SpuSetReverbAddr
.type SpuSetReverbAddr, @function
SpuSetReverbAddr:
	lui		$a3, IOBASE
	srl		$a0, 3
	sh		$a0, SPU_REVERB_ADDR($a3)
	jr		$ra
	nop
	
	
.global SpuSetReverbVolume
.type SpuSetReverbVolume, @function
SpuSetReverbVolume:
	lui		$a3, IOBASE
	sh		$a0, SPU_REVERB_VOL_L($a3)
	sh		$a1, SPU_REVERB_VOL_R($a3)
	jr		$ra
	nop