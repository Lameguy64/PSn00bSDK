/* 
 * LibPSn00b Example Programs
 *
 * Child Program Execution Example
 * 2020 - 2021 Meido-Tek Productions / PSn00bSDK Project
 *
 * This example demonstrates how to execute a child PS-EXE from a parent
 * PS-EXE using the Exec() function, and transferring execution back from
 * the child PS-EXE to the parent PS-EXE. Passing arguments to the child
 * PS-EXE is also demonstrated here.
 *
 * This is actually a modification of the balls example, modified to execute
 * a child program for this example.
 *
 * Example by Lameguy64
 *
 * Changelog:
 *
 *	May 10, 2021		- Variable types updated for psxgpu.h changes.
 *
 */
 
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxpad.h>

#define MAX_BALLS 1024

#define OT_LEN 		8

#define SCREEN_XRES	640
#define SCREEN_YRES	480

#define CENTER_X	SCREEN_XRES/2
#define CENTER_Y	SCREEN_YRES/2


/* Display and drawing environments */
DISPENV disp;
DRAWENV draw;

char pribuff[2][65536];			/* Primitive packet buffers */
uint32_t ot[2][OT_LEN];			/* Ordering tables */
char *nextpri;					/* Pointer to next packet buffer offset */
int db = 0;						/* Double buffer index */


/* Ball struct and array */
typedef struct {
	short x,y;
	short xdir,ydir;
	unsigned char r,g,b,p;
} BALL_TYPE;

BALL_TYPE balls[MAX_BALLS];

/* Ball texture reference */
extern const uint32_t ball16c[];

/* TIM image parameters for loading the ball texture and drawing sprites */
TIM_IMAGE tim;


void run_child(void);

char pad_buff[2][34];


void init(void) {
	
	int i;
	
	/* Reset GPU (also installs event handler for VSync) */
	printf("Init GPU... ");
	ResetGraph( 0 );
	printf("Done.\n");
	
	
	printf("Set video mode... ");
	
	/* Set display and draw environment parameters */
	SetDefDispEnv( &disp, 0, 0, SCREEN_XRES, SCREEN_YRES );
	SetDefDrawEnv( &draw, 0, 0, SCREEN_XRES, SCREEN_YRES );
	disp.isinter = 1; /* Enable interlace (required for hires) */
	
	/* Set clear color, area clear and dither processing */
	setRGB0( &draw, 63, 0, 127 );
	draw.isbg = 1;
	draw.dtd = 1;
	
	/* Apply the display and drawing environments */
	PutDispEnv( &disp );
	PutDrawEnv( &draw );
	
	printf("Done.\n");
	
	
	/* Upload the ball texture */
	printf("Upload texture... ");
	GetTimInfo( ball16c, &tim ); /* Get TIM parameters */
	
	LoadImage( tim.prect, tim.paddr );		/* Upload texture to VRAM */
	if( tim.mode & 0x8 ) {
		LoadImage( tim.crect, tim.caddr );	/* Upload CLUT if present */
	}
	
	printf("Done.\n");
	
	
	/* Calculate ball positions */
	printf("Calculating balls... ");
	
	for(i=0; i<MAX_BALLS; i++) {
		
		balls[i].x = (rand()%624);
		balls[i].y = (rand()%464);
		balls[i].xdir = 1-(rand()%3);
		balls[i].ydir = 1-(rand()%3);
		if( !balls[i].xdir ) balls[i].xdir = 1;
		if( !balls[i].ydir ) balls[i].ydir = 1;
		balls[i].xdir *= 2;
		balls[i].ydir *= 2;
		balls[i].r = (rand()%256);
		balls[i].g = (rand()%256);
		balls[i].b = (rand()%256);
		
	}
	
	printf("Done.\n");
	
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);
	
}

