/*
 * PSn00bSDK GPU library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __PSXGPU_H
#define __PSXGPU_H

#include <stdint.h>
#include <stddef.h>

/* Definitions */

typedef enum _GPU_DispFlags {
	DISP_WIDTH_256		= 0,
	DISP_WIDTH_320		= 1,
	DISP_WIDTH_512		= 2,
	DISP_WIDTH_640		= 3,
	DISP_HEIGHT_HIGH	= 1 << 2,
	DISP_MODE_PAL		= 1 << 3,
	DISP_24BIT_COLOR	= 1 << 4,
	DISP_INTERLACE		= 1 << 5,
	DISP_WIDTH_384		= 1 << 6
} GPU_DispFlags;

typedef enum _GPU_VideoMode {
	MODE_NTSC	= 0,
	MODE_PAL	= 1
} GPU_VideoMode;

/* Structure macros */

#define setVector(v, _x, _y, _z) \
	(v)->vx = (_x), (v)->vy = (_y), (v)->vz = (_z)

#define setRECT(v, _x, _y, _w, _h) \
	(v)->x = (_x), (v)->y = (_y), (v)->w = (_w), (v)->h = (_h)

#define setTPage(p, tp, abr, x, y)	((p)->tpage = getTPage(tp, abr, x, y))
#define setClut(p, x, y)			((p)->clut = getClut(x, y))

#define setRGB0(p, r, g, b) ((p)->r0 = (r), (p)->g0 = (g), (p)->b0 = (b))
#define setRGB1(p, r, g, b) ((p)->r1 = (r), (p)->g1 = (g), (p)->b1 = (b))
#define setRGB2(p, r, g, b) ((p)->r2 = (r), (p)->g2 = (g), (p)->b2 = (b))
#define setRGB3(p, r, g, b) ((p)->r3 = (r), (p)->g3 = (g), (p)->b3 = (b))

#define setXY0(p, _x0, _y0) \
	(p)->x0 = (_x0), (p)->y0 = (_y0)

#define setXY2(p, _x0, _y0, _x1, _y1) \
	(p)->x0 = (_x0), (p)->y0 = (_y0), \
	(p)->x1 = (_x1), (p)->y1 = (_y1)

#define setXY3(p, _x0, _y0, _x1, _y1, _x2, _y2) \
	(p)->x0 = (_x0), (p)->y0 = (_y0), \
	(p)->x1 = (_x1), (p)->y1 = (_y1), \
	(p)->x2 = (_x2), (p)->y2 = (_y2)

#define setXY4(p, _x0, _y0, _x1, _y1, _x2, _y2, _x3, _y3) \
	(p)->x0 = (_x0), (p)->y0 = (_y0), \
	(p)->x1 = (_x1), (p)->y1 = (_y1), \
	(p)->x2 = (_x2), (p)->y2 = (_y2), \
	(p)->x3 = (_x3), (p)->y3 = (_y3)

#define setWH(p, _w, _h) \
	(p)->w = (_w), (p)->h = (_h)

#define setXYWH(p, _x0, _y0, _w, _h) \
	(p)->x0 = (_x0),			(p)->y0 = (_y0), \
	(p)->x1 = ((_x0) + (_w)),	(p)->y1 = (_y0), \
	(p)->x2 = (_x0),			(p)->y2 = ((_y0) + (_h)), \
	(p)->x3 = ((_x0) + (_w)),	(p)->y3 = ((_y0) + (_h))

#define setUV0(p, _u0, _v0) \
	(p)->u0 = (_u0), (p)->v0 = (_v0)

#define setUV3(p, _u0, _v0, _u1, _v1, _u2, _v2) \
	(p)->u0 = (_u0), (p)->v0 = (_v0), \
	(p)->u1 = (_u1), (p)->v1 = (_v1), \
	(p)->u2 = (_u2), (p)->v2 = (_v2)
	
