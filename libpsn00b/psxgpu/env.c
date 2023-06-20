/*
 * PSn00bSDK GPU library (DRAWENV/DISPENV functions)
 * (C) 2022-2023 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <assert.h>
#include <psxgpu.h>
#include <hwregs_c.h>

#define _min(x, y) (((x) < (y)) ? (x) : (y))

extern GPU_VideoMode _gpu_video_mode;

/* Private utilities */

// Converts a texture window size value (a power of two in 8-128 range) into a
// bit mask by setting the leading zeroes of the value:
//     0 = 0b00000000 -> 0b00000
//     8 = 0b00001000 -> 0b11111
//    16 = 0b00010000 -> 0b11110
//    32 = 0b00100000 -> 0b11100
//    64 = 0b01000000 -> 0b11000
//   128 = 0b10000000 -> 0b10000
// The GPU uses the mask to process texture coordinates as follows:
//   x &= ~(mask << 3)
//   x |= (offset << 3) & (mask << 3)
static inline uint32_t _get_window_mask(int size) {
	uint32_t mask = size >> 3;

	mask |= mask << 1;
	mask |= mask << 2;
	mask |= mask << 4;
	return mask & 0x1f;
}

static const uint32_t *_build_drawenv_ot(const uint32_t *ot, DRAWENV *env) {
	// All commands are grouped into a single display list packet for
	// performance reasons using tagless primitives (the GPU does not care
	// about the grouping as the display list is parsed by the CPU).
	DR_ENV *prim = &(env->dr_env);
	setaddr(prim, ot);
	setlen(prim, 5);

	// Texture page (reset active page and set dither/mask bits)
	setDrawTPage_T(&(prim->tpage), env->dfe & 1, env->dtd & 1, env->tpage);

	// Texture window
	//setTexWindow_T(&(prim->twin), &(env->tw));
	prim->twin.code[0]  = 0xe2000000;
	prim->twin.code[0] |= _get_window_mask(env->tw.w);
	prim->twin.code[0] |= _get_window_mask(env->tw.h) << 5;
	prim->twin.code[0] |= (env->tw.x & 0xf8) << 7;  // ((tw.x / 8) & 0x1f) << 10
	prim->twin.code[0] |= (env->tw.y & 0xf8) << 12; // ((tw.y / 8) & 0x1f) << 15

	// Set drawing area
	setDrawArea_T(&(prim->area), &(env->clip));
	setDrawOffset_T(
		&(prim->offset), env->clip.x + env->ofs[0], env->clip.y + env->ofs[1]
	);

	if (env->isbg) {
		FILL_T *fill = &(prim->fill);
		setlen(prim, 8);

		// Rectangle fill
		// FIXME: reportedly this command doesn't accept height values >511...
		setFill_T(fill);
		setColor0(fill, *((const uint32_t *) &(env->isbg)) >> 8);
		setXY0(fill, env->clip.x, env->clip.y);
		setWH(fill, env->clip.w, _min(env->clip.h, 0x1ff));
	}

	return (const uint32_t *) prim;
}

/* Drawing API */

void _send_linked_list(GPU_DrawOpType type, const uint32_t *ot);

DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h) {
	_sdk_validate_args(env && (w > 0) && (h > 0), 0);

	env->clip.x = x;
	env->clip.y = y;
	env->clip.w = w;
	env->clip.h = h;

	env->ofs[0] = 0;
	env->ofs[1] = 0;

	env->tw.x = 0;
	env->tw.y = 0;
	env->tw.w = 256;
	env->tw.h = 256;

	env->tpage = 0x0a;
	env->dtd   = 1;
	env->dfe   = 0;
	env->isbg  = 0;
	setRGB0(env, 0, 0, 0);

	env->dr_env.tag = 0;
	return env;
}

int DrawOTagEnv(const uint32_t *ot, DRAWENV *env) {
	_sdk_validate_args(ot && env, -1);

	return EnqueueDrawOp(
		(void *)   &_send_linked_list,
		(uint32_t) DRAWOP_TYPE_DMA,
		(uint32_t) _build_drawenv_ot(ot, env),
		0
	);
}

int DrawOTagEnvIRQ(const uint32_t *ot, DRAWENV *env) {
	_sdk_validate_args(ot && env, -1);

	return EnqueueDrawOp(
		(void *)   &_send_linked_list,
		(uint32_t) DRAWOP_TYPE_GPU_IRQ,
		(uint32_t) _build_drawenv_ot(ot, env),
		0
	);
}

void PutDrawEnv(DRAWENV *env) {
	_sdk_validate_args_void(env);

	DrawOTagEnv((const uint32_t *) 0xffffff, env);
}

