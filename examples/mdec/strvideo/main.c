/*
 * PSn00bSDK .STR FMV playback example
 * (C) 2022-2023 spicyjpeg - MPL licensed
 *
 * This example demonstrates playback of full-motion video in the standard .STR
 * format, using the MDEC for frame decoding and XA for audio. Decoded frames
 * are transferred directly to the main framebuffer in this example, but could
 * also be output to another VRAM location and used as a background or texture
 * for a 2D or 3D scene.
 *
 * Playing video files requires setting up a fairly complex pipeline, involving
 * several buffers and components working in parallel:
 *
 * - .STR sectors are read continuously from the CD and each frame, usually
 *   spanning multiple sectors, is reassembled (demuxed) into a buffer in
 *   memory. In this example the task is performed by cd_sector_handler(). The
 *   CD drive handles XA-ADPCM sectors automatically, so no CPU intervention is
 *   necessary to play the audio track interleaved with the video.
 * - Once a full frame has been demuxed, the bitstream data is parsed and
 *   decompressed by the CPU (using DecDCTvlc()) to an array of run-length codes
 *   to be fed to the MDEC. This is done in the main loop.
 * - At the same time the last frame decompressed is read from RAM by the MDEC,
 *   which decodes it and outputs one 16-pixel-wide vertical slice at a time.
 * - When a slice is ready, it is uploaded by mdec_dma_handler() to the current
 *   framebuffer in VRAM while the MDEC is decoding the next slice.
 * - A text overlay is drawn on top of the framebuffer using the GPU after the
 *   entire frame has been decoded.
 *
 * Since pretty much all buffers used are going to be read and written at the
 * same time, double buffering is required for all of them. Every part of the
 * pipeline must also run in lockstep with each other to prevent frame
 * corruption, hence several functions and flag variables are used to stall the
 * main loop until a frame is available for decoding and the MDEC is ready.
 * Playback is stopped once the .STR header is no longer present in sectors
 * read.
 *
 * PSn00bSDK's bitstream decoding API supports both version 2 and 3 bitstreams.
 * Encoding your .STR files as v3 may result in slightly higher quality
 * depending on the encoder, but also higher CPU usage during playback compared
 * to the older v2.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxspu.h>
#include <psxcd.h>
#include <psxpress.h>
#include <hwregs_c.h>

// Uncomment to display the video in 24bpp mode. Note that the GPU does not
// support 24bpp rendering, so the text overlay is only enabled in 16bpp mode.
//#define DISP_24BPP

// Uncomment to enable waiting for vertical sync after each frame is decoded.
// This will get rid of screen tearing, but may result in the player failing to
// play .STR files with higher frame rates (see main() for more details and
// possible workarounds).
//#define ENABLE_VSYNC

/* Display/GPU context utilities */

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define BGCOLOR_R 0
#define BGCOLOR_G 0
#define BGCOLOR_B 0

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
	SetDefDispEnv(&(db->disp), 0,           0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw), 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), BGCOLOR_R, BGCOLOR_G, BGCOLOR_B);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	db = &(ctx->db[1]);
	SetDefDispEnv(&(db->disp), 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw), 0,           0, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), BGCOLOR_R, BGCOLOR_G, BGCOLOR_B);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	PutDrawEnv(&(db->draw));
	//PutDispEnv(&(db->disp));

	// Create a text stream at the top of the screen.
	FntLoad(960, 0);
	FntOpen(4, 12, 312, 16, 2, 256);
}

void display(RenderContext *ctx) {
	Framebuffer *db;
	ctx->db_active ^= 1;

	DrawSync(0);
	//VSync(0);

	db = &(ctx->db[ctx->db_active]);
	PutDrawEnv(&(db->draw));
	PutDispEnv(&(db->disp));
	SetDispMask(1);
}

/* CD and MDEC interrupt handlers */

#ifdef DISP_24BPP
#define BLOCK_SIZE 24
#else
#define BLOCK_SIZE 16
#define DRAW_OVERLAY
#endif

