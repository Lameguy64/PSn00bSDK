/* Controller support header
 * Part of PSn00bSDK
 * 2019 Lameguy64 / Meido-Tek Productions
 *
 * Currently only provides a bunch of definitions and a few structs but no
 * handling functions yet. Use the code in pad.s in one of the sample
 * programs for the meantime instead.
 *
 * Work in progress, subject to change significantly in future releases.
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

// Struct for digital, joystick, dual analog and Dualshock controllers
typedef struct {
	unsigned char	stat;		// Status
	unsigned char	len:4;		// Data length (in halfwords)
	unsigned char	type:4;		// Device type:
								//  0x4 - digital pad 
								//  0x5 - analog joystick 
								//  0x7 - dual analog & Dualshock 
	unsigned short	btn;		// Button states
	unsigned char	rs_x,rs_y;	// Right stick coordinates
	unsigned char	ls_x,ls_y;	// Left stick coordinates
} PADTYPE;

// Struct for a mouse controller
typedef struct {
	unsigned char	stat;
	unsigned char	len:4;
	unsigned char	type:4;		// Device type (0x1)
	unsigned short	btn;
	char			x_mov;		// X movement of mouse
	char			y_mov;		// Y movement of mouse
} MOUSETYPE;

// Struct for a neGcon controller (for Namco neGcon)
typedef struct {
	unsigned char	stat;
	unsigned char	len:4;
	unsigned char	type:4;		// (0x2)
	unsigned short	btn;
    unsigned char	twist;		// Controller twist
	unsigned char	btn_i;		// I button value
	unsigned char	btn_ii;		// II button value
	unsigned char	trg_l;		// L trigger value
} NCONTYPE;

// Struct for a Jogcon controller (for Namco Jogcon)
typedef struct {
	unsigned char	stat;
	unsigned char	len:4;
	unsigned char	type:4;		// (0xE)
	unsigned short	btn;
    unsigned short	jog_rot;	// Jog rotation
} JCONTYPE;

// Struct for a Gun-Con controller (for Namco Gun-Con)
typedef struct {
	unsigned char	status;
	unsigned char	len:4;
	unsigned char	type:4;		// (0x6)
	unsigned short	btn;
	unsigned short	gun_x;		// Gun X position in dotclocks
	unsigned short	gun_y;		// Gun Y position in scanlines
} GCONTYPE;

#endif