/*
 * PSn00bSDK SPU audio streaming example
 * (C) 2021 spicyjpeg - MPL licensed
 *
 * This example demonstrates how to play a large multi-channel audio file
 * "manually" by streaming it through the SPU, without having to rely on the CD
 * drive's ability to play audio tracks or XA files.
 *
 * The way this works is by splitting the audio file into a series of ~1 second
 * "chunks", each of which in turn is an array of concatenated buffers holding
 * SPU ADPCM data (one for each channel, so a stereo stream would have 2
 * buffers per chunk). All buffers in a chunk are played simultaneously using
 * multiple SPU channels; each buffer has the loop flag set at the end, so each
 * channel will jump to its loop address (SPU_CHANNELS[n].loop_addr) once the
 * chunk is played.
 *
 * Since the loop point doesn't necessarily have to be within the chunk itself,
 * we can abuse it to "queue" another set of buffers to be played immediately
 * after the currently playing chunk. This allows us to fetch a chunk from the
 * CD, upload it to SPU RAM (2048 bytes at a time to avoid having to keep
 * another large buffer in main RAM) and queue it for playback while a
 * previously buffered chunk is playing in the background. SPU RAM always holds
 * two chunks, one of which is played while the other one is buffered. This is
 * the layout used in this example:
 *
 *          /================================================\
 *          |              /==================\              |
 *          v Loop point   |                  v Loop point   |
 * +-------+----------------+----------------+----------------+----------------+
 * | Dummy | Left buffer 0  | Right buffer 0 | Left buffer 1  | Right buffer 1 |
 * +-------+----------------+----------------+----------------+----------------+
 *          \____________Chunk 0____________/ \____________Chunk 1____________/
 *
 * It's pretty much the same thing as GPU double buffering (aka page flipping),
 * just with chunks instead of framebuffers.
 *
 * We need to know when the chunk we've buffered actually starts playing in
 * order to start buffering the next one. The SPU can be configured to trigger
 * an interrupt whenever a specific address in SPU RAM is read by a channel, so
 * we can just point it to the beginning of the buffered chunk's first buffer.
 * The interrupt callback will then kick off CD reading and adjust the loop/IRQ
 * addresses to the ones of the chunk that is going to be buffered next.
 *
 * Chunks are read from a STREAM.BIN file which is just a series of sector
 * aligned chunks, arranged as follows:
 *
 *  +--Sector--+--Sector--+--Sector--+--Sector--+--Sector--+--Sector--+----
 *  | +--------------------------+--------------------------+         |
 *  | | Left channel data        | Right channel data       | Padding | ...
 *  | +--------------------------+--------------------------+         |
 *  +----------+----------+----------+----------+----------+----------+----
 *    \________________________Chunk________________________/
 *
 * Such file isn't provided as PSn00bSDK doesn't yet have a tool for audio
 * transcoding. A Python script is included to generate STREAM.BIN from one or
 * more SPU ADPCM (.VAG) files, one for each channel (the .VAG format only
 * supports mono).
 *
 * Of course SPU streaming isn't the only way to play music, as the CD drive
 * can play CD-DA tracks and XA files natively with zero CPU overhead. However
 * streaming has a number of advantages over CD audio or XA:
 *
 * - Any sample rate up to 44.1 kHz can be used. The sample rate can also be
 *   changed on-the-fly to play the stream at different speeds and pitches (as
 *   long as the CD drive can keep up of course), or even interpolated for
 *   effects like tape stops or DJ scratches.
 * - Manual streaming is not limited to mono or stereo but can be expanded to
 *   as many channels as needed, only limited by the amount of SPU RAM required
 *   for chunks and CD bandwidth. Having more than 2 channels can be useful for
 *   e.g. crossfading between tracks (not possible with XA) or controlling
 *   volume and panning of each individual instrument.
 * - Depending on how streaming/interleaving is implemented it is possible to
 *   have 500-1000ms idle periods during which the CD drive isn't buffering the
 *   stream, that can be used to read small amounts of other data without ever
 *   interrupting playback. This is different from XA-style interleaving as the
 *   drive is free to seek to *any* region of the disc during these periods
 *   (it must seek back to the stream's next chunk afterwards though).
 * - Thanks to the idle periods it is possible to seek back to the beginning of
 *   the stream and preload the first chunk before the end is reached, allowing
 *   the track to be looped seamlessly without having to resort to tricks like
 *   filler samples.
 * - Unlike XA, SPU streaming can be used on some PS1-based arcade boards such
 *   as the Konami System 573. These systems usually use IDE/SCSI CD drives or
 *   flash memory, neither of which supports XA playback.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxspu.h>
#include <psxcd.h>

// To maximize STREAM.BIN packing efficiency and get rid of padding between
// chunks, buffer size should be a multiple of sector size (2048 bytes). Buffer
// size can be increased to get more idle time between CD reads, however it is
// usually best to keep it to 1-2 seconds as SPU RAM is only 512 KB.
#define SAMPLE_RATE		0x1000	// 44100 Hz
#define BUFFER_SIZE		26624	// (26624 / 16 * 28) / 44100 = 1.05 seconds

#define NUM_CHANNELS	2
#define CHANNEL_MASK	0x03

/* Register definitions */

