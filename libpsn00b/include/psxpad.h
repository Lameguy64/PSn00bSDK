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
 
#ifndef _PSXPAD_H
#define _PSXPAD_H

// Pad button definitions for digital pad, joystick, dual analog, 
// Dualshock and Jogcon
#define PAD_SELECT		1
#define PAD_L3			2
#define PAD_R3			4
#define PAD_START		8
#define PAD_UP			16
#define PAD_RIGHT		32
#define PAD_DOWN		64
#define PAD_LEFT		128
#define PAD_L2			256
#define PAD_R2			512
#define PAD_L1			1024
#define PAD_R1			2048
#define PAD_TRIANGLE	4096
#define PAD_CIRCLE		8192
#define PAD_CROSS		16384
#define PAD_SQUARE		32768

// Mouse button definitions
#define MOUSE_RIGHT		1024
#define MOUSE_LEFT		2048

// neGcon button definitions
#define NCON_START		8
#define NCON_UP			16
#define NCON_RIGHT		32
#define NCON_DOWN		64
#define NCON_LEFT		128
#define NCON_R			256
#define NCON_B			512
#define NCON_A			1024

// Guncon button definitions
#define GCON_A			8
#define GCON_TRIGGER	8192
#define GCON_B			16384

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
} PAD_TYPEID;

// Controller command definitions
typedef enum {
	PAD_CMD_INIT_PRESSURE	= '@', // Initialize DS2 button pressure sensors (in config mode)
	PAD_CMD_READ			= 'B', // Read pad state and set rumble
	PAD_CMD_CONFIG_MODE		= 'C', // Toggle DualShock configuration mode
	PAD_CMD_SET_ANALOG		= 'D', // Set analog mode/LED state (in config mode)
	PAD_CMD_GET_ANALOG		= 'E', // Get analog mode/LED state (in config mode)
	PAD_CMD_REQUEST_CONFIG	= 'M', // Configure request/unlock vibration (in config mode)
	PAD_CMD_RESPONSE_CONFIG	= 'O'  // Configure response/unlock DS2 pressure (in config mode)
} PAD_COMMAND;

// Memory card command/response definitions
typedef enum {
	MCD_CMD_READ		= 'R', // Read sector
	MCD_CMD_IDENTIFY	= 'S', // Retrieve ID and card size information
	MCD_CMD_WRITE		= 'W'  // Write sector
} MCD_COMMAND;

typedef enum {
	MCD_STAT_OK				= 'G',
	MCD_STAT_BAD_CHECKSUM	= 'N',
	MCD_STAT_BAD_SECTOR		= 0xff
} MCD_STATUS;

#define MCD_CMD_READ_LEN		139
#define MCD_CMD_IDENTIFY_LEN	9
#define MCD_CMD_WRITE_LEN		137

// Memory card status flags
#define MCD_FLAG_WRITE_ERROR	4	// Last write command failed
#define MCD_FLAG_NOT_WRITTEN	8	// No writes have been issued yet
#define MCD_FLAG_UNKNOWN		16	// Might be set on third-party cards

