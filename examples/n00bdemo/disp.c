#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>
#include "smd.h"
#include "disp.h"

DISPENV disp;
DRAWENV draw;

char pribuff[2][131072];
unsigned int ot[2][OT_LEN];
char *nextpri;
int db = 0;

MATRIX mtx;


void initDisplay() {
	
	ResetGraph( 3 );
	
	if( GetVideoMode() == MODE_NTSC ) {
		SetDefDispEnv( &disp, 0, 0, 640, 480 );
		SetDefDrawEnv( &draw, 0, 0, 640, 480 );
		scSetClipRect( 0, 0, 640, 480 );
		printf("NTSC System.\n");
	} else {
		SetDefDispEnv( &disp, 0, 0, 640, 512 );
		SetDefDrawEnv( &draw, 0, 0, 640, 512 );
		scSetClipRect( 0, 0, 640, 512 );
		disp.screen.y = 20;
		disp.screen.h = 256;
		printf("PAL System.\n");
	}
	
	disp.isinter = 1;
	draw.isbg = 1;
	
	PutDispEnv( &disp );
	PutDrawEnv( &draw );
	
	ClearOTagR( ot[0], OT_LEN );
	ClearOTagR( ot[1], OT_LEN );
	nextpri = pribuff[0];
	
	InitGeom();
	gte_SetGeomScreen( 320 );
	
	if( GetVideoMode() == MODE_NTSC ) {
		gte_SetGeomOffset( 320, 240 );
	} else {
		gte_SetGeomOffset( 320, 256 );
	}
	
}

void display() {

	VSync();
	DrawSync();
	
	PutDrawEnv( &draw );
	DrawOTag( ot[db]+OT_LEN-1 );
	
	db ^= 1;
	ClearOTagR( ot[db], OT_LEN );
	nextpri = pribuff[db];
	
	SetDispMask( 1 );
	
}