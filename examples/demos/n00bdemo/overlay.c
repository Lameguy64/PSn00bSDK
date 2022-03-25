#include <sys/types.h>
#include <psxgte.h>
#include <psxgpu.h>
#include "disp.h"

extern const char scroll_text[];
extern unsigned short lamelotl_tpage,psn00b_tpage;
extern unsigned short font_tpage,font_clut;

extern volatile int frame_counter, frame_rate;

extern SPRT llotl_sprite;
extern SPRT psn00b_sprite;

u_char font_width[] = {
	10	,6	,12	,16	,12	,14	,14	,8	,10	,10	,14	,12	,8	,14	,6	,14	,
	14	,6	,12	,13	,14	,14	,14	,14	,14	,14	,6	,8	,12	,14	,12	,14	,
	14	,14	,14	,13	,14	,14	,14	,14	,14	,14	,14	,14	,12	,15	,14	,14	,
	14	,14	,14	,13	,14	,15	,15	,16	,16	,16	,16	,10	,14	,10	,14	,14	,
	8	,14	,13	,11	,13	,12	,13	,12	,13	,6	,14	,14	,8	,16	,14	,14	,
	14	,14	,13	,12	,12	,14	,14	,14	,13	,13	,14	,10	,6	,10	,16	,16
};

int scrolltext_pos = 640;
int scrolltext_cpos = 0;
int overlay_count = 0;

void sort_fps_counter() {
	int tens = frame_rate / 10, units = frame_rate % 10;
	int x = 10;

	SPRT_16 *spr16 = (SPRT_16 *) nextpri;

	setSprt16(spr16);
	setSemiTrans(spr16, 1);
	setXY0(spr16, x, 56);
	setUV0(spr16, tens << 4, 16);
	setRGB0(spr16, 20, 20, 20);
	spr16->clut = font_clut;
	addPrim(ot[db], spr16);
	spr16++;
	x += font_width[16 | tens];

	setSprt16(spr16);
	setSemiTrans(spr16, 1);
	setXY0(spr16, x, 56);
	setUV0(spr16, units << 4, 16);
	setRGB0(spr16, 20, 20, 20);
	spr16->clut = font_clut;
	addPrim(ot[db], spr16);
	spr16++;

	nextpri = (char *) spr16;
	frame_counter++;
}

void sort_overlay(int showlotl) {
	
	SPRT *spr;
	SPRT_16 *spr16;
	DR_TPAGE *tp;
	POLY_G4 *quad;
	LINE_G2 *line;
	
	sort_fps_counter();

	int i = scrolltext_cpos;
	int j, k, tx, par_end = 0;
	
	tx = scrolltext_pos;
	
	while( scroll_text[i] != 0 ) {
		
		if( scroll_text[i] == '\n' ) {
			par_end = 1;
			break;
		}
		
		j = scroll_text[i]-32;
		
		if( j > 0 ) {
			
			spr16 = (SPRT_16*)nextpri;
			setSprt16( spr16 );
			setSemiTrans( spr16, 1 );
			setXY0( spr16, tx, 28 );
			setUV0( spr16, (j%16)<<4, (j>>4)<<4 );
			
			if( tx < 128 ) {
				k = tx;
				if( k < 0 )
					k = 0;
				setRGB0( spr16, k, k, k );
			} else if( tx > 512 ) {
				k = 128-(tx-512);
				if( k < 0 )
					k = 0;
				setRGB0( spr16, k, k, k );
			} else {
				setRGB0( spr16, 128, 128, 128 );
			}
			
			spr16->clut = font_clut;
			addPrim( ot[db], spr16 );
			nextpri += sizeof(SPRT_16);
			
		}
		
		tx += font_width[j];
		
		if( tx > 640 )
			break;
		
		i++;
		
	}
	
	scrolltext_pos -= 4;
	j = scroll_text[scrolltext_cpos]-32;
	if( j >= 0 ) {
		
		if( scrolltext_pos <= -font_width[j] ) {
			
			scrolltext_pos += font_width[j];
			scrolltext_cpos++;
			
		}
		
	} else {
		
		scrolltext_pos = 0;
		scrolltext_cpos++;
		
		if( par_end )
			scrolltext_pos = 640;
		
		if( scroll_text[scrolltext_cpos] == 0 ) {
			scrolltext_cpos = 0;
			scrolltext_pos = 640;
		}
		
	}
	
	line = (LINE_G2*)nextpri;
	setLineG2( line );
	setSemiTrans( line, 1 );
	setXY2( line, 0, 46, 320, 46 );
	setRGB0( line, 0, 0, 0 );
	setRGB1( line, 255, 255, 255 );
	addPrim( ot[db], line );
	line++;
	setLineG2( line );
	setSemiTrans( line, 1 );
	setXY2( line, 320, 46, 640, 46 );
	setRGB0( line, 255, 255, 255 );
	setRGB1( line, 0, 0, 0 );
	addPrim( ot[db], line );
	line++;
	nextpri = (char*)line;
	
	tp = (DR_TPAGE*)nextpri;
	setDrawTPage( tp, 0, 1, font_tpage );
	addPrim( ot[db], tp );
	nextpri += sizeof(DR_TPAGE);
	
	quad = (POLY_G4*)nextpri;
	setPolyG4( quad );
	setSemiTrans( quad, 1 );
	setXY4( quad,
		0, 25, 320, 25,
		0, 46, 320, 46 );
	setRGB0( quad, 0, 0, 0 );
	setRGB1( quad, 64, 64, 64 );
	setRGB2( quad, 0, 0, 0 );
	setRGB3( quad, 64, 64, 64 );
	addPrim( ot[db], quad );
	quad++;
	setPolyG4( quad );
	setSemiTrans( quad, 1 );
	setXY4( quad,
		320, 25, 640, 25,
		320, 46, 640, 46 );
	setRGB0( quad, 64, 64, 64 );
	setRGB1( quad, 0, 0, 0 );
	setRGB2( quad, 64, 64, 64 );
	setRGB3( quad, 0, 0, 0 );
	addPrim( ot[db], quad );
	quad++;
	nextpri = (char*)quad;
	
	i = isin( overlay_count )>>9;
	
	if( GetVideoMode() == MODE_PAL ) {
		j = 32;
	} else {
		j = 0;
	}
	
	tp = (DR_TPAGE*)nextpri;
	setDrawTPage( tp, 0, 1, getTPage( 0, 2, 0, 0 ) );
	addPrim( ot[db], tp );
	nextpri += sizeof(DR_TPAGE);
	
	if( showlotl ) {
		
		spr = (SPRT*)nextpri;
		*spr = llotl_sprite;
		setXY0( spr, 440, (200+j)+i );
		addPrim( ot[db], spr );
		nextpri += sizeof(SPRT);
		
		tp = (DR_TPAGE*)nextpri;
		setDrawTPage( tp, 0, 1, lamelotl_tpage );
		addPrim( ot[db], tp );
		nextpri += sizeof(DR_TPAGE);
		
	}
	
	spr = (SPRT*)nextpri;
	*spr = psn00b_sprite;
	setXY0( spr, 8, (400+j)-i );
	addPrim( ot[db], spr );
	nextpri += sizeof(SPRT);
	
	tp = (DR_TPAGE*)nextpri;
	setDrawTPage( tp, 0, 1, psn00b_tpage );
	addPrim( ot[db], tp );
	nextpri += sizeof(DR_TPAGE);
	
	overlay_count += 32;
	
}