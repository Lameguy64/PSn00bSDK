/* 
 * LibPSn00b Example Programs
 *
 * Off-screen Render to Texture Example
 * 2019 - 2021 Meido-Tek Productions / PSn00bSDK Project
 *
 * Demonstrates quick render to texture for multi-texture style effects,
 * view screens and more. This example also shows how to use multiple
 * ordering tables and chaining them together so it can all be rendered
 * with a single DrawOTag() call.
 *
 * Example by Lameguy64
 *
 * Changelog:
 *
 *	May 10, 2021		- Variable types updated for psxgpu.h changes.
 *
 *  Oct 26, 2019		- Initial version.
 *
 */
 
#include <sys/types.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>


/* OT and Packet Buffer sizes */
#define OT_LEN			256
#define PACKET_LEN		1024


/* Screen resolution */
/* (note: display/draw code is hardcoded for double buffer) */
#define SCREEN_XRES		320
#define SCREEN_YRES		240


/* Screen center position */
#define CENTERX			SCREEN_XRES>>1
#define CENTERY			SCREEN_YRES>>1


/* Double buffer structure */
typedef struct DB
{
	DISPENV	disp;			/* Display environment */
	DRAWENV	draw;			/* Drawing environment */
	u_long 	ot[OT_LEN];		/* Main ordering table */
	u_long	sub_ot[2][4];	/* Second ordering table for r2t stuff */
	char 	p[PACKET_LEN];	/* Packet buffer */
} DB;


/* Double buffer variables */
DB		db[2];
int		db_active = 0;
char	*db_nextpri;


/* For easier handling of vertex indices */
typedef struct {
	short v0,v1,v2,v3;
} INDEX;

/* Cube vertices */
SVECTOR cube_verts[] = {
	{ -100, -100, -100, 0 },
	{  100, -100, -100, 0 },
	{ -100,  100, -100, 0 },
	{  100,  100, -100, 0 },
	{  100, -100,  100, 0 },
	{ -100, -100,  100, 0 },
	{  100,  100,  100, 0 },
	{ -100,  100,  100, 0 }
};

/* Cube face normals */
SVECTOR cube_norms[] = {
	{ 0, 0, -ONE, 0 },
	{ 0, -ONE, 0, 0 },
	{ 0, 0, ONE, 0 },
	{ 0, ONE, 0, 0 },
	{ ONE, 0, 0, 0 },
	{ -ONE, 0, 0, 0 }
};

/* Cube vertex indices */
INDEX cube_indices[] = {
	{ 0, 1, 2, 3 },
	{ 5, 4, 0, 1 },
	{ 4, 5, 6, 7 },
	{ 6, 7, 3, 2 },
	{ 3, 1, 6, 4 },
	{ 0, 2, 5, 7 }
};

/* Number of cube faces */
#define CUBE_FACES 6

/* Light color matrix */
MATRIX color_mtx = {
	ONE, 0, 0,	/* Red   */
	ONE, 0, 0,	/* Green */
	ONE, 0, 0	/* Blue  */
};

/* Light matrix */
MATRIX light_mtx = {
	/* X,  Y,  Z */
	-2048 , -2048 , -2048,
	0	  , 0	  , 0,
	0	  , 0	  , 0
};


/* Reference texture data */
extern u_long tim_blendpattern[];


/* TPage and CLUT values */
unsigned short rendertex_tpage;		/* For the render to texture cube */
unsigned short bpattern_tpage;		/* For the scrolling blending pattern */
unsigned short bpattern_clut;


/* Function declarations */
void init();
void display();

/* This function sorts a cube that is drawn
 * to an offscreen area specified by *area */
void sort_cube(u_long *ot, RECT *area);
void sort_multitex(u_long *ot, RECT *area, int count);