#define setUV4(p, _u0, _v0, _u1, _v1, _u2, _v2, _u3, _v3) \
	(p)->u0 = (_u0), (p)->v0 = (_v0), \
	(p)->u1 = (_u1), (p)->v1 = (_v1), \
	(p)->u2 = (_u2), (p)->v2 = (_v2), \
	(p)->u3 = (_u3), (p)->v3 = (_v3)

#define setUVWH(p, _u0, _v0, _w, _h) \
	(p)->u0 = (_u0),			(p)->v0 = (_v0), \
	(p)->u1 = ((_u0) + (_w)),	(p)->v1 = (_v0), \
	(p)->u2 = (_u0),			(p)->v2 = ((_v0) + (_h)), \
	(p)->u3 = ((_u0) + (_w)),	(p)->v3 = ((_v0) + (_h))

/* Primitive handling macros */

#define setlen(p, _len)		(((P_TAG *) (p))->len = (uint8_t) (_len))
#define setaddr(p, _addr)	(((P_TAG *) (p))->addr = (uint32_t) (_addr))
#define setcode(p, _code)	(((P_TAG *) (p))->code = (uint8_t) (_code))
#define getlen(p)			(((P_TAG *) (p))->len)
#define getaddr(p)			(((P_TAG *) (p))->addr)
#define getcode(p)			(((P_TAG *) (p))->code)

#define nextPrim(p)			(void *) (0x80000000 | (((P_TAG *) (p))->addr))
#define isendprim(p)		((((P_TAG *) (p))->addr) == 0xffffff)
#define addPrim(ot, p)		setaddr(p, getaddr(ot)), setaddr(ot, p)
#define addPrims(ot, a, b)	setaddr(b, getaddr(ot)), setaddr(ot, a)
#define catPrim(a, b)		setaddr(a, b)
#define termPrim(p)			setaddr(p, 0xffffff)

#define setSemiTrans(p, abe) \
	((abe) ? (getcode(p) |= 2) : (getcode(p) &= ~2))

#define setShadeTex(p, tge) \
	((tge) ? (getcode(p) |= 1) : (getcode(p) &= ~1))

#define getTPage(tp, abr, x, y) ( \
	(((x)  /  64) & 15) | \
	((((y) / 256) &  1) <<  4) | \
	(((abr)       &  3) <<  5) | \
	(((tp)        &  3) <<  7) | \
	((((y) / 512) &  1) << 11) \
)

#define getClut(x, y) (((y) << 6) | (((x) >> 4) & 0x3f))

/* Primitive initializer macros */

#define setPolyF3(p)	setlen(p,  4), setcode(p, 0x20)
#define setPolyFT3(p)	setlen(p,  7), setcode(p, 0x24)
#define setPolyG3(p)	setlen(p,  6), setcode(p, 0x30)
#define setPolyGT3(p)	setlen(p,  9), setcode(p, 0x34)
#define setPolyF4(p)	setlen(p,  5), setcode(p, 0x28)
#define setPolyFT4(p)	setlen(p,  9), setcode(p, 0x2c)
#define setPolyG4(p)	setlen(p,  8), setcode(p, 0x38)
#define setPolyGT4(p)	setlen(p, 12), setcode(p, 0x3c)
#define setSprt8(p)		setlen(p,  3), setcode(p, 0x74)
#define setSprt16(p)	setlen(p,  3), setcode(p, 0x7c)
#define setSprt(p)		setlen(p,  4), setcode(p, 0x64)
#define setTile1(p)		setlen(p,  2), setcode(p, 0x68)
#define setTile8(p)		setlen(p,  2), setcode(p, 0x70)
#define setTile16(p)	setlen(p,  2), setcode(p, 0x78)
#define setTile(p)		setlen(p,  3), setcode(p, 0x60)
#define setLineF2(p)	setlen(p,  3), setcode(p, 0x40)
#define setLineG2(p)	setlen(p,  4), setcode(p, 0x50)
#define setLineF3(p)	setlen(p,  5), setcode(p, 0x48), \
	(p)->pad = 0x55555555
