#ifndef __PSXGPU_H
#define __PSXGPU_H

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


#define MODE_NTSC			0
#define MODE_PAL			1


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
	setlen( p, 1 ), p->code[0] = sb|(mt<<1), \
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
typedef struct {
	unsigned int addr:24;
	unsigned int len:8;
	unsigned char r,g,b;
	unsigned char code;
} P_TAG;

/*
 *	Polygon primitive definitions
 */
typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	short			x1,y1;
	short			x2,y2;
} POLY_F3;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	short			x1,y1;
	short			x2,y2;
	short			x3,y3;
} POLY_F4;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	u0,v0;
	unsigned short	clut;
	short			x1,y1;
	unsigned char	u1,v1;
	unsigned short	tpage;
	short			x2,y2;
	unsigned char	u2,v2;
	unsigned short	pad;
} POLY_FT3;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	u0,v0;
	unsigned short	clut;
	short			x1,y1;
	unsigned char	u1,v1;
	unsigned short	tpage;
	short			x2,y2;
	unsigned char	u2,v2;
	unsigned short	pad0;
	short			x3,y3;
	unsigned char	u3,v3;
	unsigned short	pad1;
} POLY_FT4;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	r1,g1,b1,pad0;
	short			x1,y1;
	unsigned char	r2,g2,b2,pad1;
	short			x2,y2;
} POLY_G3;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	r1,g1,b1,pad0;
	short			x1,y1;
	unsigned char	r2,g2,b2,pad1;
	short			x2,y2;
	unsigned char	r3,g3,b3,pad2;
	short			x3,y3;
} POLY_G4;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	u0,v0;
	unsigned short	clut;
	unsigned char	r1,g1,b1,pad0;
	short			x1,y1;
	unsigned char	u1,v1;
	unsigned short	tpage;
	unsigned char	r2,g2,b2,pad1;
	short			x2,y2;
	unsigned char	u2,v2;
	unsigned short	pad2;
} POLY_GT3;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	u0,v0;
	unsigned short	clut;
	unsigned char	r1,g1,b1,pad0;
	short			x1,y1;
	unsigned char	u1,v1;
	unsigned short	tpage;
	unsigned char	r2,g2,b2,pad1;
	short			x2,y2;
	unsigned char	u2,v2;
	unsigned short	pad2;
	unsigned char	r3,g3,b3,pad3;
	short			x3,y3;
	unsigned char	u3,v3;
	unsigned short	pad4;
} POLY_GT4;

/*
 *	Line primitive definitions
 */
typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	short			x1,y1;
} LINE_F2;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	r1,g1,b1,p1;
	short			x1,y1;
} LINE_G2;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	short			x1,y1;
	short			x2,y2;
	unsigned int	pad;
} LINE_F3;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	r1,g1,b1,p1;
	short			x1,y1;
	unsigned char	r2,g2,b2,p2;
	short			x2,y2;
	unsigned int	pad;
} LINE_G3;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	short			x1,y1;
	short			x2,y2;
	short			x3,y3;
	unsigned int	pad;
} LINE_F4;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	r1,g1,b1,p1;
	short			x1,y1;
	unsigned char	r2,g2,b2,p2;
	short			x2,y2;
	unsigned char	r3,g3,b3,p3;
	short			x3,y3;
	unsigned int	pad;
} LINE_G4;

/*
 *	Tile and sprite primitive definitions
 */
typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	short			w,h;
} TILE;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
} TILE_1;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
} TILE_8;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
} TILE_16;

/*
 *	Sprite primitive definitions
 */
typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	u0,v0;
	unsigned short	clut;
	unsigned short	w,h;
} SPRT;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	u0,v0;
	unsigned short	clut;
} SPRT_8;

typedef struct {
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	short			x0,y0;
	unsigned char	u0,v0;
	unsigned short	clut;
} SPRT_16;

/*
 *	VRAM fill and transfer primitive definitions
 */

typedef struct DR_ENV {
	unsigned int	tag;
	unsigned int	code[15];
} DR_ENV;

