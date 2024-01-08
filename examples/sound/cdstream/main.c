/*
 * PSn00bSDK SPU CD-ROM streaming example
 * (C) 2022-2023 spicyjpeg - MPL licensed
 *
 * This is an extended version of the sound/spustream example demonstrating
 * playback of a large multi-channel audio file from the CD-ROM using the SPU,
 * without having to rely on the CD drive's own ability to play CD-DA or XA
 * tracks.
 *
 * A ring buffer takes the place of the stream_data array from the spustream
 * example. This buffer is filled from the CD-ROM by the main thread and drained
 * by the SPU IRQ handler, which pulls a single chunk at a time out of it and
 * transfers it to SPU RAM for playback. The feed_stream() function handles
 * fetching chunks, which are read once again from an interleaved .VAG file laid
 * out on the disc as follows:
 *
 *  +--Sector--+--Sector--+--Sector--+--Sector--+--Sector--+--Sector--+----
 *  |          | +--------------------+---------------------+         |
 *  |   .VAG   | | Left channel data  | Right channel data  | Padding | ...
 *  |  header  | +--------------------+---------------------+         |
 *  +----------+----------+----------+----------+----------+----------+----
 *               \__________________Chunk___________________/
 *
 * Note that the ring buffer must be large enough to give the drive enough time
 * to seek from one chunk to another. A larger buffer will take up more main RAM
 * but will not influence SPU RAM usage, which depends only on the chunk size
 * (interleave) and channel count of the .VAG file. Generally, interleave values
 * in the 2048-4096 byte range work well (the interleaving script in the
 * spustream directory uses 4096 bytes by default).
 *
 * Implementing SPU streaming might seem pointless, but it actually has a number
 * of advantages over CD-DA or XA:
 *
 * - Any sample rate up to 44.1 kHz can be used. The sample rate can also be
 *   changed on-the-fly to play the stream at different speeds and pitches (as
 *   long as the CD drive can keep up), or even interpolated for effects like
 *   tape stops.
 * - Manual streaming is not limited to mono or stereo but can be expanded to as
 *   many channels as needed, only limited by the amount of SPU RAM required for
 *   chunks and CD bandwidth. Having more than 2 channels can be useful for e.g.
 *   smoothly crossfading between tracks (not possible with XA) or controlling
 *   volume and panning of each instrument separately.
 * - XA playback tends to skip on consoles with a worn out drive, as XA sectors
 *   cannot have any error correction data. SPU streaming is not subject to this
 *   limitation since sectors are read and processed in software.
 * - Depending on how streaming/interleaving is implemented it is possible to
 *   have 500-1000ms idle periods during which the CD drive isn't buffering the
 *   stream, that can be used to read small amounts of other data without ever
 *   interrupting playback. This is different from XA-style interleaving as the
 *   drive is free to seek to *any* region of the disc during these periods (it
 *   must seek back to the stream's next chunk afterwards though).
 * - It is also possible to seek back to the beginning of the stream and load
 *   the first chunk before the end is reached, allowing for seamless looping
 *   without having to resort to tricks like separate filler samples.
 * - Finally, SPU streaming can be used on some PS1-based arcade boards that use
 *   IDE/SCSI drives or flash memory for storage and thus lack support for XA or
 *   CD-DA playback.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxspu.h>
#include <psxcd.h>
#include <hwregs_c.h>

#include "stream.h"

// Size of the ring buffer in main RAM in bytes.
#define RAM_BUFFER_SIZE 0x18000

// Minimum number of sectors that will be read from the CD-ROM at once. Higher
// values will improve efficiency at the cost of requiring a larger buffer in
// order to prevent underruns and glitches in the audio output.
#define REFILL_THRESHOLD 24

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
	uint32_t magic;			// 0x69474156 ("VAGi") for interleaved files
	uint32_t version;
	uint32_t interleave;	// Little-endian, size of each channel buffer
	uint32_t size;			// Big-endian, in bytes
	uint32_t sample_rate;	// Big-endian, in Hertz
	uint16_t _reserved[5];
	uint16_t channels;		// Little-endian, channel count (stereo if 0)
	char     name[16];
} VAG_Header;

/* Helper functions */

