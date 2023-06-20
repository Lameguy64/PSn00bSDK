/*
 * LibPSn00b Example Programs
 *
 * n00bDEMO Source Code
 * 2019 - 2021 Meido-Tek Productions / PSn00bSDK Project
 *
 * To build, simply run make. Make sure you have the lzpack tool accessible
 * through your PATH environment variable.
 *
 * Demo by Lameguy64
 *
 * Changelog:
 *
 *	Mar 24, 2022 - Added FPS counter.
 *
 *	Mar 12, 2022 - Added Konami System 573 support.
 *
 *	May 10, 2021 - Variable types updated for psxgpu.h changes.
 *
 *	Apr 4, 2019	 - Some code clean-up and added more comments.
 *
 *	Mar 20, 2019 - Initial completed version.
 *
 */

// Uncomment to enable Konami System 573 support
// (seems to break the demo when loading it using Caetla on a cheat cartridge)
//#define SYSTEM_573_SUPPORT

#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <psxetc.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxspu.h>
#include <psxapi.h>
#include <inline_c.h>
#include <hwregs_c.h>
#include <string.h>
#include <lzp/lzp.h>
#include <lzp/lzqlp.h>
#include <smd/smd.h>

#include "data.h"
#include "disp.h"
#include "logo.h"
#include "plasma_tbl.h"


// Standard light color matrix
MATRIX lgt_colmtx = {
	//#1,#2,#3
	ONE, 0, 0,		// R
	ONE, 0, 0,		// G
	ONE, 0, 0		// B
};

// SMD data global pointers
SMD *o_rbowshade;
SMD *o_world, *o_lightbulb;
SMD *o_timerift, *o_star;
SMD *o_hatkid,*o_bungirl;;

// TPage and CLUT globals
unsigned short lamelotl_tpage,psn00b_tpage;
unsigned short font_tpage,font_clut;
SPRT llotl_sprite;
SPRT psn00b_sprite;

// Timer and FPS counter
volatile int timer_counter = 0, frame_counter = 0, frame_rate = 0;

// Some function definition
void sort_overlay(int showlotl);
void lightdemo();

#define K573_WATCHDOG	*((volatile unsigned short *) 0xbf5c0000)
#define K573_EXP1_CFG	0x24173f47

void timerTick() {
	if (!(--timer_counter)) {
		timer_counter	= 100;
		frame_rate		= frame_counter;
		frame_counter	= 0;
	}

#ifdef SYSTEM_573_SUPPORT
	/*
		The only thing required to support the 573 is to periodically reset the
		watchdog. Hooking the vblank IRQ (through VSyncCallback) is the "right"
		way to do it, however using a hardware timer running at a higher rate
		(100 Hz) seems to improve stability.
	*/
	K573_WATCHDOG = 0;
#endif
}

void timerSetup() {
	EnterCriticalSection();

#ifdef SYSTEM_573_SUPPORT
	BUS_EXP1_ADDR	= 0x1f000000;
	BUS_EXP1_CFG	= K573_EXP1_CFG;
#endif

	TIMER_CTRL(2)	= 0x0258;				// CLK/8 input, IRQ on reload
	TIMER_RELOAD(2)	= (F_CPU / 8) / 100;	// 100 Hz

	// Configure timer 2 IRQ
	ChangeClearRCnt(2, 0);
	InterruptCallback(6, &timerTick);
	ExitCriticalSection();
}

void UploadTIM(TIM_IMAGE *tim) {

	/*
		Very simple texture upload function
	*/

	LoadImage( tim->prect, tim->paddr );

	if( tim->mode & 0x8 )
		LoadImage( tim->crect, tim->caddr );

}

