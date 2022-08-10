/*
 * PSn00bSDK GPU library (common functions)
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stdio.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <hwregs_c.h>

#define QUEUE_LENGTH		8
#define DMA_CHUNK_LENGTH	8
#define VSYNC_TIMEOUT		0x100000

/* Internal globals */

GPU_VideoMode _gpu_video_mode;

static void (*_vsync_callback)(void);
static void (*_drawsync_callback)(void);

static const uint32_t *volatile _draw_queue[QUEUE_LENGTH];
static volatile uint8_t  _queue_head, _queue_tail, _queue_length;
static volatile uint32_t _vblank_counter, _last_hblank;

/* Interrupt handlers */

static void _vblank_handler(void) {
	_vblank_counter++;

	if (_vsync_callback)
		_vsync_callback();
}

static void _gpu_dma_handler(void) {
	//while (DMA_CHCR(2) & (1 << 24))
		//__asm__ volatile("");
	while (!(GPU_GP1 & (1 << 28)))
		__asm__ volatile("");

	if (_queue_length) {
		DrawOTag2(_draw_queue[_queue_head++]);

		_queue_length--;
		_queue_head %= QUEUE_LENGTH;
	} else {
		GPU_GP1 = 0x04000000; // Disable DMA request

		if (_drawsync_callback)
			_drawsync_callback();
	}
}

/* GPU reset and system initialization */

void ResetGraph(int mode) {
	// Perform some basic system initialization when ResetGraph() is called for
	// the first time.
	static int setup_done = 0;
	if (!setup_done) {
		EnterCriticalSection();

		DMA_DPCR = 0x03333333;
		DMA_DICR = 0;
		IRQ_MASK = 0;

		InterruptCallback(0, &_vblank_handler);
		DMACallback(2, &_gpu_dma_handler);
		RestartCallback();
		_96_remove();

		_gpu_video_mode = (GPU_GP1 >> 20) & 1;
		setup_done      = 1;

		ExitCriticalSection();
		printf("psxgpu: setup done, default mode is %s\n", _gpu_video_mode ? "PAL" : "NTSC");
	}

	if (mode == 3) {
		GPU_GP1 = 0x01000000; // Reset command buffer
		return;
	}

	DMA_DPCR   |= 0x0b000b00; // Enable DMA2 and DMA6
	DMA_CHCR(2) = 0x00000201; // Stop DMA2
	DMA_CHCR(6) = 0x00000200; // Stop DMA6

	if (mode == 1) {
		GPU_GP1 = 0x01000000; // Reset command buffer
		return;
	}

	GPU_GP1       = 0x00000000; // Reset GPU
	TIMER_CTRL(0) = 0x0500;
	TIMER_CTRL(1) = 0x0500;

	_queue_head     = 0;
	_queue_tail     = 0;
	_queue_length   = 0;
	_vblank_counter = 0;
	_last_hblank    = 0;
}

/* Syncing API */

// TODO: add support for no$psx's "halt" register
static void _vsync_halt(void) {
	int counter = _vblank_counter;

	for (int i = VSYNC_TIMEOUT; i; i--) {
		if (counter != _vblank_counter)
			return;
	}

	printf("psxgpu: VSync() timeout\n");
	ChangeClearPAD(0);
	ChangeClearRCnt(3, 0);
}

int VSync(int mode) {
	if (mode < 0)
		return _vblank_counter;
	if (mode == 1)
		return TIMER_VALUE(1) - _last_hblank;

	uint32_t status = GPU_GP1;

	// Wait for at least one vertical blank event to occur.
	do {
		_vsync_halt();

		// If interlaced mode is enabled, wait until the GPU starts displaying
		// the next field.
		if (status & (1 << 22)) {
			while (!((GPU_GP1 ^ status) & (1 << 31)))
				__asm__ volatile("");
		}
	} while ((--mode) > 0);

	// Update the horizontal blank counter and return the time elapsed since
	// the last time it was updated.
	uint16_t counter = TIMER_VALUE(1);
	uint16_t delta   = counter - _last_hblank;

	_last_hblank = counter;
	return delta;
}