// This function skips rebuilding the cached packet whenever possible and is
// useful if the DRAWENV structure is never modified (which is the case most of
// the time).
void PutDrawEnvFast(DRAWENV *env) {
	_sdk_validate_args_void(env);

	if (!(env->dr_env.tag))
		DrawOTagEnv((const uint32_t *) 0xffffff, env);
	else
		DrawOTag((const uint32_t *) &(env->dr_env));
}

/* Display API */

DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h) {
	_sdk_validate_args(env && (w > 0) && (h > 0), 0);

	env->disp.x = x;
	env->disp.y = y;
	env->disp.w = w;
	env->disp.h = h;

	env->screen.x = 0;
	env->screen.y = 0;
	env->screen.w = 0;
	env->screen.h = 0;

	env->isinter = 0;
	env->isrgb24 = 0;
	env->reverse = 0;

	return env;
}

void PutDispEnv(const DISPENV *env) {
	_sdk_validate_args_void(env);

	uint32_t fb_pos, h_range, v_range, mode;

	mode  = _gpu_video_mode << 3;
	mode |= (env->isrgb24 & 1) << 4;
	mode |= (env->isinter & 1) << 5;
	mode |= (env->reverse & 1) << 7;

	int h_span = env->screen.w ? env->screen.w : 256;
	int v_span = env->screen.h ? env->screen.h : 240;

	// Calculate the horizontal and vertical display range values.
	h_span *= 10;

	if (env->disp.w > 560) {
		// 640 pixels
		mode   |= 3 << 0;
		//h_span *= 4;
	} else if (env->disp.w > 400) {
		// 512 pixels
		mode   |= 2 << 0;
		//h_span *= 5;
	} else if (env->disp.w > 352) {
		// 368 pixels
		mode   |= 1 << 6;
		//h_span *= 7;
	} else if (env->disp.w > 280) {
		// 320 pixels
		mode   |= 1 << 0;
		//h_span *= 8;
	} else {
		// 256 pixels
		mode   |= 0 << 0;
		//h_span *= 10;
	}

	if (env->disp.h > 256) {
		mode   |= 1 << 2;
		//v_span /= 2;
	}

	int x   = env->screen.x + 0x760;
	int y   = env->screen.y + (_gpu_video_mode ? 0xa3 : 0x88);
	h_span /= 2;
	v_span /= 2;

	fb_pos   = (env->disp.x  & 0x3ff);
	fb_pos  |= (env->disp.y  & 0x1ff) << 10;
	h_range  = ((x - h_span) & 0xfff);
	h_range |= ((x + h_span) & 0xfff) << 12;
	v_range  = ((y - v_span) & 0x3ff);
	v_range |= ((y + v_span) & 0x3ff) << 10;

	GPU_GP1 = 0x05000000 | fb_pos;  // Set VRAM location to display
	GPU_GP1 = 0x06000000 | h_range; // Set horizontal display range
	GPU_GP1 = 0x07000000 | v_range; // Set vertical display range
	GPU_GP1 = 0x08000000 | mode;    // Set video mode
}

/* Deprecated "raw" display API */

void PutDispEnvRaw(const DISPENV_RAW *env) {
	_sdk_validate_args_void(env);

	uint32_t h_range, v_range, fb_pos;

	h_range  =   608 + env->vid_xpos;
	h_range |= (3168 + env->vid_xpos) << 12;

	// FIXME: these hardcoded values are for NTSC displays.
	v_range  =  (136 - 120 + env->vid_ypos) & 0x3ff;
	v_range |= ((136 + 120 + env->vid_ypos) & 0x3ff) << 12;

	fb_pos  =  env->fb_x & 0x3ff;
	fb_pos |= (env->fb_y & 0x1ff) << 10;

	GPU_GP1 = 0x06000000 | h_range;       // Set horizontal display range
	GPU_GP1 = 0x07000000 | v_range;       // Set vertical display range
	GPU_GP1 = 0x08000000 | env->vid_mode; // Set video mode
	GPU_GP1 = 0x05000000 | fb_pos;        // Set VRAM location to display
}

/* Misc. display functions */

GPU_VideoMode GetVideoMode(void) {
	return _gpu_video_mode;
}

void SetVideoMode(GPU_VideoMode mode) {
	uint32_t _mode, stat = GPU_GP1;

	_gpu_video_mode = mode & 1;

	_mode  = (mode & 1) << 3;
	_mode |= (stat >> 17) & 0x37; // GPUSTAT bits 17-22 -> command bits 0-5
	_mode |= (stat >> 10) & 0x40; // GPUSTAT bit 16 -> command bit 6
	_mode |= (stat >>  7) & 0x80; // GPUSTAT bit 14 -> command bit 7

	GPU_GP1 = 0x08000000 | _mode;
}

int GetODE(void) {
	return (GPU_GP1 >> 31);
}

void SetDispMask(int mask) {
	GPU_GP1 = 0x03000000 | (mask ? 0 : 1);
}