/* Main function */
int main() {

	int i,p,xy_temp;
	int count = 0;
	
	SVECTOR	rot = { 0 };			/* Rotation vector for Rotmatrix */
	VECTOR	pos = { 0, 0, 400 };	/* Translation vector for TransMatrix */
	MATRIX	mtx,lmtx;				/* Rotation matrices for geometry and lighting */
	
	VECTOR	spos = { 0, 0, 250 };
	SVECTOR	srot = { 0 };
	
	RECT	texarea,area,dtexarea;
	RECT	cubearea;
	
	POLY_FT4	*pol4;					/* Flat shaded quad primitive pointer */
	
	
	/* Init graphics and GTE */
	init();
	
	setRECT(&area, 704, 0, 64, 64);
	setRECT(&cubearea, 704, 64, 64, 64);
	
	setRECT(&texarea, 0, 0, 64>>3, 64>>3);
	setRECT(&dtexarea, 0, 0, 0, 0);
	
	rendertex_tpage = getTPage(2, 0, area.x, area.y);
	
	/* Main loop */
	while( 1 ) {
		
		/* Sort multi-texture stuff */
		sort_multitex(db[db_active].ot+(OT_LEN-1), &area, count);
		
		/* Matrix stuff for render to texture cube */
		RotMatrix( &srot, &mtx );
		TransMatrix( &mtx, &spos );
		
		MulMatrix0( &light_mtx, &mtx, &lmtx );
		
		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );
		gte_SetLightMatrix( &lmtx );
		
		/* Reduce FOV to fit in area and sort cube */
		gte_SetGeomScreen(32);
		
		sort_cube(db[db_active].ot+(OT_LEN-1), &cubearea);
		
		gte_SetGeomScreen(CENTERX);
		
		
		/* Set rotation and translation to the matrix */
		RotMatrix( &rot, &mtx );
		TransMatrix( &mtx, &pos );
		
		/* Multiply light matrix by rotation matrix so light source */
		/* won't appear relative to the model's rotation */
		MulMatrix0( &light_mtx, &mtx, &lmtx );
		
		/* Set rotation and translation matrix */
		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );
		
		/* Set light matrix */
		gte_SetLightMatrix( &lmtx );
		
		/* Make the cube SPEEN */
		rot.vx += 4;
		rot.vz += 4;
		
		srot.vx += 8;
		srot.vy += 8;
		srot.vz -= 8;
		
		/* Draw the cube */
		pol4 = (POLY_FT4*)db_nextpri;	
		
		for( i=0; i<CUBE_FACES; i++ ) {
			
			/* Load the first 3 vertices of a quad to the GTE */
			gte_ldv3( 
				&cube_verts[cube_indices[i].v0], 
				&cube_verts[cube_indices[i].v1], 
				&cube_verts[cube_indices[i].v2] );
				
			/* Rotation, Translation and Perspective Triple */
			gte_rtpt();
			
			/* Compute normal clip for backface culling */
			gte_nclip();
			
			/* Get result*/
			gte_stopz( &p );
			
			/* Skip this face if backfaced */
			if( p < 0 )
				continue;
			
			/* Calculate average Z for depth sorting */
			gte_avsz4();
			gte_stotz( &p );
			
			/* Skip if clipping off */
			/* (the shift right operator is to scale the depth precision) */
			if( (p>>2) > OT_LEN )
				continue;
			
			/* Initialize a quad primitive */
			setPolyFT4( pol4 );
			
			/* Set the projected vertices to the primitive */
			gte_stsxy0( &pol4->x0 );
			gte_stsxy1( &pol4->x1 );
			gte_stsxy2( &pol4->x2 );
			
			/* Compute the last vertex and set the result */
			gte_ldv0( &cube_verts[cube_indices[i].v3] );
			gte_rtps();
			gte_stsxy( &pol4->x3 );
			
			/* Load primitive color even though gte_ncs() doesn't use it. */
			/* This is so the GTE will output a color result with the */
			/* correct primitive code. */
			gte_ldrgb( &pol4->r0 );
			
			/* Load the face normal */
			gte_ldv0( &cube_norms[i] );
			
			/* Normal Color Single */
			gte_ncs();
			
			/* Store result to the primitive */
			gte_strgb( &pol4->r0 );
			
			// Map to render to texture texture
			pol4->tpage = rendertex_tpage;
			if( (i&0x1) == 0 )
				setUVWH(pol4, 0, 0, 63, 63);
			else
				setUVWH(pol4, 0, 64, 63, 63);
				
			
			/* Sort primitive to the ordering table */
			addPrim( db[db_active].ot+(p>>2), pol4 );
			
			/* Advance to make another primitive */
			pol4++;
			
		}
		
		/* Update nextpri variable */
		/* (IMPORTANT if you plan to sort more primitives after this) */
		db_nextpri = (char*)pol4;
		
		/* Swap buffers and draw the primitives */
		display();
		
		count++;
		
	}
	
	return 0;
	
}

