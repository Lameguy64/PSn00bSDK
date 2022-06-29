#ifndef __PSXGPU_H
#define __PSXGPU_H

#include <stddef.h>
#include <sys/types.h>

// Low-level display parameters for DISPENV_RAW. A leftover from prototyping
#define DISP_WIDTH_256		0
#define DISP_WIDTH_320		1
#define DISP_WIDTH_384		64
#define DISP_WIDTH_512		2
#define DISP_WIDTH_640		3

#define DISP_HEIGHT_LOW		0	// Could be 240 for NTSC, 256 for PAL
#define DISP_HEIGHT_HIGH	4	// Could be 480 for NTSC, 512 for PAL
#define DISP_INTERLACE		32
#define DISP_24BIT_COLOR	16
#define DISP_MODE_NTSC		0
#define DISP_MODE_PAL		8

typedef enum _VIDEO_MODE
{
	MODE_NTSC	= 0,
	MODE_PAL	= 1
} VIDEO_MODE;

// Vector macros

#define setVector( v, _x, _y, _z ) \
	(v)->vx = _x, (v)->vy = _y, (v)->vz = _z

#define setRECT( v, _x, _y, _w, _h ) \
	(v)->x = _x, (v)->y = _y, (v)->w = _w, (v)->h = _h


// Primitive macros	

#define setDrawTPage( p, dfe, dtd, tpage ) \
	( (p)->code[0] = tpage|(dfe<<10)|(dtd<<9), \
	setlen( p, 1 ), setcode( p, 0xe1 ) )
	
/*#define setVram2Vram( p ) ( setlen( p, 8 ), setcode( p, 0x80 ), \
	(p)->nop[0] = 0, (p)->nop[1] = 0, (p)->nop[2] = 0, (p)->nop[3] = 0 )*/
	
/*
	
#define setTPagePri2( p, dth, tp, abr, x, y ) \
	( (p)->code[0] = getTPage( tp, abr, x, y )|(dth<<9), \
	setlen( p, 1 ), setcode( p, 0xe1 ) )*/

/*
 *	Set primitive attributes
 */
#define setTPage( p, tp, abr, x, y ) \
	( (p)->tpage = getTPage( tp, abr, x, y ) )

#define setClut( p, x, y ) \
	( (p)->clut = getClut(x, y) )
	

/*
 *	Set primitive colors
 */
#define setRGB0( p, r, g, b ) ( (p)->r0 = r, (p)->g0 = g, (p)->b0 = b )
#define setRGB1( p, r, g, b ) ( (p)->r1 = r, (p)->g1 = g, (p)->b1 = b )
#define setRGB2( p, r, g, b ) ( (p)->r2 = r, (p)->g2 = g, (p)->b2 = b )
#define setRGB3( p, r, g, b ) ( (p)->r3 = r, (p)->g3 = g, (p)->b3 = b )


/*
 *	Set primitive screen coordinates
 */
#define setXY0( p, _x0, _y0 ) \
	(p)->x0 = _x0, (p)->y0 = _y0

#define setXY2( p, _x0, _y0, _x1, _y1 ) \
	(p)->x0 = _x0, (p)->y0 = _y0, \
	(p)->x1 = _x1, (p)->y1 = _y1
	
#define setXY3( p, _x0, _y0, _x1, _y1, _x2, _y2 ) \
	(p)->x0 = _x0, (p)->y0 = _y0, \
	(p)->x1 = _x1, (p)->y1 = _y1, \
	(p)->x2 = _x2, (p)->y2 = _y2

#define setXY4( p, _x0, _y0, _x1, _y1, _x2, _y2, _x3, _y3 ) \
	(p)->x0 = _x0, (p)->y0 = _y0, \
	(p)->x1 = _x1, (p)->y1 = _y1, \
	(p)->x2 = _x2, (p)->y2 = _y2, \
	(p)->x3 = _x3, (p)->y3 = _y3

#define setWH( p, _w, _h ) \
	(p)->w = _w, (p)->h = _h