typedef struct DR_AREA {
	unsigned int	tag;
	unsigned int	code[2];
} DR_AREA;

typedef struct DR_OFFSET {
	unsigned int	tag;
	unsigned int	code[1];
} DR_OFFSET;

typedef struct DR_TWIN {
	unsigned int	tag;
	unsigned int	code[2];
} DR_TWIN;

typedef struct DR_TPAGE {
	unsigned int	tag;
	unsigned int	code[1];
} DR_TPAGE;

typedef struct DR_MASK {	/* ORIGINAL */
	unsigned int	tag;
	unsigned int	code[1];
} DR_MASK;

typedef struct FILL {		/* ORIGINAL */
	unsigned int	tag;
	unsigned char	r0,g0,b0,code;
	unsigned short	x0,y0;		// Note: coordinates must be in 16 pixel steps
	unsigned short	w,h;
} FILL;

typedef struct VRAM2VRAM {	/* ORIGINAL */
	unsigned int	tag;
	unsigned char	p0,p1,p2,code;
	unsigned short	x0,y0;
	unsigned short	x1,y1;
	unsigned short	w,h;
	unsigned int	nop[4];
} VRAM2VRAM;

// General structs

typedef struct RECT {
	short x,y;
	short w,h;
} RECT;

typedef struct DISPENV_RAW {
	unsigned int vid_mode;		// Video mode
	short vid_xpos,vid_ypos;	// Video position (not framebuffer)
	short fb_x,fb_y;			// Framebuffer display position	
} DISPENV_RAW;

typedef struct DISPENV {
	RECT	disp;
	RECT	screen;
	char	isinter;
	char	isrgb24;
	char	reverse;
	char	pad;
} DISPENV;

typedef struct DRAWENV {
	RECT			clip;		// Drawing area
	short			ofs[2];		// GPU draw offset (relative to draw area)
	RECT			tw;			// Texture window (doesn't do anything atm)
	unsigned short	tpage;		// Initial tpage value
	unsigned char	dtd;		// Dither processing flag (simply OR'ed to tpage)
	unsigned char	dfe;		// Drawing to display area blocked/allowed (simply OR'ed to tpage)
	unsigned char	isbg;		// Clear draw area if non-zero
	unsigned char	r0,g0,b0;	// Draw area clear color (if isbg iz nonzero)
	DR_ENV			dr_env;		// Draw mode packet area (used by PutDrawEnv)
} DRAWENV;

typedef struct TIM_IMAGE {
	unsigned int mode;
	RECT *crect;
	unsigned int *caddr;
	RECT *prect;
	unsigned int *paddr;
} TIM_IMAGE;


#ifdef __cplusplus
extern "C" {
#endif

// Function definitions (asm)

void ResetGraph(int mode);

int GetVideoMode();
void SetVideoMode(int mode);

void PutDispEnvRaw(DISPENV_RAW *disp);
void PutDispEnv(DISPENV *disp);
void PutDrawEnv(DRAWENV *draw);

void SetDispMask(int mask);

int VSync(int m);
int DrawSync(int m);
void WaitGPUcmd();
void WaitGPUdma();

// Callback hook functions
void *VSyncCallback(void (*func)());
void *DrawSyncCallback(void (*func)());

// Interrupt callback functions
void *DMACallback(int dma, void (*func)());
void *InterruptCallback(int irq, void (*func)());
void *GetInterruptCallback(int irq);				// Original
void RestartCallback();

void LoadImage(RECT *rect, unsigned int *data);
void StoreImage(RECT *rect, unsigned int *data);

void ClearOTagR(unsigned int* ot, int n);
void DrawOTag(unsigned int* ot);
void DrawPrim(void *pri);

void AddPrim(unsigned int* ot, void* pri);

// Function definitions (C)

int GetTimInfo(unsigned int *tim, TIM_IMAGE *timimg);

DISPENV *SetDefDispEnv(DISPENV *disp, int x, int y, int w, int h);
DRAWENV *SetDefDrawEnv(DRAWENV *draw, int x, int y, int w, int h);

#ifdef __cplusplus
}
#endif

#endif
