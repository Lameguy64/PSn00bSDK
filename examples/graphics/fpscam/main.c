/* 
 * LibPSn00b Example Programs
 *
 * First-Person and Look-At Camera Example
 * 2019 Meido-Tek Productions / PSn00bSDK Project
 *
 * Demonstrates both a first person perspective camera implementation with
 * full six degrees of movement using fixed point integer math and a look-at
 * tracking perspective. This example also shows how to use BIOS controller
 * functions and how to parse analog controller input.
 *
 * Controls:
 *  Up			- Look up
 *	Down		- Look down
 *	Left		- Look left
 *	Right		- Look right
 *	Triangle	- Move forward
 *	Cross		- Move backward
 *	Square		- Strafe left
 *	Circle		- Strafe right
 *	R1			- Slide up
 *	R2			- Slide down
 *	L1			- Look at cube
 *	Select		- Exit program (only works with CD loaders)
 *
 *
 * Example by Lameguy64
 *
 * Changelog:
 *
 *  July 18, 2019 - Initial version.
 *
 *  Sep 24, 2019 - Added camera position display and _boot() exit.
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxpad.h>
#include <psxapi.h>
#include <psxetc.h>
#include <inline_c.h>

#include "clip.h"
#include "lookat.h"

// OT and Packet Buffer sizes
#define OT_LEN			1024
#define PACKET_LEN		8192

// Screen resolution
#define SCREEN_XRES		320
#define SCREEN_YRES		240

// Screen center position
#define CENTERX			SCREEN_XRES>>1
#define CENTERY			SCREEN_YRES>>1


// Double buffer structure
typedef struct {
	DISPENV	disp;			// Display environment
	DRAWENV	draw;			// Drawing environment
	u_long 	ot[OT_LEN];		// Ordering table
	char 	p[PACKET_LEN];	// Packet buffer
} DB;

// Double buffer variables
DB		db[2];
int		db_active = 0;
char	*db_nextpri;
RECT	screen_clip;

// Pad data buffer
char pad_buff[2][34];


// For easier handling of vertex indexes
typedef struct {
	short v0,v1,v2,v3;
} INDEX;

// Cube vertices
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

// Cube face normals
SVECTOR cube_norms[] = {
	{ 0, 0, -ONE, 0 },
	{ 0, 0, ONE, 0 },
	{ 0, -ONE, 0, 0 },
	{ 0, ONE, 0, 0 },
	{ -ONE, 0, 0, 0 },
	{ ONE, 0, 0, 0 }
};

// Cube vertex indices
INDEX cube_indices[] = {
	{ 0, 1, 2, 3 },
	{ 4, 5, 6, 7 },
	{ 5, 4, 0, 1 },
	{ 6, 7, 3, 2 },
	{ 0, 2, 5, 7 },
	{ 3, 1, 6, 4 }
};

// Number of faces of cube
#define CUBE_FACES 6


// Light color matrix
// Each column represents the color matrix of each light source and is
// used as material color when using gte_ncs() or multiplied by a
// source color when using gte_nccs(). 4096 is 1.0 in this matrix
// A column of zeroes effectively disables the light source.
MATRIX color_mtx = {
	ONE, 0, 0,	// Red
	0, 0, 0,	// Green
	ONE, 0, 0	// Blue
};

// Light matrix
// Each row represents a vector direction of each light source.
// An entire row of zeroes effectively disables the light source.
MATRIX light_mtx = {
	/* X,  Y,  Z */
	-2048 , -2048 , -2048,
	0	  , 0	  , 0,
	0	  , 0	  , 0
};


// Function declarations
void init();
void display();
void sort_cube(MATRIX *mtx, VECTOR *pos, SVECTOR *rot);


