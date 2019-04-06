.set noreorder

.include "hwregs_a.h"


.set PARAM_L,		0
.set PARAM_R,		2
.set PARAM_FREQ,	4
.set PARAM_ADDR,	6
.set PARAM_LOOP,	8
.set PARAM_RES,		10
.set PARAM_ADSR,	12


.section .text

.global SpuSetVoiceRaw
.type SpuSetVoiceRaw, @function
SpuSetVoiceRaw:
	
	# a0 - Voice number
	# a1 - Address to parameters
	
	sll		$a0, 4
	addiu	$a0, SPU_VOICE_BASE
	
	lui		$v1, IOBASE
	or		$a0, $v1
	
	lhu		$v0, PARAM_L($a1)
	nop
	sh		$v0, SPU_VOICE_VOL_L($a0)
	
	lhu		$v0, PARAM_R($a1)
	nop
	sh		$v0, SPU_VOICE_VOL_R($a0)
	
	lhu		$v0, PARAM_FREQ($a1)
	nop
	sh		$v0, SPU_VOICE_FREQ($a0)
	
	lhu		$v0, PARAM_ADDR($a1)
	nop
	sh		$v0, SPU_VOICE_ADDR($a0)
	
	lhu		$v0, PARAM_LOOP($a1)
	nop
	sh		$v0, SPU_VOICE_LOOP($a0)
	
	
	lw		$v0, PARAM_ADSR($a1)
	nop
	sh		$v0, SPU_VOICE_ADSR_L($a0)
	srl		$v0, 16
	sh		$v0, SPU_VOICE_ADSR_H($a0)
	
	
	jr		$ra
	nop
	