/*
 * PSn00bSDK interrupt management library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __PSXETC_H
#define __PSXETC_H

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

void *InterruptCallback(int irq, void (*func)(void));
void *GetInterruptCallback(int irq);
void *DMACallback(int dma, void (*func)(void));
void *GetDMACallback(int dma);

int ResetCallback(void);
void RestartCallback(void);
void StopCallback(void);

#ifdef __cplusplus
}
#endif

#endif