void init() {

	TIM_IMAGE tim;
	
	/* Reset the GPU, also installs a VSync event handler */
	ResetGraph( 0 );

	/* Set display and draw environment areas */
	/* (display and draw areas must be separate, otherwise hello flicker) */
	SetDefDispEnv( &db[0].disp, 0, 0, SCREEN_XRES, SCREEN_YRES );
	SetDefDrawEnv( &db[0].draw, SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES );
	
	/* Enable draw area clear and dither processing */
	setRGB0( &db[0].draw, 63, 0, 127 );
	db[0].draw.isbg = 1;
	db[0].draw.dtd = 1;
	
	
	/* Define the second set of display/draw environments */
	SetDefDispEnv( &db[1].disp, SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES );
	SetDefDrawEnv( &db[1].draw, 0, 0, SCREEN_XRES, SCREEN_YRES );
	
	setRGB0( &db[1].draw, 63, 0, 127 );
	db[1].draw.isbg = 1;
	db[1].draw.dtd = 1;
	
	
	/* Apply the drawing environment of the first double buffer */
	PutDrawEnv( &db[0].draw );
	
	
	/* Clear both ordering tables to make sure they are clean at the start */
	ClearOTagR( db[0].ot, OT_LEN );
	ClearOTagR( db[1].ot, OT_LEN );
	ClearOTagR( db[0].sub_ot[0], 4 );
	ClearOTagR( db[0].sub_ot[1], 4 );
	ClearOTagR( db[1].sub_ot[0], 4 );
	ClearOTagR( db[1].sub_ot[1], 4 );
	
	/* Set primitive pointer address */
	db_nextpri = db[0].p;
	
	/* Initialize the GTE */
	InitGeom();
	
	/* Set GTE offset (recommended method  of centering) */
	gte_SetGeomOffset( CENTERX, CENTERY );
	
	/* Set screen depth (basically FOV control, W/2 works best) */
	gte_SetGeomScreen( CENTERX );
	
	/* Set light ambient color and light color matrix */
	gte_SetBackColor( 63, 63, 63 );
	gte_SetColorMatrix( &color_mtx );
	
	GetTimInfo(tim_blendpattern, &tim);
	if( tim.mode & 0x8 )
	{
		LoadImage( tim.crect, tim.caddr );	/* Upload CLUT if present */
	}
	LoadImage( tim.prect, tim.paddr );		/* Upload texture to VRAM */
	
	bpattern_tpage	= getTPage(0, 1, tim.prect->x, tim.prect->y);
	bpattern_clut	= getClut(tim.crect->x, tim.crect->y); 
	
}

void display() {
	
	/* Wait for GPU to finish drawing and vertical retrace */
	DrawSync( 0 );
	VSync( 0 );
	
	/* Swap buffers */
	db_active ^= 1;
	db_nextpri = db[db_active].p;
	
	/* Clear the OT of the next frame */
	ClearOTagR( db[db_active].ot, OT_LEN );
	ClearOTagR( db[db_active].sub_ot[0], 4 );
	ClearOTagR( db[db_active].sub_ot[1], 4 );
	
	/* Apply display/drawing environments */
	PutDrawEnv( &db[db_active].draw );
	PutDispEnv( &db[db_active].disp );
	
	/* Enable display */
	SetDispMask( 1 );
	
	/* Start drawing the OT of the last buffer */
	DrawOTag( db[1-db_active].ot+(OT_LEN-1) );
	
}

