/*
 * PSn00bSDK SPU CD-ROM streaming example
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * This is an extended version of the sound/spustream example demonstrating
 * playback of a large multi-channel audio file from the CD using the SPU,
 * without having to rely on the CD drive's own ability to play CD-DA or XA
 * tracks.
 *
 * The main difference from spustream is that the SPU IRQ handler does not
 * upload a chunk from main RAM to SPU RAM immediately, it only sets a flag.
 * The main loop checks if the flag has been set and starts reading the next
 * chunk from the CD into a buffer in RAM asynchronously; the chunk is then
 * uploaded to the SPU and the IRQ is re-enabled.
 *
 * Chunks are read once again from an interleaved .VAG file, laid out on the
 * disc as follows:
 *
 *  +--Sector--+--Sector--+--Sector--+--Sector--+--Sector--+--Sector--+----
 *  |          | +--------------------+---------------------+         |
 *  |   .VAG   | | Left channel data  | Right channel data  | Padding | ...
 *  |  header  | +--------------------+---------------------+         |
 *  +----------+----------+----------+----------+----------+----------+----
 *               \__________________Chunk___________________/
 *
 * Note that chunks have to be large enough to give the drive enough time to
 * seek from one chunk to another. The included .VAG file has been encoded with
 * a chunk size of 0x7000 bytes, however you might want to try smaller sizes to
 * reduce SPU RAM usage. Chunk size can be set by passing the -b option to the
 * .VAG interleaving script included in the spustream directory.
 *
 * Implementing SPU streaming might seem pointless, but it actually has a
 * number of advantages over CD-DA or XA:
 *
 * - Any sample rate up to 44.1 kHz can be used. The sample rate can also be
 *   changed on-the-fly to play the stream at different speeds and pitches (as
 *   long as the CD drive can keep up), or even interpolated for effects like
 *   tape stops.
 * - Manual streaming is not limited to mono or stereo but can be expanded to
 *   as many channels as needed, only limited by the amount of SPU RAM required
 *   for chunks and CD bandwidth. Having more than 2 channels can be useful for
 *   e.g. smoothly crossfading between tracks (not possible with XA) or
 *   controlling volume and panning of each instrument separately.
 * - XA playback tends to skip on consoles with a worn out drive, as XA sectors
 *   cannot have any error correction data. SPU streaming is not subject to
 *   this limitation since sectors are read and processed in software.
 * - Depending on how streaming/interleaving is implemented it is possible to
 *   have 500-1000ms idle periods during which the CD drive isn't buffering the
 *   stream, that can be used to read small amounts of other data without ever
 *   interrupting playback. This is different from XA-style interleaving as the
 *   drive is free to seek to *any* region of the disc during these periods (it
 *   must seek back to the stream's next chunk afterwards though).
 * - It is also possible to seek back to the beginning of the stream and load
 *   the first chunk before the end is reached, allowing for seamless looping
 *   without having to resort to tricks like separate filler samples.
 * - Finally, SPU streaming can be used on some PS1-based arcade boards that
 *   use IDE/SCSI drives or flash memory for storage and thus lack support for
 *   XA or CD-DA playback.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxspu.h>
#include <psxcd.h>
#include <hwregs_c.h>

extern const uint8_t stream_data[];

#define NUM_CHANNELS 2

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
	uint32_t _reserved[3];
	char     name[16];
} VAG_Header;

#define SWAP_ENDIAN(x) ( \
	(((uint32_t) (x) & 0x000000ff) << 24) | \
	(((uint32_t) (x) & 0x0000ff00) <<  8) | \
	(((uint32_t) (x) & 0x00ff0000) >>  8) | \
	(((uint32_t) (x) & 0xff000000) >> 24) \
)

/* Interrupt callbacks */

// The first 4 KB of SPU RAM are reserved for capture buffers and psxspu
// additionally uploads a dummy sample (16 bytes) at 0x1000 by default, so the
// chunks must be placed after those. The dummy sample is going to be used to
// keep unused SPU channels busy, preventing them from accidentally triggering
// the SPU IRQ and throwing off the timing (all channels are always reading
// from SPU RAM, even when "stopped").
// https://problemkaputt.de/psx-spx.htm#spuinterrupt
#define DUMMY_BLOCK_ADDR  0x1000
#define BUFFER_START_ADDR 0x1010

typedef enum {
	STATE_IDLE,
	STATE_DATA_NEEDED,
	STATE_READING,
	STATE_BUFFERING
} StreamState;

