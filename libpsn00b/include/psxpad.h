/* Controller support header
 * Part of PSn00bSDK
 * 2019 Lameguy64 / Meido-Tek Productions
 *
 * Currently only provides a bunch of definitions and a few structs but no
 * handling functions yet. See the io/pads example for a simple manual pollling
 * implementation (with support for DualShock 2 controllers).
 *
 * Work in progress, subject to change significantly in future releases.
 *
 * Reference: https://gist.github.com/scanlime/5042071
 */

#pragma once

#include <stdint.h>

/* Controller type and button definitions */

typedef enum {
	// Standard pads, analog joystick, Jogcon
	PAD_SELECT		= 1 << 0,
	PAD_L3			= 1 << 1,
	PAD_R3			= 1 << 2,
	PAD_START		= 1 << 3,
	PAD_UP			= 1 << 4,
	PAD_RIGHT		= 1 << 5,
	PAD_DOWN		= 1 << 6,
	PAD_LEFT		= 1 << 7,
	PAD_L2			= 1 << 8,
	PAD_R2			= 1 << 9,
	PAD_L1			= 1 << 10,
	PAD_R1			= 1 << 11,
	PAD_TRIANGLE	= 1 << 12,
	PAD_CIRCLE		= 1 << 13,
	PAD_CROSS		= 1 << 14,
	PAD_SQUARE		= 1 << 15,

	// Mouse
	MOUSE_RIGHT		= 1 << 10,
	MOUSE_LEFT		= 1 << 11,

	// neGcon
	NCON_START		= 1 << 3,
	NCON_UP			= 1 << 4,
	NCON_RIGHT		= 1 << 5,
	NCON_DOWN		= 1 << 6,
	NCON_LEFT		= 1 << 7,
	NCON_R			= 1 << 11,
	NCON_B			= 1 << 12,
	NCON_A			= 1 << 13,

	// Guncon
	GCON_A			= 1 << 3,
	GCON_TRIGGER	= 1 << 13,
	GCON_B			= 1 << 14
} PadButton;

typedef enum {
	PAD_ID_MOUSE		= 0x1, // Sony PS1 mouse
	PAD_ID_NEGCON		= 0x2, // Namco neGcon
	PAD_ID_IRQ10_GUN	= 0x3, // "Konami" lightgun without composite video passthrough
	PAD_ID_DIGITAL		= 0x4, // Digital pad or Dual Analog/DualShock in digital mode
	PAD_ID_ANALOG_STICK	= 0x5, // Flight stick or Dual Analog in green LED mode
	PAD_ID_GUNCON		= 0x6, // Namco Guncon (lightgun with composite video passthrough)
	PAD_ID_ANALOG		= 0x7, // Dual Analog/DualShock in analog (red LED) mode
	PAD_ID_MULTITAP		= 0x8, // Multitap adapter (when tap_mode == 1)
	PAD_ID_JOGCON		= 0xe, // Namco Jogcon
	PAD_ID_CONFIG_MODE	= 0xf, // Dual Analog/DualShock in config mode (if len == 0x3)
	PAD_ID_NONE			= 0xf  // No pad connected (if len == 0xf)
} PadTypeID;

/* Pad and memory card commands */

typedef enum {
	PAD_CMD_INIT_PRESSURE	= '@', // Initialize DS2 button pressure sensors (in config mode)
	PAD_CMD_READ			= 'B', // Read pad state (exchange poll request/response)
	PAD_CMD_CONFIG_MODE		= 'C', // Toggle DualShock configuration mode
	PAD_CMD_SET_ANALOG		= 'D', // Set analog mode/LED state (in config mode)
	PAD_CMD_GET_ANALOG		= 'E', // Get analog mode/LED state (in config mode)
	PAD_CMD_GET_MOTOR_INFO	= 'F', // Get information about a vibration motor (in config mode)
	PAD_CMD_GET_MOTOR_LIST	= 'G', // Get list of all vibration motors (in config mode)
	PAD_CMD_GET_MOTOR_STATE	= 'H', // Get current state of vibration motors (in config mode)
	PAD_CMD_GET_MODE		= 'L', // Get list of supported controller modes? (in config mode)
	PAD_CMD_REQUEST_CONFIG	= 'M', // Configure poll request format (in config mode)
	PAD_CMD_RESPONSE_CONFIG	= 'O', // Configure poll response format (in config mode)

	MCD_CMD_READ_SECTOR		= 'R', // Read 128-byte sector
	MCD_CMD_IDENTIFY		= 'S', // Retrieve ID and card size information (Sony cards only)
	MCD_CMD_WRITE_SECTOR	= 'W'  // Erase and write 128-byte sector
} PadCommand;

typedef enum {
	MCD_STAT_OK				= 'G',
	MCD_STAT_BAD_CHECKSUM	= 'N',
	MCD_STAT_BAD_SECTOR		= 0xff
} MemCardStatus;

typedef enum {
	MCD_FLAG_WRITE_ERROR	= 1 << 2,	// Last write command failed
	MCD_FLAG_NOT_WRITTEN	= 1 << 3,	// No writes have been issued yet
	MCD_FLAG_UNKNOWN		= 1 << 4	// Might be set on third-party cards
} MemCardStatusFlag;

#define MEMCARD_CMD_READ_LEN		139
#define MEMCARD_CMD_IDENTIFY_LEN	9
#define MEMCARD_CMD_WRITE_LEN		137

/* Controller response as returned by BIOS driver */

