/*
 * PSn00bSDK SPU library
 * (C) 2019-2023 Lameguy64, spicyjpeg - MPL licensed
 */

/**
 * @file psxspu.h
 * @brief SPU library header
 *
 * @details The PSn00bSDK SPU library allows for SPU initialization, DMA
 * transfers (both sample data uploads and capture buffer reads) and provides
 * helper macros for accessing SPU control registers, which can be used to
 * control sample playback on each channel, configure reverb and enable more
 * advanced features such as interrupts.
 *
 * This library currently has fewer functions than its Sony SDK counterpart, in
 * part because it is not yet complete but also since the vast majority of the
 * Sony library's functions are redundant, inefficient and can be replaced with
 * simple SPU register writes.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <hwregs_c.h>

/* Definitions */

#if 0
typedef enum _SPU_AttrMask {
	SPU_VOICE_VOLL			= 1 << 0,	// Left volume
	SPU_VOICE_VOLR			= 1 << 1,	// Right volume
	SPU_VOICE_VOLMODEL		= 1 << 2,	// Left volume mode
	SPU_VOICE_VOLMODER		= 1 << 3,	// Right volume mode
	SPU_VOICE_PITCH			= 1 << 4,	// Pitch tone
	SPU_VOICE_NOTE			= 1 << 5,	// Pitch note
	SPU_VOICE_SAMPLE_NOTE	= 1 << 6,	// Sample base frequency?
	SPU_VOICE_WDSA			= 1 << 7,	// Sample start address (in SPU RAM)
	SPU_VOICE_ADSR_AMODE	= 1 << 8,	// ADSR attack mode
	SPU_VOICE_ADSR_SMODE	= 1 << 9,	// ADSR sustain mode
	SPU_VOICE_ADSR_RMODE	= 1 << 10,	// ADSR release mode
	SPU_VOICE_ADSR_AR		= 1 << 11,	// ADSR attack rate
	SPU_VOICE_ADSR_DR		= 1 << 12,	// ADSR decay rate
	SPU_VOICE_ADSR_SR		= 1 << 13,	// ADSR sustain rate
	SPU_VOICE_ADSR_RR		= 1 << 14,	// ADSR release rate
	SPU_VOICE_ADSR_SL		= 1 << 15,	// ADSR sustain level
	SPU_VOICE_LSAX			= 1 << 16,	// Loop start address (in SPU RAM)
	SPU_VOICE_ADSR_ADSR1	= 1 << 17,
	SPU_VOICE_ADSR_ADSR2	= 1 << 18
} SPU_AttrMask;
#endif

typedef enum _SPU_TransferMode {
	SPU_TRANSFER_BY_DMA	= 0,
	SPU_TRANSFER_BY_IO	= 1
} SPU_TransferMode;

typedef enum _SPU_WaitMode {
	SPU_TRANSFER_PEEK	= 0,
	SPU_TRANSFER_WAIT	= 1
} SPU_WaitMode;

/* Structure definitions */

#if 0
typedef struct _SpuVolume {
	int16_t left, right;
} SpuVolume;

typedef struct _SpuVoiceAttr {
	uint32_t	voice;
	uint32_t	mask;
	SpuVolume	volume, volmode, volumex;
	uint16_t	pitch, note, sample_note;
	int16_t		envx;
	uint32_t	addr, loop_addr;
	int			a_mode, s_mode, r_mode;
	uint16_t	ar, dr, sr, rr, sl;
	uint16_t	adsr1, adsr2;
} SpuVoiceAttr;

typedef struct _SpuExtAttr {
	SpuVolume	volume;
	int			reverb, mix;
} SpuExtAttr;

typedef struct _SpuCommonAttr {
	uint32_t	mask;
	SpuVolume	mvol, mvolmode, mvolx;
	SpuExtAttr	cd, ext;
} SpuCommonAttr;
#endif

/* Macros */

#define getSPUAddr(addr)		((uint16_t) (((addr) + 7) / 8))
#define getSPUSampleRate(rate)	((uint16_t) (((rate) * (1 << 12)) / 44100))

#define getSPUADSR(ar, dr, sr, rr, sl) ( \
	(sl) | \
	((dr) <<  4) | \
	((ar) <<  8) | \
	((rr) << 16) | \
	((sr) << 22) | \
	(1    << 30) \
)

/* "Useless" macros for official SDK compatibility */

#define SpuSetCommonMasterVolume(left, right) \
	(SPU_MASTER_VOL_L = (left), SPU_MASTER_VOL_R = (right))
#define SpuSetCommonCDVolume(left, right) \
	(SPU_CD_VOL_L = (left), SPU_CD_VOL_R = (right))
#define SpuSetCommonCDReverb(enable) \
	((enable) ? (SPU_CTRL |= 0x0004) : (SPU_CTRL &= 0xfffb))
#define SpuSetCommonExtVolume(left, right) \
	(SPU_EXT_VOL_L = (left), SPU_EXT_VOL_R = (right))
#define SpuSetCommonExtReverb(enable) \
	((enable) ? (SPU_CTRL |= 0x0002) : (SPU_CTRL &= 0xfffd))

#define SpuSetReverbAddr(addr) \
	(SPU_REVERB_ADDR = getSPUAddr(addr))
#define SpuSetIRQAddr(addr) \
	(SPU_IRQ_ADDR = getSPUAddr(addr))

#define SpuSetVoiceVolume(ch, left, right) \
	(SPU_CH_VOL_L(ch) = (left), SPU_CH_VOL_R(ch) = (right))
#define SpuSetVoicePitch(ch, pitch) \
	(SPU_CH_FREQ(ch) = (pitch))
#define SpuSetVoiceStartAddr(ch, addr) \
	(SPU_CH_ADDR(ch) = getSPUAddr(addr))
#define SpuSetVoiceADSR(ch, ar, dr, sr, rr, sl) ( \
	SPU_CH_ADSR1(ch) = (sl) | ((dr) << 4) | ((ar) << 8), \
	SPU_CH_ADSR2(ch) = (rr) | ((sr) << 6) | (1 << 14) \
)

#define SpuSetKey(enable, voice_bit) \
	((enable) ? ( \
		SPU_KEY_ON1 = (uint16_t) (voice_bit), \
		SPU_KEY_ON2 = (uint16_t) ((voice_bit) >> 16) \
	) : ( \
		SPU_KEY_OFF1 = (uint16_t) (voice_bit), \
		SPU_KEY_OFF2 = (uint16_t) ((voice_bit) >> 16) \
	))

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

void SpuInit(void);

size_t SpuRead(uint32_t *data, size_t size);
size_t SpuWrite(const uint32_t *data, size_t size);
size_t SpuWritePartly(const uint32_t *data, size_t size);
SPU_TransferMode SpuSetTransferMode(SPU_TransferMode mode);
SPU_TransferMode SpuGetTransferMode(void);
uint32_t SpuSetTransferStartAddr(uint32_t addr);
uint32_t SpuGetTransferStartAddr(void);
int SpuIsTransferCompleted(int mode);

#ifdef __cplusplus
}
#endif
