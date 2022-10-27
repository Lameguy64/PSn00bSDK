/*
 * PSn00bSDK interrupt management library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __PSXETC_H
#define __PSXETC_H

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

void *InterruptCallback(IRQ_Channel irq, void (*func)(void));
void *GetInterruptCallback(IRQ_Channel irq);
void *DMACallback(DMA_Channel dma, void (*func)(void));
void *GetDMACallback(DMA_Channel dma);

int ResetCallback(void);
void RestartCallback(void);
void StopCallback(void);

#ifdef __cplusplus
}
#endif

#endif