void loadTextures() {

	/*
		Unpack textures from an embedded LZP archive and upload them to VRAM.
	*/
	int i;
	int *ttim,j;
	QLP_HEAD *tex_buff;
	TIM_IMAGE tim;

	i = lzpSearchFile( "textures", lz_resources );
	tex_buff = (QLP_HEAD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( tex_buff, lz_resources, i );


	for( j=0; j<qlpFileCount( tex_buff )-4; j++ ) {

		if( !GetTimInfo( (u_long*)qlpFileAddr( j, tex_buff ), &tim ) ) {

			UploadTIM( &tim );

		}

	}


	GetTimInfo( (u_long*)qlpFileAddr(
		qlpFindFile( "n00blogo", tex_buff ), tex_buff ), &tim );

	UploadTIM( &tim );

	psn00b_tpage = getTPage( 0, 0, tim.prect->x, tim.prect->y )|0x200;
	setSprt( &psn00b_sprite );
	setClut( &psn00b_sprite, tim.crect->x, tim.crect->y );
	setWH( &psn00b_sprite, tim.prect->w<<2, tim.prect->h );
	setUV0( &psn00b_sprite, (tim.prect->x%64)<<2, tim.prect->y );
	setRGB0( &psn00b_sprite, 128, 128, 128 );


	GetTimInfo( (u_long*)qlpFileAddr(
		qlpFindFile( "lamelotl", tex_buff ), tex_buff ), &tim );

	UploadTIM( &tim );

	lamelotl_tpage = getTPage( 0, 0, tim.prect->x, tim.prect->y )|0x200;
	setSprt( &llotl_sprite );
	setClut( &llotl_sprite, tim.crect->x, tim.crect->y );
	setWH( &llotl_sprite, tim.prect->w<<2, tim.prect->h );
	setUV0( &llotl_sprite, 0, 0 );
	setRGB0( &llotl_sprite, 128, 128, 128 );


	GetTimInfo( (u_long*)qlpFileAddr(
		qlpFindFile( "celmap", tex_buff ), tex_buff ), &tim );

	UploadTIM( &tim );

	smdSetCelTex( getTPage( 0, 2, tim.prect->x, tim.prect->y )|0x200,
		getClut( tim.crect->x, tim.crect->y ) );
	smdSetCelParam( 3, 3, 0x4f4f4f );


	GetTimInfo( (u_long*)qlpFileAddr(
		qlpFindFile( "font", tex_buff ), tex_buff ), &tim );

	UploadTIM( &tim );

	font_tpage	= getTPage( 0, 1, tim.prect->x, tim.prect->y )|0x200;
	font_clut	= getClut( tim.crect->x, tim.crect->y );

	free( tex_buff );
}

void unpackModels() {

	/*
		Unpack model data from an embedded LZP archive.
	*/

	int i;

	i = lzpSearchFile( "rbowshade", lz_resources );
	o_rbowshade = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_rbowshade, lz_resources, i );
	smdInitData( o_rbowshade );

	i = lzpSearchFile( "bungirl", lz_resources );
	o_bungirl = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_bungirl, lz_resources, i );
	smdInitData( o_bungirl );

	i = lzpSearchFile( "lightworld", lz_resources );
	o_world = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_world, lz_resources, i );
	smdInitData( o_world );

	i = lzpSearchFile( "lightbulb", lz_resources );
	o_lightbulb = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_lightbulb, lz_resources, i );
	smdInitData( o_lightbulb );

	i = lzpSearchFile( "timerift", lz_resources );
	o_timerift = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_timerift, lz_resources, i );
	smdInitData( o_timerift );

	i = lzpSearchFile( "starmask", lz_resources );
	o_star = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_star, lz_resources, i );
	smdInitData( o_star );

	i = lzpSearchFile( "hatkid", lz_resources );
	o_hatkid = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_hatkid, lz_resources, i );
	smdInitData( o_hatkid );

}

void init() {
	// Init display
	initDisplay();
	timerSetup();

	FntLoad( 960, 0 );

	// Just to remove all sound
	SpuInit();

	// Load all textures
	loadTextures();

}

// Bungirl stuff
void bungirldemo() {

	/*
	The bunny girl demo is just a simple demonstration of rendering a 1390
	polygon 3D model of a bunny girl with lighting using SMD drawing routines
	from Scarlet Engine.
	*/

	SC_OT s_ot;

	MATRIX lmtx;

	SVECTOR rot,brot,srot;
	VECTOR pos;

	int timeout = SCENE_TIME;

	// Set clear color
	setRGB0( &draw, 63, 0, 127 );

	// Set tpage base value for SMD drawing routines
	smdSetBaseTPage( 0x200 );

	// Set initial rotation values
	setVector( &rot, 192, 0, 0 );
	setVector( &brot, 0, 0, 128 );
	setVector( &srot, 0, 0, 0 );

	// Set lighting color matrix
	gte_SetColorMatrix( &lgt_colmtx );

	// Set 'backside' or ambient light color
	gte_SetBackColor( 113, 113, 113 );


	// Demo loop
	while( 1 ) {

		// Bungirl position
		setVector( &pos, 0, 50, 350 );

		TransMatrix( &mtx, &pos );
		RotMatrix( &brot, &mtx );

		// Calculate light matrix
		lmtx.m[0][0] = isin( -brot.vy<<2 );
		lmtx.m[0][1] = -2048;
		lmtx.m[0][2] = icos( -brot.vy<<2 );

		MulMatrix0( &lmtx, &mtx, &lmtx );

		// Set matrices
		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );
		gte_SetLightMatrix( &lmtx );

		// Sort the bungirl model
		s_ot.ot		= ot[db];
		s_ot.otlen	= OT_LEN;
		s_ot.zdiv	= 1;
		s_ot.zoff	= 0;
		nextpri = smdSortModel( &s_ot, nextpri, o_bungirl );


		// Sort the rotating rainbow background
		setVector( &pos, 0, 0, 200 );

		TransMatrix( &mtx, &pos );
		RotMatrix( &srot, &mtx );

		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );
		nextpri = smdSortModelFlat( ot[db]+(OT_LEN-1), nextpri, o_rbowshade );

		brot.vy += 8;
		srot.vz += 4;


		// Sort overlay then display
		sort_overlay( 0 );

		display();

		timeout--;
		if( timeout < 0 )
			break;

	}

}

