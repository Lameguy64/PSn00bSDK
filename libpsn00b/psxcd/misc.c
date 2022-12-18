/*
 * PSn00bSDK CD-ROM library (misc. functions)
 * (C) 2020-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <psxetc.h>
#include <psxcd.h>
#include <hwregs_c.h>

#define DATA_SYNC_TIMEOUT	0x100000

/* Private types */

typedef struct {
	uint8_t status, first_track, last_track;
} TrackInfo;

/* Sector DMA transfer functions */

int CdGetSector(void *madr, int size) {
	//while (!(CD_REG(0) & (1 << 6)))
		//__asm__ volatile("");

	DMA_MADR(DMA_CD) = (uint32_t) madr;
	DMA_BCR(DMA_CD)  = size | (1 << 16);
	DMA_CHCR(DMA_CD) = 0x11000000;

	while (DMA_CHCR(DMA_CD) & (1 << 24))
		__asm__ volatile("");

	return 1;
}

int CdGetSector2(void *madr, int size) {
	//while (!(CD_REG(0) & (1 << 6)))
		//__asm__ volatile("");

	DMA_MADR(DMA_CD) = (uint32_t) madr;
	DMA_BCR(DMA_CD)  = size | (1 << 16);
	DMA_CHCR(DMA_CD) = 0x11400100; // Transfer 1 word every 16 CPU cycles

	return 1;
}

int CdDataSync(int mode) {
	if (mode)
		return (DMA_CHCR(DMA_CD) >> 24) & 1;

	for (int i = DATA_SYNC_TIMEOUT; i; i--) {
		if (!(DMA_CHCR(DMA_CD) & (1 << 24)))
			return 0;
	}

	_sdk_log("CdDataSync() timeout\n");
	return -1;
}

/* LBA/MSF conversion */

CdlLOC *CdIntToPos(int i, CdlLOC *p) {
	i += 150;

	p->minute = itob(i / (75 * 60));
	p->second = itob((i / 75) % 60);
	p->sector = itob(i % 75);
	return p;
}

int CdPosToInt(const CdlLOC *p) {	
	return (
		(btoi(p->minute) * (75 * 60)) +
		(btoi(p->second) * 75) +
		btoi(p->sector)
	) - 150;
}

/* Misc. functions */

int CdGetToc(CdlLOC *toc) {
	TrackInfo track_info;

	if (!CdCommand(CdlGetTN, 0, 0, (uint8_t *) &track_info))
		return 0;
	if (CdSync(1, 0) != CdlComplete)
		return 0;

	int first  = btoi(track_info.first_track);
	int tracks = btoi(track_info.last_track) + 1 - first;
	//assert(first == 1);

	for (int i = 0; i < tracks; i++) {
		uint8_t track = itob(first + i);

		if (!CdCommand(CdlGetTD, &track, 1, (uint8_t *) &toc[i]))
			return 0;
		if (CdSync(1, 0) != CdlComplete)
			return 0;

		toc[i].sector = 0;
		toc[i].track  = track;
	}

	return tracks;
}

CdlRegionCode CdGetRegion(void) {
	uint8_t param = 0x22;
	uint8_t result[16];

	// Test command 0x22 is unsupported in firmware version C0, which was used
	// exclusively in the SCPH-1000 Japanese model. It's thus safe to assume
	// that the console is Japanese if the command returns a valid error.
	// https://psx-spx.consoledev.net/cdromdrive/#19h22h-int3for-europe
	memset(result, 0, 16);

	if (!CdCommand(CdlTest, &param, 1, result)) {
		_sdk_log("failed to probe drive region\n");
		return (result[1] == 0x10) ? CdlRegionSCEI : CdlRegionUnknown;
	}

	_sdk_log("drive region: %s\n", result);

	if (!strcmp(result, "for Japan"))
		return CdlRegionSCEI;
	if (!strcmp(result, "for U/C"))
		return CdlRegionSCEA;
	if (!strcmp(result, "for Europe"))
		return CdlRegionSCEE;
	if (!strcmp(result, "for NETNA") || !strcmp(result, "for NETEU"))
		return CdlRegionSCEW;
	if (!strcmp(result, "for US/AEP"))
		return CdlRegionDebug;

	return CdlRegionUnknown;
}

int CdMix(const CdlATV *vol) {
	CD_REG(0) = 2;
	CD_REG(2) = vol->val0;
	CD_REG(3) = vol->val1;

	CD_REG(0) = 3;
	CD_REG(1) = vol->val2;
	CD_REG(2) = vol->val3;

	CD_REG(3) = 0x20; // Unmute XA, apply volume changes
	return 1;
}
