# PSn00bSDK hardware registers definitions
# (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed

## Constants

.set IOBASE,			0xbf80
.set EXP1BASE,			0xbf00

.set F_CPU,				33868800
.set F_GPU,				53222400

## GPU

.set GPU_GP0,			0x1810		# Also GPUREAD
.set GPU_GP1,			0x1814		# Also GPUSTAT

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
.set SPU_KEY_ON1,		0x1d88
.set SPU_KEY_ON1,		0x1d8a
.set SPU_KEY_OFF1,		0x1d8c
.set SPU_KEY_OFF2,		0x1d8e
.set SPU_FM_MODE1,		0x1d90
.set SPU_FM_MODE2,		0x1d92
.set SPU_NOISE_MODE1,	0x1d94
.set SPU_NOISE_MODE2,	0x1d96
.set SPU_REVERB_ON1,	0x1d98
.set SPU_REVERB_ON2,	0x1d9a
.set SPU_CHAN_STATUS1,	0x1d9c
.set SPU_CHAN_STATUS2,	0x1d9e

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

.set SPU_CH_VOL_L,		0x00
.set SPU_CH_VOL_R,		0x02
.set SPU_CH_FREQ,		0x04
.set SPU_CH_ADDR,		0x06
.set SPU_CH_ADSR1,		0x08
.set SPU_CH_ADSR2,		0x0a
.set SPU_CH_LOOP_ADDR,	0x0e

## MDEC

.set MDEC0,				0x1820
.set MDEC1,				0x1824

## SPI and serial interfaces

.set SIO0_DATA,			0x1040
.set SIO0_STAT,			0x1044
.set SIO0_MODE,			0x1048
.set SIO0_CTRL,			0x104a
.set SIO0_BAUD,			0x104e

.set SIO1_DATA,			0x1050
.set SIO1_STAT,			0x1054
.set SIO1_MODE,			0x1058
.set SIO1_CTRL,			0x105a
.set SIO1_BAUD,			0x105e

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

.set TIMER0_VALUE,		0x1100
.set TIMER0_CTRL,		0x1104
.set TIMER0_RELOAD,		0x1108

.set TIMER1_VALUE,		0x1110
.set TIMER1_CTRL,		0x1114
.set TIMER1_RELOAD,		0x1118

.set TIMER2_VALUE,		0x1120
.set TIMER2_CTRL,		0x1124
.set TIMER2_RELOAD,		0x1128

## Memory/bus control

.set BUS_EXP1_ADDR,		0x1000
.set BUS_EXP2_ADDR,		0x1004
.set BUS_EXP1_CFG,		0x1008
.set BUS_EXP3_CFG,		0x100c
.set BUS_BIOS_CFG,		0x1010
.set BUS_SPU_CFG,		0x1014
.set BUS_CD_CFG,		0x1018
.set BUS_EXP2_CFG,		0x101c
.set BUS_COM_DELAY,		0x1020
.set BUS_RAM_SIZE,		0x1060
