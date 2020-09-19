#ifndef _PSXETC_H
#define _PSXETC_H

#ifdef __cplusplus
extern "C" {
#endif

// Interrupt callback functions
void *DMACallback(int dma, void (*func)(void));
void *InterruptCallback(int irq, void (*func)(void));
void *GetInterruptCallback(int irq);				// Original
void RestartCallback();

#ifdef __cplusplus
}
#endif

#endif