#define DUMMY_BLOCK_ADDR   0x1000
#define STREAM_BUFFER_ADDR 0x1010

typedef struct {
	int start_lba, stream_length, sample_rate;

	volatile int    next_sector;
	volatile size_t refill_length;
} StreamReadContext;

static Stream_Context    stream_ctx;
static StreamReadContext read_ctx;

void cd_read_handler(CdlIntrResult event, uint8_t *payload) {
	// Mark the data that has just been read as valid.
	if (event != CdlDiskError)
		Stream_Feed(&stream_ctx, read_ctx.refill_length * 2048);
}

// This isn't actually required for this example, however it is necessary if the
// stream buffers are going to be allocated into a region of SPU RAM that was
// previously used (to make sure the IRQ is not going to be triggered by any
// inactive channels).
void reset_spu_channels(void) {
	SpuSetKey(0, 0x00ffffff);

	for (int i = 0; i < 24; i++) {
		SPU_CH_ADDR(i) = getSPUAddr(DUMMY_BLOCK_ADDR);
		SPU_CH_FREQ(i) = 0x1000;
	}

	SpuSetKey(1, 0x00ffffff);
}

bool feed_stream(void) {
	// Do nothing if the drive is already busy reading a chunk.
	if (CdReadSync(1, 0) > 0)
		return true;

	// To improve efficiency, do not start refilling immediately but wait until
	// there is enough space in the buffer (see REFILL_THRESHOLD).
	if (Stream_GetRefillLength(&stream_ctx) < (REFILL_THRESHOLD * 2048))
		return false;

	uint8_t *ptr;
	size_t  refill_length = Stream_GetFeedPtr(&stream_ctx, &ptr) / 2048;

	// Figure out how much data can be read in one shot. If the end of the file
	// would be reached before the buffer is full, split the read into two
	// separate reads.
	int next_sector = read_ctx.next_sector;
	int max_length  = read_ctx.stream_length - next_sector;

	while (max_length <= 0) {
		next_sector -= read_ctx.stream_length;
		max_length  += read_ctx.stream_length;
	}

	if (refill_length > max_length)
		refill_length = max_length;

	// Start reading the next chunk from the CD-ROM into the buffer.
	CdlLOC pos;

	CdIntToPos(read_ctx.start_lba + next_sector, &pos);
	CdControl(CdlSetloc, &pos, 0);
	CdReadCallback(&cd_read_handler);
	CdRead(refill_length, (uint32_t *) ptr, CdlModeSpeed);

	read_ctx.next_sector   = next_sector + refill_length;
	read_ctx.refill_length = refill_length;

	return true;
}

void setup_stream(const CdlLOC *pos) {
	// Read the .VAG header from the first sector of the file.
	uint32_t header[512];
	CdControl(CdlSetloc, pos, 0);

	CdReadCallback(0);
	CdRead(1, header, CdlModeSpeed);
	CdReadSync(0, 0);

	VAG_Header    *vag = (VAG_Header *) header;
	Stream_Config config;

	int num_channels = vag->channels ? vag->channels : 2;
	int num_chunks   =
		(__builtin_bswap32(vag->size) + vag->interleave - 1) / vag->interleave;

	__builtin_memset(&config, 0, sizeof(Stream_Config));

	config.spu_address = STREAM_BUFFER_ADDR;
	config.interleave  = vag->interleave;
	config.buffer_size = RAM_BUFFER_SIZE;
	config.sample_rate = __builtin_bswap32(vag->sample_rate);

	// Use the first N channels of the SPU and pan them left/right in pairs
	// (this assumes the stream contains one or more stereo tracks).
	for (int ch = 0; ch < num_channels; ch++) {
		config.channel_mask = (config.channel_mask << 1) | 1;

		SPU_CH_VOL_L(ch) = (ch % 2) ? 0x0000 : 0x3fff;
		SPU_CH_VOL_R(ch) = (ch % 2) ? 0x3fff : 0x0000;
	}

	Stream_Init(&stream_ctx, &config);

	read_ctx.start_lba     = CdPosToInt(pos) + 1;
	read_ctx.stream_length =
		(num_channels * num_chunks * vag->interleave + 2047) / 2048;
	read_ctx.sample_rate   = config.sample_rate;
	read_ctx.next_sector   = 0;
	read_ctx.refill_length = 0;

	// Ensure the buffer is full before starting playback.
	while (feed_stream())
		__asm__ volatile("");
}

