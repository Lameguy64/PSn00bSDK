/*
 * PSn00bSDK SPU library (common functions)
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <assert.h>
#include <psxspu.h>
#include <hwregs_c.h>

#define _min(x, y) (((x) < (y)) ? (x) : (y))

#define WRITABLE_AREA_ADDR	0x200
#define DMA_CHUNK_LENGTH	16
#define STATUS_TIMEOUT		0x100000

/* Internal globals */

static SPU_TransferMode	_transfer_mode = SPU_TRANSFER_BY_DMA;
static uint16_t			_transfer_addr = WRITABLE_AREA_ADDR;

/* Private utilities */

static void _wait_status(uint16_t mask, uint16_t value) {
	for (int i = STATUS_TIMEOUT; i; i--) {
		if ((SPU_STAT & mask) == value)
			return;
	}

	_sdk_log("status register timeout (0x%04x)\n", SPU_STAT);
}

static size_t _dma_transfer(uint32_t *data, size_t length, int write) {
	if (length % 4)
		_sdk_log("can't transfer a number of bytes that isn't multiple of 4\n");

	length /= 4;
	if ((length >= DMA_CHUNK_LENGTH) && (length % DMA_CHUNK_LENGTH)) {
		_sdk_log("transfer data length (%d) is not a multiple of %d, rounding\n", length, DMA_CHUNK_LENGTH);
		length += DMA_CHUNK_LENGTH - 1;
	}

	SPU_CTRL &= 0xffcf; // Disable DMA request
	_wait_status(0x0030, 0x0000);

	// Enable DMA request for writing (2) or reading (3)
	uint16_t ctrl = write ? 0x0020 : 0x0030;

	SPU_ADDR  = _transfer_addr;
	SPU_CTRL |= ctrl;
	_wait_status(0x0030, ctrl);

	DMA_MADR(4) = (uint32_t) data;
	if (length < DMA_CHUNK_LENGTH)
		DMA_BCR(4) = 0x00010000 | length;
	else
		DMA_BCR(4) = DMA_CHUNK_LENGTH | ((length / DMA_CHUNK_LENGTH) << 16);

	DMA_CHCR(4) = 0x01000200 | write;
	return length;
}

static size_t _manual_write(const uint16_t *data, size_t length) {
	if (length % 2)
		_sdk_log("can't transfer a number of bytes that isn't multiple of 2\n");

	length /= 2;

	SPU_CTRL &= 0xffcf; // Disable DMA request
	_wait_status(0x0030, 0x0000);

	// Manual transfers have to be done by filling up the SPU's transfer buffer
	// and then letting the SPU empty it one 64-byte chunk at a time.
	uint16_t addr = _transfer_addr;

	while (length) {
		size_t chunk = _min(DMA_CHUNK_LENGTH * 2, length);
		length      -= chunk;

		SPU_ADDR = addr;
		addr    += chunk / 4;

		for (; chunk; chunk--)
			SPU_DATA = *(data++);

		SPU_CTRL |= 0x0010; // Manual transfer mode
		_wait_status(0x0030, 0x0010);
		_wait_status(0x0400, 0x0000);

		// This additional delay is required according to nocash docs.
		for (int i = 0; i < 1000; i++)
			__asm__ volatile("");
	}

	return length;
}

/* Public API */

void SpuInit(void) {
	SPU_CTRL = 0x0000; // SPU disabled
	_wait_status(0x001f, 0x0000);

	SPU_MASTER_VOL_L	= 0;
	SPU_MASTER_VOL_R	= 0;
	SPU_REVERB_VOL_L	= 0;
	SPU_REVERB_VOL_R	= 0;
	SPU_KEY_OFF			= 0x00ffffff;
	SPU_FM_MODE			= 0;
	SPU_NOISE_MODE		= 0;
	SPU_REVERB_ON		= 0;
	SPU_REVERB_ADDR		= 0xfffe;
	SPU_CD_VOL_L		= 0;
	SPU_CD_VOL_R		= 0;
	SPU_EXT_VOL_L		= 0;
	SPU_EXT_VOL_R		= 0;

	DMA_DPCR   |= 0x000b0000; // Enable DMA4
	DMA_CHCR(4) = 0x00000201; // Stop DMA4

	SPU_DMA_CTRL = 0x0004; // Reset transfer mode
	SPU_CTRL     = 0xc001; // Enable SPU, DAC, CD audio, disable DMA request
	_wait_status(0x003f, 0x0001);

	// Upload a dummy looping ADPCM block to the first 16 bytes of SPU RAM.
	// This may be freely used or overwritten.
	uint32_t block[4] = { 0x0500, 0, 0, 0 };

	_transfer_addr = WRITABLE_AREA_ADDR;
	_manual_write((const uint16_t *) block, 16);

	// "Play" the dummy block on all channels. This will reset the start
	// address and ADSR envelope status of each channel.
	for (int i = 0; i < 24; i++) {
		SPU_CH_VOL_L(i) = 0;
		SPU_CH_VOL_R(i) = 0;
		SPU_CH_FREQ(i)  = 0x1000;
		SPU_CH_ADDR(i)  = WRITABLE_AREA_ADDR;
	}

	// Sony's implementation leaves everything muted, however it makes sense to
	// turn up at least the master and CD audio volume by default.
	SPU_KEY_ON			= 0x00ffffff;
	SPU_MASTER_VOL_L	= 0x3fff;
	SPU_MASTER_VOL_R	= 0x3fff;
	SPU_CD_VOL_L		= 0x7fff;
	SPU_CD_VOL_R		= 0x7fff;
}

size_t SpuRead(uint32_t *data, size_t size) {
	return _dma_transfer(data, size, 0) * 4;
}

size_t SpuWrite(const uint32_t *data, size_t size) {
	if (_transfer_addr < WRITABLE_AREA_ADDR)
		return 0;

	// I/O transfer mode is not that useful, but whatever.
	if (_transfer_mode)
		return _manual_write((const uint16_t *) data, size) * 2;
	else
		return _dma_transfer((uint32_t *) data, size, 1) * 4;
}

size_t SpuWritePartly(const uint32_t *data, size_t size) {
	size_t _size = SpuWrite(data, size);

	_transfer_addr += (_size + 1) / 2;
	return _size;
}

SPU_TransferMode SpuSetTransferMode(SPU_TransferMode mode) {
	_transfer_mode = mode;
	return mode;
}

uint32_t SpuSetTransferStartAddr(uint32_t addr) {
	if (addr > 0x7ffff)
		return 0;

	_transfer_addr = (addr + 7) / 8;
	return addr;
}

int SpuIsTransferCompleted(int mode) {
	if (!mode)
		return ((SPU_STAT >> 10) & 1) ^ 1;

	_wait_status(0x0400, 0x0000);
	return 1;
}
