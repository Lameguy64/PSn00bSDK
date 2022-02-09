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

#define F_CPU				33868800UL
#define F_GPU				53222400UL

/* GPU */

#define GP0					_MMIO32(0x1f801810)
#define GP1					_MMIO32(0x1f801814)

/* CD drive */

#define CD_STAT				_MMIO8(0x1f801800)
#define CD_CMD				_MMIO8(0x1f801801)
#define CD_DATA				_MMIO8(0x1f801802)
#define CD_IRQ				_MMIO8(0x1f801803)

#define CD_REG(N)			_MMIO8(0x1f801800 + (N))

/* SPU */

#define SPU_MASTER_VOL_L	_MMIO16(0x1f801d80)
#define SPU_MASTER_VOL_R	_MMIO16(0x1f801d82)
#define SPU_REVERB_VOL_L	_MMIO16(0x1f801d84)
#define SPU_REVERB_VOL_R	_MMIO16(0x1f801d86)
#define SPU_KEY_ON			_MMIO32(0x1f801d88)
#define SPU_KEY_OFF			_MMIO32(0x1f801d8c)
#define SPU_FM_MODE			_MMIO32(0x1f801d90)
#define SPU_NOISE_MODE		_MMIO32(0x1f801d94)
#define SPU_REVERB_ON		_MMIO32(0x1f801d98)
#define SPU_CHAN_STATUS		_MMIO32(0x1f801d9c)

#define SPU_REVERB_ADDR		_MMIO16(0x1f801da2)
#define SPU_IRQ_ADDR		_MMIO16(0x1f801da4)
#define SPU_ADDR			_MMIO16(0x1f801da6)
#define SPU_DATA			_MMIO16(0x1f801da8)

#define SPU_CTRL			_MMIO16(0x1f801daa)
#define SPU_DMA_CTRL		_MMIO16(0x1f801dac)
#define SPU_STAT			_MMIO16(0x1f801dae)

#define SPU_CD_VOL_L		_MMIO16(0x1f801db0)
#define SPU_CD_VOL_R		_MMIO16(0x1f801db2)
#define SPU_EXT_VOL_L		_MMIO16(0x1f801db4)
#define SPU_EXT_VOL_R		_MMIO16(0x1f801db6)
#define SPU_CURRENT_VOL_L	_MMIO16(0x1f801db8)
#define SPU_CURRENT_VOL_R	_MMIO16(0x1f801dba)

// These are not named SPU_VOICE_* to avoid name clashes with SPU attribute
// flags defined in psxspu.h.
#define SPU_CH_VOL_L(N)		_MMIO16(0x1f801c00 + 16 * (N))
#define SPU_CH_VOL_R(N)		_MMIO16(0x1f801c02 + 16 * (N))
#define SPU_CH_FREQ(N)		_MMIO16(0x1f801c04 + 16 * (N))
#define SPU_CH_ADDR(N)		_MMIO16(0x1f801c06 + 16 * (N))
#define SPU_CH_ADSR(N)		_MMIO32(0x1f801c08 + 16 * (N))
#define SPU_CH_LOOP_ADDR(N)	_MMIO16(0x1f801c0e + 16 * (N))

/* MDEC */

#define MDEC0				_MMIO32(0x1f801820)
#define MDEC1				_MMIO32(0x1f801824)

/* SPI controller port */

// IMPORTANT: even though JOY_TXRX is a 32-bit register, it should only be
// accessed as 8-bit. Reading it as 16 or 32-bit works fine on real hardware,
// but leads to problems in some emulators.
#define JOY_TXRX			_MMIO8(0x1f801040)
#define JOY_STAT			_MMIO16(0x1f801044)
#define JOY_MODE			_MMIO16(0x1f801048)
#define JOY_CTRL			_MMIO16(0x1f80104a)
#define JOY_BAUD			_MMIO16(0x1f80104e)

/* Serial port */

#define SIO_TXRX			_MMIO8(0x1f801050)
#define SIO_STAT			_MMIO16(0x1f801054)
#define SIO_MODE			_MMIO16(0x1f801058)
#define SIO_CTRL			_MMIO16(0x1f80105a)
#define SIO_BAUD			_MMIO16(0x1f80105e)

/* IRQ controller */

#define IRQ_STAT			_MMIO32(0x1f801070)
#define IRQ_MASK			_MMIO32(0x1f801074)

/* DMA */

#define DMA_DPCR			_MMIO32(0x1f8010f0)
#define DMA_DICR			_MMIO32(0x1f8010f4)

#define DMA_MADR(N)			_MMIO32(0x1f801080 + 16 * (N))
#define DMA_BCR(N)			_MMIO32(0x1f801084 + 16 * (N))
#define DMA_CHCR(N)			_MMIO32(0x1f801088 + 16 * (N))

/* Timers */

#define TIM_VALUE(N)		_MMIO32(0x1f801100 + 16 * (N))
#define TIM_CTRL(N)			_MMIO32(0x1f801104 + 16 * (N))
#define TIM_RELOAD(N)		_MMIO32(0x1f801108 + 16 * (N))

/* Memory control */

#define EXP1_ADDR			_MMIO32(0x1f801000)
#define EXP2_ADDR			_MMIO32(0x1f801004)
#define EXP1_DELAY_SIZE		_MMIO32(0x1f801008)
#define EXP3_DELAY_SIZE		_MMIO32(0x1f80100c)
#define BIOS_DELAY_SIZE		_MMIO32(0x1f801010)
#define SPU_DELAY_SIZE		_MMIO32(0x1f801014)
#define CD_DELAY_SIZE		_MMIO32(0x1f801018)
#define EXP2_DELAY_SIZE		_MMIO32(0x1f80101c)
#define COM_DELAY_CFG		_MMIO32(0x1f801020)
#define RAM_SIZE_CFG		_MMIO32(0x1f801060)

#endif
