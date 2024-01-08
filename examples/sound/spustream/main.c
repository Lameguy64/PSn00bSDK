/*
 * PSn00bSDK SPU .VAG streaming example
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * This example shows how to play arbitrarily long sounds, which normally would
 * not fit into SPU RAM in their entirety, by streaming them to the SPU from
 * main RAM. In this example audio data is streamed from an in-memory file,
 * however the code can easily be modified to stream from the CD instead (see
 * the cdstream example).
 *
 * The way SPU streaming works is by splitting the audio data into a series of
 * small "chunks", each of which in turn is an array of concatenated buffers
 * holding SPU ADPCM data (one for each channel, so a stereo stream would have
 * 2 buffers per chunk). All buffers in a chunk are played simultaneously using
 * multiple SPU channels; each buffer has the loop flag set at the end, so the
 * SPU will jump to the loop point set in the SPU_CH_LOOP_ADDR registers after
 * the chunk is played.
 *
 * As the loop point doesn't necessarily have to be within the chunk itself, it
 * can be used to "queue" another chunk to be played immediately after the
 * current one. This allows for double buffering: two chunks are always kept in
 * SPU RAM and one is overwritten with a new chunk while the other is playing.
 * Chunks are laid out in SPU RAM as follows:
 *
 *           ________________________________________________
 *          /               __________________               \
 *          |              /                  \              |
 *          v Loop point   | Loop flag        v Loop point   | Loop flag
 * +-------+----------------+----------------+----------------+----------------+
 * | Dummy | Left buffer 0  | Right buffer 0 | Left buffer 1  | Right buffer 1 |
 * +-------+----------------+----------------+----------------+----------------+
 *          \____________Chunk 0____________/ \____________Chunk 1____________/
 *
 * In order to keep streaming continuously we need to know when each chunk
 * actually starts playing. The SPU can be configured to trigger an interrupt
 * whenever a specific address in SPU RAM is read by a channel, so we can just
 * point it to the beginning of the buffered chunk's first buffer and wait
 * until the IRQ is fired before loading the next chunk.
 *
 * Chunks are read from a special type of .VAG file which has been interleaved
 * ahead-of-time and already contains the loop flags required to make streaming
 * work. A Python script is provided to generate such file from one or more
 * mono .VAG files.
 */

#include <stdint.h>
#include <stddef.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxspu.h>
#include <hwregs_c.h>

extern const uint8_t stream_data[];

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
	uint16_t channels;		// Little-endian, if 0 the file is mono
	char     name[16];
} VAG_Header;

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

typedef struct {
	const uint8_t *data;
	int buffer_size, num_chunks, sample_rate, channels;

	volatile int    next_chunk, spu_addr;
	volatile int8_t db_active, buffering;
} StreamContext;

static StreamContext stream_ctx;

void spu_irq_handler(void) {
	// Acknowledge the interrupt to ensure it can be triggered again. The only
	// way to do this is actually to disable the interrupt entirely; we'll
	// enable it again once the chunk is ready.
	SPU_CTRL &= ~(1 << 6);

	int chunk_size = stream_ctx.buffer_size * stream_ctx.channels;
	int chunk      = (stream_ctx.next_chunk + 1) % (uint32_t) stream_ctx.num_chunks;

	stream_ctx.db_active ^= 1;
	stream_ctx.buffering  = 1;
	stream_ctx.next_chunk = chunk;

	// Configure to SPU to trigger an IRQ once the chunk that is going to be
	// filled now starts playing (so the next buffer can be loaded) and
	// override both channels' loop addresses to make them "jump" to the new
	// buffers, rather than actually looping when they encounter the loop flag
	// at the end of the currently playing buffers.
	int addr = BUFFER_START_ADDR + (stream_ctx.db_active ? chunk_size : 0);
	stream_ctx.spu_addr = addr;

	SPU_IRQ_ADDR = getSPUAddr(addr);
	for (int i = 0; i < stream_ctx.channels; i++)
		SPU_CH_LOOP_ADDR(i) = getSPUAddr(addr + stream_ctx.buffer_size * i);

	// Start uploading the next chunk to the SPU.
	SpuSetTransferStartAddr(addr);
	SpuWrite((const uint32_t *) &stream_ctx.data[chunk * chunk_size], chunk_size);
}