#define setXYWH( p, _x0, _y0, _w, _h ) 		\
	(p)->x0 = _x0, 		(p)->y0 = _y0,		\
	(p)->x1 = _x0+(_w),	(p)->y1 = _y0,		\
	(p)->x2 = _x0, 		(p)->y2 = _y0+(_h),	\
	(p)->x3 = _x0+(_w), (p)->y3 = _y0+(_h)

/*
 *	Set texture coordinates
 */
#define setUV0( p, _u0, _v0 ) \
	(p)->u0 = _u0, (p)->v0 = _v0

#define setUV3( p, _u0, _v0, _u1, _v1, _u2, _v2 ) \
	(p)->u0 = _u0, (p)->v0 = _v0,	\
	(p)->u1 = _u1, (p)->v1 = _v1,	\
	(p)->u2 = _u2, (p)->v2 = _v2
	
#define setUV4( p, _u0, _v0, _u1, _v1, _u2, _v2, _u3, _v3 ) \
	(p)->u0 = _u0, (p)->v0 = _v0,	\
	(p)->u1 = _u1, (p)->v1 = _v1,	\
	(p)->u2 = _u2, (p)->v2 = _v2,	\
	(p)->u3 = _u3, (p)->v3 = _v3

#define setUVWH( p, _u0, _v0, _w, _h ) 		\
	(p)->u0 = _u0, 		(p)->v0 = _v0,		\
	(p)->u1 = _u0+(_w),	(p)->v1 = _v0,		\
	(p)->u2 = _u0, 		(p)->v2 = _v0+(_h),	\
	(p)->u3 = _u0+(_w), (p)->v3 = _v0+(_h)


/*
 *	Primitive handling macros
 */
#define setlen( p, _len )		( ((P_TAG*)(p))->len = (unsigned char)(_len) )
#define setaddr( p, _addr )		( ((P_TAG*)(p))->addr = (unsigned int)(_addr) )
#define setcode( p, _code )		( ((P_TAG*)(p))->code = (unsigned char)(_code) )

#define getlen( p )				( ((P_TAG*)(p))->len )
#define getaddr( p )			( ((P_TAG*)(p))->addr )
#define getcode( p )			( ((P_TAG*)(p))->code )

#define nextPrim( p )			(void*)((((P_TAG*)(p))->addr)|0x80000000)
#define isendprim( p )			((((P_TAG*)(p))->addr)==0xffffff)

#define addPrim( ot, p )		setaddr( p, getaddr( ot ) ), setaddr( ot, p )
#define addPrims( ot, p0, p1 )	setaddr( p1, getaddr( ot ) ), setaddr( ot, p0 )

#define catPrim( p0, p1 )		setaddr( p0, p1 )
#define termPrim( p )			setaddr( p, 0xffffff )

#define setSemiTrans( p, abe ) \
	( (abe)?setcode( p, getcode( p )|0x2 ):setcode( p, getcode( p )&~0x2 ) )

#define setShadeTex( p, tge ) \
	( (tge)?setcode( p, getcode( p )|0x1 ):setcode( p, getcode( p )&~0x1 ) )

	
/* ORIGINAl CODE */
#define setDrawMask( p, sb, mt ) \
	setlen( p, 1 ), (p)->code[0] = sb|(mt<<1), \
	setcode( p, 0xe6 )
	
	
#define getTPage( tp, abr, x, y ) \
	( (((x)&0x3ff)>>6) | (((y)>>8)<<4) | (((abr)&0x3)<<5) | (((tp)&0x3)<<7) )

#define getClut( x, y ) \
	( ((y)<<6)|(((x)>>4)&0x3f) )


/*
 *	Primitive initializers
 */ 
#define setPolyF3( p )	setlen( p, 4 ), 	setcode( p, 0x20 )
#define setPolyFT3( p )	setlen( p, 7 ), 	setcode( p, 0x24 )
#define setPolyG3( p )	setlen( p, 6 ), 	setcode( p, 0x30 )
#define setPolyGT3( p )	setlen( p, 9 ), 	setcode( p, 0x34 )