#define VRAM_X_COORD(x) ((x) * BLOCK_SIZE / 16)

// All non-audio sectors in .STR files begin with this 32-byte header, which
// contains metadata about the sector and is followed by a chunk of frame
// bitstream data.
// https://problemkaputt.de/psx-spx.htm#cdromfilevideostrstreamingandbspicturecompressionsony
typedef struct {
	uint16_t magic;			// Always 0x0160
	uint16_t type;			// 0x8001 for MDEC
	uint16_t sector_id;		// Chunk number (0 = first chunk of this frame)
	uint16_t sector_count;	// Total number of chunks for this frame
	uint32_t frame_id;		// Frame number
	uint32_t bs_length;		// Total length of this frame in bytes

	uint16_t width, height;
	uint8_t  bs_header[8];
	uint32_t _reserved;
} STR_Header;

typedef struct {
	uint16_t width, height;
	uint32_t bs_data[0x2000];	// Bitstream data read from the disc
	uint32_t mdec_data[0x8000];	// Decompressed data to be fed to the MDEC
} StreamBuffer;

typedef struct {
	StreamBuffer frames[2];
	uint32_t     slices[2][BLOCK_SIZE * SCREEN_YRES / 2];

	int  frame_id, sector_count;
	int  dropped_frames;
	RECT slice_pos;
	int  frame_width;

	volatile int8_t sector_pending, frame_ready;
	volatile int8_t cur_frame, cur_slice;
} StreamContext;

// This structure contains all buffers required for playback as well as the
// current state of the player. Note that it takes up a significant amount of
// RAM, so allocating it on the heap when needed and keeping a pointer to it
// may be a better option.
static StreamContext str_ctx;

// This buffer is used by cd_sector_handler() as a temporary area for sectors
// read from the CD. Due to DMA limitations it can't be allocated on the stack.
static STR_Header sector_header;

void cd_sector_handler(void) {
	StreamBuffer *frame = &str_ctx.frames[str_ctx.cur_frame];

	// Fetch the .STR header of the sector that has been read and make sure it
	// is valid. If not, assume the file has ended and set frame_ready as a
	// signal for the main loop to stop playback.
	CdGetSector(&sector_header, sizeof(STR_Header) / 4);

	if (sector_header.magic != 0x0160) {
		str_ctx.frame_ready = -1;
		return;
	}

	// Ignore any non-MDEC sectors that might be present in the stream.
	if (sector_header.type != 0x8001)
		return;

	// If this sector is actually part of a new frame, validate the sectors
	// that have been read so far and flip the bitstream data buffers. If the
	// frame number is actually lower than the current one, assume the drive
	// has started reading another .STR file and stop playback.
	if ((int) sector_header.frame_id < str_ctx.frame_id) {
		str_ctx.frame_ready = -1;
		return;
	}

	if ((int) sector_header.frame_id > str_ctx.frame_id) {
		// Do not set the ready flag if any sector has been missed.
		if (str_ctx.sector_count)
			str_ctx.dropped_frames++;
		else
			str_ctx.frame_ready = 1;

		str_ctx.frame_id     = sector_header.frame_id;
		str_ctx.sector_count = sector_header.sector_count;
		str_ctx.cur_frame   ^= 1;

		frame = &str_ctx.frames[str_ctx.cur_frame];

		// Initialize the next frame. Dimensions must be rounded up to the
		// nearest multiple of 16 as the MDEC operates on 16x16 pixel blocks.
		frame->width  = (sector_header.width  + 15) & 0xfff0;
		frame->height = (sector_header.height + 15) & 0xfff0;
	}

	// Append the payload contained in this sector to the current buffer.
	str_ctx.sector_count--;
	CdGetSector(
		&(frame->bs_data[2016 / 4 * sector_header.sector_id]),
		2016 / 4
	);
}

