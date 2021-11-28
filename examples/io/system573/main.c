/*
 * PSn00bSDK Konami System 573 example
 * (C) 2021 spicyjpeg - MPL licensed
 *
 * This is a minimal example demonstrating how to target the Konami System 573
 * using PSn00bSDK. The System 573 is a PS1-based arcade motherboard that
 * powered various Konami arcade games throughout the late 1990s, most notably
 * Dance Dance Revolution and other Bemani rhythm games. It came in several
 * configurations, with slightly different I/O connectors depending on the game
 * and two optional add-on modules (known as the "analog I/O" and "digital I/O"
 * boards respectively) providing light control outputs and, in the case of the
 * digital I/O board, MP3 audio playback.
 *
 * Unlike other arcade systems based on PS1 hardware, the 573 is mostly
 * identical to a regular PS1, with almost all custom extensions mapped into
 * the expansion port region at 0x1f000000. The major differences are:
 *
 * - RAM is 4 MB instead of 2, and VRAM is 2 MB instead of 1. It is recommended
 *   *not* to use the additional memory to preserve PS1 compatibility.
 *
 * - The CD drive is replaced by a standard IDE/ATAPI drive (which most of the
 *   time is going to be an aftermarket DVD drive, as the original drives the
 *   system shipped with were prone to failure and couldn't read CD-Rs). This
 *   also means the 573 has no support at all for XA audio playback, as XA is
 *   not part of the CD-ROM specification implemented by IDE drives. CD audio
 *   is supported by most IDE drives, but 573 units with the digital I/O board
 *   installed have the 4-pin audio cable plugged into that instead of the
 *   drive. The IDE bus is connected to IRQ10 and DMA5 (expansion port) instead
 *   of IRQ2 and DMA3, which go unused.
 *
 * - The BIOS seems to have most file I/O APIs removed and exposes no functions
 *   whatsoever for accessing the IDE drive or the filesystem on the disc. The
 *   launcher/shell is completely different from Sony's shell and is capable of
 *   loading an executable from the CD drive, a PCMCIA memory-mapped flash card
 *   or the internal 16 MB flash memory.
 *
 * - The SPI controller bus seems to be left unconnected. Inputs are routed to
 *   a JAMMA PCB edge connector and handled through two custom/relabeled chips,
 *   which expose the inputs as memory-mapped registers. There is also a JVS
 *   port (i.e. RS-485 serial bus wired to a USB-A connector, commonly used for
 *   daisy-chaining peripherals in arcade cabinets) managed by a
 *   microcontroller.
 *
 * - There is a "security cartridge" slot, which breaks out the serial port as
 *   well as several GPIO pins. All security cartridge communication and DRM is
 *   handled by games rather than by the BIOS, so a security cartridge is *not*
 *   required to boot homebrew. Each game came with a different cartridge type;
 *   many of them expose the serial port or provide additional game-specific
 *   I/O connectors.
 *
 * Currently the only publicly available documentation for the custom registers
 * is the System 573 MAME driver. Also keep in mind that the psxcd library does
 * not yet support IDE drives, so the 573's drive can only be accessed by
 * writing a custom ATAPI driver and ISO9660 parser (which is out of the scope
 * of this example).
 *
 * https://github.com/mamedev/mame/blob/master/src/mame/drivers/ksys573.cpp
 * https://github.com/mamedev/mame/blob/master/src/mame/machine/k573dio.cpp
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxpad.h>

/* Register definitions */

#define EXP1_ADDR		*((volatile uint32_t *) 0x1f801000)
#define EXP1_CTRL		*((volatile uint32_t *) 0x1f801008)

#define K573_IN0		*((volatile uint16_t *) 0x1f400000)
#define K573_IN1_L		*((volatile uint16_t *) 0x1f400004)
#define K573_IN1_H		*((volatile uint16_t *) 0x1f400006)
#define K573_IN2		*((volatile uint16_t *) 0x1f400008)
#define K573_IN3_L		*((volatile uint16_t *) 0x1f40000c)
#define K573_IN3_H		*((volatile uint16_t *) 0x1f40000e)
#define K573_BANK		*((volatile uint16_t *) 0x1f500000)
#define K573_WATCHDOG	*((volatile uint16_t *) 0x1f5c0000)

