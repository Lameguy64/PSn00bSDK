/*
 * PSn00bSDK Konami System 573 example (I/O driver)
 * (C) 2022 spicyjpeg - MPL licensed
 */

#ifndef __K573IO_H
#define __K573IO_H

#include <stdint.h>

/* Register definitions */

#define K573_BANK_SWITCH	*((volatile uint16_t *) 0x1f500000)
#define K573_IDE_RESET		*((volatile uint16_t *) 0x1f560000)
#define K573_WATCHDOG		*((volatile uint16_t *) 0x1f5c0000)
#define K573_EXT_OUT		*((volatile uint16_t *) 0x1f600000)
#define K573_JVS_INPUT		*((volatile uint16_t *) 0x1f680000)
#define K573_SECURITY_OUT	*((volatile uint16_t *) 0x1f6a0000)

#define K573_FLASH			((volatile uint16_t *) 0x1f000000)
#define K573_IO_CHIP		((volatile uint16_t *) 0x1f400000)
#define K573_IDE_CS0		((volatile uint16_t *) 0x1f480000)
#define K573_IDE_CS1		((volatile uint16_t *) 0x1f4c0000)
#define K573_RTC			((volatile uint16_t *) 0x1f620000)
#define K573_IO_BOARD		((volatile uint16_t *) 0x1f640000)

typedef enum _K573_IOChipRegister {
	IO_REG_OUT0		= 0x0,
	IO_REG_IN0		= 0x0,
	IO_REG_IN1_LOW	= 0x2,
	IO_REG_IN1_HIGH	= 0x3,
	IO_REG_IN2		= 0x4,
	IO_REG_IN3_LOW	= 0x6,
	IO_REG_IN3_HIGH	= 0x7
} K573_IOChipRegister;

