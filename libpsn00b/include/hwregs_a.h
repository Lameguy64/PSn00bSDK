# Hardware register definitions for GNU assembler (as)
#
# Part of the PSn00bSDK Project by Lameguy64
# 2019 Meido-Tek Productions


.set IOBASE,			0x1f80		# IO segment base

# GPU
.set GP0,				0x1810		# Also GPUREAD
.set GP1,				0x1814		# Also GPUSTAT

# CD
.set CD_STAT,			0x1800
.set CD_CMD,			0x1801		# Also response FIFO
.set CD_DATA,			0x1802		# Also parameters
.set CD_IRQ,			0x1803

.set CD_REG0,			0x1800
.set CD_REG1,			0x1801
.set CD_REG2,			0x1802
.set CD_REG3,			0x1803

.set SBUS_5,			0x1018
.set COM_DELAY,			0x1020

# SPU (must be used with 16-bit load/store instructions)
.set SPU_VOICE_BASE,	0x1c00

.set SPU_MASTER_VOL,	0x1d80
.set SPU_REVERB_VOL,	0x1d84
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

.set SPUCNT,			0x1daa
.set SPUDTCNT,			0x1dac
.set SPUSTAT,			0x1dae

.set SPU_CD_VOL,		0x1db0
.set SPU_EXT_VOL,		0x1db4
.set SPU_CURRENT_VOL,	0x1db8

.set SPU_VOICE_VOL_L,	0x00
.set SPU_VOICE_VOL_R,	0x02
.set SPU_VOICE_FREQ,	0x04
.set SPU_VOICE_ADDR,	0x06
.set SPU_VOICE_ADSR_L,	0x08
.set SPU_VOICE_ADSR_H,	0x0a		
.set SPU_VOICE_LOOP,	0x0e

# Pads
.set JOY_TXRX,	0x1040
.set JOY_STAT,	0x1044
.set JOY_MODE,	0x1048
.set JOY_CTRL,	0x104A
.set JOY_BAUD,	0x104E

# Serial
.set SIO_TXRX,	0x1050
.set SIO_STAT,	0x1054
.set SIO_MODE,	0x1058
.set SIO_CTRL,	0x105a
.set SIO_BAUD,	0x105e

# IRQ
.set ISTAT,		0x1070
.set IMASK,		0x1074

# DMA
.set DPCR,		0x10f0     
.set DICR,		0x10f4

.set D2_MADR,	0x10a0
.set D2_BCR,	0x10a4
.set D2_CHCR,	0x10a8

.set D3_MADR,	0x10b0
.set D3_BCR,	0x10b4
.set D3_CHCR,	0x10b8

.set D4_MADR,	0x10c0
.set D4_BCR,	0x10c4
.set D4_CHCR,	0x10c8

.set D6_MADR,	0x10e0
.set D6_BCR,	0x10e4
.set D6_CHCR,	0x10e8

# Timers
.set T0_CNT,	0x1100
.set T0_MODE,	0x1104
.set T0_TGT,	0x1108

.set T1_CNT,	0x1110
.set T1_MODE,	0x1114
.set T1_TGT,	0x1118

.set T2_CNT,	0x1120
.set T2_MODE,	0x1124
.set T2_TGT,	0x1128
