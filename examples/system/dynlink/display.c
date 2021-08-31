/*
 * PSn00bSDK dynamic linker example (utilities)
 * (C) 2021 spicyjpeg - MPL licensed
 */

#include <psxgpu.h>

#include "library/dll_common.h"

#define SCREEN_XRES 320
#define SCREEN_YRES 240

/* Display/GPU context utilities */

void init_context(CONTEXT *ctx) {
	DB *db;

	ResetGraph(0);
	ctx->xres      = SCREEN_XRES;
	ctx->yres      = SCREEN_YRES;
	ctx->db_active = 0;

	db = &(ctx->db[0]);
	SetDefDispEnv(&(db->disp),           0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), 63, 0, 127);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	db = &(ctx->db[1]);
	SetDefDispEnv(&(db->disp), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw),           0, 0, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), 63, 0, 127);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	// Set up the ordering tables and primitive buffers.
	db = &(ctx->db[0]);
	ctx->db_nextpri = db->p;
	ClearOTagR((u_long *) db->ot, OT_LEN);

	PutDrawEnv(&(db->draw));
	//PutDispEnv(&(db->disp));

	db = &(ctx->db[1]);
	ClearOTagR((u_long *) db->ot, OT_LEN);

	// Create a text stream at the top of the screen.
	FntLoad(960, 0);
	FntOpen(4, 12, 312, 32, 2, 256);
}

void display(CONTEXT *ctx) {
	DB *db;

	DrawSync(0);
	VSync(0);
	ctx->db_active ^= 1;

	db = &(ctx->db[ctx->db_active]);
	ctx->db_nextpri = db->p;
	ClearOTagR((u_long *) db->ot, OT_LEN);

	PutDrawEnv(&(db->draw));
	PutDispEnv(&(db->disp));
	SetDispMask(1);

	db = &(ctx->db[!ctx->db_active]);
	DrawOTag((u_long *) &(db->ot[OT_LEN - 1]));
}
