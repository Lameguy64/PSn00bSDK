.set noreorder
.set noat

.include "hwregs_a.h"

.section .data


.global SpuInit
.type SpuInit, @function
SpuInit:
	
	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	lui		$v1, IOBASE
	
	# Stop and mute everything
	
	sh		$0 , SPUCNT($v1)			# Clear control settings
	jal		SpuCtrlSync
	move	$a0, $0
	
	sh		$0 , SPU_MASTER_VOL($v1)	# Clear master volume
	sh		$0 , SPU_MASTER_VOL+2($v1)

	sh		$0 , SPU_REVERB_VOL($v1)	# Clear reverb volume
	sh		$0 , SPU_REVERB_VOL+2($v1)
	
	sh		$0 , SPU_CD_VOL($v1)		# Clear CD volume
	sh		$0 , SPU_CD_VOL+2($v1)
	
	sh		$0 , SPU_EXT_VOL($v1)		# Clear external audio volume
	sh		$0 , SPU_EXT_VOL+2($v1)
	
	sh		$0 , SPU_FM_MODE($v1)		# Turn off FM modes
	sh		$0 , SPU_FM_MODE+2($v1)
	
	sh		$0 , SPU_NOISE_MODE($v1)	# Turn off noise modes
	sh		$0 , SPU_NOISE_MODE+2($v1)
	
	sh		$0 , SPU_REVERB_ON($v1)	# Turn off reverb modes
	sh		$0 , SPU_REVERB_ON+2($v1)
	
	li		$v0, 0xfffe
	sh		$v0, SPU_REVERB_ADDR($v1)
	
	lui		$v0, 0x0200;
	ori		$v0, 0x3fff;
	
	# Clear all voices
	
	addiu	$a1, $sp, -20
	sw		$0 , 0($a1)
	sw		$0 , 4($a1)
	sw		$0 , 8($a1)
	sw		$0 , 12($a1)
	
	li		$a2, 23
	
.clear_voices:
	jal		SpuSetVoiceRaw
	move	$a0, $a2
	addiu	$a2, -1
	bgez	$a2, .clear_voices
	nop
	
	li		$v0, 0xffff					# Set all keys to off
	sh		$v0, SPU_KEY_OFF($v1)
	sh		$v0, SPU_KEY_OFF+2($v1)
	
	li		$v0, 0x4					# Set SPU data transfer control
	sh		$v0, SPUDTCNT($v1)			# (usually always 0x4)
	
	lw		$v0, DPCR($v1)				# Enable DMA channel 4 (SPU DMA)
	lui		$at, 0xb
	or		$v0, $at
	sw		$v0, DPCR($v1)
	
	li		$v0, 0xc000					# Enable SPU
	sh		$v0, SPUCNT($v1)
	jal		SpuCtrlSync
	move	$a0, $v0
	
	li		$v0, 0x3fff					# Activate master volume
	sh		$v0, SPU_MASTER_VOL($v1)
	sh		$v0, SPU_MASTER_VOL+2($v1)
	
	sh		$v0, SPU_CD_VOL($v1)		# Activate CD volume
	sh		$v0, SPU_CD_VOL+2($v1)
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop
	

# Waits until bits 0-5 of SPUSTAT are equal to SPUCNT
#
# Destroys v0, v1, a0
#
.global SpuCtrlSync		
.type SpuCtrlSync, @function
SpuCtrlSync:
	lui		$v1, IOBASE
	andi	$a0, 0x3f
.ctrl_wait:
	lhu		$v0, SPUSTAT($v1)		# Get SPUSTAT value
	nop
	andi	$v0, 0x3f
	bne		$v0, $a0, .ctrl_wait	# Wait until SPUCNT and SPUSTAT are equal
	nop
	jr		$ra
	nop

	
# Waits until SPU has finished transfers
#
.global SpuWait
.type SpuWait, @function
SpuWait:
	lui		$v0, IOBASE
	lhu		$v0, SPUSTAT($v0)
	nop
	andi	$v0, 0x400
	bnez	$v0, SpuWait
	nop
	jr		$ra
	nop
	