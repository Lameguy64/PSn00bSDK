/*
 * PSn00bSDK interrupt management library
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <psxapi.h>
#include <psxetc.h>
#include <hwregs_c.h>

#define NUM_IRQ_CHANNELS	11
#define NUM_DMA_CHANNELS	7
#define ISR_STACK_SIZE		0x1000

/* Internal globals */

static void (*_irq_handlers[NUM_IRQ_CHANNELS])(void);
static void (*_dma_handlers[NUM_DMA_CHANNELS])(void);
static int  _num_dma_handlers = 0;

static uint16_t _saved_irq_mask;
static uint32_t _saved_dma_dpcr, _saved_dma_dicr;
static int _isr_installed = 0;

/* Custom ISR jmp_buf */

// The ISR and all functions called by it (thus, all callbacks registered using
// InterruptCallback() and DMACallback()) use an independent stack, isolated
// from the main thread's stack. As the size of this stack is limited, custom
// callbacks shall keep the number of nested subroutine calls to a minimum and
// avoid allocating large buffers (e.g. for receiving a sector from the CD
// drive) on the stack.
static uint8_t _isr_stack[ISR_STACK_SIZE];

extern uint8_t _gp[];
static void _global_isr(void);

static const JumpBuffer _isr_jmp_buf = {
	.ra = (uint32_t) &_global_isr,
	.sp = (uint32_t) &_isr_stack[ISR_STACK_SIZE],
	.fp = 0,
	.s0 = 0,
	.s1 = 0,
	.s2 = 0,
	.s3 = 0,
	.s4 = 0,
	.s5 = 0,
	.s6 = 0,
	.s7 = 0,
	.gp = (uint32_t) _gp
};

/* Internal IRQ and DMA handlers */

static void _global_isr(void) {
	uint16_t stat = IRQ_STAT & IRQ_MASK;

	for (; stat; stat = IRQ_STAT & IRQ_MASK) {
		//for (int i = 0; i < NUM_IRQ_CHANNELS; i++) {
		for (int i = 0, mask = 1; stat; i++, stat >>= 1, mask <<= 1) {
			if (!(stat & 1))
				continue;

			// Acknowledge the current IRQ. Note that clearing all IRQ flags in one
			// shot would result in hard-to-debug race conditions (been there, done
			// that).
			IRQ_STAT = (uint16_t) (mask ^ 0xffff);

			if (_irq_handlers[i])
				_irq_handlers[i]();
		}
	}

	ReturnFromException();
}

static void _global_dma_handler(void) {
	uint32_t dicr = DMA_DICR;
	uint32_t stat = (dicr >> 24) & 0x7f;

	for (; stat; dicr = DMA_DICR, stat = (dicr >> 24) & 0x7f) {
		uint32_t base = dicr & 0x00ffffff;

		//for (int i = 0; i < NUM_DMA_CHANNELS; i++) {
		for (int i = 0, mask = (1 << 24); stat; i++, stat >>= 1, mask <<= 1) {
			if (!(stat & 1))
				continue;

			// Acknowledge the current DMA channel's IRQ. For whatever reason
			// DMA IRQ flags are cleared by writing 1 to them rather than 0.
			DMA_DICR = base | mask;

			if (_dma_handlers[i])
				_dma_handlers[i]();
		}
	}
}

/* IRQ and DMA handler API */

void *InterruptCallback(IRQ_Channel irq, void (*func)(void)) {
	_sdk_validate_args((irq >= 0) && (irq < NUM_IRQ_CHANNELS), 0);

	void *old_callback = _irq_handlers[irq];
	_irq_handlers[irq] = func;

	// Enable or disable the IRQ in the IRQ_MASK register depending on whether
	// the callback is being registered or removed.
	if (func)
		IRQ_MASK |= 1 << irq;
	else
		IRQ_MASK &= ~(1 << irq);

	return old_callback;
}

void *GetInterruptCallback(IRQ_Channel irq) {
	_sdk_validate_args((irq >= 0) && (irq < NUM_IRQ_CHANNELS), 0);

	return _irq_handlers[irq];
}