// Stencil effect stuff
void stencilstuff() {

	/*
	The stencil demo is achieved by utilizing the mask bit setting
	primitive GP0(E6h). The structure of this primitive is defined as
	DR_STP initialized and set by setDrawStp().

	The DR_STP primitive controls mask bit operations for drawing
	primitives such as setting mask bits on every pixel drawn or mask
	bit test where pixels won't be drawn on pixels with the mask bit set.
	It applies to most graphics drawing primitives except VRAM fill.
	The mask bits are stored in the 16th bit of each pixel drawn.

	The semi-transparency bits of a texture always carry over as mask
	bits in textured primitives. The only way to clear mask bits is by
	using VRAM fill commands or drawing primitives with the set mask
	bit operation disabled.

	The stencil effect featured in this demo is achieved by enabling set
	mask bit with DR_STP, drawing semi-transparent primitives using
	additive blending but color is all zero to make it completely invisible
	but is enough to update the mask bits, disable mask set bit but enable
	mask test with DR_STP and then drawing a rectangle that fills the
	entire screen. Semi-transparency mask in textures must not be used when
	drawing the scene that will be 'below' the mask layer.
	*/
	int timeout = SCENE_TIME;

	int spin=0;

	DR_STP *mask;
	TILE *rect;

	SC_OT s_ot;

	SVECTOR rot;
	SVECTOR srot;
	VECTOR pos;

	// Set clear color
	setRGB0( &draw, 127, 0, 63 );

	// Set tpage base value for SMD drawing routines
	smdSetBaseTPage( 0x200 );

	// Base rotation coordinates
	setVector( &rot, 0, 0, 0 );
	setVector( &srot, 0, 0, 0 );

	// Set perspective
	gte_SetGeomScreen( 320 );


	// Demo loop
	while( timeout > 0 ) {


		// Draw the timerift background
		setVector( &rot, 0, spin, 0 );
		setVector( &pos, 0, 0, 0 );

		TransMatrix( &mtx, &pos );
		RotMatrix( &rot, &mtx );

		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );

		s_ot.ot		= ot[db];
		s_ot.otlen 	= 32;
		s_ot.zdiv	= 2;
		s_ot.zoff	= 20;

		nextpri = smdSortModelFlat( ot[db]+(OT_LEN-1), nextpri, o_timerift );


		// Sort mask primitive that enables setting mask bits
		mask = (DR_STP*)nextpri;
		setDrawStp( mask, 1, 0 );
		addPrim( ot[db]+20, mask );
		nextpri += sizeof(DR_STP);


		// Sort the stars
		setVector( &rot, 0, 0, spin<<2 );
		setVector( &pos,
			(isin(spin<<2)*icos(spin))>>16,
			(icos(spin<<2)*isin(spin))>>16, 400 );

		TransMatrix( &mtx, &pos );
		RotMatrix( &rot, &mtx );

		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );

		nextpri = smdSortModelFlat( ot[db]+19, nextpri, o_star );

		setVector( &rot, 0, 0, -spin<<2 );
		setVector( &pos,
			(isin(-spin<<2)*icos(-spin))>>16,
			(icos(-spin<<2)*isin(-spin))>>16, 400 );

		TransMatrix( &mtx, &pos );
		RotMatrix( &rot, &mtx );

		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );

		nextpri = smdSortModelFlat( ot[db]+19, nextpri, o_star );


		// Sort mask primitive that enables mask bit test
		mask = (DR_STP*)nextpri;
		setDrawStp( mask, 0, 1 );
		addPrim( ot[db]+18, mask );
		nextpri += sizeof(DR_STP);


		// Sort rectangle that fills the screen
		rect = (TILE*)nextpri;
		setTile( rect );
		setXY0( rect, 0, 0 );
		setWH( rect, 640, 511 );
		setRGB0( rect, 128, 0, 255 );
		addPrim( ot[db]+17, rect );
		nextpri += sizeof(TILE);


		// Clear all mask settings
		mask = (DR_STP*)nextpri;
		setDrawStp( mask, 0, 0 );
		addPrim( ot[db]+15, mask );
		nextpri += sizeof(DR_STP);


		// Sort overlay then display
		sort_overlay( 0 );

		display();

		spin += 4;
		timeout--;
	}

}

