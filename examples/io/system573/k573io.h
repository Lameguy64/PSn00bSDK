/*
 * PSn00bSDK Konami System 573 example (I/O driver)
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * Reference: https://psx-spx.consoledev.net/konamisystem573/#register-map
 */

#ifndef __K573IO_H
#define __K573IO_H

#include <stdint.h>
#include <hwregs_c.h>

/* Register definitions */

#define K573_MISC_OUT		_MMIO16(EXP1BASE | 0x400000)
#define K573_DIP_CART_IN	_MMIO16(EXP1BASE | 0x400004)
#define K573_MISC_IN		_MMIO16(EXP1BASE | 0x400006)
#define K573_JAMMA_IN		_MMIO16(EXP1BASE | 0x400008)
#define K573_JVS_RX_DATA	_MMIO16(EXP1BASE | 0x40000a)
#define K573_EXT_IN_P1		_MMIO16(EXP1BASE | 0x40000c)
#define K573_EXT_IN_P2		_MMIO16(EXP1BASE | 0x40000e)

#define K573_BANK_SEL		_MMIO16(EXP1BASE | 0x500000)
#define K573_JVS_RESET		_MMIO16(EXP1BASE | 0x520000)
#define K573_IDE_RESET		_MMIO16(EXP1BASE | 0x560000)
#define K573_WATCHDOG		_MMIO16(EXP1BASE | 0x5c0000)
#define K573_EXT_OUT		_MMIO16(EXP1BASE | 0x600000)
#define K573_JVS_TX_DATA	_MMIO16(EXP1BASE | 0x680000)
#define K573_CART_OUT		_MMIO16(EXP1BASE | 0x6a0000)

#define K573_FLASH			_ADDR16(EXP1BASE | 0x000000)
#define K573_IDE_CS0		_ADDR16(EXP1BASE | 0x480000)
#define K573_IDE_CS1		_ADDR16(EXP1BASE | 0x4c0000)
#define K573_RTC			_ADDR16(EXP1BASE | 0x620000)
#define K573_IO_BOARD		_ADDR16(EXP1BASE | 0x640000)

typedef enum _K573_IOBoardRegister {
	ANALOG_IO_REG_LIGHTS_A		= 0x40,
	ANALOG_IO_REG_LIGHTS_B		= 0x44,
	ANALOG_IO_REG_LIGHTS_C		= 0x48,
	ANALOG_IO_REG_LIGHTS_D		= 0x4c,

	DIGITAL_IO_REG_FPGA_STATUS	= 0x7b,
	DIGITAL_IO_REG_FPGA_PROGRAM	= 0x7c,
	DIGITAL_IO_REG_FPGA_DATA	= 0x7d,

	FISHBAIT_IO_REG_UNKNOWN		= 0x08,
	FISHBAIT_IO_REG_MOTOR		= 0x40,
	FISHBAIT_IO_REG_BRAKE		= 0x44,
	FISHBAIT_IO_REG_ENCODER		= 0x4c,
	FISHBAIT_IO_REG_RESET_Y		= 0x50
} K573_IOBoardRegister;

// The 573's real-time clock chip is an M48T58, which behaves like a standard
// 8 KB battery-backed SRAM with a bunch of special registers. Official games
// store highscores and settings in RTC RAM.
typedef enum _K573_RTCRegister {
	RTC_REG_CTRL			= 0x1ff8,
	RTC_REG_SECONDS			= 0x1ff9,
	RTC_REG_MINUTES			= 0x1ffa,
	RTC_REG_HOURS			= 0x1ffb,
	RTC_REG_DAY_OF_WEEK		= 0x1ffc,
	RTC_REG_DAY_OF_MONTH	= 0x1ffd,
	RTC_REG_MONTH			= 0x1ffe,
	RTC_REG_YEAR			= 0x1fff
} K573_RTCRegister;

/* Bitfields */