void sort_multitex(u_long *ot, RECT *area, int count)
{
	DR_TPAGE	*ptpage;
	FILL		*pfill;
	DR_AREA		*parea;
	DR_TWIN		*ptwin;
	DR_OFFSET 	*poffs;
	
	SPRT		*psprt;
	
	/* Texture window constraint */
	/* (coordinates specified in units of 8 pixels) */
	RECT texwindow = { 0, 0, 64>>3, 64>>3 };
	
	/* Sort the sub OT to the specified OT level */
	addPrims(
		ot,							/* Target OT			*/
		db[db_active].sub_ot[0]+3,	/* Start of OT to sort	*/
		db[db_active].sub_ot[0]);	/* End of OT to sort	*/
	
	
	/* Sort a FILL primitive to clear the off-screen area */
	pfill = (FILL*)db_nextpri;
	setFill(pfill);
	setXY0(pfill, area->x, area->y);
	setWH(pfill, 64, 64);
	setRGB0(pfill, 0, 0, 0);
	addPrim(db[db_active].sub_ot[0]+3, pfill);
	db_nextpri += sizeof(FILL);
	
	
	/* Sort draw area primitives to set the drawing target */
	parea = (DR_AREA*)db_nextpri;
	
	setDrawArea(parea, area);					/* Sets to off-screen area */
	addPrim(db[db_active].sub_ot[0]+3, parea);
	parea++;
	
	setDrawArea(parea,							/* Reverts to draw area */
		&db[1-db_active].draw.clip);
	addPrim(db[db_active].sub_ot[0]+1, parea);
	parea++;
	db_nextpri = (char*)parea;
	
	
	/* Sort offset primitives to set the drawing offset to the target */
	poffs = (DR_OFFSET*)db_nextpri;
	
	setDrawOffset(poffs, area->x, area->y);		/* Sets to off-screen area */
	addPrim(db[db_active].sub_ot[0]+3, poffs);
	poffs++;
	
	setDrawOffset(poffs, 						/* Reverts to draw area */
		db[1-db_active].draw.clip.x,
		db[1-db_active].draw.clip.y);
	addPrim(db[db_active].sub_ot[0]+1, poffs);
	poffs++;
	db_nextpri = (char*)poffs;
	
	
	/* This sets the active texture page for the SPRT primitives */
	ptpage = (DR_TPAGE*)db_nextpri;
	setDrawTPage(ptpage, 1, 0, bpattern_tpage);
	addPrim(db[db_active].sub_ot[0]+3, ptpage);
	ptpage++;
	db_nextpri = (char*)ptpage;
	
	
	/* Sort a DR_TWIN primitive to wrap texture coordinates to 64x64 */
	ptwin = (DR_TWIN*)db_nextpri;
	
	setTexWindow(ptwin, &texwindow);			/* Set window constraint */
	addPrim(db[db_active].sub_ot[0]+3, ptwin);
	ptwin++;
	
	texwindow.w = 0;							/* Clear window constraint */
	texwindow.h = 0;
	setTexWindow(ptwin, &texwindow);
	addPrim(db[db_active].sub_ot[0]+1, ptwin);
	ptwin++;
	db_nextpri = (char*)ptwin;
	
	
	/* Sort blending and scrolling sprites layering over one another */
	psprt = (SPRT*)db_nextpri;
	
	/* Sort pattern in green scrolling up-left */
	setSprt(psprt);
	setSemiTrans(psprt, 1);
	setXY0(psprt, 0, 0);
	setWH(psprt, 64, 64);
	setUV0(psprt, (count>>1)&0x3F, count&0x3F);
	setRGB0(psprt, 0, 91, 0);
	psprt->clut = bpattern_clut;
	addPrim(db[db_active].sub_ot[0]+1, psprt);
	psprt++;
	
	/* Sort pattern in blue scrolling up-right*/
	setSprt(psprt);
	setSemiTrans(psprt, 1);
	setXY0(psprt, 0, 0);
	setWH(psprt, 64, 64);
	setUV0(psprt, (-count>>1)&0x3F, (count>>1)&0x3F);
	setRGB0(psprt, 0, 0, 91);
	psprt->clut = bpattern_clut;
	addPrim(db[db_active].sub_ot[0]+1, psprt);
	psprt++;
	
	/* Sort pattern in red scrolling down-right */
	setSprt(psprt);
	setSemiTrans(psprt, 1);
	setXY0(psprt, 0, 0);
	setWH(psprt, 64, 64);
	setUV0(psprt, (-count>>1)&0x3F, (-count>>1)&0x3F);
	setRGB0(psprt, 91, 0, 0);
	psprt->clut = bpattern_clut;
	addPrim(db[db_active].sub_ot[0]+1, psprt);
	psprt++;
	
	/* Sort pattern in grey scrolling up-left */
	setSprt(psprt);
	setXY0(psprt, 0, 0);
	setWH(psprt, 64, 64);
	setUV0(psprt, count&0x3F, (count>>1)&0x3F);
	setRGB0(psprt, 64, 64, 64);
	psprt->clut = bpattern_clut;
	addPrim(db[db_active].sub_ot[0]+1, psprt);
	psprt++;
	
	db_nextpri = (char*)psprt;
	
}