// Struct for data returned by controllers
typedef struct _PADTYPE {
	union {									// Header:
		struct __attribute__((packed)) {	//  When parsing data returned by BIOS:
			unsigned char		stat;		//   Status
			unsigned char		len:4;		//   Payload length / 2, 0 for multitap
			unsigned char		type:4;		//   Device type (PAD_TYPEID)
		};
		struct __attribute__((packed)) {	//  When parsing raw controller response:
			unsigned char		len:4;		//   Payload length / 2, 0 for multitap
			unsigned char		type:4;		//   Device type (PAD_TYPEID)
			unsigned char		prefix;		//   Must be 0x5a
		} raw;
	};
	struct {								// Payload:
		unsigned short			btn;		//  Button states
		union {
			struct {						//  Analog controller:
				unsigned char	rs_x,rs_y;	//   Right stick coordinates
				unsigned char	ls_x,ls_y;	//   Left stick coordinates
				unsigned char	press[12];	//   Button pressure (DualShock 2 only)
			};
			struct {						//  Mouse:
				char			x_mov;		//   X movement of mouse
				char			y_mov;		//   Y movement of mouse
			};
			struct {						//  neGcon:
				unsigned char	twist;		//   Controller twist
				unsigned char	btn_i;		//   I button value
				unsigned char	btn_ii;		//   II button value
				unsigned char	trg_l;		//   L trigger value
			};
			struct {						//  Jogcon:
				unsigned short	jog_rot;	//   Jog rotation
			};
			struct {						//  Guncon:
				unsigned short	gun_x;		//   Gun X position in dotclocks
				unsigned short	gun_y;		//   Gun Y position in scanlines
			};
		};
	};
} PADTYPE;

typedef struct _MCDRESPONSE {
	unsigned char	flags;		// Status flags
	unsigned char	type1;		// Must be 0x5a
	unsigned char	type2;		// Must be 0x5d
	union {
		struct {						// MCD_CMD_READ response:
			unsigned char	dummy[2];
			unsigned char	ack1;		//  Must be 0x5c
			unsigned char	ack2;		//  Must be 0x5d
			unsigned char	lba_h;
			unsigned char	lba_l;
			unsigned char	data[128];
			unsigned char	checksum;	//  = lba_h ^ lba_l ^ data
			unsigned char	stat;		//  Status (MCD_STATUS)
		} read;
		struct {						// MCD_CMD_IDENTIFY response:
			unsigned char	ack1;		//  Must be 0x5c
			unsigned char	ack2;		//  Must be 0x5d
			unsigned char	size_h;		//  Card capacity bits 8-15 (0x04 = 128KB)
			unsigned char	size_l;		//  Card capacity bits 0-7 (0x00 = 128KB)
			unsigned char	blksize_h;	//  Sector size bits 8-15 (must be 0x00)
			unsigned char	blksize_l;	//  Sector size bits 0-7 (must be 0x80)
		} identify;
		struct {						// MCD_CMD_WRITE response:
			unsigned char	dummy[131];
			unsigned char	ack1;		//  Must be 0x5c
			unsigned char	ack2;		//  Must be 0x5d
			unsigned char	stat;		//  Status (MCD_STATUS)
		} write;
	};
} MCDRESPONSE;

//typedef PADTYPE MOUSETYPE;
//typedef PADTYPE NCONTYPE;
//typedef PADTYPE JCONTYPE;
//typedef PADTYPE GCONTYPE;

// Structs for raw controller request
typedef struct _PADREQUEST {
	unsigned char	addr;		// Must be 0x01 (or 02/03/04 for multitap pads)
	unsigned char	cmd;		// Command (PAD_COMMAND)
	unsigned char	tap_mode;	// 0x01 to enable multitap response
	unsigned char	motor_r;	// Right motor control (on/off)
	unsigned char	motor_l;	// Left motor control (PWM)
	unsigned char	dummy[4];
} PADREQUEST;

// Structs for raw memory card request
typedef struct _MCDREQUEST {
	unsigned char	addr;		// Must be 0x81 (or 02/03/04 for multitap cards)
	unsigned char	cmd;		// Command (MCD_COMMAND)
	unsigned char	dummy[2];
	unsigned char	lba_h;		// Sector address bits 8-15 (dummy for CMD_IDENTIFY)
	unsigned char	lba_l;		// Sector address bits 0-7 (dummy for CMD_IDENTIFY)
	unsigned char	data[128];	// Sector payload (dummy for CMD_READ/CMD_IDENTIFY)
	unsigned char	checksum;	// = lba_h ^ lba_l ^ data (CMD_WRITE only)
	unsigned char	dummy2[3];
} MCDREQUEST;

#endif