typedef enum _K573_MiscOutFlag {
	MISC_OUT_ADC_MOSI		= 1 << 0,
	MISC_OUT_ADC_CS			= 1 << 1,
	MISC_OUT_ADC_SCK		= 1 << 2,
	MISC_OUT_COIN_COUNTER1	= 1 << 3,
	MISC_OUT_COIN_COUNTER2	= 1 << 4,
	MISC_OUT_AMP_ENABLE		= 1 << 5,
	MISC_OUT_CDDA_ENABLE	= 1 << 6,
	MISC_OUT_SPU_ENABLE		= 1 << 7,
	MISC_OUT_MCU_CLOCK		= 1 << 8
} K573_MiscOutFlag;

typedef enum _K573_JAMMAInput {
	// K573_JAMMA_IN bits 0-15
	JAMMA_P2_LEFT		= 1 <<  0,
	JAMMA_P2_RIGHT		= 1 <<  1,
	JAMMA_P2_UP			= 1 <<  2,
	JAMMA_P2_DOWN		= 1 <<  3,
	JAMMA_P2_BUTTON1	= 1 <<  4,
	JAMMA_P2_BUTTON2	= 1 <<  5,
	JAMMA_P2_BUTTON3	= 1 <<  6,
	JAMMA_P2_START		= 1 <<  7,
	JAMMA_P1_LEFT		= 1 <<  8,
	JAMMA_P1_RIGHT		= 1 <<  9,
	JAMMA_P1_UP			= 1 << 10,
	JAMMA_P1_DOWN		= 1 << 11,
	JAMMA_P1_BUTTON1	= 1 << 12,
	JAMMA_P1_BUTTON2	= 1 << 13,
	JAMMA_P1_BUTTON3	= 1 << 14,
	JAMMA_P1_START		= 1 << 15,

	// K573_EXT_IN_P1 bits 8-11
	JAMMA_P1_BUTTON4	= 1 << 16,
	JAMMA_P1_BUTTON5	= 1 << 17,
	JAMMA_TEST			= 1 << 18,
	JAMMA_P1_BUTTON6	= 1 << 19,

	// K573_EXT_IN_P2 bits 8-11
	JAMMA_P2_BUTTON4	= 1 << 20,
	JAMMA_P2_BUTTON5	= 1 << 21,
	JAMMA_UNKNOWN		= 1 << 22,
	JAMMA_P2_BUTTON6	= 1 << 23,

	// K573_MISC_IN bits 8-12
	JAMMA_COIN1			= 1 << 24,
	JAMMA_COIN2			= 1 << 25,
	JAMMA_PCMCIA1		= 1 << 26,
	JAMMA_PCMCIA2		= 1 << 27,
	JAMMA_SERVICE		= 1 << 28,

	// K573_DIP_CART_IN bits 0-2
	JAMMA_DIP1			= 1 << 29,
	JAMMA_DIP2			= 1 << 30,
	JAMMA_DIP3			= 1 << 31
} K573_JAMMAInput;