#define K573_IDE_CS0	((volatile uint16_t *) 0x1f480000)
#define K573_IDE_CS1	((volatile uint16_t *) 0x1f4c0000)
#define K573_RTC		((volatile uint16_t *) 0x1f620000)
#define K573_IO_BOARD	((volatile uint16_t *) 0x1f640000)

typedef enum {
	ANALOG_IO_LIGHTS0	= 0x20,
	ANALOG_IO_LIGHTS1	= 0x22,
	ANALOG_IO_LIGHTS2	= 0x24,
	ANALOG_IO_LIGHTS3	= 0x26,

	// The digital I/O board has a lot more registers than these, but there
	// seems to be no DIGITAL_IO_LIGHTS6 register. WTF
	DIGITAL_IO_LIGHTS1	= 0x70,
	DIGITAL_IO_LIGHTS0	= 0x71,
	DIGITAL_IO_LIGHTS3	= 0x72,
	DIGITAL_IO_LIGHTS7	= 0x73,
	DIGITAL_IO_LIGHTS4	= 0x7d,
	DIGITAL_IO_LIGHTS5	= 0x7e,
	DIGITAL_IO_LIGHTS2	= 0x7f
} IO_BOARD_REG;

// The 573's real-time clock chip is an M48T58, which behaves like a standard
// 8 KB battery-backed SRAM with a bunch of special registers. Official games
// store highscores and settings in RTC RAM.
typedef enum {
	RTC_CTRL			= 0x1ff8,
	RTC_SECONDS			= 0x1ff9,
	RTC_MINUTES			= 0x1ffa,
	RTC_HOURS			= 0x1ffb,
	RTC_DAY_OF_WEEK		= 0x1ffc,
	RTC_DAY_OF_MONTH	= 0x1ffd,
	RTC_MONTH			= 0x1ffe,
	RTC_YEAR			= 0x1fff
} RTC_REG;

#define btoi(x) ((((x) >> 4) & 0xf) * 10 + ((x) & 0xf))

/* Display/GPU context utilities */

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define BGCOLOR_R 48
#define BGCOLOR_G 24
#define BGCOLOR_B  0

typedef struct {
	DISPENV  disp;
	DRAWENV  draw;
} DB;

typedef struct {
	DB       db[2];
	uint32_t db_active;
} CONTEXT;

void init_context(CONTEXT *ctx) {
	DB *db;

	ResetGraph(0);
	ctx->db_active = 0;

	db = &(ctx->db[0]);
	SetDefDispEnv(&(db->disp),           0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), BGCOLOR_R, BGCOLOR_G, BGCOLOR_B);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	db = &(ctx->db[1]);
	SetDefDispEnv(&(db->disp), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw),           0, 0, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), BGCOLOR_R, BGCOLOR_G, BGCOLOR_B);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	PutDrawEnv(&(db->draw));
	//PutDispEnv(&(db->disp));

	// Create a text stream at the top of the screen.
	FntLoad(960, 0);
	FntOpen(8, 16, 304, 208, 2, 512);
}

void display(CONTEXT *ctx) {
	DB *db;

	DrawSync(0);
	VSync(0);
	ctx->db_active ^= 1;

	db = &(ctx->db[ctx->db_active]);
	PutDrawEnv(&(db->draw));
	PutDispEnv(&(db->disp));
	SetDispMask(1);
}

/* Input polling utilities */

typedef struct {
	uint8_t p1_joy, p1_btn;
	uint8_t p2_joy, p2_btn;
	uint8_t coin, dip_sw;
} JAMMA_INPUTS;