int DrawSync(int mode) {
	if (mode)
		return (DMA_BCR(2) >> 16);

	// Wait for the queue to become empty.
	// TODO: add a timeout
	while (_queue_length)
		__asm__ volatile("");

	// Wait for any DMA transfer to finish if DMA is enabled.
	if (GPU_GP1 & (3 << 29)) {
		while (DMA_CHCR(2) & (1 << 24))
			__asm__ volatile("");
		while (!(GPU_GP1 & (1 << 28)))
			__asm__ volatile("");
	}

	while (!(GPU_GP1 & (1 << 26)))
		__asm__ volatile("");

	return 0;
}

void *VSyncCallback(void (*func)(void)) {
	EnterCriticalSection();

	void *old_callback  = _vsync_callback;
	_vsync_callback     = func;

	ExitCriticalSection();
	return old_callback;
}

void *DrawSyncCallback(void (*func)(void)) {
	EnterCriticalSection();

	void *old_callback = _drawsync_callback;
	_drawsync_callback = func;

	ExitCriticalSection();
	return old_callback;
}

/* OT and primitive drawing API */

void ClearOTagR(uint32_t *ot, size_t length) {
	DMA_MADR(6) = (uint32_t) &ot[length - 1];
	DMA_BCR(6)  = length & 0xffff;
	DMA_CHCR(6) = 0x11000002;

	//while (DMA_CHCR(6) & (1 << 24))
		//__asm__ volatile("");
}

void ClearOTag(uint32_t *ot, size_t length) {
	// DMA6 only supports writing to RAM in reverse order (last to first), so
	// the OT has to be cleared in software here. This function is thus much
	// slower than ClearOTagR().
	// https://problemkaputt.de/psx-spx.htm#dmachannels
	for (int i = 0; i < (length - 1); i++)
		ot[i] = (uint32_t) &ot[i + 1] & 0x00ffffff;

	ot[length - 1] = 0x00ffffff;
}

void DrawOTag(const uint32_t *ot) {
	// If GPU DMA is currently busy, append the OT to the queue instead of
	// drawing it immediately.
	if (DMA_CHCR(2) & (1 << 24)) {
		if (_queue_length >= QUEUE_LENGTH) {
			printf("psxgpu: DrawOTag() failed, draw queue full\n");
			return;
		}

		_draw_queue[_queue_tail++] = ot;
		_queue_length++;
		_queue_tail %= QUEUE_LENGTH;
		return;
	}

	DrawOTag2(ot);
}

void DrawOTag2(const uint32_t *ot) {
	GPU_GP1 = 0x04000002;

	while (!(GPU_GP1 & (1 << 26)))
		__asm__ volatile("");

	DMA_MADR(2) = (uint32_t) ot;
	DMA_BCR(2)  = 0;
	DMA_CHCR(2) = 0x01000401;
}

void DrawPrim(const uint32_t *pri) {	
	size_t length = getlen(pri);

	DrawSync(0);
	GPU_GP1 = 0x04000002;

	// NOTE: if length >= DMA_CHUNK_LENGTH then it also has to be a multiple of
	// DMA_CHUNK_LENGTH, otherwise the DMA channel will get stuck waiting for
	// more data indefinitely.
	DMA_MADR(2) = (uint32_t) &pri[1];
	if (length < DMA_CHUNK_LENGTH)
		DMA_BCR(2) = 0x00010000 | length;
	else
		DMA_BCR(2) = DMA_CHUNK_LENGTH | ((length / DMA_CHUNK_LENGTH) << 16);

	DMA_CHCR(2) = 0x01000201;
}

void AddPrim(uint32_t *ot, const void *pri) {
	addPrim(ot, pri);
}

/* Misc. functions */

GPU_VideoMode GetVideoMode(void) {
	return _gpu_video_mode;
}

void SetVideoMode(GPU_VideoMode mode) {
	uint32_t _mode, stat = GPU_GP1;

	_gpu_video_mode = mode & 1;

	_mode  = (mode & 1) << 3;
	_mode |= (stat >> 17) & 0x37; // GPUSTAT 17-22  -> cmd bits 0-5
	_mode |= (stat >> 10) & 0x40; // GPUSTAT bit 16 -> cmd bit 6
	_mode |= (stat >>  7) & 0x80; // GPUSTAT bit 14 -> cmd bit 7

	GPU_GP1 = 0x08000000 | mode;
}

int GetODE(void) {
	return (GPU_GP1 >> 31);
}

void SetDispMask(int mask) {
	GPU_GP1 = 0x03000000 | (mask ? 0 : 1);
}
