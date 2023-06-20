/*
 * PSn00bSDK GPU library (common functions)
 * (C) 2022-2023 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <assert.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <hwregs_c.h>

#define QUEUE_LENGTH	16
#define VSYNC_TIMEOUT	0x100000

static void _default_vsync_halt(void);

/* Private types */

typedef struct {
	void     (*func)(uint32_t, uint32_t, uint32_t);
	uint32_t arg1, arg2, arg3;
} DrawOp;

/* Internal globals */

GPU_VideoMode _gpu_video_mode;

static void (*_vsync_halt_func)(void)   = &_default_vsync_halt;
static void (*_vsync_callback)(void)    = (void *) 0;
static void (*_drawsync_callback)(void) = (void *) 0;

static volatile DrawOp   _draw_queue[QUEUE_LENGTH];
static volatile uint8_t  _queue_head, _queue_tail, _queue_length, _drawop_type;
static volatile uint32_t _vblank_counter, _last_vblank;
static volatile uint16_t _last_hblank;

/* Private interrupt handlers */

static void _vblank_handler(void) {
	_vblank_counter++;

	if (_vsync_callback)
		_vsync_callback();
}

static void _process_drawop(void) {
	int length = _queue_length;
	if (!length)
		return;

	if (--length) {
		int head    = _queue_head;
		_queue_head = (head + 1) % QUEUE_LENGTH;

		volatile DrawOp *entry = &_draw_queue[head];
		entry->func(entry->arg1, entry->arg2, entry->arg3);
	} else {
		GPU_GP1 = 0x04000000; // Disable DMA request

		if (_drawsync_callback)
			_drawsync_callback();
	}

	_queue_length = length;
}

static void _gpu_irq_handler(void) {
	GPU_GP1 = 0x02000000; // Reset IRQ

	if (_drawop_type == DRAWOP_TYPE_GPU_IRQ)
		_process_drawop();
}

static void _gpu_dma_handler(void) {
	if (_drawop_type == DRAWOP_TYPE_DMA)
		_process_drawop();
}

/* GPU reset and system initialization */

void ResetGraph(int mode) {
	_queue_head   = 0;
	_queue_tail   = 0;
	_queue_length = 0;
	_drawop_type  = 0;

	// Perform some basic system initialization when ResetGraph() is called for
	// the first time.
	if (!ResetCallback()) {
		int _exit = EnterCriticalSection();

		InterruptCallback(IRQ_VBLANK, &_vblank_handler);
		InterruptCallback(IRQ_GPU, &_gpu_irq_handler);
		DMACallback(DMA_GPU, &_gpu_dma_handler);
		_gpu_video_mode = (GPU_GP1 >> 20) & 1;

		if (_exit)
			ExitCriticalSection();

		_sdk_log("setup done, default mode is %s\n", _gpu_video_mode ? "PAL" : "NTSC");
	}

	if (mode) {
		GPU_GP1 = 0x01000000; // Reset command buffer
		GPU_GP1 = 0x02000000; // Reset IRQ
		GPU_GP1 = 0x04000000; // Disable DMA request

		if (mode == 1)
			return;
	} else {
		GPU_GP1 = 0x00000000; // Reset GPU
	}

	SetDMAPriority(DMA_GPU, 3);
	SetDMAPriority(DMA_OTC, 3);
	DMA_CHCR(DMA_GPU) = 0x00000201; // Stop DMA
	DMA_CHCR(DMA_OTC) = 0x00000200; // Stop DMA

	TIMER_CTRL(0) = 0x0500;
	TIMER_CTRL(1) = 0x0500;

	_vblank_counter = 0;
	_last_vblank    = 0;
	_last_hblank    = 0;
}

/* VSync() API */

// TODO: add support for no$psx's "halt" register
static void _default_vsync_halt(void) {
	int counter = _vblank_counter;
	for (int i = VSYNC_TIMEOUT; i; i--) {
		if (counter != _vblank_counter)
			return;
	}

	_sdk_log("VSync() timeout\n");
	ChangeClearPAD(0);
	ChangeClearRCnt(3, 0);
}

