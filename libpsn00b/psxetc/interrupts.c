/*
 * PSn00bSDK interrupt management library
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
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

static uint32_t _saved_irq_mask, _saved_dma_dpcr, _saved_dma_dicr;
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

static const struct JMP_BUF _isr_jmp_buf = {
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
	uint32_t stat = IRQ_STAT, mask = IRQ_MASK;

	// Clear all IRQ flags in one shot. This is not the "proper" way to do it
	// but it's much faster than clearing one flag at a time.
	IRQ_STAT = ~mask;

	//for (int i = 0; i < NUM_IRQ_CHANNELS; i++) {
	for (int i = 0; stat; i++, stat >>= 1) {
		if (!(stat & 1))
			continue;

		if (_irq_handlers[i])
			_irq_handlers[i]();
	}

	ReturnFromException();
}

static void _global_dma_handler(void) {
	uint32_t stat = DMA_DICR;

	// Clear all DMA IRQ flags in one shot (note that flags are cleared by
	// writing 1 to them rather than 0).
	stat    &= 0x7fff0000;
	DMA_DICR = stat;
	stat   >>= 24;

	//for (int i = 0; i < NUM_DMA_CHANNELS; i++) {
		for (int i = 0; stat; i++, stat >>= 1) {
		if (!(stat & 1))
			continue;

		if (_dma_handlers[i])
			_dma_handlers[i]();
	}
}

/* Callback registration API */

void *InterruptCallback(int irq, void (*func)(void)) {
	if ((irq < 0) || (irq >= NUM_IRQ_CHANNELS))
		return 0;

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

void *GetInterruptCallback(int irq) {
	if ((irq < 0) || (irq >= NUM_IRQ_CHANNELS))
		return 0;

	return _irq_handlers[irq];
}

void *DMACallback(int dma, void (*func)(void)) {
	if ((dma < 0) || (dma >= NUM_DMA_CHANNELS))
		return 0;

	void *old_callback = _dma_handlers[dma];
	_dma_handlers[dma] = func;

	// Enable or disable the IRQ in the DMA_DICR register depending on whether
	// the callback is being registered or removed. The main DMA IRQ dispatcher
	// is also registered if this is the first DMA callback being configured,
	// or disabled if it's the last one being removed.
	if (func) {
		DMA_DICR |= (0x10000 << dma) | (1 << 23);

		if (!(_num_dma_handlers++))
			InterruptCallback(3, &_global_dma_handler);
	} else {
		if (--_num_dma_handlers) {
			DMA_DICR &= ~(0x10000 << dma);
		} else {
			DMA_DICR = 0;
			InterruptCallback(3, 0);
		}
	}

	return old_callback;
}

void *GetDMACallback(int dma) {
	if ((dma < 0) || (dma >= NUM_DMA_CHANNELS))
		return 0;

	return _dma_handlers[dma];
}

/* Hook installation/removal API */

int ResetCallback(void) {
	if (_isr_installed)
		return -1;

	EnterCriticalSection();
	_saved_irq_mask = 1 << 3; // Enable DMA IRQ by default
	_saved_dma_dpcr = 0x03333333;
	_saved_dma_dicr = 0;

	for (int i = 0; i < NUM_IRQ_CHANNELS; i++)
		_irq_handlers[i] = (void *) 0;
	for (int i = 0; i < NUM_DMA_CHANNELS; i++)
		_dma_handlers[i] = (void *) 0;

	// Set up the DMA IRQ handler. This handler shall *not* be overridden using
	// InterruptCallback().
	_irq_handlers[3] = &_global_dma_handler;

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
	SetCustomExitFromException(&_isr_jmp_buf);
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

	SetDefaultExitFromException();
	ChangeClearPAD(1);
	ChangeClearRCnt(0, 1);
	ChangeClearRCnt(1, 1);
	ChangeClearRCnt(2, 1);
	ChangeClearRCnt(3, 1);

	_isr_installed = 0;
}