// Main function
int main() {

	int i,p,xy_temp;
	int px,py;
	
	SVECTOR	rot;			// Rotation vector for cube
	VECTOR	pos;			// Position vector for cube
	
	SVECTOR verts[17][17];	// Vertex array for floor
	
	VECTOR	cam_pos;		// Camera position (in fixed point integers)
	VECTOR	cam_rot;		// Camera view angle (in fixed point integers)
	int		cam_mode;		// Camera mode (between first-person and look-at)
	
	VECTOR	tpos;			// Translation value for matrix calculations
	SVECTOR	trot;			// Rotation value for matrix calculations
	MATRIX	mtx,lmtx;		// Rotation matrices for geometry and lighting
	
	PADTYPE *pad;			// Pad structure pointer for parsing controller
	
	POLY_F4	*pol4;			// Flat shaded quad primitive pointer
	
	
	// Init graphics and GTE
	init();
	
	
	// Set coordinates to the vertex array for the floor
	for( py=0; py<17; py++ ) {
		for( px=0; px<17; px++ ) {
			
			setVector( &verts[py][px], 
				(100*(px-8))-50, 
				0, 
				(100*(py-8))-50 );
			
		}
	}
	
	
	// Camera default coordinates
	setVector( &cam_pos, 0, ONE*-200, 0 );
	setVector( &cam_rot, 0, 0, 0 );
	
	
	// Main loop
	while( 1 ) {
		
		// Set pad buffer data to pad pointer
		pad = (PADTYPE*)&pad_buff[0][0];
		
		// Parse controller input
		cam_mode = 0;
		
		// Divide out fractions of camera rotation
		trot.vx = cam_rot.vx >> 12;
		trot.vy = cam_rot.vy >> 12;
		trot.vz = cam_rot.vz >> 12;
		
		if( pad->stat == 0 ) {
			
			// For digital pad, dual-analog and dual-shock
			if( ( pad->type == 0x4 ) || ( pad->type == 0x5 ) || ( pad->type == 0x7 ) ) {
				
				// The button status bits are inverted,
				// so 0 means pressed in this case
				
				// Look controls
				if( !(pad->btn&PAD_UP) ) {
					
					// Look up
					cam_rot.vx -= ONE*8;
					
				} else if( !(pad->btn&PAD_DOWN) ) {
					
					// Look down
					cam_rot.vx += ONE*8;
					
				}
				
				if( !(pad->btn&PAD_LEFT) ) {
					
					// Look left
					cam_rot.vy += ONE*8;
					
				} else if( !(pad->btn&PAD_RIGHT) ) {
					
					// Look right
					cam_rot.vy -= ONE*8;
					
				}
				
				// Movement controls
				if( !(pad->btn&PAD_TRIANGLE) ) {
					
					// Move forward
					cam_pos.vx -= (( isin( trot.vy )*icos( trot.vx ) )>>12)<<2;
					cam_pos.vy += isin( trot.vx )<<2;
					cam_pos.vz += (( icos( trot.vy )*icos( trot.vx ) )>>12)<<2;
					
				} else if( !(pad->btn&PAD_CROSS) ) {
					
					// Move backward
					cam_pos.vx += (( isin( trot.vy )*icos( trot.vx ) )>>12)<<2;
					cam_pos.vy -= isin( trot.vx )<<2;
					cam_pos.vz -= (( icos( trot.vy )*icos( trot.vx ) )>>12)<<2;
					
				}
				
				if( !(pad->btn&PAD_SQUARE ) ) {
					
					// Slide left
					cam_pos.vx -= icos( trot.vy )<<2;
					cam_pos.vz -= isin( trot.vy )<<2;
					
				} else if( !(pad->btn&PAD_CIRCLE ) ) {
					
					// Slide right
					cam_pos.vx += icos( trot.vy )<<2;
					cam_pos.vz += isin( trot.vy )<<2;
					
				}
				
				if( !(pad->btn&PAD_R1) ) {
					
					// Slide up
					cam_pos.vx -= (( isin( trot.vy )*isin( trot.vx ) )>>12)<<2;
					cam_pos.vy -= icos( trot.vx )<<2;
					cam_pos.vz += (( icos( trot.vy )*isin( trot.vx ) )>>12)<<2;
					
				}
				
				if( !(pad->btn&PAD_R2) ) {
					
					// Slide down
					cam_pos.vx += (( isin( trot.vy )*isin( trot.vx ) )>>12)<<2;
					cam_pos.vy += icos( trot.vx )<<2;
					cam_pos.vz -= (( icos( trot.vy )*isin( trot.vx ) )>>12)<<2;
					
				}
				
				// Look at cube
				if( !(pad->btn&PAD_L1) ) {
					
					cam_mode = 1;
					
				}
				
				if( !(pad->btn&PAD_SELECT) ) {
					_boot();
				}
				
			}
			
			// For dual-analog and dual-shock (analog input)
			if( ( pad->type == 0x5 ) || ( pad->type == 0x7 ) ) {
				
				// Moving forwards and backwards
				if( ( (pad->ls_y-128) < -16 ) || ( (pad->ls_y-128) > 16 ) ) {
					
					cam_pos.vx += ((( isin( trot.vy )*icos( trot.vx ) )>>12)*(pad->ls_y-128))>>5;
					cam_pos.vy -= (isin( trot.vx )*(pad->ls_y-128))>>5;
					cam_pos.vz -= ((( icos( trot.vy )*icos( trot.vx ) )>>12)*(pad->ls_y-128))>>5;
					
				}
				
				// Strafing left and right
				if( ( (pad->ls_x-128) < -16 ) || ( (pad->ls_x-128) > 16 ) ) {
					cam_pos.vx += (icos( trot.vy )*(pad->ls_x-128))>>5;
					cam_pos.vz += (isin( trot.vy )*(pad->ls_x-128))>>5;
				}
				
				// Look up and down
				if( ( (pad->rs_y-128) < -16 ) || ( (pad->rs_y-128) > 16 ) ) {
					cam_rot.vx += (pad->rs_y-128)<<9;
				}
				
				// Look left and right
				if( ( (pad->rs_x-128) < -16 ) || ( (pad->rs_x-128) > 16 ) ) {
					cam_rot.vy -= (pad->rs_x-128)<<9;
				}
				
			}
		
		}
		
		// Print out some info
		FntPrint(-1, "BUTTONS=%04x\n", pad->btn);
		FntPrint(-1, "X=%d Y=%d Z=%d\n", 
			cam_pos.vx>>12, 
			cam_pos.vy>>12, 
			cam_pos.vz>>12);
		FntPrint(-1, "RX=%d RY=%d\n", 
			cam_rot.vx>>12, 
			cam_rot.vy>>12);
		
		// First-person camera mode
		if( cam_mode == 0 ) {
			
			// Set rotation to the matrix
			RotMatrix( &trot, &mtx );
			
			// Divide out the fractions of camera coordinates and invert
			// the sign, so camera coordinates will line up to world
			// (or geometry) coordinates
			tpos.vx = -cam_pos.vx >> 12;
			tpos.vy = -cam_pos.vy >> 12;
			tpos.vz = -cam_pos.vz >> 12;
		
			// Apply rotation of matrix to translation value to achieve a
			// first person perspective
			ApplyMatrixLV( &mtx, &tpos, &tpos );
			
			// Set translation matrix
			TransMatrix( &mtx, &tpos );
			
		// Tracking mode
		} else {
			
			// Vector that defines the 'up' direction of the camera
			SVECTOR up = { 0, -ONE, 0 };
			
			// Divide out fractions of camera coordinates
			tpos.vx = cam_pos.vx >> 12;
			tpos.vy = cam_pos.vy >> 12;
			tpos.vz = cam_pos.vz >> 12;
			
			// Look at the cube
			LookAt(&tpos, &pos, &up, &mtx);
			
		}
		
		// Set rotation and translation matrix
		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );
		
		// Draw the floor
		pol4 = (POLY_F4*)db_nextpri;
		
		for( py=0; py<16; py++ ) {
			for( px=0; px<16; px++ ) {
				
				// Load first three vertices to GTE
				gte_ldv3( 
					&verts[py][px], 
					&verts[py][px+1], 
					&verts[py+1][px] );
				
				gte_rtpt();
				
				gte_avsz3();
				gte_stotz( &p );
				
				if( ( (p>>2) >= OT_LEN ) || ( (p>>2) <= 0 ) )
					continue;
				
				setPolyF4( pol4 );
				
				// Set the projected vertices to the primitive
				gte_stsxy0( &pol4->x0 );
				gte_stsxy1( &pol4->x1 );
				gte_stsxy2( &pol4->x2 );
				
				// Compute the last vertex and set the result
				gte_ldv0( &verts[py+1][px+1] );
				gte_rtps();
				gte_stsxy( &pol4->x3 );
				
				// Test if quad is off-screen, discard if so
				// Clipping is important as it not only prevents primitive
				// overflows (tends to happen on textured polys) but also
				// saves packet buffer space and speeds up rendering.
				if( quad_clip( &screen_clip, 
				(DVECTOR*)&pol4->x0, (DVECTOR*)&pol4->x1, 
				(DVECTOR*)&pol4->x2, (DVECTOR*)&pol4->x3 ) )
					continue;
				
				gte_avsz4();
				gte_stotz( &p );
				
				if((px+py)&0x1) {
					setRGB0( pol4, 128, 128, 128 );
				} else {
					setRGB0( pol4, 255, 255, 255 );
				}
				
				addPrim( db[db_active].ot+(p>>2), pol4 );
				pol4++;
			
			}
		}
		
		// Update nextpri variable (very important)
		db_nextpri = (char*)pol4;
		
		
		// Position the cube going around the floor bouncily
		setVector( &pos, 
			isin( rot.vy )>>4, 
			-300+(isin( rot.vy<<2 )>>5), 
			icos( rot.vy )>>3 );
		
		// Sort cube
		sort_cube( &mtx, &pos, &rot );
		
		// Make the cube SPEEN
		rot.vx += 8;
		rot.vy += 8;
		
		
		// Flush text to drawing area
		FntFlush(-1);
	
		// Swap buffers and draw the primitives
		display();
		
	}
	
	return 0;
	
}

