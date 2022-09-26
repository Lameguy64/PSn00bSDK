/*
 * PSn00bSDK hardware registers definitions
 * (C) 2022 spicyjpeg - MPL licensed
 */

#ifndef __HWREGS_C_H
#define __HWREGS_C_H

#include <stdint.h>

#define _MMIO8(addr)		*((volatile uint8_t *) (addr))
#define _MMIO16(addr)		*((volatile uint16_t *) (addr))
#define _MMIO32(addr)		*((volatile uint32_t *) (addr))

/* Constants */

#define IOBASE				0xbf800000
#define F_CPU				33868800UL
#define F_GPU				53222400UL

/* GPU */

#define GPU_GP0				_MMIO32(IOBASE | 0x1810)
#define GPU_GP1				_MMIO32(IOBASE | 0x1814)

/* CD drive */

#define CD_STAT				_MMIO8(IOBASE | 0x1800)
#define CD_CMD				_MMIO8(IOBASE | 0x1801)
#define CD_DATA				_MMIO8(IOBASE | 0x1802)
#define CD_IRQ				_MMIO8(IOBASE | 0x1803)

#define CD_REG(N)			_MMIO8(IOBASE | 0x1800 + (N))

/* SPU */

#define SPU_MASTER_VOL_L	_MMIO16(IOBASE | 0x1d80)
#define SPU_MASTER_VOL_R	_MMIO16(IOBASE | 0x1d82)
#define SPU_REVERB_VOL_L	_MMIO16(IOBASE | 0x1d84)
#define SPU_REVERB_VOL_R	_MMIO16(IOBASE | 0x1d86)
#define SPU_KEY_ON			_MMIO32(IOBASE | 0x1d88)
#define SPU_KEY_OFF			_MMIO32(IOBASE | 0x1d8c)
#define SPU_FM_MODE			_MMIO32(IOBASE | 0x1d90)
#define SPU_NOISE_MODE		_MMIO32(IOBASE | 0x1d94)
#define SPU_REVERB_ON		_MMIO32(IOBASE | 0x1d98)
#define SPU_CHAN_STATUS		_MMIO32(IOBASE | 0x1d9c)

#define SPU_REVERB_ADDR		_MMIO16(IOBASE | 0x1da2)
#define SPU_IRQ_ADDR		_MMIO16(IOBASE | 0x1da4)
#define SPU_ADDR			_MMIO16(IOBASE | 0x1da6)
#define SPU_DATA			_MMIO16(IOBASE | 0x1da8)

#define SPU_CTRL			_MMIO16(IOBASE | 0x1daa)
#define SPU_DMA_CTRL		_MMIO16(IOBASE | 0x1dac)
#define SPU_STAT			_MMIO16(IOBASE | 0x1dae)

#define SPU_CD_VOL_L		_MMIO16(IOBASE | 0x1db0)
#define SPU_CD_VOL_R		_MMIO16(IOBASE | 0x1db2)
#define SPU_EXT_VOL_L		_MMIO16(IOBASE | 0x1db4)
#define SPU_EXT_VOL_R		_MMIO16(IOBASE | 0x1db6)
#define SPU_CURRENT_VOL_L	_MMIO16(IOBASE | 0x1db8)
#define SPU_CURRENT_VOL_R	_MMIO16(IOBASE | 0x1dba)

// These are not named SPU_VOICE_* to avoid name clashes with SPU attribute
// flags defined in psxspu.h.
#define SPU_CH_VOL_L(N)		_MMIO16(IOBASE | 0x1c00 + 16 * (N))
#define SPU_CH_VOL_R(N)		_MMIO16(IOBASE | 0x1c02 + 16 * (N))
#define SPU_CH_FREQ(N)		_MMIO16(IOBASE | 0x1c04 + 16 * (N))
#define SPU_CH_ADDR(N)		_MMIO16(IOBASE | 0x1c06 + 16 * (N))
#define SPU_CH_ADSR(N)		_MMIO32(IOBASE | 0x1c08 + 16 * (N))
#define SPU_CH_LOOP_ADDR(N)	_MMIO16(IOBASE | 0x1c0e + 16 * (N))

/* MDEC */

#define MDEC0				_MMIO32(IOBASE | 0x1820)
#define MDEC1				_MMIO32(IOBASE | 0x1824)

/* SPI controller port */

// IMPORTANT: even though JOY_TXRX is a 32-bit register, it should only be
// accessed as 8-bit. Reading it as 16 or 32-bit works fine on real hardware,
// but leads to problems in some emulators.
#define JOY_TXRX			_MMIO8 (IOBASE | 0x1040)
#define JOY_STAT			_MMIO16(IOBASE | 0x1044)
#define JOY_MODE			_MMIO16(IOBASE | 0x1048)
#define JOY_CTRL			_MMIO16(IOBASE | 0x104a)
#define JOY_BAUD			_MMIO16(IOBASE | 0x104e)

/* Serial port */

#define SIO_TXRX			_MMIO8 (IOBASE | 0x1050)
#define SIO_STAT			_MMIO16(IOBASE | 0x1054)
#define SIO_MODE			_MMIO16(IOBASE | 0x1058)
#define SIO_CTRL			_MMIO16(IOBASE | 0x105a)
#define SIO_BAUD			_MMIO16(IOBASE | 0x105e)

/* IRQ controller */

#define IRQ_STAT			_MMIO16(IOBASE | 0x1070)
#define IRQ_MASK			_MMIO16(IOBASE | 0x1074)

/* DMA */

#define DMA_DPCR			_MMIO32(IOBASE | 0x10f0)
#define DMA_DICR			_MMIO32(IOBASE | 0x10f4)

#define DMA_MADR(N)			_MMIO32(IOBASE | 0x1080 + 16 * (N))
#define DMA_BCR(N)			_MMIO32(IOBASE | 0x1084 + 16 * (N))
#define DMA_CHCR(N)			_MMIO32(IOBASE | 0x1088 + 16 * (N))

/* Timers */

#define TIMER_VALUE(N)		_MMIO32(IOBASE | 0x1100 + 16 * (N))
#define TIMER_CTRL(N)		_MMIO32(IOBASE | 0x1104 + 16 * (N))
#define TIMER_RELOAD(N)		_MMIO32(IOBASE | 0x1108 + 16 * (N))

/* Memory control */

#define EXP1_ADDR			_MMIO32(IOBASE | 0x1000)
#define EXP2_ADDR			_MMIO32(IOBASE | 0x1004)
#define EXP1_DELAY_SIZE		_MMIO32(IOBASE | 0x1008)
#define EXP3_DELAY_SIZE		_MMIO32(IOBASE | 0x100c)
#define BIOS_DELAY_SIZE		_MMIO32(IOBASE | 0x1010)
#define SPU_DELAY_SIZE		_MMIO32(IOBASE | 0x1014)
#define CD_DELAY_SIZE		_MMIO32(IOBASE | 0x1018)
#define EXP2_DELAY_SIZE		_MMIO32(IOBASE | 0x101c)
#define COM_DELAY_CFG		_MMIO32(IOBASE | 0x1020)
#define RAM_SIZE_CFG		_MMIO32(IOBASE | 0x1060)

#endif