// Orbiting around cel-shaded hatkid stuff
void hatkidstuff() {

	/*
		The cel-shading effect works in a similar manner as conventional
		per-vertex light source calculation except the color values are used
		in a different manner. The resulting color values are divided down
		using bit shifts to fit as texture coordinates (usually within the
		range of 0-31) which are then used to map to a shading map which is a
		simple texture of 3 to 4 shading levels. This technique is very similar
		to how cel-shading is achieved on the Gamecube except all the vertex to
		texture coordinate conversion is all done on the GX itself.

		To achieve this effect on untextured and textured polygons seamlessly
		the shading map is drawn over the original unshaded polygons of the
		model as semi-transparent polygons with subtractive blending. The
		shading map had to be inverted for this effect to work properly since
		blending is subtractive and not multiplicative after all.
	*/

	int timeout = SCENE_TIME;
	int spin=0;

	SC_OT s_ot;

	MATRIX lmtx;

	SVECTOR rot;
	VECTOR pos;

	// Set clear color
	setRGB0( &draw, 127, 0, 63 );

	// Sets base TPage value for SMD drawing routines
	smdSetBaseTPage( 0x200 );

	setVector( &rot, 0, 0, 0 );

	// Set light color matrix
	gte_SetColorMatrix( &lgt_colmtx );

	// Set back/ambient color to black (required for good cel-shading)
	gte_SetBackColor( 0, 0, 0 );

	gte_SetGeomScreen( 320 );

	// Demo loop
	while( timeout > 0 ) {

		// Sort 3D timerift background
		setVector( &rot, spin, spin, spin>>1 );
		setVector( &pos, 0, 0, 0 );

		TransMatrix( &mtx, &pos );
		RotMatrix( &rot, &mtx );

		// Fixed light source direction
		lmtx.m[0][0] = -2048;
		lmtx.m[0][1] = -2048;
		lmtx.m[0][2] = -2048;

		// Multiply by rotation matrix to make it relative to the camera rotation
		MulMatrix0( &lmtx, &mtx, &lmtx );

		gte_SetLightMatrix( &lmtx );
		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );

		nextpri = smdSortModelFlat( ot[db]+(OT_LEN-1), nextpri, o_timerift );


		// Sort the cel-shaded hatkid
		setVector( &pos, 0, 0, 600 );
		TransMatrix( &mtx, &pos );
		gte_SetTransMatrix( &mtx );

		s_ot.ot		= ot[db];
		s_ot.otlen 	= 250;
		s_ot.zdiv	= 1;
		s_ot.zoff	= 0;

		nextpri = smdSortModelCel( &s_ot, nextpri, o_hatkid );


		// Sort overlay then display
		sort_overlay( 1 );
		display();

		spin += 8;
		timeout--;

	}

}

// Plasma stuff
void genPlasma(char *out, int count);
char *sortPlasma(u_long *ot, char *pri, char *map);

void plasmastuff() {

	DR_TPAGE* tp;
	char plasbuff[1271];

	int pcount = 0;
	int timeout = SCENE_TIME;

	while( timeout > 0 ) {

		genPlasma( plasbuff, pcount );
		nextpri = sortPlasma( ot[db]+1, nextpri, plasbuff );

		sort_overlay( 1 );

		display();
		timeout--;
		pcount++;

	}

}

// Simple stripe transition effect
void transition() {

	int count = 0;
	int bheight[16] = { 0 };

	TILE *tile = (TILE*)nextpri;
	draw.isbg = 0;

	while( 1 ) {

		int comp = 0;

		for( int i=0; i<16; i++ ) {

			if( bheight[i] > 0 ) {


				setTile( tile );
				setXY0( tile, 0, 32*i );
				setRGB0( tile, 0, 151, 255 );
				setWH( tile, 640, bheight[i] );
				addPrim( ot[db], tile );
				tile++;

				if( bheight[i] < 32 )
					bheight[i]++;
				else
					comp++;

			}

		}

		if( bheight[count>>1] == 0 )
			bheight[count>>1] = 1;
		display();
		count++;

		if( comp >= 16 )
			break;
	}

	DrawSync(0);

	draw.isbg = 1;

}

int main(int argc, const char *argv[]) {

	// Init
	init();

	// Do transition
	transition();

	// Do Meido-Tek, PSn00bSDK and n00bDEMO logo intros
	intro();

	// Unpack model data
	unpackModels();

	// Demo sequence loop
	timer_counter = 100;
	while( 1 ) {

		lightdemo();
		bungirldemo();
		stencilstuff();
		hatkidstuff();
		plasmastuff();

	}

	return 0;
}