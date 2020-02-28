#ifndef _LIBPSXCD_H
#define _LIBPSXCD_H

#include <sys/types.h>

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
#define CdlModeSize0		0x10
#define CdlModeSize1		0x20
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

#define btoi(b)		((b)/16*10+(b)%16)	/* Convert BCD value to integer */
#define itob(i)		((i)/10*16+(i)%10)	/* Convert integer to BCD value */

/*
 * CD-ROM disc location struct
 */
typedef struct _CdlLOC
{
	u_char	minute;
	u_char	second;
	u_char	sector;
	u_char	track;
} CdlLOC;

/*
 * CD-ROM audio attenuation struct (volume)
 */
typedef struct _CdlATV
{
	u_char	val0;	/* L -> SPU L */
	u_char	val1;	/* L -> SPU R */
	u_char	val2;	/* R -> SPU R */
	u_char	val3;	/* R -> SPU L */
} CdlATV;

/*
 * CD-ROM file information struct
 */
typedef struct _CdlFILE
{
	CdlLOC	loc;
	u_int	size;
	char	name[16];
} CdlFILE;

typedef struct _CdlFILTER
{
	u_char	file;
	u_char	chan;
	u_short	pad;
} CdlFILTER;

/* Directory query context */
typedef void* CdlDIR;

/* Data callback */
typedef void (*CdlCB)(int, unsigned char *);

#ifdef __cplusplus
extern "C" {
#endif

int		CdInit(int mode);

CdlLOC*	CdIntToPos(int i, CdlLOC *p);
int		CdPosToInt(CdlLOC *p);
int		CdGetToc(CdlLOC *toc);

int		CdControl(unsigned char com, unsigned char *param, unsigned char *result);
int		CdControlB(unsigned char com, unsigned char *param, unsigned char *result);
int		CdControlF(unsigned char com, unsigned char *param);
int		CdSync(int mode, unsigned char *result);
unsigned int CdSyncCallback(CdlCB func);

long	CdReadyCallback(CdlCB func);
int		CdGetSector(void *madr, int size);

CdlFILE* CdSearchFile(CdlFILE *loc, const char *filename);

int		CdRead(int sectors, unsigned int *buf, int mode);
int		CdReadSync(int mode, unsigned char *result);
unsigned int CdReadCallback(CdlCB func);

int		CdStatus(void);
int		CdMode(void);

int		CdMix(CdlATV *vol);

/* ORIGINAL CODE */
CdlDIR*	CdOpenDir(const char* path);
int		CdReadDir(CdlDIR* dir, CdlFILE* file);
void	CdCloseDir(CdlDIR* dir);

int		CdGetVolumeLabel(char* label);

long*	CdAutoPauseCallback(void(*func)());
int		CdIsoError();

#ifdef __cplusplus
}
#endif

#endif /* _LIBPSXCD_H */