#define setPolyF4( p )	setlen( p, 5 ), 	setcode( p, 0x28 )
#define setPolyFT4( p )	setlen( p, 9 ), 	setcode( p, 0x2c )
#define setPolyG4( p )	setlen( p, 8 ), 	setcode( p, 0x38 )
#define setPolyGT4( p )	setlen( p, 12 ),	setcode( p, 0x3c )

#define setSprt8( p )	setlen( p, 3 ), 	setcode( p, 0x74 )
#define setSprt16( p )	setlen( p, 3 ), 	setcode( p, 0x7c )
#define setSprt( p )	setlen( p, 4 ), 	setcode( p, 0x64 )

#define setTile1( p )	setlen( p, 2 ), 	setcode( p, 0x68 )
#define setTile8( p )	setlen( p, 2 ), 	setcode( p, 0x70 )
#define setTile16( p )	setlen( p, 2 ), 	setcode( p, 0x78 )
#define setTile( p )	setlen( p, 3 ), 	setcode( p, 0x60 )

#define setLineF2( p )	setlen( p, 3 ),	setcode( p, 0x40 )
#define setLineG2( p )	setlen( p, 4 ),	setcode( p, 0x50 )

#define setLineF3( p )	setlen( p, 5 ), setcode( p, 0x48 ), (p)->pad = 0x55555555
#define setLineG3( p )	setlen( p, 7 ), setcode( p, 0x58 ), (p)->pad = 0x55555555, \
	(p)->p1 = 0, (p)->p2 = 0

#define setLineF4( p )	setlen( p, 6 ), setcode( p, 0x4c ), (p)->pad = 0x55555555
#define setLineG4( p )	setlen( p, 9 ), setcode( p, 0x5c ), (p)->pad = 0x55555555, \
	(p)->p1 = 0, (p)->p2 = 0, (p)->p3 = 0
			
#define setFill( p ) 	setlen( p, 3 ), 	setcode( p, 0x02 )

#define setDrawOffset( p, _x, _y ) \
	setlen( p, 1 ), \
	(p)->code[0] = (_x&0x3FF)|((_y&0x3FF)<<11), \
	((char*)(p)->code)[3] = 0xE5
	
#define setDrawArea( p, r ) \
	setlen( p, 2 ), \
	(p)->code[0] = ((r)->x&0x3FF)|(((r)->y&0x1FF)<<10),	\
	(p)->code[1] = (((r)->x+(r)->w-1)&0x3FF)|((((r)->y+(r)->h-1)&0x1FF)<<10), \
	((char*)&(p)->code[0])[3] = 0xE3, \
	((char*)&(p)->code[1])[3] = 0xE4

#define setTexWindow( p, r ) \
	setlen( p, 1 ), \
	(p)->code[0] = ((r)->w&0x1F)|(((r)->h&0x1F)<<5)|(((r)->x&0x1F)<<10)|(((r)->y&0x1F)<<15), \
	((char*)&(p)->code[0])[3] = 0xE2
	
/*
 *	Primitive definitions
 */
typedef struct _P_TAG
{
	unsigned	addr:24;
	unsigned	len:8;
	u_char		r,g,b,code;
} P_TAG;

/*
 *	Polygon primitive definitions
 */
typedef struct _POLY_F3
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	short		x1,y1;
	short		x2,y2;
} POLY_F3;

typedef struct _POLY_F4
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	short		x1,y1;
	short		x2,y2;
	short		x3,y3;
} POLY_F4;

typedef struct _POLY_FT3
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		u0,v0;
	u_short		clut;
	short		x1,y1;
	u_char		u1,v1;
	u_short		tpage;
	short		x2,y2;
	u_char		u2,v2;
	u_short		pad;
} POLY_FT3;

typedef struct _POLY_FT4
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	u_short		x0,y0;
	u_char		u0,v0;
	u_short		clut;
	short		x1,y1;
	u_char		u1,v1;
	u_short		tpage;
	short		x2,y2;
	u_char		u2,v2;
	u_short		pad0;
	short		x3,y3;
	u_char		u3,v3;
	u_short		pad1;
} POLY_FT4;

