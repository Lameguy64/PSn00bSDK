#ifndef __PSXSPU_H
#define __PSXSPU_H

#include <sys/types.h>

// Mask settings bits for specifying voice channels

#define SPU_00CH	(1<<0)
#define SPU_01CH	(1<<1)
#define SPU_02CH	(1<<2)
#define SPU_03CH	(1<<3)
#define SPU_04CH	(1<<4)
#define	SPU_05CH	(1<<5)
#define SPU_06CH	(1<<6)
#define SPU_07CH	(1<<7)
#define SPU_08CH	(1<<8)
#define SPU_09CH	(1<<9)
#define SPU_10CH	(1<<10)
#define SPU_11CH	(1<<11)
#define SPU_12CH	(1<<12)
#define SPU_13CH	(1<<13)
#define SPU_14CH	(1<<14)
#define SPU_15CH	(1<<15)
#define SPU_16CH	(1<<16)
#define SPU_17CH	(1<<17)
#define SPU_18CH	(1<<18)
#define SPU_19CH	(1<<19)
#define SPU_20CH	(1<<20)
#define SPU_21CH	(1<<21)
#define SPU_22CH	(1<<22)
#define SPU_23CH	(1<<23)

#define SPU_0CH		SPU_00CH
#define SPU_1CH		SPU_01CH
#define SPU_2CH		SPU_02CH
#define SPU_3CH		SPU_03CH
#define SPU_4CH		SPU_04CH
#define SPU_5CH		SPU_05CH
#define SPU_6CH		SPU_06CH
#define SPU_7CH		SPU_07CH
#define SPU_8CH		SPU_08CH
#define SPU_9CH		SPU_09CH

#define SPU_KEYCH(x)	(1<<(x))
#define SPU_VOICECH(x)	SPU_KEYCH(x)


// Mask setting bits for SpuVoiceAttr.mask

#define SPU_VOICE_VOLL			(1<<0)	// Left volume
#define SPU_VOICE_VOLR			(1<<1)	// Right volume
#define SPU_VOICE_VOLMODEL		(1<<2)	// Left volume mode
#define SPU_VOICE_VOLMODER		(1<<3)	// Right volume mode
#define SPU_VOICE_PITCH			(1<<4)	// Pitch tone
#define SPU_VOICE_NOTE			(1<<5)	// Pitch note
#define SPU_VOICE_SAMPLE_NOTE	(1<<6)	// Sample base frequency?
#define SPU_VOICE_WDSA			(1<<7)	// Sample start address (in SPU RAM)
#define SPU_VOICE_ADSR_AMODE	(1<<8)	// ADSR attack mode
#define SPU_VOICE_ADSR_SMODE	(1<<9)	// ADSR sustain mode
#define SPU_VOICE_ADSR_RMODE	(1<<10)	// ADSR release mode
#define SPU_VOICE_ADSR_AR		(1<<11)	// ADSR attack rate
#define SPU_VOICE_ADSR_DR		(1<<12)	// ADSR decay rate
#define SPU_VOICE_ADSR_SR		(1<<13)	// ADSR sustain rate
#define SPU_VOICE_ADSR_RR		(1<<14)	// ADSR release rate
#define SPU_VOICE_ADSR_SL		(1<<15) // ADSR sustain level
#define SPU_VOICE_LSAX			(1<<16)	// Loop start address (in SPU RAM)
#define SPU_VOICE_ADSR_ADSR1	(1<<17)	// adsr1 for VagAtr (?)
#define SPU_VOICE_ADSR_ADSR2	(1<<18) // adsr2 for VagAtr (?)


#define	SPU_TRANSFER_BY_DMA		0


typedef struct {
	short left;
	short right;
} SpuVolume;

typedef struct {
	SpuVolume		vol;			// 0
	unsigned short	freq;			// 4
	unsigned short	addr;			// 6
	unsigned short	loop_addr;		// 8
	unsigned short	res;			// 10
	unsigned int	adsr_param;		// 12
} SpuVoiceRaw;

typedef struct {
	u_int voice;
	u_int mask;
	SpuVolume volume;
	SpuVolume volmode;
	SpuVolume volumex;
	u_short	pitch;
	u_short	note;
	u_short	sample_note;
	short envx;
	u_int addr;
	u_int loop_addr;
	int	a_mode;
	int	s_mode;
	int	r_mode;
	u_short	ar;
	u_short	dr;
	u_short	sr;
	u_short	rr;
	u_short	sl;
	u_short	adsr1;
	u_short	adsr2;
} SpuVoiceAttr;

#ifdef __cplusplus
extern "C" {
#endif

void SpuInit();

void SpuSetVoiceRaw( int voice, SpuVoiceRaw* param );
void SpuReverbOn( int voice );
void SpuSetReverb();

void SpuSetReverbAddr( int addr );
void SpuSetReverbVolume( int left, int right );


void SpuSetKey(int on_off, u_int voice_bit);

// SPU transfer functions
int SpuSetTransferMode(int mode);
int SpuSetTransferStartAddr(int addr);
int SpuWrite(unsigned char* addr, int size);
void SpuWait();

#ifdef __cplusplus
}
#endif

#endif