void sort_cube(u_long *ot, RECT *area)
{
	int i,p;
	POLY_FT4* 	pol4;
	FILL*		pfill;
	DR_AREA*	parea;
	DR_OFFSET*	poffs;
	
	addPrims(
		ot,
		db[db_active].sub_ot[1]+3,
		db[db_active].sub_ot[1]);
	
	pfill = (FILL*)db_nextpri;
	setFill(pfill);
	setXY0(pfill, area->x, area->y);
	setWH(pfill, 64, 64);
	setRGB0(pfill, 128, 91, 0);
	addPrim(db[db_active].sub_ot[1]+3, pfill);
	db_nextpri += sizeof(FILL);
	
	parea = (DR_AREA*)db_nextpri;
	setDrawArea(parea, area);
	addPrim(db[db_active].sub_ot[1]+3, parea);
	parea++;
	setDrawArea(parea, &db[1-db_active].draw.clip);
	addPrim(db[db_active].sub_ot[1]+1, parea);
	parea++;
	db_nextpri = (char*)parea;
	
	poffs = (DR_OFFSET*)db_nextpri;
	setDrawOffset(poffs, area->x, area->y);
	addPrim(db[db_active].sub_ot[1]+3, poffs);
	poffs++;
	setDrawOffset(poffs, db[1-db_active].draw.clip.x, db[1-db_active].draw.clip.y);
	addPrim(db[db_active].sub_ot[1]+1, poffs);
	poffs++;
	db_nextpri = (char*)poffs;
	
	
	gte_SetGeomOffset(32, 32);
	
	// Sort the cube
	pol4 = (POLY_FT4*)db_nextpri;
	
	for( i=0; i<CUBE_FACES; i++ ) {
		
		// Load the first 3 vertices of a quad to the GTE 
		gte_ldv3( 
			&cube_verts[cube_indices[i].v0], 
			&cube_verts[cube_indices[i].v1], 
			&cube_verts[cube_indices[i].v2] );
			
		// Rotation, Translation and Perspective Triple
		gte_rtpt();
		
		// Compute normal clip for backface culling
		gte_nclip();
		
		// Get result
		gte_stopz( &p );
		
		// Skip this face if backfaced
		if( p < 0 )
			continue;
		
		// Calculate average Z for depth sorting
		gte_avsz3();
		gte_stotz( &p );
		
		// Skip if clipping off
		// (the shift right operator is to scale the depth precision)
		/*if( ((p>>6) <= 0) || ((p>>6) >= 4) )
			continue;*/
		
		// Initialize a quad primitive
		setPolyFT4( pol4 );
		
		// Set the projected vertices to the primitive
		gte_stsxy0( &pol4->x0 );
		gte_stsxy1( &pol4->x1 );
		gte_stsxy2( &pol4->x2 );
		
		// Compute the last vertex and set the result
		gte_ldv0( &cube_verts[cube_indices[i].v3] );
		gte_rtps();
		gte_stsxy( &pol4->x3 );
		
		// Load primitive color even though gte_ncs() doesn't use it.
		// This is so the GTE will output a color result with the
		// correct primitive code.
		gte_ldrgb( &pol4->r0 );
		
		// Load the face normal
		gte_ldv0( &cube_norms[i] );
		
		// Normal Color Single
		gte_ncs();
		
		// Store result to the primitive
		gte_strgb( &pol4->r0 );
		
		gte_avsz4();
		gte_stotz( &p );
		
		pol4->tpage = rendertex_tpage;
		setUVWH(pol4, 0, 0, 63, 63);
			
		// Sort primitive to the ordering table
		addPrim( db[db_active].sub_ot[1]+1, pol4 );
		
		// Advance to make another primitive
		pol4++;
		
	}
	
	// Update nextpri
	db_nextpri = (char*)pol4;
	
	gte_SetGeomOffset(CENTERX, CENTERY);
}