void *DMACallback(DMA_Channel dma, void (*func)(void)) {
	_sdk_validate_args((dma >= 0) && (dma < NUM_DMA_CHANNELS), 0);

	void *old_callback = _dma_handlers[dma];
	_dma_handlers[dma] = func;

	// Enable or disable the IRQ in the DMA_DICR register depending on whether
	// the callback is being registered or removed. The main DMA IRQ dispatcher
	// is also registered if this is the first DMA callback being configured,
	// or disabled if it's the last one being removed.
	if (!old_callback && func) {
		DMA_DICR |= (0x10000 << dma) | (1 << 23);

		if (!(_num_dma_handlers++))
			InterruptCallback(IRQ_DMA, &_global_dma_handler);
	} else if (old_callback && !func) {
		if (--_num_dma_handlers) {
			DMA_DICR &= ~(0x10000 << dma);
		} else {
			DMA_DICR = 0;
			InterruptCallback(IRQ_DMA, 0);
		}
	}

	return old_callback;
}

void *GetDMACallback(DMA_Channel dma) {
	_sdk_validate_args((dma >= 0) && (dma < NUM_DMA_CHANNELS), 0);

	return _dma_handlers[dma];
}

/* DMA channel priority API */

int SetDMAPriority(DMA_Channel dma, int priority) {
	_sdk_validate_args((dma >= 0) && (dma < NUM_DMA_CHANNELS), -1);

	uint32_t dpcr    = DMA_DPCR;
	uint32_t channel = dpcr >> (dma * 4);

	dpcr &= ~(0xf << (dma * 4));
	if (priority >= 0)
		dpcr |= ((priority & 7) | 8) << (dma * 4);

	DMA_DPCR = dpcr;
	return (channel & 8) ? (channel & 7) : -1;
}

int GetDMAPriority(DMA_Channel dma) {
	_sdk_validate_args((dma >= 0) && (dma < NUM_DMA_CHANNELS), -1);

	uint32_t channel = DMA_DPCR >> (dma * 4);
	return (channel & 8) ? (channel & 7) : -1;
}

/* Hook installation/removal API */

int ResetCallback(void) {
	if (_isr_installed)
		return -1;

	EnterCriticalSection();
	_saved_irq_mask = 0;
	_saved_dma_dpcr = 0x03333333;
	_saved_dma_dicr = 0;

	for (int i = 0; i < NUM_IRQ_CHANNELS; i++)
		_irq_handlers[i] = (void *) 0;
	for (int i = 0; i < NUM_DMA_CHANNELS; i++)
		_dma_handlers[i] = (void *) 0;

	BUS_COM_DELAY = 0x00001325;
	_96_remove();
	RestartCallback();
	return 0;
}

void RestartCallback(void) {
	if (_isr_installed)
		return;

	IRQ_STAT = 0;
	IRQ_MASK = _saved_irq_mask;
	DMA_DPCR = _saved_dma_dpcr;
	DMA_DICR = _saved_dma_dicr;

	// Install the ISR hook and prevent the kernel's internal handlers from
	// automatically acknowledging SPI and timer IRQs.
	HookEntryInt(&_isr_jmp_buf);
	ChangeClearPAD(0);
	ChangeClearRCnt(0, 0);
	ChangeClearRCnt(1, 0);
	ChangeClearRCnt(2, 0);
	ChangeClearRCnt(3, 0);

	_isr_installed = 1;
	ExitCriticalSection();
}

void StopCallback(void) {
	if (!_isr_installed)
		return;

	// Save the state of IRQ and DMA registers, then reset them and undo the
	// changes that were made to the kernel's state.
	EnterCriticalSection();
	_saved_irq_mask = IRQ_MASK;
	_saved_dma_dpcr = DMA_DPCR;
	_saved_dma_dicr = DMA_DICR;

	IRQ_STAT = 0;
	IRQ_MASK = 0;
	DMA_DPCR = _saved_dma_dpcr & 0x07777777;
	DMA_DICR = 0;

	ResetEntryInt();
	ChangeClearPAD(1);
	ChangeClearRCnt(0, 1);
	ChangeClearRCnt(1, 1);
	ChangeClearRCnt(2, 1);
	ChangeClearRCnt(3, 1);

	_isr_installed = 0;
}