#define setLineG3(p)	setlen(p,  7), setcode(p, 0x58), \
	(p)->pad = 0x55555555, (p)->p1 = 0, (p)->p2 = 0
#define setLineF4(p)	setlen(p,  6), setcode(p, 0x4c), \
	(p)->pad = 0x55555555
#define setLineG4(p)	setlen(p,  9), setcode(p, 0x5c), \
	(p)->pad = 0x55555555, (p)->p1 = 0, (p)->p2 = 0, (p)->p3 = 0
#define setFill(p) 		setlen(p,  3), setcode(p, 0x02)
#define setVram2Vram(p)	setlen(p,  8), setcode(p, 0x80), \
	(p)->pad[0] = 0, (p)->pad[1] = 0, (p)->pad[2] = 0, (p)->pad[3] = 0

#define setDrawTPage(p, dfe, dtd, tpage) \
	setlen(p, 1), \
	(p)->code[0] = (0xe1000000 | \
		(tpage) | \
		((dtd) <<  9) | \
		((dfe) << 10) \
	)

#define setDrawOffset(p, _x, _y) \
	setlen(p, 1), \
	(p)->code[0] = (0xe5000000 | \
		((_x)  % 1024) | \
		(((_y) % 1024) << 11) \
	)

#define setDrawMask(p, sb, mt) \
	setlen(p, 1), \
	(p)->code[0] = (0xe6000000 | (sb) | ((mt) << 1))

#define setDrawArea(p, r) \
	setlen(p, 2), \
	(p)->code[0] = (0xe3000000 | \
		((r)->x  % 1024) | \
		(((r)->y % 1024) << 10) \
	), \
	(p)->code[1] = (0xe4000000 | \
		(((r)->x  + (r)->w - 1) % 1024) | \
		((((r)->y + (r)->h - 1) % 1024) << 10) \
	)

#define setTexWindow(p, r) \
	setlen(p, 1), \
	(p)->code[0] = (0xe2000000 | \
		((r)->w  % 32) | \
		(((r)->h % 32) <<  5) | \
		(((r)->x % 32) << 10) | \
		(((r)->y % 32) << 15) \
	)

/* Primitive structure definitions */

typedef struct _P_TAG {
	uint32_t	addr:24;
	uint32_t	len:8;
	uint8_t		r, g, b, code;
} P_TAG;

typedef struct _POLY_F3 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	int16_t		x1, y1;
	int16_t		x2, y2;
} POLY_F3;

typedef struct _POLY_F4 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	int16_t		x1, y1;
	int16_t		x2, y2;
	int16_t		x3, y3;
} POLY_F4;

typedef struct _POLY_FT3 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		u0, v0;
	uint16_t	clut;
	int16_t		x1, y1;
	uint8_t		u1, v1;
	uint16_t	tpage;
	int16_t		x2, y2;
	uint8_t		u2, v2;
	uint16_t	pad;
} POLY_FT3;

typedef struct _POLY_FT4 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	uint16_t	x0, y0;
	uint8_t		u0, v0;
	uint16_t	clut;
	int16_t		x1, y1;
	uint8_t		u1, v1;
	uint16_t	tpage;
	int16_t		x2, y2;
	uint8_t		u2, v2;
	uint16_t	pad0;
	int16_t		x3, y3;
	uint8_t		u3, v3;
	uint16_t	pad1;
} POLY_FT4;

typedef struct _POLY_G3 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		r1, g1, b1, pad0;
	int16_t		x1, y1;
	uint8_t		r2, g2, b2, pad1;
	int16_t		x2, y2;
} POLY_G3;

typedef struct _POLY_G4 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		r1, g1, b1, pad0;
	int16_t		x1, y1;
	uint8_t		r2, g2, b2, pad1;
	int16_t		x2, y2;
	uint8_t		r3, g3, b3, pad2;
	int16_t		x3, y3;
} POLY_G4;