void get_jamma_inputs(JAMMA_INPUTS *output) {
	uint16_t in1l = K573_IN1_L;
	uint16_t in1h = K573_IN1_H;
	uint16_t in2  = K573_IN2;
	uint16_t in3l = K573_IN3_L;
	uint16_t in3h = K573_IN3_H;
	uint8_t  p1_btn, p2_btn, coin;

	// Rearrange the bits read from the input register into something that's
	// easier to parse and display. Refer to MAME for information on what each
	// bit in the IN* registers does.
	p1_btn  = ((in2  >> 15) & 0x0001);      // Bit 0   = start button
	p1_btn |= ((in2  >>  8) & 0x0007) << 1; // Bit 1-3 = buttons 1-3
	p1_btn |= ((in3l >>  8) & 0x0003) << 4; // Bit 4-5 = buttons 4-5
	p1_btn |= ((in3l >> 11) & 0x0001) << 6; // Bit 6   = button 6
	p2_btn  = ((in2  >>  7) & 0x0001);      // Bit 0   = start button
	p2_btn |= ((in2  >>  4) & 0x0007) << 1; // Bit 1-3 = buttons 1-3
	p2_btn |= ((in3h >>  8) & 0x0003) << 4; // Bit 4-5 = buttons 4-5
	p2_btn |= ((in3h >> 11) & 0x0001) << 6; // Bit 6   = button 6
	coin    = ((in1h >>  8) & 0x0003);      // Bit 0-1 = coin switches
	coin   |= ((in1h >> 12) & 0x0001) << 2; // Bit 2   = service button
	coin   |= ((in3l >> 10) & 0x0001) << 3; // Bit 3   = test button
	coin   |= ((in1h >> 10) & 0x0003) << 4; // Bit 4-5 = PCMCIA cards

	output->p1_joy = (in2 >> 8) & 0x000f;
	output->p1_btn = p1_btn;
	output->p2_joy = in2 & 0x000f;
	output->p2_btn = p2_btn;
	output->coin   = coin;
	output->dip_sw = in1l & 0x000f;
}

/* I/O board (light control) utilities */

// This function controls light outputs on analog I/O boards.
void set_lights_analog(uint32_t lights) {
	uint32_t bits;

	bits  = (lights & 0x01010101) << 7; // Lamp 0 -> bit 7
	bits |= (lights & 0x02020202) << 5; // Lamp 1 -> bit 6
	bits |= (lights & 0x04040404) >> 1; // Lamp 2 -> bit 1
	bits |= (lights & 0x08080808) >> 3; // Lamp 3 -> bit 0
	bits |= (lights & 0x10101010) << 1; // Lamp 4 -> bit 5
	bits |= (lights & 0x20202020) >> 1; // Lamp 5 -> bit 4
	bits |= (lights & 0x40404040) >> 3; // Lamp 6 -> bit 3
	bits |= (lights & 0x80808080) >> 5; // Lamp 7 -> bit 2

	K573_IO_BOARD[ANALOG_IO_LIGHTS0] = (bits)       & 0xff;
	K573_IO_BOARD[ANALOG_IO_LIGHTS1] = (bits >>  8) & 0xff;
	K573_IO_BOARD[ANALOG_IO_LIGHTS2] = (bits >> 16) & 0xff;
	K573_IO_BOARD[ANALOG_IO_LIGHTS3] = (bits >> 24) & 0xff;
}

// This function controls light outputs on digital I/O boards (i.e. the ones
// that include MP3 playback hardware in addition to the light control).
// TODO: test this on real hardware -- it might not work if lights are handled
// by the board's FPGA, which requires a binary blob...
void set_lights_digital(uint32_t lights) {
	uint32_t bits;

	bits  = (lights & 0x11111111);      // Lamp 0 -> bit 0
	bits |= (lights & 0x22222222) << 1; // Lamp 1 -> bit 2
	bits |= (lights & 0x44444444) << 1; // Lamp 2 -> bit 3
	bits |= (lights & 0x88888888) >> 2; // Lamp 3 -> bit 1

	K573_IO_BOARD[DIGITAL_IO_LIGHTS0] = ((bits)       & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_LIGHTS1] = ((bits >>  4) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_LIGHTS2] = ((bits >>  8) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_LIGHTS3] = ((bits >> 12) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_LIGHTS4] = ((bits >> 16) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_LIGHTS5] = ((bits >> 20) & 0xf) << 12;
	//K573_IO_BOARD[DIGITAL_IO_LIGHTS6] = ((bits >> 24) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_LIGHTS7] = ((bits >> 28) & 0xf) << 12;
}

/* Main */

static CONTEXT ctx;

#define SHOW_STATUS(...) { FntPrint(-1, __VA_ARGS__); FntFlush(-1); display(&ctx); }
#define SHOW_ERROR(...)  { SHOW_STATUS(__VA_ARGS__); while (1) __asm__("nop"); }

