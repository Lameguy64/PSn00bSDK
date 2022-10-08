/*
 * PSn00bSDK GPU library (image and VRAM transfer functions)
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stdio.h>
#include <psxgpu.h>
#include <hwregs_c.h>

#define DMA_CHUNK_LENGTH 8

/* Private utilities */

#ifdef DEBUG
#define _LOG(...) printf(__VA_ARGS__)
#else
#define _LOG(...)
#endif

static void _load_store_image(
	uint32_t	command,
	int			mode,
	const RECT	*rect,
	uint32_t	*data
) {
	size_t length = rect->w * rect->h;
	if (length % 2)
		_LOG("psxgpu: can't transfer an odd number of pixels\n");

	length /= 2;
	if ((length >= DMA_CHUNK_LENGTH) && (length % DMA_CHUNK_LENGTH)) {
		_LOG("psxgpu: transfer data length (%d) is not a multiple of %d, rounding\n", length, DMA_CHUNK_LENGTH);
		length += DMA_CHUNK_LENGTH - 1;
	}

	DrawSync(0);
	GPU_GP1 = 0x04000000; // Disable DMA request
	GPU_GP0 = 0x01000000; // Flush cache

	GPU_GP0 = command;
	//GPU_GP0 = rect->x | (rect->y << 16);
	GPU_GP0 = *((const uint32_t *) &(rect->x));
	//GPU_GP0 = rect->w | (rect->h << 16);
	GPU_GP0 = *((const uint32_t *) &(rect->w));

	// Enable DMA request, route to GP0 (2) or from GPU_READ (3)
	GPU_GP1 = 0x04000000 | mode;

	DMA_MADR(2) = (uint32_t) data;
	if (length < DMA_CHUNK_LENGTH)
		DMA_BCR(2) = 0x00010000 | length;
	else
		DMA_BCR(2) = DMA_CHUNK_LENGTH | ((length / DMA_CHUNK_LENGTH) << 16);

	DMA_CHCR(2) = 0x01000200 | ((mode & 1) ^ 1);
}

/* VRAM transfer API */

void LoadImage(const RECT *rect, const uint32_t *data) {
	_load_store_image(0xa0000000, 2, rect, (uint32_t *) data);
}

void StoreImage(const RECT *rect, uint32_t *data) {
	_load_store_image(0xc0000000, 3, rect, data);
}

/* .TIM image parsers */

// This is the only libgs function PSn00bSDK is ever going to implement. The
// difference from GetTimInfo() is that it copies RECTs rather than merely
// returning pointers to them, which become useless once the .TIM file is
// unloaded from main RAM.
int GsGetTimInfo(const uint32_t *tim, GsIMAGE *info) {
	if ((*(tim++) & 0xffff) != 0x0010)
		return 1;

	info->pmode = *(tim++);
	if (info->pmode & 8) {
		const uint32_t *palette_end = tim;
		palette_end += *(tim++) / 4;

		*((uint32_t *) &(info->cx)) = *(tim++);
		*((uint32_t *) &(info->cw)) = *(tim++);
		info->clut = (uint32_t *) tim;

		tim = palette_end;
	} else {
		info->clut = 0;
	}

	tim++;
	*((uint32_t *) &(info->px)) = *(tim++);
	*((uint32_t *) &(info->pw)) = *(tim++);
	info->pixel = (uint32_t *) tim;

	return 0;
}

int GetTimInfo(const uint32_t *tim, TIM_IMAGE *info) {
	if ((*(tim++) & 0xffff) != 0x0010)
		return 1;

	info->mode = *(tim++);
	if (info->mode & 8) {
		const uint32_t *palette_end = tim;
		palette_end += *(tim++) / 4;

		info->crect = (RECT *)     tim;
		info->caddr = (uint32_t *) &tim[2];

		tim = palette_end;
	} else {
		info->caddr = 0;
	}

	tim++;
	info->prect = (RECT *)     tim;
	info->paddr = (uint32_t *) &tim[2];

	return 0;
}