typedef struct _POLY_G3
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		r1,g1,b1,pad0;
	short		x1,y1;
	u_char		r2,g2,b2,pad1;
	short		x2,y2;
} POLY_G3;

typedef struct _POLY_G4
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		r1,g1,b1,pad0;
	short		x1,y1;
	u_char		r2,g2,b2,pad1;
	short		x2,y2;
	u_char		r3,g3,b3,pad2;
	short		x3,y3;
} POLY_G4;

typedef struct _POLY_GT3
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		u0,v0;
	u_short		clut;
	u_char		r1,g1,b1,pad0;
	short		x1,y1;
	u_char		u1,v1;
	u_short		tpage;
	u_char		r2,g2,b2,pad1;
	short		x2,y2;
	u_char		u2,v2;
	u_short		pad2;
} POLY_GT3;

typedef struct _POLY_GT4
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		u0,v0;
	u_short		clut;
	u_char		r1,g1,b1,pad0;
	short		x1,y1;
	u_char		u1,v1;
	u_short		tpage;
	u_char		r2,g2,b2,pad1;
	short		x2,y2;
	u_char		u2,v2;
	u_short		pad2;
	u_char		r3,g3,b3,pad3;
	short		x3,y3;
	u_char		u3,v3;
	u_short		pad4;
} POLY_GT4;

/*
 *	Line primitive definitions
 */
typedef struct _LINE_F2
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	short		x1,y1;
} LINE_F2;

typedef struct _LINE_G2
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		r1,g1,b1,p1;
	short		x1,y1;
} LINE_G2;

typedef struct _LINE_F3
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	short		x1,y1;
	short		x2,y2;
	u_long		pad;			/* actually a terminator for line loops */
} LINE_F3;

typedef struct _LINE_G3
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		r1,g1,b1,p1;
	short		x1,y1;
	u_char		r2,g2,b2,p2;
	short		x2,y2;
	u_long		pad;			/* actually a terminator for line loops */
} LINE_G3;

typedef struct _LINE_F4
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	short		x1,y1;
	short		x2,y2;
	short		x3,y3;
	u_long		pad;
} LINE_F4;

typedef struct _LINE_G4
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		r1,g1,b1,p1;
	short		x1,y1;
	u_char		r2,g2,b2,p2;
	short		x2,y2;
	u_char		r3,g3,b3,p3;
	short		x3,y3;
	u_long		pad;
} LINE_G4;

/*
 *	Tile and sprite primitive definitions
 */
typedef struct _TILE
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	short		w,h;
} TILE;

typedef struct _TILE_1
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
} TILE_1;

typedef struct _TILE_8
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
} TILE_8;

typedef struct _TILE_16
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
} TILE_16;

/*
 *	Sprite primitive definitions
 */
typedef struct _SPRT
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		u0,v0;
	u_short		clut;
	u_short		w,h;
} SPRT;

typedef struct _SPRT_8
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		u0,v0;
	u_short		clut;
} SPRT_8;

typedef struct _SPRT_16
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	short		x0,y0;
	u_char		u0,v0;
	u_short		clut;
} SPRT_16;

/*
 *	VRAM fill and transfer primitive definitions
 */

typedef struct _DR_ENV
{
	u_long		tag;
	u_long		code[15];
} DR_ENV;

typedef struct _DR_AREA
{
	u_long		tag;
	u_long		code[2];
} DR_AREA;

typedef struct _DR_OFFSET
{
	u_long		tag;
	u_long		code[1];
} DR_OFFSET;

typedef struct _DR_TWIN
{
	u_long		tag;
	u_long		code[2];
} DR_TWIN;

typedef struct _DR_TPAGE
{
	u_long		tag;
	u_long		code[1];
} DR_TPAGE;

typedef struct _DR_MASK		/* ORIGINAL */
{
	u_long		tag;
	u_long		code[1];
} DR_MASK;