/* Main */

static RenderContext ctx;

#define SHOW_STATUS(...) { FntPrint(-1, __VA_ARGS__); FntFlush(-1); display(&ctx); }
#define SHOW_ERROR(...)  { SHOW_STATUS(__VA_ARGS__); while (1) __asm__("nop"); }

int main(int argc, const char* argv[]) {
	init_context(&ctx);
	SpuInit();
	CdInit();
	reset_spu_channels();
	SHOW_STATUS("");

	// Set up controller polling.
	uint8_t pad_buff[2][34];
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);

	CdlFILE file;
	SHOW_STATUS("OPENING STREAM FILE\n");
	if (!CdSearchFile(&file, "\\STREAM.VAG"))
		SHOW_ERROR("FAILED TO FIND STREAM.VAG\n");

	SHOW_STATUS("BUFFERING STREAM\n");
	setup_stream(&file.pos);
	Stream_Start(&stream_ctx, false);

	int sectors_per_chunk = (stream_ctx.chunk_size + 2047) / 2048;

	bool paused      = false;
	int  sample_rate = read_ctx.sample_rate;

	uint16_t last_buttons = 0xffff;

	while (1) {
		bool buffering = feed_stream();

		FntPrint(-1, "PLAYING SPU STREAM\n\n");
		FntPrint(-1, "BUFFER: %d\n", stream_ctx.db_active);
		FntPrint(-1, "STATUS: %s\n\n", buffering ? "READING" : "IDLE");

		FntPrint(-1, "BUFFERED: %d/%d\n", stream_ctx.buffer.length, stream_ctx.config.buffer_size);
		FntPrint(-1, "POSITION: %d/%d\n",  read_ctx.next_sector, read_ctx.stream_length);
		FntPrint(-1, "SMP RATE: %5d HZ\n\n", sample_rate);

		FntPrint(-1, "[START]      %s\n", paused ? "RESUME" : "PAUSE");
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
		if (
			(pad->type != PAD_ID_DIGITAL) &&
			(pad->type != PAD_ID_ANALOG_STICK) &&
			(pad->type != PAD_ID_ANALOG)
		)
			continue;

		if ((last_buttons & PAD_START) && !(pad->btn & PAD_START)) {
			paused ^= 1;
			if (paused)
				Stream_Stop();
			else
				Stream_Start(&stream_ctx, true);
		}

		// Note that seeking will only work correctly with .VAG files whose
		// interleave (chunk size) is a multiple of 2048.
		if (!(pad->btn & PAD_LEFT) && (read_ctx.next_sector > 0))
			read_ctx.next_sector -= sectors_per_chunk;
		if (!(pad->btn & PAD_RIGHT))
			read_ctx.next_sector += sectors_per_chunk;
		if ((last_buttons & PAD_CIRCLE) && !(pad->btn & PAD_CIRCLE))
			read_ctx.next_sector = 0;

		if (!(pad->btn & PAD_DOWN) && (sample_rate > 11000)) {
			sample_rate -= 100;
			Stream_SetSampleRate(&stream_ctx, sample_rate);
		}
		if (!(pad->btn & PAD_UP) && (sample_rate < 88200)) {
			sample_rate += 100;
			Stream_SetSampleRate(&stream_ctx, sample_rate);
		}
		if ((last_buttons & PAD_CROSS) && !(pad->btn & PAD_CROSS)) {
			sample_rate = read_ctx.sample_rate;
			Stream_SetSampleRate(&stream_ctx, sample_rate);
		}

		last_buttons = pad->btn;
	}

	return 0;
}
