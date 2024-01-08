/*
 * PSn00bSDK SPU CD-ROM streaming example (streaming API)
 * (C) 2022-2023 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <psxgpu.h>
#include <psxspu.h>
#include <psxetc.h>
#include <psxapi.h>
#include <hwregs_c.h>

#include "stream.h"

// The first 4 KB of SPU RAM are reserved for capture buffers and psxspu
// additionally uploads a dummy sample (16 bytes) at 0x1000 by default, so the
// chunks must be placed after those. The dummy sample is going to be used to
// keep unused SPU channels busy, preventing them from accidentally triggering
// the SPU IRQ and throwing off the timing (all channels are always reading
// from SPU RAM, even when "stopped").
// https://problemkaputt.de/psx-spx.htm#spuinterrupt
#define DUMMY_BLOCK_ADDR 0x1000

#define _min(x, y) (((x) < (y)) ? (x) : (y))

/* Private utilities */

static volatile Stream_Context *volatile _active_ctx = (void *) 0;

static Stream_Time _default_timer_function(void) {
	return VSync(-1);
}

static int _get_default_timer_rate(void) {
	return (GetVideoMode() == MODE_PAL) ? 50 : 60;
}

/* Interrupt handlers */

static void _spu_irq_handler(void) {
	volatile Stream_Context *ctx = _active_ctx;

	// Acknowledge the interrupt to ensure it can be triggered again. The only
	// way to do this is actually to disable the interrupt entirely; we'll
	// enable it again once the chunk is ready.
	SPU_CTRL &= ~(1 << 6);

	if (!ctx)
		return;

	// Ensure enough data is available. If not, re-enable the IRQ (to prevent
	// the SPU from getting stuck, even though this will produce nasty noises!)
	// and fire the underrun callback.
	int length = (int) ctx->buffer.length - (int) ctx->chunk_size;

	if (length < 0) {
		if (ctx->config.underrun_callback)
			ctx->config.underrun_callback();

		SPU_CTRL |= 1 << 6; // FIXME: figure out another way around this
		return;
	}

	// Pull a chunk from the ring buffer and invoke the refill callback (if any)
	// once the buffer's length is below the refill threshold.
	ctx->db_active ^= 1;
	ctx->buffering  = true;

	ctx->play_time   += ctx->samples_per_chunk;
	ctx->last_updated = ctx->config.timer_function();

	size_t tail        = ctx->buffer.tail;
	uint8_t *ptr       = &ctx->buffer.data[ctx->buffer.tail];
	ctx->buffer.tail   = (tail + ctx->chunk_size) % ctx->config.buffer_size;
	ctx->buffer.length = length;

	if ((length <= ctx->config.refill_threshold) && !ctx->callback_issued) {
		if (ctx->config.refill_callback)
			ctx->config.refill_callback();

		ctx->callback_issued = true;
	}

	// Configure to SPU to trigger an IRQ once the chunk that is going to be
	// filled now starts playing (so the next buffer can be loaded) and override
	// both channels' loop addresses to make them "jump" to the new buffers,
	// rather than actually looping when they encounter the loop flag at the end
	// of the currently playing buffers.
	uint32_t address =
		ctx->config.spu_address + (ctx->db_active ? ctx->chunk_size : 0);

	int sample_rate         = ctx->new_sample_rate;
	ctx->config.sample_rate = sample_rate;

	SpuSetTransferStartAddr(address);
	SPU_IRQ_ADDR = getSPUAddr(address);

	for (uint32_t ch = 0, mask = ctx->config.channel_mask; mask; ch++, mask >>= 1) {
		if (!(mask & 1))
			continue;

		SPU_CH_FREQ     (ch) = getSPUSampleRate(sample_rate);
		SPU_CH_LOOP_ADDR(ch) = getSPUAddr(address);
		address             += ctx->config.interleave;

		// Make sure this channel's data ends with an appropriate loop flag.
		//ptr[offset - 15] |= 0x03;
	}

	// Start uploading the next chunk to the SPU.
	SpuWrite((const uint32_t *) ptr, ctx->chunk_size);
}

static void _spu_dma_handler(void) {
	// Re-enable the SPU IRQ once the new chunk has been fully uploaded.
	SPU_CTRL |= 1 << 6;

	_active_ctx->buffering = false;
}

/* Public API */

void Stream_Init(Stream_Context *ctx, const Stream_Config *config) {
	__builtin_memset(ctx, 0, sizeof(Stream_Context));
	__builtin_memcpy(&(ctx->config), config, sizeof(Stream_Config));

	ctx->num_channels = 0;
	for (uint32_t mask = config->channel_mask; mask; mask >>= 1) {
		if (mask & 1)
			ctx->num_channels++;
	}

	assert(ctx->num_channels);

	if (!ctx->config.timer_function) {
		ctx->config.timer_rate     = _get_default_timer_rate();
		ctx->config.timer_function = &_default_timer_function;
	}

	ctx->chunk_size        = ctx->config.interleave * ctx->num_channels;
	ctx->samples_per_chunk = ctx->config.interleave / 16 * 28;
	ctx->new_sample_rate   = ctx->config.sample_rate;
	ctx->buffer.data       = malloc(config->buffer_size);

	assert(ctx->buffer.data);

	int _exit            = EnterCriticalSection();
	ctx->old_irq_handler = InterruptCallback(IRQ_SPU, &_spu_irq_handler);
	ctx->old_dma_handler = DMACallback(DMA_SPU, &_spu_dma_handler);

	if (_exit)
		ExitCriticalSection();
}