void sort_cube(MATRIX *mtx, VECTOR *pos, SVECTOR *rot) {

	int i,p;
	POLY_F4 *pol4;
	
	// Object and light matrix for object
	MATRIX omtx,lmtx;
	
	// Set object rotation and position
	RotMatrix( rot, &omtx );
	TransMatrix( &omtx, pos );
	
	// Multiply light matrix to object matrix
	MulMatrix0( &light_mtx, &omtx, &lmtx );
	
	// Set result to GTE light matrix
	gte_SetLightMatrix( &lmtx );
	
	// Composite coordinate matrix transform, so object will be rotated and
	// positioned relative to camera matrix (mtx), so it'll appear as
	// world-space relative.
	CompMatrixLV( mtx, &omtx, &omtx );
	
	// Save matrix
	PushMatrix();
	
	// Set matrices
	gte_SetRotMatrix( &omtx );
	gte_SetTransMatrix( &omtx );
	
	// Sort the cube
	pol4 = (POLY_F4*)db_nextpri;
	
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
		if( ((p>>2) <= 0) || ((p>>2) >= OT_LEN) )
			continue;
		
		// Initialize a quad primitive
		setPolyF4( pol4 );
		
		// Set the projected vertices to the primitive
		gte_stsxy0( &pol4->x0 );
		gte_stsxy1( &pol4->x1 );
		gte_stsxy2( &pol4->x2 );
		
		// Compute the last vertex and set the result
		gte_ldv0( &cube_verts[cube_indices[i].v3] );
		gte_rtps();
		gte_stsxy( &pol4->x3 );
		
		// Test if quad is off-screen, discard if so
		if( quad_clip( &screen_clip,
		(DVECTOR*)&pol4->x0, (DVECTOR*)&pol4->x1, 
		(DVECTOR*)&pol4->x2, (DVECTOR*)&pol4->x3 ) )
			continue;
		
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
		
		// Sort primitive to the ordering table
		addPrim( db[db_active].ot+(p>>2), pol4 );
		
		// Advance to make another primitive
		pol4++;
		
	}
	
	// Update nextpri
	db_nextpri = (char*)pol4;
	
	// Restore matrix
	PopMatrix();
	
}