int main(int argc, const char* argv[]) {
	
	SPRT_16 *sprt;
	DR_TPAGE *tpri;
	
	int i,counter=0;
	
	
	/* Init graphics and stuff before doing anything else */
	init();
	
	
	/* Main loop */
	printf("Entering loop...\n");
	
	while(1) {
		
		PADTYPE *pad = (PADTYPE*)pad_buff[0];
		
		if( pad->stat == 0 ) {
			
			// For digital pad, dual-analog and dual-shock
			if( ( pad->type == 0x4 ) || ( pad->type == 0x5 ) || ( pad->type == 0x7 ) ) {
				
				if( !(pad->btn&PAD_START) ) {
					DrawSync(0);
					run_child();
				}
				
			}
			
		}
		
		/* Clear ordering table and set start address of primitive */
		/* buffer for next frame */
		ClearOTagR( ot[db], OT_LEN );
		nextpri = pribuff[db];
		
		/* Sort a balls snake */
		sprt = (SPRT_16*)nextpri;
		srand( 64 );
		for( i=0; i<32; i++ ) {
		
			setSprt16( sprt );
			setXY0( sprt, 
				(CENTER_X-8)+(isin((counter-(i<<4))<<3)>>5), 
				(CENTER_Y-8)-(icos((counter-(i<<2))<<3)>>5) );
			setRGB0( sprt, rand()%256, rand()%256, rand()%256 );
			setUV0( sprt, 0, 0 );
			setClut( sprt, tim.crect->x, tim.crect->y );
			
			addPrim( ot[db]+(OT_LEN-1), sprt );
			sprt++;
		
		}
		
		/* Sort the balls */
		for( i=0; i<MAX_BALLS; i++ ) {
			
			setSprt16( sprt );
			setXY0( sprt, balls[i].x, balls[i].y );
			setRGB0( sprt, balls[i].r, balls[i].g, balls[i].b );
			setUV0( sprt, 0, 0 );
			setClut( sprt, tim.crect->x, tim.crect->y );
			
			addPrim( ot[db]+(OT_LEN-1), sprt );
			sprt++;
			
			balls[i].x += balls[i].xdir;
			balls[i].y += balls[i].ydir;
			
			if( ( balls[i].x+16 ) > 640 ) {
				balls[i].xdir = -2;
			} else if( balls[i].x < 0 ) {
				balls[i].xdir = 2;
			}
			
			if( ( balls[i].y+16 ) > 480 ) {
				balls[i].ydir = -2;
			} else if( balls[i].y < 0 ) {
				balls[i].ydir = 2;
			}
			
		}
		nextpri = (char*)sprt;
		
		
		/* Sort a TPage primitive so the sprites will draw pixels from */
		/* the correct texture page in VRAM */
		tpri = (DR_TPAGE*)nextpri;
		setDrawTPage( tpri, 0, 0, 
			getTPage( 0, 0, tim.prect->x, tim.prect->y ) );
		addPrim( ot[db]+(OT_LEN-1), tpri );
		nextpri += sizeof(DR_TPAGE);
		
		/* Wait for GPU and VSync */
		DrawSync( 0 );
		VSync( 0 );
		
		/* Enable video output */
		SetDispMask( 1 );
	
		/* Since draw.isbg is non-zero this clears the screen */
		PutDrawEnv( &draw );
		
		/* Begin drawing the new frame */
		DrawOTag( ot[db]+(OT_LEN-1) );
		
		/* Alternate to the next buffer */
		db = !db;
		
		/* Increment counter for the snake animation */
		counter++;
		
	}
	
	return 0;

}

// PS-EXE header structure
typedef struct {
	char		id[16];
	struct EXEC	param;
	char		pad[1972];
} EXE_HEAD;

// Child program address
extern char child_exe[];

void run_child(void) {
	
	// Arguments for the child program
	const char *args[] =
	{
		"SAMPLE=0",
		"SESSION=1",
		"ARGH!"
	};
	
	// So child header is readable
	EXE_HEAD *exe = (EXE_HEAD*)child_exe;
	
	// Copy child executable to its intended adddress
	memcpy((void*)exe->param.t_addr, child_exe+2048, exe->param.t_size);

	// Prepare for program execution and disable interrupts
	DrawSync(0);
	StopPAD();
	StopCallback();
	FlushCache();

	// Execute child
	printf("Executing child...\n");
	Exec(&exe->param, 3, args);

	// Restore interrupts for this PS-EXE
	RestartCallback();
	printf("Child returned\n");

	// Re-init and re-enable pads
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);

	// Set this program's display mode
	SetDispMask(0);
	PutDispEnv(&disp);
}
