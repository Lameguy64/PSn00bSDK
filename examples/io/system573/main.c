/*
 * PSn00bSDK Konami System 573 example
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * This is a minimal example demonstrating how to target the Konami System 573
 * using PSn00bSDK. The System 573 is a PS1-based arcade motherboard that
 * powered various Konami arcade games throughout the late 1990s, most notably
 * Dance Dance Revolution and other Bemani rhythm games. It came in several
 * configurations, with slightly different I/O connectors depending on the game
 * and optional add-on boards providing extra features such as light control
 * outputs or MP3 audio playback.
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
#include <psxapi.h>
#include <psxgpu.h>

#include "k573io.h"

#define _btoi(x) ((((x) >> 4) & 0xf) * 10 + ((x) & 0xf))

/* Display/GPU context utilities */

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define BGCOLOR_R 48
#define BGCOLOR_G 24
#define BGCOLOR_B  0

typedef struct {
	DISPENV disp;
	DRAWENV draw;
} Framebuffer;

typedef struct {
	Framebuffer db[2];
	int         db_active;
} RenderContext;

void init_context(RenderContext *ctx) {
	Framebuffer *db;

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

void display(RenderContext *ctx) {
	Framebuffer *db;

	DrawSync(0);
	VSync(0);
	ctx->db_active ^= 1;

	db = &(ctx->db[ctx->db_active]);
	PutDrawEnv(&(db->draw));
	PutDispEnv(&(db->disp));
	SetDispMask(1);
}

/* Main */

static RenderContext ctx;

int main(int argc, const char* argv[]) {
	init_context(&ctx);
	K573_Init();

	const char *const version = (const char *const) GetSystemInfo(0x02);

	uint32_t counter       = 0;
	uint32_t inputs        = K573_GetJAMMAInputs();
	uint32_t last_inputs   = 0xff;
	uint32_t current_light = 0;

	K573_SetAnalogLights(1);

	while (1) {
		inputs = K573_GetJAMMAInputs();

		FntPrint(-1, "COUNTER=%d\n", counter++);

		FntPrint(-1, "\nJAMMA INPUTS:\n");
		FntPrint(-1, " MAIN=%016@\n",  inputs & 0xffff);
		FntPrint(-1, " EXT1=%04@\n",  (inputs >> 16) & 0x0f);
		FntPrint(-1, " EXT2=%04@\n",  (inputs >> 20) & 0x0f);
		FntPrint(-1, " COIN=%05@\n",  (inputs >> 24) & 0x1f);

		FntPrint(-1, "\nANALOG IO LIGHT TEST:\n");
		FntPrint(-1, " LAMP=%d\n", current_light);
		FntPrint(-1, " PRESS [TEST] TO CHANGE LAMP\n");

		// Request the current date/time from the RTC and display it.
		K573_RTC[RTC_REG_CTRL] |= 0x40;
		FntPrint(-1, "\nRTC:\n");
		FntPrint(
			-1,
			" %02d-%02d-%02d %02d:%02d:%02d\n",
			_btoi(K573_RTC[RTC_REG_YEAR]),
			_btoi(K573_RTC[RTC_REG_MONTH]),
			_btoi(K573_RTC[RTC_REG_DAY_OF_MONTH] & 0x3f),
			_btoi(K573_RTC[RTC_REG_HOURS]),
			_btoi(K573_RTC[RTC_REG_MINUTES]),
			_btoi(K573_RTC[RTC_REG_SECONDS] & 0x7f)
		);

		FntPrint(-1, "\nSYSTEM:\n");
		FntPrint(-1, " KERNEL=%s\n",   version);
		FntPrint(-1, " DIP SW=%03@\n", inputs >> 29);

		FntFlush(-1);
		display(&ctx);

		// Reset the watchdog. This must be done at least once per frame to
		// prevent the 573 from rebooting.
		K573_RESET_WATCHDOG();

		// Change the currently active light if the test button on the 573's
		// front panel is pressed. DDR non-light outputs are skipped.
		if (!(last_inputs & JAMMA_TEST) && (inputs & JAMMA_TEST)) {
			do {
				current_light = (current_light + 1) % 28;
			} while (
				(current_light ==  4) || // LIGHT_DDR_P1_IO_DATA
				(current_light ==  5) || // LIGHT_DDR_P1_IO_CLK
				(current_light == 12) || // LIGHT_DDR_P2_IO_DATA
				(current_light == 13)    // LIGHT_DDR_P2_IO_CLK
			);

			K573_SetAnalogLights(1 << current_light);
		}

		last_inputs = inputs;
	}

	return 0;
}