typedef struct _FILL		/* ORIGINAL */
{
	u_long		tag;
	u_char		r0,g0,b0,code;
	u_short		x0,y0;		// Note: coordinates must be in 16 pixel steps
	u_short		w,h;
} FILL;

typedef struct _VRAM2VRAM	/* ORIGINAL */
{
	u_long		tag;
	u_char		p0,p1,p2,code;
	u_short		x0,y0;
	u_short		x1,y1;
	u_short		w,h;
	u_long		nop[4];
} VRAM2VRAM;

/*
 *	General structs
 */

typedef struct _RECT
{
	short x,y;
	short w,h;
} RECT;

typedef struct _DISPENV_RAW	/* obsolete */
{
	unsigned int vid_mode;		// Video mode
	short vid_xpos,vid_ypos;	// Video position (not framebuffer)
	short fb_x,fb_y;			// Framebuffer display position	
} DISPENV_RAW;

typedef struct _DISPENV
{
	RECT	disp;
	RECT	screen;
	char	isinter;
	char	isrgb24;
	char	reverse;
	char	pad;
} DISPENV;

typedef struct _DRAWENV
{
	RECT	clip;			// Drawing area
	short	ofs[2];			// GPU draw offset (relative to draw area)
	RECT	tw;				// Texture window (doesn't do anything atm)
	u_short	tpage;			// Initial tpage value
	u_char	dtd;			// Dither processing flag (simply OR'ed to tpage)
	u_char	dfe;			// Drawing to display area blocked/allowed (simply OR'ed to tpage)
	u_char	isbg;			// Clear draw area if non-zero
	u_char	r0,g0,b0;		// Draw area clear color (if isbg iz nonzero)
	DR_ENV	dr_env;			// Draw mode packet area (used by PutDrawEnv)
} DRAWENV;

typedef struct _TIM_IMAGE
{
	u_long	mode;
	RECT	*crect;
	u_long	*caddr;
	RECT	*prect;
	u_long	*paddr;
} TIM_IMAGE;

typedef struct _GsIMAGE
{
	u_long	pmode;
	short	px, py, pw, ph;
	u_long	*pixel;
	short	cx, cy, cw, ch;
	u_long	*clut;
} GsIMAGE;

#ifdef __cplusplus
extern "C" {
#endif

// Function definitions

void ResetGraph(int mode);

VIDEO_MODE GetVideoMode(void);
void SetVideoMode(VIDEO_MODE mode);

int GetODE(void);

void PutDispEnvRaw(const DISPENV_RAW *env);	/* obsolete */
void PutDispEnv(const DISPENV *env);
void PutDrawEnv(DRAWENV *env);
void PutDrawEnvFast(DRAWENV *env);

void SetDispMask(int mask);

int VSync(int mode);
int DrawSync(int mode);
//void WaitGPUcmd(void);
//void WaitGPUdma(void);

// Callback hook functions
void *VSyncCallback(void (*func)(void));
void *DrawSyncCallback(void (*func)(void));

void LoadImage(const RECT *rect, const u_long *data);
void StoreImage(const RECT *rect, u_long *data);

void ClearOTagR(u_long *ot, size_t length);
void ClearOTag(u_long *ot, size_t length);
void DrawOTag(const u_long *ot);
void DrawOTag2(const u_long *ot);
void DrawOTagEnv(const u_long *ot, DRAWENV *env);
void DrawPrim(const u_long *pri);

void AddPrim(u_long *ot, const void *pri);

int GsGetTimInfo(const u_long *tim, GsIMAGE *info);
int GetTimInfo(const u_long *tim, TIM_IMAGE *info);	/* deprecated */

DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h);
DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);

// Debug font functions

void FntLoad(int x, int y);
char *FntSort(u_long *ot, char *pri, int x, int y, const char *text);
int FntOpen(int x, int y, int w, int h, int isbg, int n);
int FntPrint(int id, const char *fmt, ...);
char *FntFlush(int id);

#ifdef __cplusplus
}
#endif

#endif
