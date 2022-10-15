/*
 * PSn00bSDK dynamic linker example (DLL 2)
 * (C) 2021 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxpad.h>
#include <inline_c.h>

#include "dll_common.h"

extern const uint32_t ball16c[];

/* Balls data */

typedef struct {
	int16_t x, y;
	int16_t xdir, ydir;
	uint8_t r, g, b, p;
} Ball;

#define MAX_BALLS 512

/* Functions called by the main executable */

// NOTE: DLLs have no main(), _start() or other defined entry point. C++ global
// objects are automatically constructed when loading the library, and their
// destructors are called when unloading it via dlclose(). Other than that, the
// main executable can freely call DLL functions in any order, however it's
// still recommended (at least for C code) to have an init() function to e.g.
// initialize variables or hardware.

static uint32_t  frame = 0;
static Ball      balls[MAX_BALLS];
static TIM_IMAGE ball_tim;

void init(RenderContext *ctx) {
	Framebuffer *db = &(ctx->db[ctx->db_active]);

	GetTimInfo(ball16c, &ball_tim);
	LoadImage(ball_tim.prect, ball_tim.paddr);
	if (ball_tim.mode & 8)
		LoadImage(ball_tim.crect, ball_tim.caddr);

	// Initialize the balls by giving them a random initial position, velocity
	// and color.
	for (uint32_t i = 0; i < MAX_BALLS; i++) {
		Ball *b = &(balls[i]);

		b->x    = rand() % (db->draw.clip.w - 16);
		b->y    = rand() % (db->draw.clip.h - 16);
		b->xdir = ((rand() & 1) ? 1 : -1) * ((rand() % 3) + 1);
		b->ydir = ((rand() & 1) ? 1 : -1) * ((rand() % 3) + 1);
		b->r    = rand() & 0xff;
		b->g    = rand() & 0xff;
		b->b    = rand() & 0xff;
	}
}

void render(RenderContext *ctx, uint16_t buttons) {
	Framebuffer *db   = &(ctx->db[ctx->db_active]);
	SPRT_16     *sprt = (SPRT_16 *) ctx->db_nextpri;

	for (uint32_t i = 0; i < MAX_BALLS; i++) {
		Ball *b = &(balls[i]);

		setSprt16(sprt);

		setXY0(sprt, b->x, b->y);
		setRGB0(sprt, b->r, b->g, b->b);
		setUV0(sprt, 0, 0);
		setClut(sprt, ball_tim.crect->x, ball_tim.crect->y);

		addPrim(&(db->ot[OT_LEN - 1]), sprt);
		sprt++;

		// Update the ball's velocity and acceleration, moving them slower if
		// cross is pressed.
		int16_t step = !(buttons & PAD_CROSS) ? 1 : 0;
		b->x += b->xdir >> step;
		b->y += b->ydir >> step;

		if (
			(b->x < 0) ||
			((b->x + 16) > db->draw.clip.w)
		)
			b->xdir *= -1;
		if (
			(b->y < 0) ||
			((b->y + 16) > db->draw.clip.h)
		)
			b->ydir *= -1;
	}

	ctx->db_nextpri = (uint8_t *) sprt;

	// Add a TPAGE "primitive" to ensure the GPU finds the ball texture.
	DR_TPAGE *tpri = (DR_TPAGE * ) ctx->db_nextpri;

	setDrawTPage(
		tpri,
		0,
		0, 
		getTPage(0, 0, ball_tim.prect->x, ball_tim.prect->y)
	);
	addPrim(&(db->ot[OT_LEN - 1]), tpri);
	tpri++;

	ctx->db_nextpri = (uint8_t *) tpri;

	// Due to our custom resolver, this will actually call dll_printf() in the
	// main executable.
	printf("DRAWING BALLS, COUNTER=%d\n", frame++);
}