void Stream_Destroy(Stream_Context *ctx) {
	free(ctx->buffer.data);

	int _exit = EnterCriticalSection();
	InterruptCallback(IRQ_SPU, ctx->old_irq_handler);
	DMACallback(DMA_SPU, ctx->old_dma_handler);

	if (_exit)
		ExitCriticalSection();
}

bool Stream_Start(Stream_Context *ctx, bool resume) {
	if (_active_ctx)
		return false;

	_active_ctx = ctx;

	// Wait for the first chunk to be buffered and ready to play.
	if (!resume) {
		_spu_irq_handler();
		SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
	}

	uint32_t address =
		ctx->config.spu_address + (ctx->db_active ? ctx->chunk_size : 0);

	int sample_rate         = ctx->new_sample_rate;
	ctx->config.sample_rate = sample_rate;

	// Disable the IRQ as we're going to call spu_irq_handler() manually (due to
	// finicky SPU timings).
	SPU_CTRL &= ~(1 << 6);
	SpuSetKey(0, ctx->config.channel_mask);

	for (uint32_t ch = 0, mask = ctx->config.channel_mask; mask; ch++, mask >>= 1) {
		if (!(mask & 1))
			continue;

		SPU_CH_ADDR (ch) = getSPUAddr(address);
		SPU_CH_FREQ (ch) = getSPUSampleRate(sample_rate);
		SPU_CH_ADSR1(ch) = 0x00ff;
		SPU_CH_ADSR2(ch) = 0x0000;

		address += ctx->config.interleave;
	}

	SpuSetKey(1, ctx->config.channel_mask);
	_spu_irq_handler();

	return true;
}

bool Stream_Stop(void) {
	volatile Stream_Context *ctx = _active_ctx;

	if (!ctx)
		return false;

	// Prevent the channels from triggering the SPU IRQ by stopping them and
	// pointing them to the dummy block.
	SpuSetKey(0, ctx->config.channel_mask);

	for (uint32_t ch = 0, mask = ctx->config.channel_mask; mask; ch++, mask >>= 1) {
		if (mask & 1)
			SPU_CH_ADDR(ch) = getSPUAddr(DUMMY_BLOCK_ADDR);
	}

	SpuSetKey(1, ctx->config.channel_mask);

	ctx->last_stopped = ctx->config.timer_function();
	_active_ctx       = (void *) 0;

	return true;
}

void Stream_SetSampleRate(Stream_Context *ctx, int value) {
	ctx->new_sample_rate = value;
}

bool Stream_IsActive(const Stream_Context *ctx) {
	return (ctx == _active_ctx);
}

uint32_t Stream_GetSamplesPlayed(const Stream_Context *ctx) {
	// Calculate the time elapsed from the last update (or since the stream was
	// stopped) and use the value to estimate how many samples have been played
	// since then.
	Stream_Time delta;

	if (ctx == _active_ctx)
		delta = ctx->config.timer_function() - ctx->last_updated;
	else
		delta = ctx->last_stopped - ctx->last_updated;

	return ctx->play_time + (
		(delta * ctx->config.sample_rate) / ctx->config.timer_rate
	);
}

void Stream_ResetSamplesPlayed(Stream_Context *ctx) {
	ctx->play_time = 0;
}

size_t Stream_GetRefillLength(const Stream_Context *ctx) {
	int unbuf_total = (int) ctx->config.buffer_size - (int) ctx->buffer.length;

	if (unbuf_total <= 0)
		return 0;

	return unbuf_total;
}

size_t Stream_GetFeedPtr(const Stream_Context *ctx, uint8_t **ptr) {
	// Check if filling up the entire buffer would require wrapping around its
	// boundary. If that is the case, only return the length of the first
	// contiguous region. The second region will be returned once the first one
	// has been filled up.
	FastEnterCriticalSection();

	size_t head = ctx->buffer.head;

	int unbuf_total = (int) ctx->config.buffer_size - (int) ctx->buffer.length;
	int unbuf_head  = (int) ctx->config.buffer_size - (int) head;

	FastExitCriticalSection();

	if (unbuf_total <= 0)
		return 0;

	*ptr = &(ctx->buffer.data[head]);
	return (size_t) _min(unbuf_total, unbuf_head);
}

void Stream_Feed(Stream_Context *ctx, size_t length) {
	int unbuf_total = (int) ctx->config.buffer_size - (int) ctx->buffer.length;
	length          = _min(length, unbuf_total);

	FastEnterCriticalSection();

	size_t new_length  = ctx->buffer.length + length;
	ctx->buffer.head   = (ctx->buffer.head + length) % ctx->config.buffer_size;
	ctx->buffer.length = new_length;

	if (new_length > ctx->config.refill_threshold)
		ctx->callback_issued = false;

	FastExitCriticalSection();
}