typedef enum _K573_Light {
	LIGHT_A0				= 1 << 0,
	LIGHT_A1				= 1 << 1,
	LIGHT_A2				= 1 << 2,
	LIGHT_A3				= 1 << 3,
	LIGHT_A4				= 1 << 4,
	LIGHT_A5				= 1 << 5,
	LIGHT_A6				= 1 << 6,
	LIGHT_A7				= 1 << 7,
	LIGHT_B0				= 1 << 8,
	LIGHT_B1				= 1 << 9,
	LIGHT_B2				= 1 << 10,
	LIGHT_B3				= 1 << 11,
	LIGHT_B4				= 1 << 12,
	LIGHT_B5				= 1 << 13,
	LIGHT_B6				= 1 << 14,
	LIGHT_B7				= 1 << 15,
	LIGHT_C0				= 1 << 16,
	LIGHT_C1				= 1 << 17,
	LIGHT_C2				= 1 << 18,
	LIGHT_C3				= 1 << 19,
	LIGHT_C4				= 1 << 20,
	LIGHT_C5				= 1 << 21,
	LIGHT_C6				= 1 << 22,
	LIGHT_C7				= 1 << 23,
	LIGHT_D0				= 1 << 24,
	LIGHT_D1				= 1 << 25,
	LIGHT_D2				= 1 << 26,
	LIGHT_D3				= 1 << 27,

	// Dance Dance Revolution (2-player)
	LIGHT_DDR_P1_UP			= 1 <<  0,
	LIGHT_DDR_P1_DOWN		= 1 <<  1,
	LIGHT_DDR_P1_LEFT		= 1 <<  2,
	LIGHT_DDR_P1_RIGHT		= 1 <<  3,
	LIGHT_DDR_P1_IO_DATA	= 1 <<  4,
	LIGHT_DDR_P1_IO_CLK		= 1 <<  5,
	LIGHT_DDR_P2_UP			= 1 <<  8,
	LIGHT_DDR_P2_DOWN		= 1 <<  9,
	LIGHT_DDR_P2_LEFT		= 1 << 10,
	LIGHT_DDR_P2_RIGHT		= 1 << 11,
	LIGHT_DDR_P2_IO_DATA	= 1 << 12,
	LIGHT_DDR_P2_IO_CLK		= 1 << 13,
	LIGHT_DDR_P1_BUTTONS	= 1 << 18,
	LIGHT_DDR_P2_BUTTONS	= 1 << 19,
	LIGHT_DDR_MARQUEE_BR	= 1 << 20,
	LIGHT_DDR_MARQUEE_TR	= 1 << 21,
	LIGHT_DDR_MARQUEE_BL	= 1 << 22,
	LIGHT_DDR_MARQUEE_TL	= 1 << 23,
	LIGHT_DDR_SPEAKER		= 1 << 24,

	// Dance Dance Revolution Solo
	LIGHT_SOLO_SPEAKER		= 1 << 16,
	LIGHT_SOLO_BUTTONS		= 1 << 20,
	LIGHT_SOLO_BODY_LEFT	= 1 << 21,
	LIGHT_SOLO_BODY_CENTER	= 1 << 22,
	LIGHT_SOLO_BODY_RIGHT	= 1 << 23,

	// DrumMania 1st Mix
	LIGHT_DM_HIHAT			= 1 << 16,
	LIGHT_DM_SNARE			= 1 << 17,
	LIGHT_DM_HIGH_TOM		= 1 << 18,
	LIGHT_DM_LOW_TOM		= 1 << 19,
	LIGHT_DM_CYMBAL			= 1 << 20,
	LIGHT_DM_START_BUTTON	= 1 << 22,
	LIGHT_DM_SELECT_BUTTON	= 1 << 23,
	LIGHT_DM_SPOT			= 1 << 24,
	LIGHT_DM_NEON_TOP		= 1 << 25,
	LIGHT_DM_NEON_BOTTOM	= 1 << 27,

	// DrumMania 2nd Mix and later
	LIGHT_DM2_HIHAT			= 1 <<  0,
	LIGHT_DM2_SNARE			= 1 <<  1,
	LIGHT_DM2_HIGH_TOM		= 1 <<  2,
	LIGHT_DM2_LOW_TOM		= 1 <<  3,
	LIGHT_DM2_SPOT			= 1 <<  8,
	LIGHT_DM2_NEON_BOTTOM	= 1 <<  9,
	LIGHT_DM2_NEON_TOP		= 1 << 10,
	LIGHT_DM2_CYMBAL		= 1 << 12,
	LIGHT_DM2_START_BUTTON	= 1 << 14,
	LIGHT_DM2_SELECT_BUTTON	= 1 << 15
} K573_Light;

/* Public API */

#define K573_RESET_WATCHDOG() { \
	K573_WATCHDOG = 0; \
}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the state of JAMMA inputs and DIP switches.
 *
 * @details Returns a bitfield containing the state of all JAMMA inputs and DIP
 * switches. All bits are inverted as they represent the actual signal levels
 * on the JAMMA pins (i.e. normally pulled up by resistors, shorted to ground
 * when a button is pressed).
 *
 * @return Binary OR of K573_JAMMAInputs flags
 */
uint32_t K573_GetJAMMAInputs(void);

/**
 * @brief Sets light outputs on the analog I/O board.
 *
 * @details Sets the 32 light outputs provided by the analog I/O board (if
 * installed) to match the provided bitfield. No other I/O boards are currently
 * supported.
 *
 * @param lights Binary OR of K573_Light flags
 */
void K573_SetAnalogLights(uint32_t lights);

/**
 * @brief Initializes the expansion port registers to enable System 573 I/O
 * access.
 */
void K573_Init(void);

#ifdef __cplusplus
}
#endif

#endif