typedef struct {
	uint32_t *read_buffer;
	int lba, chunk_secs;
	int buffer_size, num_chunks, sample_rate;

	volatile int    next_chunk, spu_addr;
	volatile int8_t db_active, state;
} StreamContext;

static StreamContext str_ctx;

void spu_irq_handler(void) {
	// Acknowledge the interrupt to ensure it can be triggered again. The only
	// way to do this is actually to disable the interrupt entirely; we'll
	// enable it again once the chunk is ready.
	SPU_CTRL &= 0xffbf;

	int chunk_size = str_ctx.buffer_size * NUM_CHANNELS;
	int chunk      = (str_ctx.next_chunk + 1) % (uint32_t) str_ctx.num_chunks;

	str_ctx.db_active ^= 1;
	str_ctx.state      = STATE_DATA_NEEDED;
	str_ctx.next_chunk = chunk;

	// Configure to SPU to trigger an IRQ once the chunk that is going to be
	// filled now starts playing (so the next buffer can be loaded) and
	// override both channels' loop addresses to make them "jump" to the new
	// buffers, rather than actually looping when they encounter the loop flag
	// at the end of the currently playing buffers.
	int addr = BUFFER_START_ADDR + (str_ctx.db_active ? chunk_size : 0);
	str_ctx.spu_addr = addr;

	SPU_IRQ_ADDR = getSPUAddr(addr);
	for (int i = 0; i < NUM_CHANNELS; i++)
		SPU_CH_LOOP_ADDR(i) = getSPUAddr(addr + str_ctx.buffer_size * i);

	// Note that we can't call CdRead() here as it requires interrupts to be
	// enabled. Instead, feed_stream() (called from the main loop) will check
	// if str_ctx.state is set to STATE_DATA_NEEDED and fetch the next chunk.
}

void cd_read_handler(int event, uint8_t *payload) {
	// Attempt to read the chunk again if an error has occurred, otherwise
	// start uploading it to SPU RAM.
	if (event == CdlDiskError) {
		str_ctx.state = STATE_DATA_NEEDED;
		return;
	}

	SpuSetTransferStartAddr(str_ctx.spu_addr);
	SpuWrite(str_ctx.read_buffer, str_ctx.buffer_size * NUM_CHANNELS);

	str_ctx.state = STATE_BUFFERING;
}

void spu_dma_handler(void) {
	// Re-enable the SPU IRQ once the new chunk has been fully uploaded.
	SPU_CTRL |= 0x0040;

	str_ctx.state = STATE_IDLE;
}

/* Helper functions */

// This isn't actually required for this example, however it is necessary if
// you want to allocate the stream buffers into a region of SPU RAM that was
// previously used (to make sure the IRQ isn't going to be triggered by any
// inactive channels).
void reset_spu_channels(void) {
	SpuSetKey(0, 0x00ffffff);

	for (int i = 0; i < 24; i++) {
		SPU_CH_ADDR(i) = getSPUAddr(DUMMY_BLOCK_ADDR);
		SPU_CH_FREQ(i) = 0x1000;
	}

	SpuSetKey(1, 0x00ffffff);
}

void feed_stream(void) {
	if (str_ctx.state != STATE_DATA_NEEDED)
		return;

	// Start reading the next chunk from the CD.
	int lba = str_ctx.lba + str_ctx.next_chunk * str_ctx.chunk_secs;

	CdlLOC pos;
	CdIntToPos(lba, &pos);
	CdControl(CdlSetloc, &pos, 0);

	CdReadCallback(&cd_read_handler);
	CdRead(str_ctx.chunk_secs, str_ctx.read_buffer, CdlModeSpeed);

	str_ctx.state = STATE_READING;
}

void init_stream(const CdlLOC *pos) {
	EnterCriticalSection();
	InterruptCallback(IRQ_SPU, &spu_irq_handler);
	DMACallback(DMA_SPU, &spu_dma_handler);
	ExitCriticalSection();

	// Read the header. Note that in interleaved .VAG files the first sector.
	uint32_t header[512];
	CdControl(CdlSetloc, pos, 0);

	CdReadCallback(0);
	CdRead(1, header, CdlModeSpeed);
	CdReadSync(0, 0);

	VAG_Header *vag = (VAG_Header *) header;
	int buf_size    = vag->interleave;
	int chunk_secs  = ((buf_size * NUM_CHANNELS) + 2047) / 2048;

	str_ctx.read_buffer = malloc(chunk_secs * 2048);
	str_ctx.lba         = CdPosToInt(pos) + 1;
	str_ctx.chunk_secs  = chunk_secs;
	str_ctx.buffer_size = buf_size;
	str_ctx.num_chunks  = (SWAP_ENDIAN(vag->size) + buf_size - 1) / buf_size;
	str_ctx.sample_rate = SWAP_ENDIAN(vag->sample_rate);

	str_ctx.db_active  =  1;
	str_ctx.next_chunk = -1;

	// Ensure at least one chunk is in SPU RAM by invoking the IRQ handler
	// manually and blocking until the chunk has loaded.
	spu_irq_handler();
	while (str_ctx.state != STATE_IDLE)
		feed_stream();
}