typedef struct _POLY_GT3 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		u0, v0;
	uint16_t	clut;
	uint8_t		r1, g1, b1, pad0;
	int16_t		x1, y1;
	uint8_t		u1, v1;
	uint16_t	tpage;
	uint8_t		r2, g2, b2, pad1;
	int16_t		x2, y2;
	uint8_t		u2, v2;
	uint16_t	pad2;
} POLY_GT3;

typedef struct _POLY_GT4 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		u0, v0;
	uint16_t	clut;
	uint8_t		r1, g1, b1, pad0;
	int16_t		x1, y1;
	uint8_t		u1, v1;
	uint16_t	tpage;
	uint8_t		r2, g2, b2, pad1;
	int16_t		x2, y2;
	uint8_t		u2, v2;
	uint16_t	pad2;
	uint8_t		r3, g3, b3, pad3;
	int16_t		x3, y3;
	uint8_t		u3, v3;
	uint16_t	pad4;
} POLY_GT4;

typedef struct _LINE_F2 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	int16_t		x1, y1;
} LINE_F2;

typedef struct _LINE_G2 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		r1, g1, b1, p1;
	int16_t		x1, y1;
} LINE_G2;

typedef struct _LINE_F3 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	int16_t		x1, y1;
	int16_t		x2, y2;
	uint32_t	pad;
} LINE_F3;

typedef struct _LINE_G3 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		r1, g1, b1, p1;
	int16_t		x1, y1;
	uint8_t		r2, g2, b2, p2;
	int16_t		x2, y2;
	uint32_t	pad;
} LINE_G3;

typedef struct _LINE_F4 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	int16_t		x1, y1;
	int16_t		x2, y2;
	int16_t		x3, y3;
	uint32_t	pad;
} LINE_F4;

typedef struct _LINE_G4 {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		r1, g1, b1, p1;
	int16_t		x1, y1;
	uint8_t		r2, g2, b2, p2;
	int16_t		x2, y2;
	uint8_t		r3, g3, b3, p3;
	int16_t		x3, y3;
	uint32_t	pad;
} LINE_G4;

typedef struct _TILE {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	int16_t		w, h;
} TILE;

struct _TILE_FIXED {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
};
typedef struct _TILE_FIXED TILE_1;
typedef struct _TILE_FIXED TILE_8;
typedef struct _TILE_FIXED TILE_16;

typedef struct _SPRT {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		u0, v0;
	uint16_t	clut;
	uint16_t	w, h;
} SPRT;

struct _SPRT_FIXED {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	int16_t		x0, y0;
	uint8_t		u0, v0;
	uint16_t	clut;
};
typedef struct _SPRT_FIXED SPRT_8;
typedef struct _SPRT_FIXED SPRT_16;

typedef struct _DR_ENV {
	uint32_t tag;
	uint32_t code[15];
} DR_ENV;

typedef struct _DR_AREA {
	uint32_t tag;
	uint32_t code[2];
} DR_AREA;

typedef struct _DR_OFFSET {
	uint32_t tag;
	uint32_t code[1];
} DR_OFFSET;

typedef struct _DR_TWIN {
	uint32_t tag;
	uint32_t code[2];
} DR_TWIN;

typedef struct _DR_TPAGE {
	uint32_t tag;
	uint32_t code[1];
} DR_TPAGE;

typedef struct _DR_MASK {
	uint32_t tag;
	uint32_t code[1];
} DR_MASK;

typedef struct _FILL {
	uint32_t	tag;
	uint8_t		r0, g0, b0, code;
	uint16_t	x0, y0; // Note: coordinates must be in 16 pixel steps
	uint16_t	w, h;
} FILL;

typedef struct _VRAM2VRAM {
	uint32_t	tag;
	uint8_t		p0, p1, p2, code;
	uint16_t	x0, y0;
	uint16_t	x1, y1;
	uint16_t	w, h;
	uint32_t	pad[4];
} VRAM2VRAM;

