/*
 * PSn00bSDK GPU library (drawing/display list functions)
 * (C) 2022-2023 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <assert.h>
#include <psxetc.h>
#include <psxgpu.h>
#include <hwregs_c.h>

/* Private utilities */

// This function is actually referenced in env.c as well, so it can't be static.
void _send_linked_list(GPU_DrawOpType type, const uint32_t *ot) {
	SetDrawOpType(type);
	GPU_GP1 = 0x04000002; // Enable DMA request, route to GP0

	while (DMA_CHCR(DMA_GPU) & (1 << 24))
		__asm__ volatile("");

	DMA_MADR(DMA_GPU) = (uint32_t) ot;
	DMA_BCR(DMA_GPU)  = 0;
	DMA_CHCR(DMA_GPU) = 0x01000401;
}

static void _send_buffer(
	GPU_DrawOpType type, const uint32_t *buf, size_t length
) {
	SetDrawOpType(type);
	GPU_GP1 = 0x04000002; // Enable DMA request, route to GP0

	while (DMA_CHCR(DMA_GPU) & (1 << 24))
		__asm__ volatile("");

	DMA_MADR(DMA_GPU) = (uint32_t) buf;
	DMA_BCR(DMA_GPU)  = 0x00000001 | (length << 16);
	DMA_CHCR(DMA_GPU) = 0x01000201;
}

/* Buffer and primitive drawing API */

int DrawOTag(const uint32_t *ot) {
	_sdk_validate_args(ot, -1);

	return EnqueueDrawOp(
		(void *)   &_send_linked_list,
		(uint32_t) DRAWOP_TYPE_DMA,
		(uint32_t) ot,
		0
	);
}

int DrawOTagIRQ(const uint32_t *ot) {
	_sdk_validate_args(ot, -1);

	return EnqueueDrawOp(
		(void *)   &_send_linked_list,
		(uint32_t) DRAWOP_TYPE_GPU_IRQ,
		(uint32_t) ot,
		0
	);
}

int DrawBuffer(const uint32_t *buf, size_t length) {
	_sdk_validate_args(buf && length && (length <= 0xffff), -1);

	return EnqueueDrawOp(
		(void *)   &DrawBuffer2,
		(uint32_t) DRAWOP_TYPE_DMA,
		(uint32_t) buf,
		(uint32_t) length
	);
}

int DrawBufferIRQ(const uint32_t *buf, size_t length) {
	_sdk_validate_args(buf && length && (length <= 0xffff), -1);

	return EnqueueDrawOp(
		(void *)   &DrawBuffer2,
		(uint32_t) DRAWOP_TYPE_GPU_IRQ,
		(uint32_t) buf,
		(uint32_t) length
	);
}

void DrawOTag2(const uint32_t *ot) {
	_sdk_validate_args_void(ot);

	_send_linked_list(DRAWOP_TYPE_DMA, ot);
}

void DrawOTagIRQ2(const uint32_t *ot) {
	_sdk_validate_args_void(ot);

	_send_linked_list(DRAWOP_TYPE_GPU_IRQ, ot);
}

void DrawBuffer2(const uint32_t *buf, size_t length) {
	_sdk_validate_args_void(buf && length && (length <= 0xffff));

	_send_buffer(DRAWOP_TYPE_DMA, buf, length);
}

void DrawBufferIRQ2(const uint32_t *buf, size_t length) {
	_sdk_validate_args_void(buf && length && (length <= 0xffff));

	_send_buffer(DRAWOP_TYPE_GPU_IRQ, buf, length);
}

void DrawPrim(const void *pri) {
	_sdk_validate_args_void(pri);

	DrawSync(0);
	DrawBuffer2(((const uint32_t *) pri) + 1, getlen(pri));
}

/* Helper functions */

void ClearOTagR(uint32_t *ot, size_t length) {
	_sdk_validate_args_void(ot && length);

	DMA_MADR(DMA_OTC) = (uint32_t) &ot[length - 1];
	DMA_BCR(DMA_OTC)  = length & 0xffff;
	DMA_CHCR(DMA_OTC) = 0x11000002;

	while (DMA_CHCR(DMA_OTC) & (1 << 24))
		__asm__ volatile("");
}

void ClearOTag(uint32_t *ot, size_t length) {
	_sdk_validate_args_void(ot && length);

	// DMA6 only supports writing to RAM in reverse order (last to first), so
	// the OT has to be cleared in software here. This function is thus much
	// slower than ClearOTagR().
	// https://problemkaputt.de/psx-spx.htm#dmachannels
	for (int i = 0; i < (length - 1); i++)
		ot[i] = (uint32_t) &ot[i + 1] & 0x7fffff;

	ot[length - 1] = 0xffffff;
}

void AddPrim(uint32_t *ot, const void *pri) {
	_sdk_validate_args_void(ot && pri);

	addPrim(ot, pri);
}
