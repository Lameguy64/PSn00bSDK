/*
 * PSn00bSDK interrupt management library
 * (C) 2019-2023 Lameguy64, spicyjpeg - MPL licensed
 */

/**
 * @file psxetc.h
 * @brief Interrupt management library header
 *
 * @details This library provides basic facilities (such as interrupt handling)
 * used by all other PSn00bSDK libraries, as well as some additional
 * functionality including a dynamic linker (whose API is however defined in a
 * separate header).
 */

#pragma once

/* IRQ and DMA channel definitions */

typedef enum _IRQ_Channel {
	IRQ_VBLANK	=  0,
	IRQ_GPU		=  1,
	IRQ_CD		=  2,
	IRQ_DMA		=  3,
	IRQ_TIMER0	=  4,
	IRQ_TIMER1	=  5,
	IRQ_TIMER2	=  6,
	IRQ_SIO0	=  7,
	IRQ_SIO1	=  8,
	IRQ_SPU		=  9,
	IRQ_GUN		= 10,
	IRQ_PIO		= 10
} IRQ_Channel;

typedef enum _DMA_Channel {
	DMA_MDEC_IN		= 0,
	DMA_MDEC_OUT	= 1,
	DMA_GPU			= 2,
	DMA_CD			= 3,
	DMA_SPU			= 4,
	DMA_PIO			= 5,
	DMA_OTC			= 6
} DMA_Channel;

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sets a callback for an interrupt.
 *
 * @details Registers a function to be called whenever the specified interrupt
 * is fired. A previously registered callback can be removed by passing a null
 * pointer instead. The IRQ controller is automatically configured to only
 * enable interrupts for which a callback is registered.
 *
 * The callback will run in the exception handler's context, so it should be as
 * fast as possible and shall not call any function that relies on interrupts
 * being enabled. Each interrupt is acknowledged automatically before the
 * callback is invoked.
 *
 * The following interrupt channels are available (the ones already used
 * internally by libraries shall not be overridden to avoid conflicts):
 *
 * | ID  | Channel          | Used by                                 |
 * | --: | :--------------- | :-------------------------------------- |
 * |   0 | IRQ_VBLANK       | psxgpu (use VSyncCallback() instead)    |
 * |   1 | IRQ_GPU          | psxgpu (use DrawSyncCallback() instead) |
 * |   2 | IRQ_CD           | psxcd (use CdReadyCallback() instead)   |
 * |   3 | IRQ_DMA          | psxetc (use DMACallback() instead)      |
 * |   4 | IRQ_TIMER0       |                                         |
 * |   5 | IRQ_TIMER1       |                                         |
 * |   6 | IRQ_TIMER2       |                                         |
 * |   7 | IRQ_SIO0         |                                         |
 * |   8 | IRQ_SIO1         | psxsio (use SIO_ReadCallback() instead) |
 * |   9 | IRQ_SPU          |                                         |
 * |  10 | IRQ_GUN, IRQ_PIO |                                         |
 *
 * WARNING: even though interrupts are acknowledged automatically at the IRQ
 * controller side, most IRQ channels (1, 2, 3, 7, 8, 9) additionally require
 * acknowledging at the device side, which must be done by the callback. The
 * exact way to acknowledge interrupts varies for each device, however it
 * usually involves setting or clearing a bit in a register. See the nocash
 * documentation for more details.
 *
 * @param irq
 * @param func
 * @return Previously set callback for the channel or NULL
 */
void *InterruptCallback(IRQ_Channel irq, void (*func)(void));

/**
 * @brief Gets the callback for an interrupt.
 *
 * @details Returns a pointer to the callback currently registered to handle
 * the specified interrupt, or a null pointer if none is set.
 *
 * @param irq
 * @return Currently set callback for the channel or NULL
 *
 * @see InterruptCallback()
 */
void *GetInterruptCallback(IRQ_Channel irq);

