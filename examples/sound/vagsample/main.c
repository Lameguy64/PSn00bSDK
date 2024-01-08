/*
 * PSn00bSDK SPU .VAG playback example
 * (C) 2021-2022 Lameguy64, spicyjpeg - MPL licensed
 *
 * This example demonstrates basic usage of the SPU. Two mono audio samples (in
 * the standard PS1 .VAG format) are uploaded from main memory to SPU RAM and
 * played on one of the 24 channels by manipulating the SPU's registers. The
 * .VAG header is parsed to obtain the sample rate and data size, while the
 * actual audio data does not need any processing as it is already encoded in
 * the ADPCM format expected by the SPU.
 *
 * Note that PSn00bSDK does not yet provide any tool for SPU ADPCM encoding, so
 * you will have to use an external program to convert your samples to .VAG.
 *
 * The included sound clips are by HighTreason610 (proyt.vag) and Lameguy64
 * (3dfx.vag) respectively.
 */

#include <stdint.h>
#include <psxgpu.h>
#include <psxapi.h>
#include <psxpad.h>
#include <psxspu.h>
#include <hwregs_c.h>

extern const uint8_t proyt[];
extern const uint8_t tdfx[];

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

/* .VAG header structure */

typedef struct {
	uint32_t magic;			// 0x70474156 ("VAGp") for mono files
	uint32_t version;
	uint32_t interleave;	// Unused in mono files
	uint32_t size;			// Big-endian, in bytes
	uint32_t sample_rate;	// Big-endian, in Hertz
	uint16_t _reserved[5];
	uint16_t channels;		// Unused in mono files
	char     name[16];
} VAG_Header;

/* Helper functions */

// The first 4 KB of SPU RAM are reserved for capture buffers and psxspu
// additionally uploads a dummy sample (16 bytes) at 0x1000 by default, so the
// samples must be placed after those.
#define ALLOC_START_ADDR 0x1010

static int next_channel     = 0;
static int next_sample_addr = ALLOC_START_ADDR;

int upload_sample(const void *data, int size) {
	// Round the size up to the nearest multiple of 64, as SPU DMA transfers
	// are done in 64-byte blocks.
	int addr = next_sample_addr;
	size     = (size + 63) & ~63;

	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	SpuSetTransferStartAddr(addr);

	SpuWrite((const uint32_t *) data, size);
	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

	next_sample_addr = addr + size;
	return addr;
}

void play_sample(int addr, int sample_rate) {
	int ch = next_channel;

	// Make sure the channel is stopped.
	SpuSetKey(0, 1 << ch);

	// Set the channel's sample rate and start address. Note that the SPU
	// expects the sample rate to be in 4.12 fixed point format (with
	// 1.0 = 44100 Hz) and the address in 8-byte units; psxspu.h provides the
	// getSPUSampleRate() and getSPUAddr() macros to convert values to these
	// units.
	SPU_CH_FREQ(ch) = getSPUSampleRate(sample_rate);
	SPU_CH_ADDR(ch) = getSPUAddr(addr);

	// Set the channel's volume and ADSR parameters (0x80ff and 0x1fee are
	// dummy values that disable the ADSR envelope entirely).
	SPU_CH_VOL_L(ch) = 0x3fff;
	SPU_CH_VOL_R(ch) = 0x3fff;
	SPU_CH_ADSR1(ch) = 0x00ff;
	SPU_CH_ADSR2(ch) = 0x0000;

	// Start the channel.
	SpuSetKey(1, 1 << ch);

	next_channel = (ch + 1) % 24;
}

/* Main */

static RenderContext ctx;

int main(int argc, const char* argv[]) {
	init_context(&ctx);
	SpuInit();

	// Upload the samples to the SPU and parse their headers.
	const VAG_Header *proyt_vag = (const VAG_Header *) proyt;
	const VAG_Header *tdfx_vag  = (const VAG_Header *) tdfx;

	int proyt_addr = upload_sample(proyt_vag + 1, __builtin_bswap32(proyt_vag->size));
	int tdfx_addr  = upload_sample(tdfx_vag + 1, __builtin_bswap32(tdfx_vag->size));
	int proyt_sr   = __builtin_bswap32(proyt_vag->sample_rate);
	int tdfx_sr    = __builtin_bswap32(tdfx_vag->sample_rate);

	// Set up controller polling.
	uint8_t pad_buff[2][34];
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);

	uint16_t last_buttons = 0xffff;

	while (1) {
		FntPrint(-1, "SPU SAMPLE PLAYBACK DEMO\n\n");
		FntPrint(-1, "[X] PLAY FIRST SAMPLE\n");
		FntPrint(-1, "[O] PLAY SECOND SAMPLE\n");

		FntFlush(-1);
		display(&ctx);

		// Check if a compatible controller is connected and handle button
		// presses.
		PADTYPE *pad = (PADTYPE *) pad_buff[0];
		if (pad->stat)
			continue;
		if (
			(pad->type != PAD_ID_DIGITAL) &&
			(pad->type != PAD_ID_ANALOG_STICK) &&
			(pad->type != PAD_ID_ANALOG)
		)
			continue;

		if ((last_buttons & PAD_CROSS) && !(pad->btn & PAD_CROSS))
			play_sample(proyt_addr, proyt_sr);
		if ((last_buttons & PAD_CIRCLE) && !(pad->btn & PAD_CIRCLE))
			play_sample(tdfx_addr, tdfx_sr);

		last_buttons = pad->btn;
	}

	return 0;
}