typedef struct __attribute__((packed)) _PADTYPE {
	uint8_t				stat;		// Status
	uint8_t				len:4;		// Payload length / 2, 0 for multitap
	uint8_t				type:4;		// Device type (PadTypeID)

	uint16_t			btn;		// Button states
	union {
		struct {					// Analog controller:
			uint8_t		rs_x,rs_y;	// - Right stick coordinates
			uint8_t		ls_x,ls_y;	// - Left stick coordinates
			uint8_t		press[12];	// - Button pressure (DualShock 2 only)
		};
		struct {					// Mouse:
			int8_t		x_mov;		// - X movement of mouse
			int8_t		y_mov;		// - Y movement of mouse
		};
		struct {					// neGcon:
			uint8_t		twist;		// - Controller twist
			uint8_t		btn_i;		// - I button value
			uint8_t		btn_ii;		// - II button value
			uint8_t		trg_l;		// - L trigger value
		};
		struct {					// Jogcon:
			uint16_t	jog_rot;	// - Jog rotation
		};
		struct {					// Guncon:
			uint16_t	gun_x;		// - Gun X position in dotclocks
			uint16_t	gun_y;		// - Gun Y position in scanlines
		};
	};
} PADTYPE;

//typedef struct _PADTYPE MOUSETYPE;
//typedef struct _PADTYPE NCONTYPE;
//typedef struct _PADTYPE JCONTYPE;
//typedef struct _PADTYPE GCONTYPE;

/* Raw responses */

typedef struct __attribute__((packed)) _PadResponse {
	uint8_t				len:4;		// Payload length / 2, 0 for multitap
	uint8_t				type:4;		// Device type (PadTypeID)
	uint8_t				prefix;		// Must be 0x5a

	uint16_t			btn;		// Button states
	union {
		struct {					// Analog controller:
			uint8_t		rs_x,rs_y;	// - Right stick coordinates
			uint8_t		ls_x,ls_y;	// - Left stick coordinates
			uint8_t		press[12];	// - Button pressure (DualShock 2 only)
		};
		struct {					// Mouse:
			int8_t		x_mov;		// - X movement of mouse
			int8_t		y_mov;		// - Y movement of mouse
		};
		struct {					// neGcon:
			uint8_t		twist;		// - Controller twist
			uint8_t		btn_i;		// - I button value
			uint8_t		btn_ii;		// - II button value
			uint8_t		trg_l;		// - L trigger value
		};
		struct {					// Jogcon:
			uint16_t	jog_rot;	// - Jog rotation
		};
		struct {					// Guncon:
			uint16_t	gun_x;		// - Gun X position in dotclocks
			uint16_t	gun_y;		// - Gun Y position in scanlines
		};
	};
} PadResponse;

typedef struct __attribute__((packed)) _MemCardResponse {
	uint8_t			flags;		// Status flags (MemCardStatusFlag)
	uint8_t			type1;		// Must be 0x5a
	uint8_t			type2;		// Must be 0x5d

	union {
		struct {				// CMD_READ response:
			uint8_t	dummy[2];
			uint8_t	ack1;		//  Must be 0x5c
			uint8_t	ack2;		//  Must be 0x5d
			uint8_t	lba_h;
			uint8_t	lba_l;
			uint8_t	data[128];
			uint8_t	checksum;	//  = lba_h ^ lba_l ^ data
			uint8_t	stat;		//  Status (MemCardStatus)
		} read;
		struct {				// CMD_IDENTIFY response:
			uint8_t	ack1;		//  Must be 0x5c
			uint8_t	ack2;		//  Must be 0x5d
			uint8_t	size_h;		//  Card capacity bits 8-15 (0x04 = 128KB)
			uint8_t	size_l;		//  Card capacity bits 0-7 (0x00 = 128KB)
			uint8_t	blksize_h;	//  Sector size bits 8-15 (must be 0x00)
			uint8_t	blksize_l;	//  Sector size bits 0-7 (must be 0x80)
		} identify;
		struct {				// CMD_WRITE response:
			uint8_t	dummy[131];
			uint8_t	ack1;		//  Must be 0x5c
			uint8_t	ack2;		//  Must be 0x5d
			uint8_t	stat;		//  Status (MemCardStatus)
		} write;
	};
} MemCardResponse;

/* Raw requests */

typedef struct __attribute__((packed)) _PadRequest {
	uint8_t addr;		// Must be 0x01 (or 02/03/04 for multitap pads)
	uint8_t cmd;		// Command (PadCommand)
	uint8_t tap_mode;	// 0x01 to enable multitap response
	uint8_t motor_r;	// Right motor control (on/off)
	uint8_t motor_l;	// Left motor control (PWM)
	uint8_t dummy[4];
} PadRequest;

typedef struct __attribute__((packed)) _MemCardRequest {
	uint8_t addr;		// Must be 0x81 (or 02/03/04 for multitap cards)
	uint8_t cmd;		// Command (MemCardCommand)
	uint8_t dummy[2];
	uint8_t lba_h;		// Sector address bits 8-15 (dummy for CMD_IDENTIFY)
	uint8_t lba_l;		// Sector address bits 0-7 (dummy for CMD_IDENTIFY)
	uint8_t data[128];	// Sector payload (dummy for CMD_READ/CMD_IDENTIFY)
	uint8_t checksum;	// = lba_h ^ lba_l ^ data (CMD_WRITE only)
	uint8_t dummy2[3];
} MemCardRequest;
