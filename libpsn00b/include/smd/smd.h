/*
 * PSn00bSDK .SMD model parser library
 * (C) 2019-2023 Lameguy64, spicyjpeg - MPL licensed
 */

#pragma once

#include <stdint.h>
#include <psxgte.h>

/* Structure definitions */

typedef struct {
	uint32_t *ot;
	int16_t  otlen;
	uint8_t  zdiv,zoff;
} SC_OT;

typedef struct {
	char     id[3];
	uint8_t  version;
	uint16_t flags;
	uint16_t n_verts;
	uint16_t n_norms;
	uint16_t n_prims;
	SVECTOR  *p_verts;
	SVECTOR  *p_norms;
	void     *p_prims;
} SMD;

typedef struct {
	uint8_t type     : 2;
	uint8_t l_type   : 2;
	uint8_t c_type   : 1;
    uint8_t texture  : 1;
	uint8_t blend    : 2;
	uint8_t zoff     : 4;
	uint8_t nocull   : 1;
	uint8_t mask     : 1;
	uint8_t texwin   : 2;
	uint8_t texoff   : 2;
	uint8_t reserved : 6;
	uint8_t len;
} SMD_PRI_TYPE;

typedef enum: uint8_t {
	SMD_KIND_LINE = 0,
	SMD_KIND_TRIANGLE = 1,
	SMD_KIND_QUAD = 2,
} SMD_KIND;

typedef enum: uint8_t {
	// No shading (no normals)
	SMD_LIGHTING_NONE = 0,
	// Flat shading (1 normal)
	SMD_LIGHTING_FLAT = 1,
	// Smooth shading (3 normals)
	SMD_LIGHTING_SMOOTH = 2
} SMD_LIGHTING;

typedef enum: uint8_t {
	SMD_COLORING_SOLID = 0,
	SMD_COLORING_GOURAUD = 1
} SMD_COLORING;

typedef enum: uint8_t {
	// 50% background + 50% foreground (50% alpha)
	SMD_BLEND_ALPHA_50 = 0,
	// 100% background + 100% foreground (additive)
	SMD_BLEND_ADDITIVE = 1,
	// 100% background - 100% foreground (subtractive)
	SMD_BLEND_SUBTRACT = 2,
	// 100% background - 25% foreground (subtract 25%)
	SMD_BLEND_SUBTRACT_25 = 3
} SMD_BLEND;

typedef struct {
	SMD_PRI_TYPE prim_id;

	uint16_t v0,v1,v2,v3;		// Vertex indices
	uint16_t n0,n1,n2,n3;		// Normal indices
	uint8_t  r0,g0,b0,code;		// RGB0
	uint8_t  r1,g1,b1,p0;		// RGB1
	uint8_t  r2,g2,b2,p1;		// RGB2
	uint8_t  r3,g3,b3,p2;		// RGB3
	uint8_t  tu0,tv0;
	uint8_t  tu1,tv1;
	uint8_t  tu2,tv2;
	uint8_t  tu3,tv3;
	uint16_t tpage,clut;
} SMD_PRIM;

/* API */

#ifdef __cplusplus
extern "C" {
#endif

int OpenSMD(const void *smd);
SMD_PRIM *ReadSMD(SMD_PRIM *pri);

void scSetClipRect(int x0, int y0, int x1, int y1);

SMD *smdInitData(const void *data);
void smdSetBaseTPage(uint16_t tpage);

uint8_t *smdSortModel(SC_OT *ot, uint8_t *pribuff, SMD *smd);
uint8_t *smdSortModelFlat(uint32_t *ot, uint8_t *pribuff, SMD *smd);

void smdSetCelTex(uint16_t tpage, uint16_t clut);
void smdSetCelParam(int udiv, int vdiv, unsigned int col);
uint8_t *smdSortModelCel(SC_OT *ot, uint8_t *pribuff, SMD *smd);

#ifdef __cplusplus
}
#endif
