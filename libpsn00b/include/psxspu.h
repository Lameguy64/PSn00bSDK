/*
 * PSn00bSDK SPU library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __PSXSPU_H
#define __PSXSPU_H

#include <stdint.h>

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
	SPU_TRANSFER_BY_DMA		= 0,
	SPU_TRANSFER_BY_IO		= 1
} SPU_TransferMode;

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

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

void SpuInit(void);

void SpuReverbOn(int voice);
void SpuSetReverbAddr(int addr);
void SpuSetReverbVolume(int left, int right);
void SpuSetKey(int on_off, uint32_t voice_bit);

int SpuSetTransferMode(int mode);
int SpuSetTransferStartAddr(int addr);
int SpuWrite(const uint8_t *addr, int size);
void SpuWait(void);

#ifdef __cplusplus
}
#endif

#endif