void mdec_dma_handler(void) {
	// Handle any sectors that were not processed by cd_event_handler() (see
	// below) while a DMA transfer from the MDEC was in progress. As the MDEC
	// has just finished decoding a slice, they can be safely handled now.
	if (str_ctx.sector_pending) {
		cd_sector_handler();
		str_ctx.sector_pending = 0;
	}

	// Upload the decoded slice to VRAM and start decoding the next slice (into
	// another buffer) if any.
	LoadImage(&str_ctx.slice_pos, str_ctx.slices[str_ctx.cur_slice]);

	str_ctx.cur_slice   ^= 1;
	str_ctx.slice_pos.x += BLOCK_SIZE;

	if (str_ctx.slice_pos.x < str_ctx.frame_width)
		DecDCTout(
			str_ctx.slices[str_ctx.cur_slice],
			BLOCK_SIZE * str_ctx.slice_pos.h / 2
		);
}

void cd_event_handler(CdlIntrResult event, uint8_t *payload) {
	// Ignore all events other than a sector being ready.
	if (event != CdlDataReady)
		return;

	// Only handle sectors immediately if the MDEC is not decoding a frame,
	// otherwise defer handling to mdec_dma_handler(). This is a workaround for
	// a hardware conflict between the DMA channels used for the CD drive and
	// MDEC output, which shall not run simultaneously.
	if (DecDCTinSync(1))
		str_ctx.sector_pending = 1;
	else
		cd_sector_handler();
}

/* Stream helpers */

void init_stream(void) {
	EnterCriticalSection();
	DMACallback(1, &mdec_dma_handler);
	CdReadyCallback(&cd_event_handler);
	ExitCriticalSection();

	// Copy the lookup table used for frame decompression to the scratchpad
	// area. This is optional but makes the decompressor slightly faster. See
	// the libpsxpress documentation for more details.
	DecDCTvlcCopyTableV3((VLC_TableV3 *) 0x1f800000);

	str_ctx.cur_frame = 0;
	str_ctx.cur_slice = 0;
}

StreamBuffer *get_next_frame(void) {
	while (!str_ctx.frame_ready)
		__asm__ volatile("");

	if (str_ctx.frame_ready < 0)
		return 0;

	str_ctx.frame_ready = 0;
	return &str_ctx.frames[str_ctx.cur_frame ^ 1];
}

void start_stream(CdlFILE *file) {
	str_ctx.frame_id       = -1;
	str_ctx.dropped_frames =  0;
	str_ctx.sector_pending =  0;
	str_ctx.frame_ready    =  0;

	CdSync(0, 0);

	// Configure the CD drive to read at 2x speed and to play any XA-ADPCM
	// sectors that might be interleaved with the video data.
	uint8_t mode = CdlModeRT | CdlModeSpeed;
	CdControl(CdlSetmode, (const uint8_t *) &mode, 0);

	// Start reading in real-time mode (i.e. without retrying in case of read
	// errors) and wait for the first frame to be buffered.
	CdControl(CdlReadS, &(file->pos), 0);

	get_next_frame();
}

/* Main */

static RenderContext ctx;

#define SHOW_STATUS(...) { FntPrint(-1, __VA_ARGS__); FntFlush(-1); display(&ctx); }
#define SHOW_ERROR(...)  { SHOW_STATUS(__VA_ARGS__); while (1) __asm__("nop"); }

