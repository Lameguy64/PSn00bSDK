/*
 * PSn00bSDK controller polling example
 * (C) 2021 spicyjpeg - MPL licensed
 *
 * This example shows how to poll controllers at high speeds (250 Hz) by using
 * timer interrupts and communicating with devices on the SPI controller bus
 * manually rather than relying on the BIOS pad driver, which is limited to
 * 50/60 Hz and does not support custom commands. The example also demonstrates
 * using configuration mode commands to force DualShock pads into analog mode
 * and enable button pressure sensing on DualShock 2 (PS2) controllers.
 *
 * See spi.c for details on how the low-level SPI communication driver works.
 * The DualShock handshaking logic is implemented here (see poll_cb() and
 * dualshock_init_cb()). There is no support for memory cards in this example,
 * but the code in spi.c can be used to read/write sectors on a memory card and
 * combined with a higher-level filesystem driver for full support.
 *
 * IMPORTANT: some controller models seem to be unable to respond to config
 * mode commands reliably, even though simple high-speed polling usually works
 * without issues. Also keep in mind that many emulators emulate controllers
 * and memory cards inaccurately. It is thus recommended to test controller I/O
 * code extensively and handle as many edge cases as possible (e.g. partial but
 * valid responses, zerofilled responses, slow replies) for best compatibility.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psxetc.h>
#include <psxgpu.h>
#include <psxpad.h>

#include "spi.h"

static const char *const PAD_TYPEIDS[] = {
	"[UNKNOWN]",
	"MOUSE",
	"NEGCON",
	"IRQ10_GUN",
	"DIGITAL",
	"ANALOG_STICK",
	"GUNCON",
	"ANALOG",
	"MULTITAP",
	"[UNKNOWN]",
	"[UNKNOWN]",
	"[UNKNOWN]",
	"[UNKNOWN]",
	"[UNKNOWN]",
	"JOGCON",
	"CONFIG_MODE"
};

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

/* Pad buffers and callback */

static volatile uint8_t  pad_buff[2][34];
static volatile size_t   pad_buff_len[2];
static volatile uint32_t pad_config_attempt[2] = { 0, 0 };

// Just a wrapper around SPI_CreateRequest(). This does not send the command
// immediately but adds it to the driver's request queue.
void send_pad_cmd(
	uint32_t     port,
	PadCommand   cmd,
	uint8_t      arg1,
	uint8_t      arg2,
	SPI_Callback callback
) {
	SPI_Request *req = SPI_CreateRequest();

	req->len              = 9;
	req->port             = port;
	req->callback         = callback;
	req->pad_req.addr     = 0x01;
	req->pad_req.cmd      = cmd;
	req->pad_req.tap_mode = 0x00;
	req->pad_req.motor_r  = arg1;
	req->pad_req.motor_l  = arg2;

	// The padding bytes must be 0xff when unlocking vibration motors.
	memset(
		req->pad_req.dummy,
		(cmd == PAD_CMD_REQUEST_CONFIG) ? 0xff : 0x00,
		4
	);
}

// This callback determines whether a pad that identified as digital is
// actually a DualShock in digital mode by checking if it started identifying
// as CONFIG_MODE after receiving a configuration command. Calls to printf()
// had to be commented out due to them being too slow.
void dualshock_init_cb(uint32_t port, const volatile uint8_t *buff, size_t rx_len) {
	PadResponse *pad = (PadResponse *) buff;

	if (
		(rx_len < 2) ||
		(pad->prefix != 0x5a) ||
		(pad->type != PAD_ID_CONFIG_MODE)
	) {
		//printf("no, pad is digital-only (len = %d)\n", rx_len);

		pad_config_attempt[port]++;
		return;
	}

	//printf("yes, forcing analog mode (len = %d)\n", rx_len);

	// Issue further commands to force analog mode on, unlock rumble (not used
	// in this example) and enable longer responses containing button pressure
	// readings.
	// TODO: find out if passing 0x03 instead of 0x02 in PAD_CMD_SET_ANALOG
	// locks the analog button, as emulated by DuckStation...
	// https://gist.github.com/scanlime/5042071
	send_pad_cmd(port, PAD_CMD_CONFIG_MODE,     0x01, 0x00, 0);
	send_pad_cmd(port, PAD_CMD_SET_ANALOG,      0x01, 0x02, 0);
	send_pad_cmd(port, PAD_CMD_INIT_PRESSURE,   0x00, 0x00, 0); // Ignored by DualShock 1
	send_pad_cmd(port, PAD_CMD_REQUEST_CONFIG,  0x00, 0x01, 0);
	send_pad_cmd(port, PAD_CMD_RESPONSE_CONFIG, 0xff, 0xff, 0); // Ignored by DualShock 1
	send_pad_cmd(port, PAD_CMD_CONFIG_MODE,     0x00, 0x00, 0);
}