/* Structure definitions */

typedef struct _RECT {
	int16_t x, y, w, h;
} RECT;

typedef struct _DISPENV_RAW {
	uint32_t	vid_mode;
	int16_t		vid_xpos, vid_ypos;
	int16_t		fb_x, fb_y;
} DISPENV_RAW;

typedef struct _DISPENV {
	RECT		disp, screen;
	uint8_t		isinter, isrgb24, reverse;
	uint8_t		_reserved;
} DISPENV;

typedef struct _DRAWENV {
	RECT		clip;		// Drawing area
	int16_t		ofs[2];		// GPU draw offset (relative to draw area)
	RECT		tw;			// Texture window (doesn't do anything atm)
	uint16_t	tpage;		// Initial tpage value
	uint8_t		dtd;		// Dither processing flag (simply OR'ed to tpage)
	uint8_t		dfe;		// Drawing to display area blocked/allowed (simply OR'ed to tpage)
	uint8_t		isbg;		// Clear draw area if non-zero
	uint8_t		r0, g0, b0;	// Draw area clear color (if isbg iz nonzero)
	DR_ENV		dr_env;		// Draw mode packet area (used by PutDrawEnv)
} DRAWENV;

typedef struct _TIM_IMAGE {
	uint32_t	mode;
	RECT		*crect;
	uint32_t	*caddr;
	RECT		*prect;
	uint32_t	*paddr;
} TIM_IMAGE;

typedef struct _GsIMAGE {
	uint32_t	pmode;
	int16_t		px, py, pw, ph;
	uint32_t	*pixel;
	int16_t		cx, cy, cw, ch;
	uint32_t	*clut;
} GsIMAGE;

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

void ResetGraph(int mode);

GPU_VideoMode GetVideoMode(void);
void SetVideoMode(GPU_VideoMode mode);
void SetDispMask(int mask);

//void PutDispEnvRaw(const DISPENV_RAW *env);
void PutDispEnv(const DISPENV *env);
void PutDrawEnv(DRAWENV *env);
void PutDrawEnvFast(DRAWENV *env);

int GetODE(void);
int VSync(int mode);
void *VSyncHaltFunction(void (*func)(void));
void *VSyncCallback(void (*func)(void));

int EnqueueDrawOp(
	void		(*func)(uint32_t, uint32_t, uint32_t),
	uint32_t	arg1,
	uint32_t	arg2,
	uint32_t	arg3
);
int DrawSync(int mode);
void *DrawSyncCallback(void (*func)(void));

int LoadImage(const RECT *rect, const uint32_t *data);
int StoreImage(const RECT *rect, uint32_t *data);
int MoveImage(const RECT *rect, int x, int y);
void LoadImage2(const RECT *rect, const uint32_t *data);
void StoreImage2(const RECT *rect, uint32_t *data);
void MoveImage2(const RECT *rect, int x, int y);

void ClearOTagR(uint32_t *ot, size_t length);
void ClearOTag(uint32_t *ot, size_t length);
int DrawOTag(const uint32_t *ot);
int DrawOTagEnv(const uint32_t *ot, DRAWENV *env);
void DrawOTag2(const uint32_t *ot);
void DrawPrim(const uint32_t *pri);

void AddPrim(uint32_t *ot, const void *pri);

int GsGetTimInfo(const uint32_t *tim, GsIMAGE *info);
int GetTimInfo(const uint32_t *tim, TIM_IMAGE *info);

DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h);
DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);

void FntLoad(int x, int y);
char *FntSort(uint32_t *ot, char *pri, int x, int y, const char *text);
int FntOpen(int x, int y, int w, int h, int isbg, int n);
int FntPrint(int id, const char *fmt, ...);
char *FntFlush(int id);

#ifdef __cplusplus
}
#endif

#endif
