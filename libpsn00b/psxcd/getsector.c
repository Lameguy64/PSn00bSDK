/*
 * PSn00bSDK CD drive library (sector DMA API)
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <assert.h>
#include <psxcd.h>
#include <hwregs_c.h>

#define DATA_SYNC_TIMEOUT	0x100000

/* DMA transfer functions */

int CdGetSector(void *madr, int size) {
	//while (!(CD_STAT & (1 << 6)))
		//__asm__ volatile("");

	DMA_MADR(3) = (uint32_t) madr;
	DMA_BCR(3)  = size | (1 << 16);
	DMA_CHCR(3) = 0x11000000;

	while (DMA_CHCR(3) & (1 << 24))
		__asm__ volatile("");

	return 1;
}

int CdGetSector2(void *madr, int size) {
	//while (!(CD_STAT & (1 << 6)))
		//__asm__ volatile("");

	DMA_MADR(3) = (uint32_t) madr;
	DMA_BCR(3)  = size | (1 << 16);
	DMA_CHCR(3) = 0x11400100; // Transfer 1 word every 16 CPU cycles

	return 1;
}

int CdDataSync(int mode) {
	if (mode)
		return (DMA_CHCR(3) >> 24) & 1;

	for (int i = DATA_SYNC_TIMEOUT; i; i--) {
		if (!(DMA_CHCR(3) & (1 << 24)))
			return 0;
	}

	_sdk_log("CdDataSync() timeout\n");
	return -1;
}