typedef enum _K573_IOBoardRegister {
	ANALOG_IO_REG_LIGHTS0		= 0x40,
	ANALOG_IO_REG_LIGHTS1		= 0x44,
	ANALOG_IO_REG_LIGHTS2		= 0x48,
	ANALOG_IO_REG_LIGHTS3		= 0x4c,

	// The digital I/O board has a lot more registers than these, but there
	// seems to be no DIGITAL_IO_LIGHTS6 register. WTF
	DIGITAL_IO_REG_LIGHTS1		= 0x70,
	DIGITAL_IO_REG_LIGHTS0		= 0x71,
	DIGITAL_IO_REG_LIGHTS3		= 0x72,
	DIGITAL_IO_REG_LIGHTS7		= 0x73,
	DIGITAL_IO_REG_DS2401		= 0x77,
	DIGITAL_IO_REG_FPGA_STATUS	= 0x7b,
	DIGITAL_IO_REG_FPGA_UPLOAD	= 0x7c,
	DIGITAL_IO_REG_LIGHTS4		= 0x7d,
	DIGITAL_IO_REG_LIGHTS5		= 0x7e,
	DIGITAL_IO_REG_LIGHTS2		= 0x7f,

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

/* Inputs and lights bitfields */

typedef enum _K573_JAMMAInputs {
	// IO_REG_IN2 bits 0-15
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

	// IO_REG_IN3_LOW bits 8-11
	JAMMA_P1_BUTTON4	= 1 << 16,
	JAMMA_P1_BUTTON5	= 1 << 17,
	JAMMA_TEST			= 1 << 18,
	JAMMA_P1_BUTTON6	= 1 << 19,

	// IO_REG_IN3_HIGH bits 8-11
	JAMMA_P2_BUTTON4	= 1 << 20,
	JAMMA_P2_BUTTON5	= 1 << 21,
	JAMMA_UNKNOWN		= 1 << 22,
	JAMMA_P2_BUTTON6	= 1 << 23,

	// IO_REG_IN1_HIGH bits 8-12
	JAMMA_COIN1			= 1 << 24,
	JAMMA_COIN2			= 1 << 25,
	JAMMA_PCMCIA1		= 1 << 26,
	JAMMA_PCMCIA2		= 1 << 27,
	JAMMA_SERVICE		= 1 << 28,

	// IO_REG_IN1_LOW bits 0-2
	JAMMA_DIP1			= 1 << 29,
	JAMMA_DIP2			= 1 << 30,
	JAMMA_DIP3			= 1 << 31
} K573_JAMMAInputs;

typedef enum _K573_Light {
	// Dance Dance Revolution (2-player)
	DDR_LIGHT_P1_UP				= 1 <<  0,
	DDR_LIGHT_P1_LEFT			= 1 <<  1,
	DDR_LIGHT_P1_RIGHT			= 1 <<  2,
	DDR_LIGHT_P1_DOWN			= 1 <<  3,
	DDR_LIGHT_P1_MUX_DATA		= 1 <<  4,	// Used for stage commands
	DDR_LIGHT_P1_MUX_CLK		= 1 <<  7,	// Used for stage commands
	DDR_LIGHT_P2_UP				= 1 <<  8,
	DDR_LIGHT_P2_LEFT			= 1 <<  9,
	DDR_LIGHT_P2_RIGHT			= 1 << 10,
	DDR_LIGHT_P2_DOWN			= 1 << 11,
	DDR_LIGHT_P2_MUX_DATA		= 1 << 12,	// Used for stage commands
	DDR_LIGHT_P2_MUX_CLK		= 1 << 15,	// Used for stage commands
	DDR_LIGHT_P1_BUTTONS		= 1 << 17,
	DDR_LIGHT_P2_BUTTONS		= 1 << 18,
	DDR_LIGHT_MARQUEE_BR		= 1 << 20,
	DDR_LIGHT_MARQUEE_BL		= 1 << 21,
	DDR_LIGHT_MARQUEE_TL		= 1 << 22,
	DDR_LIGHT_MARQUEE_TR		= 1 << 23,
	DDR_LIGHT_SPEAKER_DIGITAL	= 1 << 28,	// Speaker neon on digital I/O boards
	DDR_LIGHT_SPEARKER_ANALOG	= 1 << 30,	// Speaker neon on analog I/O boards

	// Dance Dance Revolution Solo
	DDRSOLO_LIGHT_EXTRA4		= 1 <<  8,
	DDRSOLO_LIGHT_EXTRA2		= 1 <<  9,
	DDRSOLO_LIGHT_EXTRA1		= 1 << 10,
	DDRSOLO_LIGHT_EXTRA3		= 1 << 11,
	DDRSOLO_LIGHT_SPEAKER		= 1 << 16,
	DDRSOLO_LIGHT_BUTTONS		= 1 << 20,
	DDRSOLO_LIGHT_BODY_CENTER	= 1 << 21,
	DDRSOLO_LIGHT_BODY_RIGHT	= 1 << 22,
	DDRSOLO_LIGHT_BODY_LEFT		= 1 << 23,

	// DrumMania 1st Mix
	DM_LIGHT_HIHAT				= 1 << 16,
	DM_LIGHT_HIGH_TOM			= 1 << 17,
	DM_LIGHT_LOW_TOM			= 1 << 18,
	DM_LIGHT_SNARE				= 1 << 19,
	DM_LIGHT_CYMBAL				= 1 << 20,
	DM_LIGHT_START_BUTTON		= 1 << 21,
	DM_LIGHT_SELECT_BUTTON		= 1 << 22,
	DM_LIGHT_NEON_BOTTOM		= 1 << 27,
	DM_LIGHT_SPOT				= 1 << 30,
	DM_LIGHT_NEON_TOP			= 1 << 31,

	// DrumMania 2nd Mix and later
	DM2_LIGHT_HIHAT				= 1 <<  0,
	DM2_LIGHT_HIGH_TOM			= 1 <<  1,
	DM2_LIGHT_LOW_TOM			= 1 <<  2,
	DM2_LIGHT_SNARE				= 1 <<  3,
	DM2_LIGHT_SPOT				= 1 <<  8,
	DM2_LIGHT_NEON_TOP			= 1 <<  9,
	DM2_LIGHT_NEON_BOTTOM		= 1 << 11,
	DM2_LIGHT_CYMBAL			= 1 << 12,
	DM2_LIGHT_START_BUTTON		= 1 << 13,
	DM2_LIGHT_SELECT_BUTTON		= 1 << 14
} K573_Light;

/* System information structures */

typedef enum _K573_IOBoardType {
	IO_TYPE_ANALOG		= 0, // Light control board (early Bemani)
	IO_TYPE_DIGITAL		= 1, // Light control + MP3 playback board (late Bemani)
	IO_TYPE_FISHBAIT	= 2, // Fishing reel controls interface (Fisherman's Bait)
	IO_TYPE_GUNMANIA	= 3, // Gun control board (Gun Mania)
	IO_TYPE_KARAOKE		= 4, // Karaoke I/O + video mux board (DDR Karaoke Mix)
	IO_TYPE_SERIAL		= 5  // Serial port (debug?) board (Great Bishi Bashi Champ)
	// TODO: does PunchMania have its own board?
} K573_IOBoardType;

typedef enum _K573_DDRStageType {
	DDR_TYPE_NONE		= 0,
	DDR_TYPE_2PLAYER	= 1,
	DDR_TYPE_SOLO		= 2
} K573_DDRStageType;

/* Public API */

#define K573_RESET_WATCHDOG() { \
	K573_WATCHDOG = 0; \
}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns a bitfield containing the state of all JAMMA inputs and DIP
 * switches. All bits are inverted as they represent the actual signal levels
 * on the JAMMA pins (i.e. normally pulled up by resistors, shorted to ground
 * when a button is pressed).
 *
 * @return Inverted logical OR of K573_JAMMAInputs flags
 */
uint32_t K573_GetJAMMAInputs(void);

/**
 * @brief Sets the 32 light outputs provided by the the analog and digital I/O
 * boards to match the provided bitfield. K573_SetBoardType(IO_TYPE_ANALOG) or
 * K573_SetBoardType(IO_TYPE_DIGITAL) must be called beforehand to set the I/O
 * board type.
 *
 * @param lights Non-inverted logical OR of K573_Light flags
 */
void K573_SetLights(uint32_t lights);

/**
 * @brief Sets the installed I/O board type. Currently only IO_TYPE_ANALOG and
 * IO_TYPE_DIGITAL are supported.
 *
 * @param type
 */
void K573_SetBoardType(K573_IOBoardType type);

/**
 * @brief Sends a command to the multiplexer PCB embedded into DDR stage units
 * (if the system is a DDR cabinet) by bitbanging it through the light outputs.
 * K573_SetBoardType(IO_TYPE_ANALOG) or K573_SetBoardType(IO_TYPE_DIGITAL) must
 * be called beforehand to set the I/O board type.
 *
 * @param value
 * @param length Number of bits to send (1-32)
 */
//void K573_DDRStageCommand(uint32_t value, uint32_t length);

/**
 * @brief Initializes the expansion port registers to enable System 573 I/O
 * access.
 */
void K573_Init(void);

#ifdef __cplusplus
}
#endif

#endif
