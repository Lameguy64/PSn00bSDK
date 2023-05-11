#include <stdint.h>
#include <stdio.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxpad.h>
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
	DISPENV		disp;			/* Display environment */
	DRAWENV		draw;			/* Drawing environment */
	uint32_t	ot[OT_LEN];		/* Ordering table */
	char		p[PACKET_LEN];	/* Packet buffer */
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
	{ 0, 0, ONE, 0 },
	{ 0, -ONE, 0, 0 },
	{ 0, ONE, 0, 0 },
	{ -ONE, 0, 0, 0 },
	{ ONE, 0, 0, 0 }
};

/* Cube vertex indices */
INDEX cube_indices[] = {
	{ 0, 1, 2, 3 },
	{ 4, 5, 6, 7 },
	{ 5, 4, 0, 1 },
	{ 6, 7, 3, 2 },
	{ 0, 2, 5, 7 },
	{ 3, 1, 6, 4 }
};

/* Number of faces of cube */
#define CUBE_FACES 6


/* Light color matrix */
/* Each column represents the color matrix of each light source and is */
/* used as material color when using gte_ncs() or multiplied by a */
/* source color when using gte_nccs(). 4096 is 1.0 in this matrix */
/* A column of zeroes disables the light source. */
MATRIX color_mtx = {
	ONE, 0, 0,	/* Red   */
	ONE, 0, 0,	/* Green */
	ONE, 0, 0	/* Blue  */
};

/* Light matrix */
/* Each row represents a vector direction of each light source. */
/* An entire row of zeroes disables the light source. */
MATRIX light_mtx = {
	/* X,  Y,  Z */
	-2048 , -2048 , -2048,
	0	  , 0	  , 0,
	0	  , 0	  , 0
};

char pad_buff[2][34];

/* Function declarations */
void init(void);
void display(void);


/* Main function */
int main(int argc, const char *argv[]) {

	int i,p,xy_temp;
	
	SVECTOR	rot = { 0 };			/* Rotation vector for Rotmatrix */
	VECTOR	pos = { 0, 0, 400 };	/* Translation vector for TransMatrix */
	MATRIX	mtx,lmtx;				/* Rotation matrices for geometry and lighting */
	
	POLY_F4	*pol4;					/* Flat shaded quad primitive pointer */
	
	
	printf( "Arguments passed: %d\n", argc );
	for( i=0; i<argc; i++ )
	{
		printf( "%s\n", argv[i] );
	}
	
	/* Init graphics and GTE */
	init();
	
	
	/* Main loop */
	while( 1 ) {
		
		PADTYPE *pad = (PADTYPE*)pad_buff[0];
		
		if( pad->stat == 0 ) {
			
			// For digital pad, dual-analog and dual-shock
			if( ( pad->type == 0x4 ) || ( pad->type == 0x5 ) || ( pad->type == 0x7 ) ) {
				
				if( !(pad->btn&PAD_SELECT) ) {
					break;
				}
				
			}
			
		}
		
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
		rot.vx += 16;
		rot.vz += 16;
		
		
		/* Draw the cube */
		pol4 = (POLY_F4*)db_nextpri;	
		
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
			setPolyF4( pol4 );
			
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
		
	}

	DrawSync(0);
	StopPAD();
	StopCallback();

	return 0;
}

void init(void) {

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
	
	/* Set light ambient color and light color matrix */
	gte_SetBackColor( 63, 63, 63 );
	gte_SetColorMatrix( &color_mtx );
	
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);
	
}

void display(void) {
	
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