int main(int argc, const char* argv[]) {
	init_context(&ctx);

	SHOW_STATUS("INITIALIZING\n");
	SpuInit();
	CdInit();
	InitGeom(); // GTE initialization required by the VLC decompressor
	DecDCTReset(0);

	SHOW_STATUS("OPENING VIDEO FILE\n");

	CdlFILE file;
	if (!CdSearchFile(&file, "\\VIDEO.STR"))
		SHOW_ERROR("FAILED TO FIND VIDEO.STR\n");

	init_stream();
	start_stream(&file);

	// Clear the screen, then disable framebuffer clearing to get rid of
	// flickering during playback.
	display(&ctx);
	ctx.db[0].draw.isbg = 0;
	ctx.db[1].draw.isbg = 0;
#ifdef DISP_24BPP
	ctx.db[0].disp.isrgb24 = 1;
	ctx.db[1].disp.isrgb24 = 1;
#endif

	int frame_time = 1, decode_errors = 0;

	while (1) {
#ifdef DRAW_OVERLAY
		int frame_start = TIMER_VALUE(1);
#endif

		// Wait for a full frame to be read from the disc and decompress the
		// bitstream into the format expected by the MDEC. If the video has
		// ended, restart playback from the beginning.
		StreamBuffer *frame = get_next_frame();
		if (!frame) {
			printf("End of file, looping...\n");
			CdControlB(CdlPause, 0, 0);

			start_stream(&file);
			continue;
		}

#ifdef DRAW_OVERLAY
		int decode_time = TIMER_VALUE(1);
#endif

		VLC_Context vlc_ctx;
		if (DecDCTvlcStart(
			&vlc_ctx,
			frame->mdec_data,
			sizeof(frame->mdec_data) / 4,
			frame->bs_data
		)) {
			decode_errors++;
			continue;
		}

#ifdef DRAW_OVERLAY
		// Calculate CPU usage of the decompressor.
		decode_time   = (TIMER_VALUE(1) - decode_time) & 0xffff;
		int cpu_usage = decode_time * 100 / frame_time;
#endif

		// Wait for the MDEC to finish decoding the previous frame, then flip
		// the framebuffers to display it and prepare the buffer for the next
		// frame.
		// NOTE: as the refresh rate of the GPU is not synced to the video's
		// frame rate, this VSync(0) call may potentially end up waiting too
		// long and desynchronizing playback. A better solution would be to
		// implement triple buffering (i.e. always keep 2 fully decoded frames
		// in VRAM and use VSyncCallback() to register a function that displays
		// the next decoded frame if available whenever vblank occurs).
#ifdef ENABLE_VSYNC
		VSync(0);
#endif
		DecDCTinSync(0);
		DecDCToutSync(0);

#ifdef DRAW_OVERLAY
		FntPrint(-1, "FRAME:%6d      READ ERRORS:  %6d\n", str_ctx.frame_id, str_ctx.dropped_frames);
		FntPrint(-1, "CPU:  %6d%%     DECODE ERRORS:%6d\n", cpu_usage, decode_errors);
		FntFlush(-1);
#endif
		display(&ctx);

		// Feed the newly decompressed frame to the MDEC. The MDEC will not
		// actually start decoding it until an output buffer is also configured
		// by calling DecDCTout() (see below).
#ifdef DISP_24BPP
		DecDCTin(frame->mdec_data, DECDCT_MODE_24BPP);
#else
		DecDCTin(frame->mdec_data, DECDCT_MODE_16BPP);
#endif

		// Place the frame at the center of the currently active framebuffer
		// and start decoding the first slice. Decoded slices will be uploaded
		// to VRAM in the background by mdec_dma_handler().
		RECT *fb_clip = &(ctx.db[ctx.db_active].draw.clip);
		int  x_offset = (fb_clip->w - frame->width)  / 2;
		int  y_offset = (fb_clip->h - frame->height) / 2;

		str_ctx.slice_pos.x = fb_clip->x + VRAM_X_COORD(x_offset);
		str_ctx.slice_pos.y = fb_clip->y + y_offset;
		str_ctx.slice_pos.w = BLOCK_SIZE;
		str_ctx.slice_pos.h = frame->height;
		str_ctx.frame_width = VRAM_X_COORD(frame->width);

		DecDCTout(
			str_ctx.slices[str_ctx.cur_slice],
			BLOCK_SIZE * str_ctx.slice_pos.h / 2
		);

#ifdef DRAW_OVERLAY
		frame_time = (TIMER_VALUE(1) - frame_start) & 0xffff;
#endif
	}

	return 0;
}
