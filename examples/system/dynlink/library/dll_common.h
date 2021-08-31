/*
 * PSn00bSDK dynamic linker example (shared header)
 * (C) 2021 spicyjpeg - MPL licensed
 */

#ifndef __DLL_COMMON_H
#define __DLL_COMMON_H

#include <psxgpu.h>

/* Common structures shared by the main executable and DLLs */

#define OT_LEN     256
#define PACKET_LEN 16384

typedef struct {
	DISPENV  disp;
	DRAWENV  draw;
	uint32_t ot[OT_LEN];
	uint8_t  p[PACKET_LEN];
} DB;

typedef struct {
	uint16_t xres, yres;
	DB       db[2];
	uint32_t db_active;
	uint8_t	 *db_nextpri;
} CONTEXT;

#endif
