# Hardware register definitions for GNU assembler (as)
#
# Part of the PSn00bSDK Project by Lameguy64
# 2019 Meido-Tek Productions


.set IOBASE,			0x1f80		# IO segment base

## GPU

.set GP0,				0x1810		# Also GPUREAD
.set GP1,				0x1814		# Also GPUSTAT

## CD drive

.set CD_STAT,			0x1800
.set CD_CMD,			0x1801		# Also response FIFO
.set CD_DATA,			0x1802		# Also parameters
.set CD_IRQ,			0x1803

.set CD_REG0,			0x1800
.set CD_REG1,			0x1801
.set CD_REG2,			0x1802
.set CD_REG3,			0x1803

## SPU

.set SPU_VOICE_BASE,	0x1c00

.set SPU_MASTER_VOL_L,	0x1d80
.set SPU_MASTER_VOL_R,	0x1d82
.set SPU_REVERB_VOL_L,	0x1d84
.set SPU_REVERB_VOL_R,	0x1d86
.set SPU_KEY_ON,		0x1d88
.set SPU_KEY_OFF,		0x1d8c
.set SPU_FM_MODE,		0x1d90
.set SPU_NOISE_MODE,	0x1d94
.set SPU_REVERB_ON,		0x1d98
.set SPU_CHAN_STATUS,	0x1d9c

.set SPU_REVERB_ADDR,	0x1da2
.set SPU_IRQ_ADDR,		0x1da4
.set SPU_ADDR,			0x1da6
.set SPU_DATA,			0x1da8

.set SPU_CTRL,			0x1daa
.set SPU_DMA_CTRL,		0x1dac
.set SPU_STAT,			0x1dae

.set SPU_CD_VOL_L,		0x1db0
.set SPU_CD_VOL_R,		0x1db2
.set SPU_EXT_VOL_L,		0x1db4
.set SPU_EXT_VOL_R,		0x1db6
.set SPU_CURRENT_VOL_L,	0x1db8
.set SPU_CURRENT_VOL_R,	0x1dba

.set SPU_VOICE_VOL_L,	0x00
.set SPU_VOICE_VOL_R,	0x02
.set SPU_VOICE_FREQ,	0x04
.set SPU_VOICE_ADDR,	0x06
.set SPU_VOICE_ADSR_L,	0x08
.set SPU_VOICE_ADSR_H,	0x0a
.set SPU_VOICE_LOOP,	0x0e

## MDEC

.set MDEC0,				0x1820
.set MDEC1,				0x1824

## SPI controller port

.set JOY_TXRX,			0x1040
.set JOY_STAT,			0x1044
.set JOY_MODE,			0x1048
.set JOY_CTRL,			0x104a
.set JOY_BAUD,			0x104e

## Serial port

.set SIO_TXRX,			0x1050
.set SIO_STAT,			0x1054
.set SIO_MODE,			0x1058
.set SIO_CTRL,			0x105a
.set SIO_BAUD,			0x105e

## IRQ controller

.set IRQ_STAT,			0x1070
.set IRQ_MASK,			0x1074

## DMA

.set DMA_DPCR,			0x10f0
.set DMA_DICR,			0x10f4

.set DMA0_MADR,			0x1080
.set DMA0_BCR,			0x1084
.set DMA0_CHCR,			0x1088

.set DMA1_MADR,			0x1090
.set DMA1_BCR,			0x1094
.set DMA1_CHCR,			0x1098

.set DMA2_MADR,			0x10a0
.set DMA2_BCR,			0x10a4
.set DMA2_CHCR,			0x10a8

.set DMA3_MADR,			0x10b0
.set DMA3_BCR,			0x10b4
.set DMA3_CHCR,			0x10b8

.set DMA4_MADR,			0x10c0
.set DMA4_BCR,			0x10c4
.set DMA4_CHCR,			0x10c8

.set DMA5_MADR,			0x10d0
.set DMA5_BCR,			0x10d4
.set DMA5_CHCR,			0x10d8

.set DMA6_MADR,			0x10e0
.set DMA6_BCR,			0x10e4
.set DMA6_CHCR,			0x10e8

## Timers

.set TIM0_VALUE,		0x1100
.set TIM0_CTRL,			0x1104
.set TIM0_RELOAD,		0x1108

.set TIM1_VALUE,		0x1110
.set TIM1_CTRL,			0x1114
.set TIM1_RELOAD,		0x1118

.set TIM2_VALUE,		0x1120
.set TIM2_CTRL,			0x1124
.set TIM2_RELOAD,		0x1128

## Memory control

.set EXP1_ADDR,			0x1000
.set EXP2_ADDR,			0x1004
.set EXP1_DELAY_SIZE,	0x1008
.set EXP3_DELAY_SIZE,	0x100c
.set BIOS_DELAY_SIZE,	0x1010
.set SPU_DELAY_SIZE,	0x1014
.set CD_DELAY_SIZE,		0x1018
.set EXP2_DELAY_SIZE,	0x101c
.set COM_DELAY_CFG,		0x1020
.set RAM_SIZE_CFG,		0x1060