/**
 * @brief Sets a callback for a DMA interrupt.
 *
 * @details Registers a function to be called whenever the specified DMA
 * channel goes from busy to idle, i.e. when a transfer is completed. A
 * previously registered callback can be removed by passing a null pointer
 * instead. The DMA controller is automatically configured to only enable DMA
 * interrupts for which a callback is registered.
 *
 * This function uses InterruptCallback() to register a "master handler" for
 * DMA interrupts, which then dispatches the IRQ to depending on the channel
 * that triggered it.
 *
 * The callback will run in the exception handler's context, so it should be as
 * fast as possible and shall not call any function that relies on interrupts
 * being enabled. Each interrupt is acknowledged automatically before the
 * callback is invoked.
 *
 * The following DMA channels are available (the ones already used internally
 * by libraries shall not be overridden to avoid conflicts):
 *
 * | ID  | Channel      | Used by                                 |
 * | --: | :----------- | :-------------------------------------- |
 * |   0 | DMA_MDEC_IN  |                                         |
 * |   1 | DMA_MDEC_OUT |                                         |
 * |   2 | DMA_GPU      | psxgpu (use DrawSyncCallback() instead) |
 * |   3 | DMA_CD       |                                         |
 * |   4 | DMA_SPU      |                                         |
 * |   5 | DMA_PIO      |                                         |
 * |   6 | DMA_OTC      |                                         |
 *
 * @param dma
 * @param func
 * @return Previously set callback for the channel or NULL
 */
void *DMACallback(DMA_Channel dma, void (*func)(void));

/**
 * @brief Gets the callback for a DMA interrupt.
 *
 * @details Returns a pointer to the callback currently registered to handle
 * the specified DMA interrupt, or a null pointer if none is set.
 *
 * @param dma
 * @return Currently set callback for the channel or NULL
 *
 * @see DMACallback()
 */
void *GetDMACallback(DMA_Channel dma);

/**
 * @brief Enables, disables or sets the priority of a DMA channel.
 *
 * @details Enables the specified DMA channel and configures its priority (if
 * priority >= 0) or disables it (if priority = -1). The priority value must be
 * in 0-7 range, with 0 being the highest priority and 7 the lowest.
 *
 * All channels are disabled upon calling ResetCallback(); most libraries will
 * re-enable them as needed. By default the priority is set to 3 for all
 * channels.
 *
 * @param dma
 * @param priority Priority in 0-7 range or -1 to disable the channel
 * @return Previously set priority in 0-7 range, -1 if the channel was disabled
 */
int SetDMAPriority(DMA_Channel dma, int priority);

/**
 * @brief Gets the priority of a DMA channel.
 *
 * @details Returns the currently set priority value for the specified DMA
 * channel in 0-7 range, with 0 being the highest priority and 7 the lowest.
 * Returns -1 if the channel is not enabled.
 *
 * @param dma
 * @return Priority in 0-7 range, -1 if the channel is disabled
 *
 * @see SetDMAPriority()
 */
int GetDMAPriority(DMA_Channel dma);

/**
 * @brief Initializes the interrupt dispatcher and DMA controller.
 *
 * @details Sets up the interrupt handling system, hooks the BIOS to dispatch
 * interrupts to the library, clears all registered callbacks and disables all
 * DMA channels. This function must be called once at the beginning of the
 * program, prior to registering any IRQ or DMA callbacks.
 *
 * ResetCallback() is called by psxgpu's ResetGraph(), so invoking it manually
 * is usually not required. Calling ResetCallback() after ResetGraph() will
 * actually result in improper initialization, as ResetGraph() registers
 * several callbacks used internally by psxgpu.
 *
 * @return 0 or -1 if the dispatcher was already initialized
 */
int ResetCallback(void);

/**
 * @brief Restores the interrupt dispatcher.
 *
 * @details Restores the IRQ and DMA controller state saved by StopCallback()
 * and reinstalls BIOS hooks for interrupt dispatching. All callbacks
 * previously set before StopCallback() was called are preserved.
 *
 * @see StopCallback()
 */
void RestartCallback(void);

/**
 * @brief Temporarily disables the interrupt dispatcher.
 *
 * @details Saves the state of the IRQ and DMA controllers, then disables them
 * and removes BIOS hooks. This function must be called prior to launching a
 * new executable or DLL that calls ResetCallback() or ResetGraph(), or an
 * executable not built with PSn00bSDK that uses its own interrupt handling
 * subsystem (such as a retail game). The saved state can be restored after the
 * executable returns using RestartCallback().
 *
 * Note that interrupts are (obviously) disabled until RestartCallback() is
 * called.
 *
 * WARNING: any ongoing background processing or DMA transfer must be stopped
 * before calling StopCallback(), otherwise crashes may occur. This includes
 * flushing psxgpu's command queue using DrawSync(), stopping CD-ROM reading
 * and calling StopPAD() to disable the BIOS controller driver if used.
 *
 * @see RestartCallback()
 */
void StopCallback(void);

#ifdef __cplusplus
}
#endif
