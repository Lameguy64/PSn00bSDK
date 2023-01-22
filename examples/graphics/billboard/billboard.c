/* 
 * LibPSn00b Example Programs
 *
 * GTE Billboarding Sprites Example
 * 2019 - 2021 Meido-Tek Productions / PSn00bSDK Project
 *
 * Displays a bunch of sprites placed on the screen using 3D coordinates
 * that scale according to the distance from the screen. This is a quick
 * modification of the GTE cube example.
 *
 * Billboard sprites are useful for 2D projectiles flying across 3D space,
 * particle effects such as smoke as well as characters and objects
 * represented as 2D sprites.
 *
 * Example by Lameguy64
 *
 * Changelog:
 *
 *	May 10, 2021 - Variable types updated for psxgpu.h changes.
 *
 *  Sep 24, 2019 - Initial version.
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
#define SCREEN_XRES		320
#define SCREEN_YRES		240

/* Screen center position */
#define CENTERX			SCREEN_XRES>>1
#define CENTERY			SCREEN_YRES>>1


/* Double buffer structure */
typedef struct {
	DISPENV	disp;			/* Display environment */
	DRAWENV	draw;			/* Drawing environment */
	u_long 	ot[OT_LEN];		/* Ordering table */
	char 	p[PACKET_LEN];	/* Packet buffer */
} DB;

/* Double buffer variables */
DB		db[2];
int		db_active = 0;
char	*db_nextpri;

extern u_long	tim_image[];
TIM_IMAGE 		tim;

/* For easier handling of vertex indices */
typedef struct {
	short v0,v1,v2,v3;
} INDEX;

/* Sprite position vertices */
SVECTOR verts[] = {
	{ -50, -50, -50, 0 },
	{  50, -50, -50, 0 },
	{ -50,  50, -50, 0 },
	{  50,  50, -50, 0 },
	{  50, -50,  50, 0 },
	{ -50, -50,  50, 0 },
	{  50,  50,  50, 0 },
	{ -50,  50,  50, 0 }
};


/* Function declarations */
void init();
void display();


/* Main function */
int main() {

	int i,p,sz;
	
	SVECTOR	rot = { 0 };			/* Rotation vector for Rotmatrix */
	VECTOR	pos = { 0, 0, 160 };	/* Translation vector for TransMatrix */
	MATRIX	mtx,lmtx;				/* Rotation matrices for geometry and lighting */
	
	POLY_FT4	*quad;				/* Flat shaded quad primitive pointer */
	SVECTOR spos;
	
	/* Init graphics and GTE */
	init();
	
	
	/* Main loop */
	while( 1 ) {
		
		/* Set rotation and translation to the matrix */
		RotMatrix( &rot, &mtx );
		TransMatrix( &mtx, &pos );
		
		/* Set rotation and translation matrix */
		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );
		
		/* Make the sprites revolve around */
		rot.vy += 16;
		rot.vz += 16;
		
		/* Draw the sprites */
		quad = (POLY_FT4*)db_nextpri;	
		
		for( i=0; i<8; i++ ) {
			
			// Load the 3D coordinate of the sprite to GTE
			gte_ldv0(&verts[i]);
				
			// Rotation, Translation and Perspective Single
			gte_rtps();
			
			// Store depth
			gte_stsz(&p);

			// Don't sort sprite if depth is zero
			// (or divide by zero will happen later)
			if( p > 0 ) {
				
				// Store result to position vector
				gte_stsxy2(&spos);
				
				// Calculate sprite size, the divide operation might be a
				// performance killer but it's likely faster than performing
				// a lookat operation between sprite and camera, which some
				// billboard sprite implementations use.
				sz = (16*CENTERX)/p;
				
				// Prepare quad primitive
				setPolyFT4(quad);
				
				// Set quad coordinates
				setXY4(quad,
					spos.vx-sz, spos.vy-sz,
					spos.vx+sz, spos.vy-sz,
					spos.vx-sz, spos.vy+sz,
					spos.vx+sz, spos.vy+sz);
				
				// Set color
				setRGB0(quad, 128, 128, 128);
				
				// Set tpage
				quad->tpage = getTPage(tim.mode, 0, tim.prect->x, tim.prect->y);
				
				// Set CLUT
				setClut(quad, tim.crect->x, tim.crect->y);
				
				// Set texture coordinates
				setUVWH(quad, 0, 0, 64, 64);
					
				/* Sort primitive to the ordering table */
				addPrim(db[db_active].ot+(p>>2), quad);
				
				/* Advance to make another primitive */
				quad++;
			
			}
		}
		
		/* Update nextpri variable */
		/* (IMPORTANT if you plan to sort more primitives after this) */
		db_nextpri = (char*)quad;
		
		/* Swap buffers and draw the primitives */
		display();
		
	}
	
	return 0;
	
}

void init() {

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
	
	/* Set primitive pointer address */
	db_nextpri = db[0].p;
	
	/* Initialize the GTE */
	InitGeom();
	
	/* Set GTE offset (recommended method  of centering) */
	gte_SetGeomOffset( CENTERX, CENTERY );
	
	/* Set screen depth (basically FOV control, W/2 works best) */
	gte_SetGeomScreen( CENTERX );
	
	GetTimInfo( tim_image, &tim);
	
	LoadImage(tim.prect, tim.paddr);
	DrawSync(0);
	
	LoadImage(tim.crect, tim.caddr);
	DrawSync(0);
	
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
	
	/* Apply display/drawing environments */
	PutDrawEnv( &db[db_active].draw );
	PutDispEnv( &db[db_active].disp );
	
	/* Enable display */
	SetDispMask( 1 );
	
	/* Start drawing the OT of the last buffer */
	DrawOTag( db[1-db_active].ot+(OT_LEN-1) );
	
}