void start_stream(void) {
	int bits = 0x00ffffff >> (24 - NUM_CHANNELS);

	for (int i = 0; i < NUM_CHANNELS; i++) {
		SPU_CH_ADDR(i)  = getSPUAddr(str_ctx.spu_addr + str_ctx.buffer_size * i);
		SPU_CH_FREQ(i)  = getSPUSampleRate(str_ctx.sample_rate);
		SPU_CH_ADSR1(i) = 0x00ff;
		SPU_CH_ADSR2(i) = 0x0000;
	}

	// Unmute the channels and route them for stereo output. You'll want to
	// edit this if you are using more than 2 channels, and/or if you want to
	// provide an option to output mono audio instead of stereo.
	SPU_CH_VOL_L(0) = 0x3fff;
	SPU_CH_VOL_R(0) = 0x0000;
	SPU_CH_VOL_L(1) = 0x0000;
	SPU_CH_VOL_R(1) = 0x3fff;

	spu_irq_handler();
	SpuSetKey(1, bits);
}

// This is basically a variant of reset_spu_channels() that only resets the
// channels used to play the stream, to (again) prevent them from triggering
// the SPU IRQ while the stream is paused.
void stop_stream(void) {
	int bits = 0x00ffffff >> (24 - NUM_CHANNELS);

	SpuSetKey(0, bits);

	for (int i = 0; i < NUM_CHANNELS; i++)
		SPU_CH_ADDR(i) = getSPUAddr(DUMMY_BLOCK_ADDR);

	SpuSetKey(1, bits);
}

/* Main */

static RenderContext ctx;

#define SHOW_STATUS(...) { FntPrint(-1, __VA_ARGS__); FntFlush(-1); display(&ctx); }
#define SHOW_ERROR(...)  { SHOW_STATUS(__VA_ARGS__); while (1) __asm__("nop"); }

static const char *state_strings[] = { "IDLE", "DATA NEEDED", "READING", "BUFFERING" };

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
	init_stream(&file.pos);
	start_stream();

	int paused = 0, sample_rate = getSPUSampleRate(str_ctx.sample_rate);

	uint16_t last_buttons = 0xffff;

	while (1) {
		feed_stream();

		FntPrint(-1, "PLAYING SPU STREAM\n\n");
		FntPrint(-1, "BUFFER: %d\n", str_ctx.db_active);
		FntPrint(-1, "STATUS: %s\n\n", state_strings[str_ctx.state]);

		FntPrint(-1, "POSITION: %d/%d\n",  str_ctx.next_chunk, str_ctx.num_chunks);
		FntPrint(-1, "SMP RATE: %5d HZ\n\n", (sample_rate * 44100) >> 12);

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
				stop_stream();
			else
				start_stream();
		}

		if (!(pad->btn & PAD_LEFT))
			str_ctx.next_chunk--;
		if (!(pad->btn & PAD_RIGHT))
			str_ctx.next_chunk++;
		if ((last_buttons & PAD_CIRCLE) && !(pad->btn & PAD_CIRCLE))
			str_ctx.next_chunk = -1;

		if (!(pad->btn & PAD_DOWN) && (sample_rate > 0x400))
			sample_rate -= 0x40;
		if (!(pad->btn & PAD_UP) && (sample_rate < 0x2000))
			sample_rate += 0x40;
		if ((last_buttons & PAD_CROSS) && !(pad->btn & PAD_CROSS))
			sample_rate = getSPUSampleRate(str_ctx.sample_rate);

		// Only set the sample rate registers if necessary.
		if (pad->btn != 0xffff) {
			for (int i = 0; i < NUM_CHANNELS; i++)
				SPU_CH_FREQ(i) = sample_rate;
		}

		last_buttons = pad->btn;
	}

	return 0;
}