int VSync(int mode) {
	uint16_t delta = (TIMER_VALUE(1) - _last_hblank) & 0xffff;
	if (mode == 1)
		return delta;
	if (mode < 0)
		return _vblank_counter;

	// Wait for the specified number of vertical blank events since the last
	// call to VSync() to occur (if mode >= 2) or just for a single vertical
	// blank (if mode = 0).
	uint32_t target = mode ? (_last_vblank + mode) : (_vblank_counter + 1);

	while (_vblank_counter < target) {
		uint32_t status = GPU_GP1;
		_vsync_halt_func();

		// If interlaced mode is enabled, wait until the GPU starts displaying
		// the next field.
		if (status & (1 << 22)) {
			while (!((GPU_GP1 ^ status) & (1 << 31)))
				__asm__ volatile("");
		}
	}

	_last_vblank = _vblank_counter;
	_last_hblank = TIMER_VALUE(1);

	return delta;
}

void *VSyncHaltFunction(void (*func)(void)) {
	//FastEnterCriticalSection();

	void *old_callback  = _vsync_halt_func;
	_vsync_halt_func    = func;

	//FastExitCriticalSection();
	return old_callback;
}

void *VSyncCallback(void (*func)(void)) {
	FastEnterCriticalSection();

	void *old_callback  = _vsync_callback;
	_vsync_callback     = func;

	FastExitCriticalSection();
	return old_callback;
}

/* Command queue API */

void SetDrawOpType(GPU_DrawOpType type) {
	_drawop_type = type;
}

int EnqueueDrawOp(void (*func)(), uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	_sdk_validate_args(func, -1);

	// If GPU DMA is currently busy, append the command to the queue instead of
	// executing it immediately. Note that interrupts must be disabled *prior*
	// to checking if DMA is busy; disabling them afterwards would create a
	// race condition where the DMA transfer could end while interrupts are
	// being disabled. Interrupts are disabled through the IRQ_MASK register
	// rather than via syscalls for performance reasons.
	FastEnterCriticalSection();
	int length = _queue_length;

	if (!length) {
		_queue_length = 1;
		FastExitCriticalSection();

		func(arg1, arg2, arg3);
		return 0;
	}
	if (length >= QUEUE_LENGTH) {
		FastExitCriticalSection();

		_sdk_log("draw queue overflow, dropping commands\n");
		return -1;
	}

	int tail      = _queue_tail;
	_queue_tail   = (tail + 1) % QUEUE_LENGTH;
	_queue_length = length + 1;

	volatile DrawOp *entry = &_draw_queue[tail];
	entry->func = func;
	entry->arg1 = arg1;
	entry->arg2 = arg2;
	entry->arg3 = arg3;

	FastExitCriticalSection();
	return length;
}

int DrawSync(int mode) {
	if (mode)
		return _queue_length;

	// Wait for the queue to become empty.
	for (int i = VSYNC_TIMEOUT; i; i--) {
		if (!_queue_length)
			break;
	}

	if (!_queue_length) {
		// Wait for any DMA transfer to finish if DMA is enabled.
		if (GPU_GP1 & (3 << 29)) {
			while (!(GPU_GP1 & (1 << 28)) || (DMA_CHCR(DMA_GPU) & (1 << 24)))
				__asm__ volatile("");
		}

		while (!(GPU_GP1 & (1 << 26)))
			__asm__ volatile("");
	} else {
		_sdk_log("DrawSync() timeout\n");
	}

	return _queue_length;
}

void *DrawSyncCallback(void (*func)(void)) {
	FastEnterCriticalSection();

	void *old_callback = _drawsync_callback;
	_drawsync_callback = func;

	FastExitCriticalSection();
	return old_callback;
}

/* Queue pause/resume API */

int IsIdleGPU(int timeout) {
	if (timeout <= 0)
		timeout = 1;

	for (; timeout; timeout--) {
		if (GPU_GP1 & (1 << 26))
			return 0;
	}

	//_sdk_log("IsIdleGPU() timeout\n");
	return -1;
}
