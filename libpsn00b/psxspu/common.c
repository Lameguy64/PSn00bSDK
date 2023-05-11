/*
 * PSn00bSDK SPU library (common functions)
 * (C) 2022-2023 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <assert.h>
#include <psxetc.h>
#include <psxspu.h>
#include <hwregs_c.h>

#define _min(x, y) (((x) < (y)) ? (x) : (y))

#define WRITABLE_AREA_ADDR	0x200
#define DMA_CHUNK_LENGTH	16
#define STATUS_TIMEOUT		0x100000

static const uint32_t _dummy_block[4] = {
	0x00000500, 0x00000000, 0x00000000, 0x00000000
};

/* Internal globals */

static SPU_TransferMode	_transfer_mode = SPU_TRANSFER_BY_DMA;
static uint16_t			_transfer_addr = WRITABLE_AREA_ADDR;

/* Private utilities */

static void _wait_status(uint16_t mask, uint16_t value) {
	for (int i = STATUS_TIMEOUT; i; i--) {
		if ((SPU_STAT & mask) == value)
			return;
	}

	_sdk_log("timeout, status=0x%04x\n", SPU_STAT);
}

static size_t _dma_transfer(uint32_t *data, size_t length, int write) {
	if (length % 4)
		_sdk_log("can't transfer a number of bytes that isn't multiple of 4\n");

	length /= 4;
	if ((length >= DMA_CHUNK_LENGTH) && (length % DMA_CHUNK_LENGTH)) {
		_sdk_log("transfer data length (%d) is not a multiple of %d, rounding\n", length, DMA_CHUNK_LENGTH);
		length += DMA_CHUNK_LENGTH - 1;
	}

	// Increase bus delay for DMA reads
	BUS_SPU_CFG &= ~(0xf << 24);
	if (!write)
		BUS_SPU_CFG = 2 << 24;

	SPU_CTRL &= 0xffcf; // Disable DMA request
	_wait_status(0x0030, 0x0000);

	// Enable DMA request for writing (2) or reading (3)
	uint16_t ctrl = write ? 0x0020 : 0x0030;

	SPU_ADDR  = _transfer_addr;
	SPU_CTRL |= ctrl;
	_wait_status(0x0030, ctrl);

	DMA_MADR(DMA_SPU) = (uint32_t) data;
	if (length < DMA_CHUNK_LENGTH)
		DMA_BCR(DMA_SPU) = 0x00010000 | length;
	else
		DMA_BCR(DMA_SPU) = DMA_CHUNK_LENGTH |
			((length / DMA_CHUNK_LENGTH) << 16);

	DMA_CHCR(DMA_SPU) = 0x01000200 | write;
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
	BUS_SPU_CFG = 0x200931e1;

	SPU_CTRL = 0x0000; // SPU disabled
	_wait_status(0x001f, 0x0000);

	SPU_MASTER_VOL_L	= 0;
	SPU_MASTER_VOL_R	= 0;
	SPU_REVERB_VOL_L	= 0;
	SPU_REVERB_VOL_R	= 0;
	SPU_KEY_OFF1		= 0xffff;
	SPU_KEY_OFF2		= 0x00ff;
	SPU_FM_MODE1		= 0;
	SPU_FM_MODE2		= 0;
	SPU_NOISE_MODE1		= 0;
	SPU_NOISE_MODE2		= 0;
	SPU_REVERB_ON1		= 0;
	SPU_REVERB_ON2		= 0;
	SPU_REVERB_ADDR		= 0xfffe;
	SPU_CD_VOL_L		= 0;
	SPU_CD_VOL_R		= 0;
	SPU_EXT_VOL_L		= 0;
	SPU_EXT_VOL_R		= 0;

	SetDMAPriority(DMA_SPU, 3);
	DMA_CHCR(DMA_SPU) = 0x00000201; // Stop DMA

	SPU_DMA_CTRL = 0x0004; // Reset transfer mode
	SPU_CTRL     = 0xc001; // Enable SPU, DAC, CD audio, disable DMA request
	_wait_status(0x003f, 0x0001);

	// Upload a dummy looping ADPCM block to the first 16 bytes of SPU RAM.
	// This may be freely used or overwritten.
	_transfer_addr = WRITABLE_AREA_ADDR;
	_manual_write((const uint16_t *) _dummy_block, 16);

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
	SPU_KEY_ON1			= 0xffff;
	SPU_KEY_ON2			= 0x00ff;
	SPU_MASTER_VOL_L	= 0x3fff;
	SPU_MASTER_VOL_R	= 0x3fff;
	SPU_CD_VOL_L		= 0x7fff;
	SPU_CD_VOL_R		= 0x7fff;
}

size_t SpuRead(uint32_t *data, size_t size) {
	_sdk_validate_args(data && size, 0);

	return _dma_transfer(data, size, 0) * 4;
}

size_t SpuWrite(const uint32_t *data, size_t size) {
	_sdk_validate_args(data && size, 0);

	if (_transfer_addr < WRITABLE_AREA_ADDR) {
		_sdk_log("ignoring attempt to write to capture buffers at 0x%05x\n", _transfer_addr);
		return 0;
	}

	// I/O transfer mode is not that useful, but whatever.
	if (_transfer_mode)
		return _manual_write((const uint16_t *) data, size) * 2;
	else
		return _dma_transfer((uint32_t *) data, size, 1) * 4;
}

size_t SpuWritePartly(const uint32_t *data, size_t size) {
	//_sdk_validate_args(data && size, 0);

	size_t _size = SpuWrite(data, size);

	_transfer_addr += (_size + 1) / 2;
	return _size;
}

SPU_TransferMode SpuSetTransferMode(SPU_TransferMode mode) {
	_transfer_mode = mode;
	return mode;
}

SPU_TransferMode SpuGetTransferMode(void) {
	return _transfer_mode;
}

uint32_t SpuSetTransferStartAddr(uint32_t addr) {
	if (addr > 0x7ffff)
		return 0;

	_transfer_addr = getSPUAddr(addr);
	return addr;
}

uint32_t SpuGetTransferStartAddr(void) {
	return _transfer_addr * 8;
}

int SpuIsTransferCompleted(int mode) {
	if (!mode)
		return ((SPU_STAT >> 10) & 1) ^ 1;

	_wait_status(0x0400, 0x0000);
	return 1;
}