void spu_dma_handler(void) {
	// Re-enable the SPU IRQ once the new chunk has been fully uploaded.
	SPU_CTRL |= 1 << 6;

	stream_ctx.buffering = 0;
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

void init_stream(const VAG_Header *vag) {
	EnterCriticalSection();
	InterruptCallback(IRQ_SPU, &spu_irq_handler);
	DMACallback(DMA_SPU, &spu_dma_handler);
	ExitCriticalSection();

	int buf_size = vag->interleave;

	stream_ctx.data        = &((const uint8_t *) vag)[2048];
	stream_ctx.buffer_size = buf_size;
	stream_ctx.num_chunks  = (__builtin_bswap32(vag->size) + buf_size - 1) / buf_size;
	stream_ctx.sample_rate = __builtin_bswap32(vag->sample_rate);
	stream_ctx.channels    = vag->channels ? vag->channels : 1;

	stream_ctx.db_active  =  1;
	stream_ctx.next_chunk = -1;

	// Ensure at least one chunk is in SPU RAM by invoking the IRQ handler
	// manually and blocking until the chunk has loaded.
	spu_irq_handler();
	while (stream_ctx.buffering)
		__asm__ volatile("");
}

void start_stream(void) {
	int bits = 0x00ffffff >> (24 - stream_ctx.channels);

	// Disable the IRQ as we're going to call spu_irq_handler() manually (due
	// to finicky SPU timings).
	SPU_CTRL &= ~(1 << 6);

	for (int i = 0; i < stream_ctx.channels; i++) {
		SPU_CH_ADDR(i)  = getSPUAddr(stream_ctx.spu_addr + stream_ctx.buffer_size * i);
		SPU_CH_FREQ(i)  = getSPUSampleRate(stream_ctx.sample_rate);
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

	SpuSetKey(1, bits);
	spu_irq_handler();
}

// This is basically a variant of reset_spu_channels() that only resets the
// channels used to play the stream, to (again) prevent them from triggering
// the SPU IRQ while the stream is paused.
void stop_stream(void) {
	int bits = 0x00ffffff >> (24 - stream_ctx.channels);

	SpuSetKey(0, bits);

	for (int i = 0; i < stream_ctx.channels; i++)
		SPU_CH_ADDR(i) = getSPUAddr(DUMMY_BLOCK_ADDR);

	SpuSetKey(1, bits);
}

/* Main */

static RenderContext ctx;

int main(int argc, const char* argv[]) {
	init_context(&ctx);
	SpuInit();
	reset_spu_channels();

	// Set up controller polling.
	uint8_t pad_buff[2][34];
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);

	init_stream((const VAG_Header *) stream_data);
	start_stream();

	int paused = 0, sample_rate = getSPUSampleRate(stream_ctx.sample_rate);

	uint16_t last_buttons = 0xffff;

	while (1) {
		FntPrint(-1, "PLAYING SPU STREAM\n\n");
		FntPrint(-1, "BUFFER: %d\n", stream_ctx.db_active);
		FntPrint(-1, "STATUS: %s\n\n", stream_ctx.buffering ? "BUFFERING" : "IDLE");

		FntPrint(-1, "POSITION: %d/%d\n",  stream_ctx.next_chunk, stream_ctx.num_chunks);
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
			stream_ctx.next_chunk--;
		if (!(pad->btn & PAD_RIGHT))
			stream_ctx.next_chunk++;
		if ((last_buttons & PAD_CIRCLE) && !(pad->btn & PAD_CIRCLE))
			stream_ctx.next_chunk = -1;

		if (!(pad->btn & PAD_DOWN) && (sample_rate > 0x400))
			sample_rate -= 0x40;
		if (!(pad->btn & PAD_UP) && (sample_rate < 0x2000))
			sample_rate += 0x40;
		if ((last_buttons & PAD_CROSS) && !(pad->btn & PAD_CROSS))
			sample_rate = getSPUSampleRate(stream_ctx.sample_rate);

		// Only set the sample rate registers if necessary.
		if (pad->btn != 0xffff) {
			for (int i = 0; i < stream_ctx.channels; i++)
				SPU_CH_FREQ(i) = sample_rate;
		}

		last_buttons = pad->btn;
	}

	return 0;
}
