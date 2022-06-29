/*
 * PSn00bSDK GPU library (image and VRAM transfer functions)
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <psxgpu.h>
#include <hwregs_c.h>

#define DMA_CHUNK_LENGTH 8

/* Common internal load/store function */

static void _load_store_image(
	uint32_t	command,
	int			mode,
	const RECT	*rect,
	uint32_t	*data
) {
	size_t length = rect->w * rect->h;
	if (length % 2)
		printf("psxgpu: can't transfer an odd number of pixels\n");

	length /= 2;
	if ((length >= DMA_CHUNK_LENGTH) && (length % DMA_CHUNK_LENGTH))
		printf("psxgpu: transfer data length (%d) is not a multiple of %d\n", length, DMA_CHUNK_LENGTH);

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

	DMA_CHCR(2) = 0x01000200 | !(mode & 1);
}

/* Public VRAM API */

void LoadImage(const RECT *rect, const u_long *data) {
	_load_store_image(0xa0000000, 2, rect, (uint32_t *) data);
}

void StoreImage(const RECT *rect, u_long *data) {
	_load_store_image(0xc0000000, 3, rect, (uint32_t *) data);
}

/* .TIM image parsers */

// This is the only libgs function PSn00bSDK is ever going to implement. The
// difference from GetTimInfo() is that it copies RECTs rather than merely
// returning pointers to them, which become useless once the .TIM file is
// unloaded from main RAM.
int GsGetTimInfo(const u_long *tim, GsIMAGE *info) {
	if ((*(tim++) & 0xffff) != 0x0010)
		return 1;

	info->pmode = *(tim++);
	if (info->pmode & 8) {
		const u_long *palette_end = tim;
		palette_end += *(tim++) / 4;

		*((u_long *) &(info->cx)) = *(tim++);
		*((u_long *) &(info->cw)) = *(tim++);
		info->clut = (u_long *) tim;

		tim = palette_end;
	} else {
		info->clut = 0;
	}

	tim++;
	*((u_long *) &(info->px)) = *(tim++);
	*((u_long *) &(info->pw)) = *(tim++);
	info->pixel = (u_long *) tim;

	return 0;
}

int GetTimInfo(const u_long *tim, TIM_IMAGE *info) {
	if ((*(tim++) & 0xffff) != 0x0010)
		return 1;

	info->mode = *(tim++);
	if (info->mode & 8) {
		const u_long *palette_end = tim;
		palette_end += *(tim++) / 4;

		info->crect = (RECT *)   tim;
		info->caddr = (u_long *) &tim[2];

		tim = palette_end;
	} else {
		info->caddr = 0;
	}

	tim++;
	info->prect = (RECT *)   tim;
	info->paddr = (u_long *) &tim[2];

	return 0;
}
