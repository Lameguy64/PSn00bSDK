/*
 * PSn00bSDK MDEC library (low-level MDEC/DMA API)
 * (C) 2022-2023 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <assert.h>
#include <psxetc.h>
#include <psxpress.h>
#include <hwregs_c.h>

#define DMA_CHUNK_LENGTH	32
#define MDEC_SYNC_TIMEOUT	0x100000

/* Default IDCT matrix and quantization tables */

#define S0 0x5a82	// (1 << 14) * cos(0/16 * pi) * sqrt(2)
#define S1 0x7d8a	// (1 << 14) * cos(1/16 * pi) * 2
#define S2 0x7641	// (1 << 14) * cos(2/16 * pi) * 2
#define S3 0x6a6d	// (1 << 14) * cos(3/16 * pi) * 2
#define S4 0x5a82	// (1 << 14) * cos(4/16 * pi) * 2
#define S5 0x471c	// (1 << 14) * cos(5/16 * pi) * 2
#define S6 0x30fb	// (1 << 14) * cos(6/16 * pi) * 2
#define S7 0x18f8	// (1 << 14) * cos(7/16 * pi) * 2

static const DECDCTENV _default_mdec_env = {
	// The default luma and chroma quantization table is based on the MPEG-1
	// quantization table, with the only difference being the first value (2
	// instead of 8). Note that quantization tables are stored in zigzag order
	// rather than row- or column-major.
	// https://problemkaputt.de/psx-spx.htm#mdecdecompression
	.iq_y = {
		 2, 16, 16, 19, 16, 19, 22, 22,
		22, 22, 22, 22, 26, 24, 26, 27,
		27, 27, 26, 26, 26, 26, 27, 27,
		27, 29, 29, 29, 34, 34, 34, 29,
		29, 29, 27, 27, 29, 29, 32, 32,
		34, 34, 37, 38, 37, 35, 35, 34,
		35, 38, 38, 40, 40, 40, 48, 48,
		46, 46, 56, 56, 58, 69, 69, 83
	},
	.iq_c = {
		 2, 16, 16, 19, 16, 19, 22, 22,
		22, 22, 22, 22, 26, 24, 26, 27,
		27, 27, 26, 26, 26, 26, 27, 27,
		27, 29, 29, 29, 34, 34, 34, 29,
		29, 29, 27, 27, 29, 29, 32, 32,
		34, 34, 37, 38, 37, 35, 35, 34,
		35, 38, 38, 40, 40, 40, 48, 48,
		46, 46, 56, 56, 58, 69, 69, 83
	},
	/*.iq_y = {
		 16,  11,  12,  14,  12,  10,  16,  14,
		 13,  14,  18,  17,  16,  19,  24,  40,
		 26,  24,  22,  22,  24,  49,  35,  37,
		 29,  40,  58,  51,  61,  60,  57,  51,
		 56,  55,  64,  72,  92,  78,  64,  68,
		 87,  69,  55,  56,  80, 109,  81,  87,
		 95,  98, 103, 104, 103,  62,  77, 113,
		121, 112, 100, 120,  92, 101, 103,  99
	},
	.iq_c = {
		 17,  18,  18,  24,  21,  24,  47,  26,
		 26,  47,  99,  66,  56,  66,  99,  99,
		 99,  99,  99,  99,  99,  99,  99,  99,
		 99,  99,  99,  99,  99,  99,  99,  99,
		 99,  99,  99,  99,  99,  99,  99,  99,
		 99,  99,  99,  99,  99,  99,  99,  99,
		 99,  99,  99,  99,  99,  99,  99,  99,
		 99,  99,  99,  99,  99,  99,  99,  99
	},*/
	.dct = {
		S0,  S0,  S0,  S0,  S0,  S0,  S0,  S0,
		S1,  S3,  S5,  S7, -S7, -S5, -S3, -S1,
		S2,  S6, -S6, -S2, -S2, -S6,  S6,  S2,
		S3, -S7, -S1, -S5,  S5,  S1,  S7, -S3,
		S4, -S4, -S4,  S4,  S4, -S4, -S4,  S4,
		S5, -S1,  S7,  S3, -S3, -S7,  S1, -S5,
		S6, -S2,  S2, -S6, -S6,  S2, -S2,  S6,
		S7, -S5,  S3, -S1,  S1, -S3,  S5, -S7
	}
};

/* Public API */

