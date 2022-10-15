/*
 * PSn00bSDK dynamic linker example (DLL 1)
 * (C) 2021 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxpad.h>
#include <inline_c.h>

#include "dll_common.h"

/* Cube model */

typedef struct {
	int16_t v0, v1, v2, v3;
} INDEX;

static SVECTOR cube_verts[] = {
	{ -100, -100, -100, 0 },
	{  100, -100, -100, 0 },
	{ -100,  100, -100, 0 },
	{  100,  100, -100, 0 },
	{  100, -100,  100, 0 },
	{ -100, -100,  100, 0 },
	{  100,  100,  100, 0 },
	{ -100,  100,  100, 0 }
};
static SVECTOR cube_norms[] = {
	{ 0, 0, -ONE, 0 },
	{ 0, 0, ONE, 0 },
	{ 0, -ONE, 0, 0 },
	{ 0, ONE, 0, 0 },
	{ -ONE, 0, 0, 0 },
	{ ONE, 0, 0, 0 }
};
static INDEX cube_indices[] = {
	{ 0, 1, 2, 3 },
	{ 4, 5, 6, 7 },
	{ 5, 4, 0, 1 },
	{ 6, 7, 3, 2 },
	{ 0, 2, 5, 7 },
	{ 3, 1, 6, 4 }
};

#define CUBE_FACES 6

/* Light matrices */

// Each column represents the color matrix of each light source and is used as
// material color when using gte_ncs() or multiplied by a source color when
// using gte_nccs(). 4096 is 1.0 in this matrix. A column of zeroes disables
// the light source.
static MATRIX color_mtx = {
	ONE, 0, 0, // R
	ONE, 0, 0, // G
	ONE, 0, 0  // B
};

// Each row represents a vector direction of each light source. An entire row
// of zeroes disables the light source.
static MATRIX light_mtx = {
	-2048, -2048, -2048,
	    0,     0,     0,
	    0,     0,     0
};

/* Functions called by the main executable */

// NOTE: DLLs have no main(), _start() or other defined entry point. C++ global
// objects are automatically constructed when loading the library, and their
// destructors are called when unloading it via dlclose(). Other than that, the
// main executable can freely call DLL functions in any order, however it's
// still recommended (at least for C code) to have an init() function to e.g.
// initialize variables or hardware.

static uint32_t frame = 0;
static SVECTOR  rot   = { 0 };
static VECTOR   pos   = { 0, 0, 400 };
static MATRIX   mtx, lmtx;

void init(RenderContext *ctx) {
	Framebuffer *db = &(ctx->db[ctx->db_active]);

	InitGeom();

	gte_SetGeomOffset(db->draw.clip.w / 2, db->draw.clip.h / 2);
	gte_SetGeomScreen(db->draw.clip.w / 2);
	gte_SetBackColor(63, 63, 63);
	gte_SetColorMatrix(&color_mtx);
}

void render(RenderContext *ctx, uint16_t buttons) {
	RotMatrix(&rot, &mtx);
	TransMatrix(&mtx, &pos);
	MulMatrix0(&light_mtx, &mtx, &lmtx);

	gte_SetRotMatrix(&mtx);
	gte_SetTransMatrix(&mtx);
	gte_SetLightMatrix(&lmtx);

	// Spin the cube faster is cross is pressed.
	int16_t step = !(buttons & PAD_CROSS) ? 32 : 16;
	rot.vx += step;
	rot.vz += step;

	Framebuffer *db   = &(ctx->db[ctx->db_active]);
	POLY_F4	    *pol4 = (POLY_F4 *) ctx->db_nextpri;

	for (uint32_t i = 0; i < CUBE_FACES; i++) {
		int32_t p;

		gte_ldv3( 
			&cube_verts[cube_indices[i].v0], 
			&cube_verts[cube_indices[i].v1], 
			&cube_verts[cube_indices[i].v2]
		);

		gte_rtpt();
		gte_nclip();
		gte_stopz(&p);
		if (p < 0)
			continue;

		gte_avsz4();
		gte_stotz(&p);
		if ((p >> 2) > OT_LEN)
			continue;

		setPolyF4(pol4);

		gte_stsxy0(&(pol4->x0));
		gte_stsxy1(&(pol4->x1));
		gte_stsxy2(&(pol4->x2));

		gte_ldv0(&(cube_verts[cube_indices[i].v3]));
		gte_rtps();
		gte_stsxy(&(pol4->x3));

		gte_ldrgb(&(pol4->r0));
		gte_ldv0(&(cube_norms[i]));
		gte_ncs();
		gte_strgb(&(pol4->r0));

		addPrim(&(db->ot[p >> 2]), pol4);
		pol4++;
	}

	ctx->db_nextpri = (uint8_t *) pol4;

	// Due to our custom resolver, this will actually call dll_printf() in the
	// main executable.
	printf("DRAWING CUBE, COUNTER=%d\n", frame++);
}