void init() {

	// Reset the GPU, also installs a VSync event handler
	ResetGraph( 0 );

	// Set display and draw environment areas
	// (display and draw areas must be separate, otherwise hello flicker)
	SetDefDispEnv( &db[0].disp, 0, 0, SCREEN_XRES, SCREEN_YRES );
	SetDefDrawEnv( &db[0].draw, SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES );
	
	// Enable draw area clear and dither processing
	setRGB0( &db[0].draw, 63, 0, 127 );
	db[0].draw.isbg = 1;
	db[0].draw.dtd = 1;
	
	
	// Define the second set of display/draw environments
	SetDefDispEnv( &db[1].disp, SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES );
	SetDefDrawEnv( &db[1].draw, 0, 0, SCREEN_XRES, SCREEN_YRES );
	
	setRGB0( &db[1].draw, 63, 0, 127 );
	db[1].draw.isbg = 1;
	db[1].draw.dtd = 1;
	
	// Apply the drawing environment of the first double buffer
	PutDrawEnv( &db[0].draw );
	
	// Clear both ordering tables to make sure they are clean at the start
	ClearOTagR( db[0].ot, OT_LEN );
	ClearOTagR( db[1].ot, OT_LEN );
	
	// Set primitive pointer address
	db_nextpri = db[0].p;
	
	// Set clip region
	setRECT( &screen_clip, 0, 0, SCREEN_XRES, SCREEN_YRES );
	
	
	// Initialize the GTE
	InitGeom();
	
	// Set GTE offset (recommended method  of centering)
	gte_SetGeomOffset( CENTERX, CENTERY );
	
	// Set screen depth (basically FOV control, W/2 works best)
	gte_SetGeomScreen( CENTERX );
	
	// Set light ambient color and light color matrix
	gte_SetBackColor( 63, 63, 63 );
	gte_SetColorMatrix( &color_mtx );
	
	
	// Init BIOS pad driver and set pad buffers (buffers are updated
	// automatically on every V-Blank)
	InitPAD(&pad_buff[0][0], 34, &pad_buff[1][0], 34);
	
	// Start pad
	StartPAD();
	
	// Don't make pad driver acknowledge V-Blank IRQ (recommended)
	ChangeClearPAD(0);
	
	// Load font and open a text stream
	FntLoad(960, 0);
	FntOpen(0, 8, 320, 216, 0, 100);
	
}

void display() {
	
	// Wait for GPU to finish drawing and vertical retrace
	DrawSync( 0 );
	VSync( 0 );
	
	// Swap buffers
	db_active ^= 1;
	db_nextpri = db[db_active].p;
	
	// Clear the OT of the next frame
	ClearOTagR( db[db_active].ot, OT_LEN );
	
	// Apply display/drawing environments
	PutDrawEnv( &db[db_active].draw );
	PutDispEnv( &db[db_active].disp );
	
	// Enable display
	SetDispMask( 1 );
	
	// Start drawing the OT of the last buffer
	DrawOTag( db[1-db_active].ot+(OT_LEN-1) );
	
}

