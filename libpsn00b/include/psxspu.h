/*
 * PSn00bSDK SPU library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __PSXSPU_H
#define __PSXSPU_H

#include <stdint.h>
#include <stddef.h>
#include <hwregs_c.h>

/* Definitions */

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

typedef enum _SPU_TransferMode {
	SPU_TRANSFER_BY_DMA	= 0,
	SPU_TRANSFER_BY_IO	= 1
} SPU_TransferMode;

typedef enum _SPU_WaitMode {
	SPU_TRANSFER_PEEK	= 0,
	SPU_TRANSFER_WAIT	= 1
} SPU_WaitMode;

/* Structure definitions */

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
	(SPU_REVERB_ADDR = ((addr) + 7) / 8)
#define SpuSetIRQAddr(addr) \
	(SPU_IRQ_ADDR = ((addr) + 7) / 8)

#define SpuSetVoiceVolume(ch, left, right) \
	(SPU_CH_VOL_L(ch) = (left), SPU_CH_VOL_R(ch) = (right))
#define SpuSetVoicePitch(ch, pitch) \
	(SPU_CH_FREQ(ch) = (pitch))
#define SpuSetVoiceStartAddr(ch, addr) \
	(SPU_CH_ADDR(ch) = ((addr) + 7) / 8)
#define SpuSetVoiceADSR(ch, ar, dr, sr, rr, sl) \
	(SPU_CH_ADSR(ch) = ((sl)) | ((dr) << 4) | ((ar) << 8) | ((rr) << 16) | ((sr) << 22) | (1 << 30))

#define SpuSetKey(enable, voice_bit) \
	((enable) ? (SPU_KEY_ON = (voice_bit)) : (SPU_KEY_OFF = (voice_bit)))

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

void SpuInit(void);

void SpuRead(uint32_t *data, size_t size);
void SpuWrite(const uint32_t *data, size_t size);
SPU_TransferMode SpuSetTransferMode(SPU_TransferMode mode);
uint32_t SpuSetTransferStartAddr(uint32_t addr);
int SpuIsTransferCompleted(int mode);

#ifdef __cplusplus
}
#endif

#endif