void DecDCTReset(int mode) {
	SetDMAPriority(DMA_MDEC_IN,  3);
	SetDMAPriority(DMA_MDEC_OUT, 3);
	DMA_CHCR(DMA_MDEC_IN)  = 0x00000201; // Stop DMA
	DMA_CHCR(DMA_MDEC_OUT) = 0x00000200; // Stop DMA

	MDEC1 = 0x80000000; // Reset MDEC
	MDEC1 = 0x60000000; // Enable DMA in/out requests

	if (!mode)
		DecDCTPutEnv(0, 0);
}

void DecDCTPutEnv(const DECDCTENV *env, int mono) {
	DecDCTinSync(0);
	if (!env)
		env = &_default_mdec_env;

	MDEC0 = 0x60000000; // Set IDCT matrix
	DecDCTinRaw((const uint32_t *) env->dct, 32);
	DecDCTinSync(0);

	MDEC0 = 0x40000000 | (mono ? 0 : 1); // Set quantization table(s)
	DecDCTinRaw((const uint32_t *) env->iq_y, mono ? 16 : 32);
	DecDCTinSync(0);
}

void DecDCTin(const uint32_t *data, int mode) {
	_sdk_validate_args_void(data);

	uint32_t header = *data;
	DecDCTinSync(0);

	if (mode == DECDCT_MODE_RAW)
		MDEC0 = header;
	else if (mode & DECDCT_MODE_24BPP)
		MDEC0 = 0x30000000 | (header & 0xffff);
	else
		MDEC0 = 0x38000000 | (header & 0xffff) | ((mode & 2) << 24); // Bit 25 = mask

	DecDCTinRaw((const uint32_t *) &(data[1]), header & 0xffff);
}

// This is a PSn00bSDK-only function that behaves like DecDCTout(), taking the
// data length as an argument rather than parsing it from the first 4 bytes of
// the stream.
void DecDCTinRaw(const uint32_t *data, size_t length) {
	_sdk_validate_args_void(data && length);

	if ((length >= DMA_CHUNK_LENGTH) && (length % DMA_CHUNK_LENGTH)) {
		_sdk_log("input data length (%d) is not a multiple of %d, rounding\n", length, DMA_CHUNK_LENGTH);
		length += DMA_CHUNK_LENGTH - 1;
	}

	DMA_MADR(DMA_MDEC_IN) = (uint32_t) data;
	if (length < DMA_CHUNK_LENGTH)
		DMA_BCR(DMA_MDEC_IN) = 0x00010000 | length;
	else
		DMA_BCR(DMA_MDEC_IN) = DMA_CHUNK_LENGTH |
			((length / DMA_CHUNK_LENGTH) << 16);

	DMA_CHCR(DMA_MDEC_IN) = 0x01000201;
}

int DecDCTinSync(int mode) {
	if (mode)
		return (MDEC1 >> 29) & 1;

	for (int i = MDEC_SYNC_TIMEOUT; i; i--) {
		if (!(MDEC1 & (1 << 29)))
			return 0;
	}

	_sdk_log("DecDCTinSync() timeout, MDEC1=0x%08x\n", MDEC1);
	return -1;
}

void DecDCTout(uint32_t *data, size_t length) {
	_sdk_validate_args_void(data && length);

	DecDCToutSync(0);

	if ((length >= DMA_CHUNK_LENGTH) && (length % DMA_CHUNK_LENGTH)) {
		_sdk_log("output data length (%d) is not a multiple of %d, rounding\n", length, DMA_CHUNK_LENGTH);
		length += DMA_CHUNK_LENGTH - 1;
	}

	DMA_MADR(DMA_MDEC_OUT) = (uint32_t) data;
	if (length < DMA_CHUNK_LENGTH)
		DMA_BCR(DMA_MDEC_OUT) = 0x00010000 | length;
	else
		DMA_BCR(DMA_MDEC_OUT) = DMA_CHUNK_LENGTH |
			((length / DMA_CHUNK_LENGTH) << 16);

	DMA_CHCR(DMA_MDEC_OUT) = 0x01000200;
}

int DecDCToutSync(int mode) {
	if (mode)
		return (DMA_CHCR(DMA_MDEC_OUT) >> 24) & 1;

	for (int i = MDEC_SYNC_TIMEOUT; i; i--) {
		if (!(DMA_CHCR(DMA_MDEC_OUT) & (1 << 24)))
			return 0;
	}

	_sdk_log("DecDCToutSync() timeout, CHCR=0x%08x\n", DMA_CHCR(DMA_MDEC_OUT));
	return -1;
}