int main(int argc, const char* argv[]) {
	// Reinitialize the heap and relocate the stack to allow the 573's full 4
	// MB of RAM to be used. This isn't strictly required; executables designed
	// for 2 MB of RAM will also run fine on the 573 (obviously).
	// FIXME: this seems to be broken currently
	//__asm__ volatile("li $sp, 0x803fffe0");
	//_mem_init(0x400000, 0x20000);

	EXP1_ADDR     = 0x1f000000;
	EXP1_CTRL     = 0x24173f47; // 573 BIOS uses this value
	K573_WATCHDOG = 0;

	init_context(&ctx);

	// Determine whether we are running on a 573 by fetching the version string
	// from the BIOS.
	const char *const version = (const char *const) GetSystemInfo(0x02);
	//if (strncmp(version, "Konami OS", 9))
		//SHOW_ERROR("ERROR: NOT RUNNING ON A SYSTEM 573!\n\n[%s]\n", version);

	uint32_t counter       = 0;
	uint8_t  last_joystick = 0xff;
	uint8_t  last_buttons  = 0xff;
	uint32_t current_light = 0;
	uint32_t is_digital    = 0;

	while (1) {
		FntPrint(-1, "COUNTER=%d\n", counter++);

		JAMMA_INPUTS inputs;
		get_jamma_inputs(&inputs);

		FntPrint(-1, "\nJAMMA INPUTS:\n");
		FntPrint(-1, " P1 JOYSTICK =%04@\n", inputs.p1_joy);
		FntPrint(-1, " P1 BUTTONS  =%07@\n", inputs.p1_btn);
		FntPrint(-1, " P2 JOYSTICK =%04@\n", inputs.p2_joy);
		FntPrint(-1, " P2 BUTTONS  =%07@\n", inputs.p2_btn);
		FntPrint(-1, " COIN/SERVICE=%04@\n", inputs.coin);
		FntPrint(-1, " DIP SWITCHES=%04@\n", inputs.dip_sw);

		FntPrint(-1, "\nCABINET LIGHTS:\n");
		FntPrint(-1, " BOARD=%s I/O\n", is_digital ? "DIGITAL" : "ANALOG");
		FntPrint(-1, " LIGHT=%d\n\n",   current_light);
		FntPrint(-1, " [START]      CHANGE BOARD TYPE\n");
		FntPrint(-1, " [LEFT/RIGHT] SELECT LIGHT TO TEST\n");

		// Request the current date/time from the RTC and display it.
		K573_RTC[RTC_CTRL] |= 0x40;
		FntPrint(-1, "\nRTC:\n");
		FntPrint(
			-1,
			" %02d-%02d-%02d %02d:%02d:%02d\n",
			btoi(K573_RTC[RTC_YEAR]),
			btoi(K573_RTC[RTC_MONTH]),
			btoi(K573_RTC[RTC_DAY_OF_MONTH] & 0x3f),
			btoi(K573_RTC[RTC_HOURS]),
			btoi(K573_RTC[RTC_MINUTES]),
			btoi(K573_RTC[RTC_SECONDS] & 0x7f)
		);

		FntPrint(-1, "\nSYSTEM:\n");
		FntPrint(-1, " KERNEL=%s\n",   version);
		FntPrint(-1, " PCMCIA=%02@\n", inputs.coin >> 4);

		FntFlush(-1);
		display(&ctx);

		// Reset the watchdog. This must be done at least once per frame to
		// prevent the 573 from rebooting.
		K573_WATCHDOG = 0;

		if (is_digital)
			set_lights_digital(1 << current_light);
		else
			set_lights_analog(1 << current_light);

		// Handle inputs.
		if ((last_joystick & 0x01) && !(inputs.p1_joy & 0x01)) // Left
			current_light--;
		if ((last_joystick & 0x02) && !(inputs.p1_joy & 0x02)) // Right
			current_light++;
		if ((last_buttons & 0x02) && !(inputs.p1_btn & 0x02)) // Button 1
			current_light--;
		if ((last_buttons & 0x04) && !(inputs.p1_btn & 0x04)) // Button 2
			current_light++;
		if ((last_buttons & 0x01) && !(inputs.p1_btn & 0x01)) { // Start
			is_digital = !is_digital;
			if (is_digital)
				set_lights_analog(0);
			else
				set_lights_digital(0);
		}

		current_light %= 32;
		last_joystick  = inputs.p1_joy;
		last_buttons   = inputs.p1_btn;
	}

	return 0;
}
