/*
 * PSn00bSDK CD-ROM drive library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __PSXCD_H
#define __PSXCD_H

#include <stdint.h>

/*
 * CD-ROM control commands
 */
#define CdlNop				0x01	/* a.k.a. Getstat */
#define CdlSetloc			0x02
#define CdlPlay				0x03
#define CdlForward			0x04
#define CdlBackward			0x05
#define CdlReadN			0x06
#define CdlStandby			0x07	/* a.k.a. MotorOn */
#define CdlStop				0x08
#define CdlPause			0x09
#define CdlInit				0x0A
#define CdlMute				0x0B
#define CdlDemute			0x0C
#define CdlSetfilter		0x0D
#define CdlSetmode			0x0E
#define CdlGetparam			0x0F
#define CdlGetlocL			0x10
#define CdlGetlocP			0x11
#define CdlSetsession		0x12	/* ORIGINAL CODE */
#define CdlGetTN			0x13
#define CdlGetTD			0x14
#define CdlSeekL			0x15
#define CdlSeekP			0x16
#define CdlTest				0x19	/* ORIGINAL CODE */
#define CdlReadS			0x1B

/*
 * CD-ROM status bits
 */
#define CdlStatError		0x01
#define CdlStatStandby		0x02
#define CdlStatSeekError	0x04
#define CdlStatIdError		0x08	/* ORIGINAL CODE */
#define CdlStatShellOpen	0x10
#define CdlStatRead			0x20
#define CdlStatSeek			0x40
#define CdlStatPlay			0x80

/*
 * CD-ROM mode bits
 */
#define CdlModeDA			0x01
#define CdlModeAP			0x02
#define CdlModeRept			0x04
#define CdlModeSF			0x08
//#define CdlModeSize0		0x10
//#define CdlModeSize1		0x20
#define CdlModeIgnore		0x10
#define CdlModeSize			0x20
#define CdlModeRT			0x40
#define CdlModeSpeed		0x80

/*
 * CD-ROM interrupt result values
 */
#define CdlNoIntr			0x00
#define CdlDataReady		0x01
#define CdlComplete			0x02
#define CdlAcknowledge		0x03
#define CdlDataEnd			0x04
#define CdlDiskError		0x05

/*
 * CD-ROM file system error codes (original)
 */
#define CdlIsoOkay			0x00
#define CdlIsoSeekError		0x01
#define CdlIsoReadError		0x02
#define CdlIsoInvalidFs		0x03
#define CdlIsoLidOpen		0x04

#define btoi(b) ((b)/16*10+(b)%16)	/* Convert BCD value to integer */
#define itob(i) ((i)/10*16+(i)%10)	/* Convert integer to BCD value */

/*
 * CD-ROM disc location struct
 */
typedef struct _CdlLOC
{
	uint8_t minute;
	uint8_t second;
	uint8_t sector;
	uint8_t track;
} CdlLOC;

/*
 * CD-ROM audio attenuation struct (volume)
 */
typedef struct _CdlATV
{
	uint8_t val0;	/* L -> SPU L */
	uint8_t val1;	/* L -> SPU R */
	uint8_t val2;	/* R -> SPU R */
	uint8_t val3;	/* R -> SPU L */
} CdlATV;

/*
 * CD-ROM file information struct
 */
typedef struct _CdlFILE
{
	CdlLOC		pos;
	uint32_t	size;
	char		name[16];
} CdlFILE;

typedef struct _CdlFILTER
{
	uint8_t		file;
	uint8_t		chan;
	uint16_t	pad;
} CdlFILTER;

/* Directory query context */
typedef void* CdlDIR;

/* Data callback */
typedef void (*CdlCB)(int, uint8_t *);

#ifdef __cplusplus
extern "C" {
#endif

int CdInit(void);

CdlLOC* CdIntToPos(int i, CdlLOC *p);
int CdPosToInt(CdlLOC *p);
int CdGetToc(CdlLOC *toc);

int CdControl(uint8_t com, const void *param, uint8_t *result);
int CdControlB(uint8_t com, const void *param, uint8_t *result);
int CdControlF(uint8_t com, const void *param);
int CdSync(int mode, uint8_t *result);
uint32_t CdSyncCallback(CdlCB func);

long CdReadyCallback(CdlCB func);
int CdGetSector(void *madr, int size);
int CdGetSector2(void *madr, int size);
int CdDataSync(int mode);

CdlFILE* CdSearchFile(CdlFILE *loc, const char *filename);

int CdRead(int sectors, uint32_t *buf, int mode);
int CdReadSync(int mode, uint8_t *result);
uint32_t CdReadCallback(CdlCB func);

int CdStatus(void);
int CdMode(void);

int CdMix(CdlATV *vol);

/* ORIGINAL CODE */
CdlDIR* CdOpenDir(const char* path);
int CdReadDir(CdlDIR* dir, CdlFILE* file);
void CdCloseDir(CdlDIR* dir);

int CdGetVolumeLabel(char* label);

long* CdAutoPauseCallback(void(*func)());
int CdIsoError();

int CdLoadSession(int session);

#ifdef __cplusplus
}
#endif

#endif