// For some reason SpuVoiceRaw doesn't actually match the layout of SPU
// registers, so here we go.
typedef struct {
	uint16_t	vol_left;
	uint16_t	vol_right;
	uint16_t	freq;
	uint16_t	addr;
	uint32_t	adsr_param;
	uint16_t	_reserved;
	uint16_t	loop_addr;
} SPUChannel;

#define SPU_CTRL		*((volatile uint16_t *) 0x1f801daa)
#define SPU_IRQ_ADDR	*((volatile uint16_t *) 0x1f801da4)
#define SPU_KEY_ON		*((volatile uint32_t *) 0x1f801d88)
#define SPU_KEY_OFF		*((volatile uint32_t *) 0x1f801d8c)

// SPU RAM is addressed in 8-byte units, using 16-bit pointers.
#define SPU_CHANNELS	((volatile SPUChannel *) 0x1f801c00)
#define SPU_RAM_ADDR(x)	((uint16_t) (((uint32_t) (x)) >> 3))

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

/* Stream interrupt handlers */

// This is a silent looping sample used to keep unused SPU channels busy,
// preventing them from accidentally triggering the SPU RAM interrupt and
// throwing off the timing (all channels are always reading sample data, even
// when "stopped"). It is 64 bytes as that is the minimum size for SPU DMA
// transfers, however only the first 16 bytes are kept. The rest is going to be
// overwritten by chunks.
// https://problemkaputt.de/psx-spx.htm#spuinterrupt
const uint8_t SPU_DUMMY_BLOCK[] = {
	0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// The first 4 KB of SPU RAM are reserved for capture buffers, so we have to
// place stream buffers after those. Sony's SPU library additionally places a
// dummy sample at 0x1000; we are going to do the same with the block above.
#define DUMMY_BLOCK_ADDR	0x1000
#define BUFFER_START_ADDR	0x1010
#define CHUNK_SIZE			(BUFFER_SIZE * NUM_CHANNELS)

typedef struct {
	uint32_t lba;
	uint32_t length;
	uint32_t pos;

	uint32_t spu_addr;
	uint32_t spu_pos;
	uint32_t db_active;
} StreamContext;

static volatile StreamContext str_ctx;

// This buffer is used by cd_event_handler() as a temporary area for sectors
// read from the CD and uploaded to SPU RAM. Due to DMA limitations it can't be
// allocated on the stack (especially not in the interrupt callbacks' stack,
// whose size is very limited).
static uint8_t sector_buffer[2048];

void spu_irq_handler(void) {
	// Acknowledge the interrupt to ensure it can be triggered again. The only
	// way to do this is actually to disable the interrupt entirely; we'll
	// enable it again once the buffer is ready.
	SPU_CTRL &= 0xffbf;

	str_ctx.db_active ^= 1;
	str_ctx.spu_pos    = 0;

	// Align the sector counter to the size of a chunk (to prevent glitches
	// after seeking) and reset it if it exceeds the stream's length.
	str_ctx.pos %= str_ctx.length;
	str_ctx.pos -= str_ctx.pos % ((CHUNK_SIZE + 2047) / 2048);

	// Configure to SPU to trigger an IRQ once the buffer that is going to be
	// filled now starts playing (so the next buffer can be loaded) and
	// override both channels' loop addresses to make them "jump" to the new
	// buffer rather than actually looping when they encounter the loop flag at
	// the end of the currently playing buffer.
	str_ctx.spu_addr = BUFFER_START_ADDR + CHUNK_SIZE * str_ctx.db_active;
	SPU_IRQ_ADDR     = SPU_RAM_ADDR(str_ctx.spu_addr);

	for (uint32_t i = 0; i < NUM_CHANNELS; i++)
		SPU_CHANNELS[i].loop_addr = SPU_RAM_ADDR(str_ctx.spu_addr + BUFFER_SIZE * i);

	// Start loading the next chunk. cd_event_handler() will be called
	// repeatedly for each sector until the entire chunk is read.
	CdlLOC pos;
	CdIntToPos(str_ctx.lba + str_ctx.pos, &pos);
	CdControlF(CdlReadN, &pos);
}

void cd_event_handler(int32_t event, uint8_t *payload) {
	// Ignore all events other than a sector being ready.
	// TODO: read errors should be handled properly
	if (event != CdlDataReady)
		return;

	// Fetch the sector that has been read from the drive.
	CdGetSector(sector_buffer, 512);
	str_ctx.pos++;

	// Set loop flags to make sure the buffer will loop (actually jump to the
	// other buffer, as we're overriding loop addresses) at the end.
	// NOTE: this isn't actually necessary here as the stream converter script
	// already sets these flags in the file.
	/*for (uint32_t i = 0; i < NUM_CHANNELS; i++) {
		if (
			str_ctx.spu_pos >= (BUFFER_SIZE * i - 2048) &&
			str_ctx.spu_pos <  (BUFFER_SIZE * i)
		)
			sector[(BUFFER_SIZE * i - str_ctx.spu_pos) - 15] = 0x03;
	}*/

	// Copy the sector to SPU RAM, appending it to the buffer that is not
	// playing currently. As the left and right buffers are adjacent, we can
	// just treat the chunk as a single blob of data and copy it as-is; we only
	// have to trim the padding at the end (if any) to avoid overwriting other
	// data in SPU RAM.
	uint32_t length = CHUNK_SIZE - str_ctx.spu_pos;
	if (length > 2048)
		length = 2048;

	SpuSetTransferStartAddr(str_ctx.spu_addr + str_ctx.spu_pos);
	SpuWrite(sector_buffer, length);
	str_ctx.spu_pos += length;

	// If the buffer has been filled completely, stop reading and re-enable the
	// SPU IRQ.
	// TODO TODO: preload first sector
	if (str_ctx.spu_pos >= CHUNK_SIZE) {
		CdControlF(CdlPause, 0);
		SPU_CTRL |= 0x0040;
	}
}

/* Stream helpers */

void init_spu_channels(void) {
	// Upload the dummy block to the SPU and play it on all channels, locking
	// them up and stopping them from messing with the SPU interrupt.
	// TODO: is this really necessary? (needs testing on real hardware)
	SpuSetTransferStartAddr(DUMMY_BLOCK_ADDR);
	SpuWrite(SPU_DUMMY_BLOCK, 64);

	SPU_KEY_OFF = 0x00ffffff;

	for (uint32_t i = 0; i < 24; i++)
		SPU_CHANNELS[i].addr = SPU_RAM_ADDR(DUMMY_BLOCK_ADDR);

	SPU_KEY_ON = 0x00ffffff;
}

void init_stream(CdlFILE *file) {
	EnterCriticalSection();
	InterruptCallback(9, &spu_irq_handler);
	CdReadyCallback(&cd_event_handler);
	ExitCriticalSection();

	// Set the initial LBA of the stream file, which is going to be incremented
	// as the stream is played.
	str_ctx.lba    = CdPosToInt(&(file->pos));
	str_ctx.length = file->size / 2048;
	str_ctx.pos    = 0;

	// Ensure at least one chunk is in SPU RAM by invoking the SPU IRQ handler
	// manually and blocking until the chunk has loaded.
	str_ctx.db_active = 1;
	spu_irq_handler();

	while (str_ctx.spu_pos < CHUNK_SIZE)
		__asm__("nop");
}

void start_stream(void) {
	SPU_KEY_OFF = CHANNEL_MASK;

	for (uint32_t i = 0; i < NUM_CHANNELS; i++) {
		SPU_CHANNELS[i].addr       = SPU_RAM_ADDR(BUFFER_START_ADDR + BUFFER_SIZE * i);
		SPU_CHANNELS[i].freq       = SAMPLE_RATE;
		SPU_CHANNELS[i].adsr_param = 0x1fee80ff; // or 0x9fc080ff, 0xdff18087
	}

	// Unmute the channels and route them for stereo output. You'll want to
	// edit this if you are using more than 2 channels, and/or if you want to
	// provide an option to output mono audio instead of stereo.
	SPU_CHANNELS[0].vol_left  = 0x3fff;
	SPU_CHANNELS[0].vol_right = 0x0000;
	SPU_CHANNELS[1].vol_left  = 0x0000;
	SPU_CHANNELS[1].vol_right = 0x3fff;

	SPU_KEY_ON = CHANNEL_MASK;
	spu_irq_handler();
}

/* Main */

static CONTEXT ctx;

#define SHOW_STATUS(...) { FntPrint(-1, __VA_ARGS__); FntFlush(-1); display(&ctx); }
#define SHOW_ERROR(...)  { SHOW_STATUS(__VA_ARGS__); while (1) __asm__("nop"); }

int main(int argc, const char* argv[]) {
	init_context(&ctx);

	SHOW_STATUS("INITIALIZING\n");
	SpuInit();
	CdInit();
	init_spu_channels();

	SHOW_STATUS("LOCATING STREAM FILE\n");

	CdlFILE file;
	if (!CdSearchFile(&file, "\\STREAM.BIN"))
		SHOW_ERROR("FAILED TO FIND STREAM.BIN\n");

	SHOW_STATUS("BUFFERING STREAM\n");
	init_stream(&file);
	start_stream();

	// Set up controller polling.
	uint8_t pad_buff[2][34];
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);

	uint16_t sample_rate  = SAMPLE_RATE;
	uint16_t last_buttons = 0xffff;

	while (1) {
		FntPrint(-1, "PLAYING SPU STREAM\n");
		if (str_ctx.spu_pos >= CHUNK_SIZE)
			FntPrint(-1, "STATUS: IDLE\n\n");
		else if (!str_ctx.spu_pos)
			FntPrint(-1, "STATUS: SEEKING\n\n");
		else
			FntPrint(-1, "STATUS: BUFFERING\n\n");

		FntPrint(-1, "POSITION=%5d/%5d\n",  str_ctx.pos, str_ctx.length);
		FntPrint(-1, "BUFFERED=%5d/%5d\n",  str_ctx.spu_pos, CHUNK_SIZE);
		FntPrint(-1, "SMP RATE=%5d HZ\n\n", (sample_rate * 44100) >> 12);

		FntPrint(-1, "[LEFT/RIGHT] SEEK\n");
		FntPrint(-1, "[O]          RESET POSITION\n");
		FntPrint(-1, "[UP/DOWN]    CHANGE SAMPLE RATE\n");
		FntPrint(-1, "[X]          RESET SAMPLE RATE\n");

		FntFlush(-1);
		display(&ctx);

		// Check if a compatible controller is connected and handle button
		// presses.
		PADTYPE *pad = (PADTYPE *) pad_buff[0];
		if (pad->stat)
			continue;
		if ((pad->type != 4) && (pad->type != 5) && (pad->type != 7))
			continue;

		// Seeking by an arbitrary number of sectors isn't a problem as
		// spu_irq_handler() always realigns the counter.
		if (!(pad->btn & PAD_LEFT))
			str_ctx.pos -= 16;
		if (!(pad->btn & PAD_RIGHT))
			str_ctx.pos += 16;
		if ((last_buttons & PAD_CIRCLE) && !(pad->btn & PAD_CIRCLE))
			str_ctx.pos = 0;

		if (!(pad->btn & PAD_DOWN) && (sample_rate > 0x400))
			sample_rate -= 0x40;
		if (!(pad->btn & PAD_UP) && (sample_rate < 0x2000))
			sample_rate += 0x40;
		if ((last_buttons & PAD_CROSS) && !(pad->btn & PAD_CROSS))
			sample_rate = SAMPLE_RATE;

		// Only set the sample rate registers if necessary.
		if (pad->btn != 0xffff) {
			for (uint32_t i = 0; i < NUM_CHANNELS; i++)
				SPU_CHANNELS[i].freq = sample_rate;
		}

		last_buttons = pad->btn;
	}

	return 0;
}