// This function is called by the pad timer ISR each time a pad is polled and a
// response (even an invalid/incomplete one) is received.
void poll_cb(uint32_t port, const volatile uint8_t *buff, size_t rx_len) {
	// Copy the response to a persistent buffer so it can be accessed from the
	// main loop and displayed on screen.
	pad_buff_len[port] = rx_len;
	if (rx_len)
		memcpy((void *) pad_buff[port], (void *) buff, rx_len);

	PadResponse *pad = (PadResponse *) buff;

	// If this pad identifies as a digital pad, attempt to put it into analog
	// mode up to 3 times by entering configuration mode. Once the attempt
	// counter exceeds the threshold, it will be treated as digital-only. The
	// attempt counter is reset when the controller is unplugged or stops
	// returning digital pad responses.
	// NOTE: according to nocash docs, there is a hardware bug in DualShock
	// controllers that causes the prefix byte (normally 0x5a) to turn into
	// 0x00 if the analog button is pressed after config commands have been
	// used.
	if (
		rx_len &&
		((pad->prefix == 0x5a) || !(pad->prefix)) &&
		(pad->type == PAD_ID_DIGITAL)
	) {
		if (pad_config_attempt[port] < 3) {
			/*printf(
				"Detecting if pad %d supports config mode: attempt %d... ",
				port + 1,
				pad_config_attempt[port] + 1
			);*/

			// The pad only identifies as CONFIG_MODE after at least another
			// command is sent.
			send_pad_cmd(port, PAD_CMD_CONFIG_MODE, 0x01, 0x00, 0);
			send_pad_cmd(port, PAD_CMD_READ,        0x00, 0x00, &dualshock_init_cb);
		}

	} else {
		//printf("Clearing attempt counter for pad %d\n", port + 1);

		pad_config_attempt[port] = 0;
	}
}

/* Main */

static RenderContext ctx;

int main(int argc, const char* argv[]) {
	init_context(&ctx);
	SPI_Init(&poll_cb);

	uint32_t counter = 0;

	while (1) {
		FntPrint(-1, "COUNTER=%d", counter++);

		for (uint32_t port = 0; port < 2; port++) {
			// TODO.
			if (!pad_buff_len[port]) {
				FntPrint(-1, "\n\nPORT %d: NO DEVICE FOUND\n", port + 1);
				if ((counter % 64) < 32)
					FntPrint(-1, " CONNECT PAD NOW...");

				continue;
			}

			PadResponse *pad = (PadResponse *) pad_buff[port];

			/*if ((pad->prefix != 0x5a) && (pad->type != PAD_ID_ANALOG)) {
				FntPrint(-1, "\n\nPORT %d: INVALID RESPONSE\n", port + 1);
				if ((counter % 64) < 32)
					FntPrint(-1, " CHECK CONNECTION...");

				continue;
			}*/

			FntPrint(
				-1,
				"\n\nPORT %d: %s (TYPE=%d)\n",
				port + 1,
				PAD_TYPEIDS[pad->type],
				pad->type
			);

			// Print a hexdump of the payload returned by the pad.
			for (uint32_t i = 0; i < pad_buff_len[port]; i++)
				FntPrint(
					-1,
					((i - 2) % 8) ? " %02x" : "\n %02x",
					pad_buff[port][i]
				);
		}

		FntFlush(-1);
		display(&ctx);
	}

	return 0;
}
