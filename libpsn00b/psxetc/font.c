#include <stdio.h>
#include <ctype.h>
#include <psxgpu.h>

extern unsigned char dbugfont[];

unsigned short _font_tpage;
unsigned short _font_clut;

void FntLoad(int x, int y) {

	RECT pos;
	TIM_IMAGE tim;
	
	GetTimInfo( (unsigned int*)dbugfont, &tim );
	
	// Load font image
	pos = *tim.prect;
	pos.x = x;
	pos.y = y;
	
	_font_tpage = getTPage( 0, 0, pos.x, 0 ) | 0x200;
	
	LoadImage( &pos, tim.paddr );
	DrawSync(0);
	
	// Load font clut
	pos = *tim.crect;
	pos.x = x;
	pos.y = y+tim.prect->h;
	
	_font_clut = getClut( pos.x, pos.y );
	
	LoadImage( &pos, tim.caddr );
	DrawSync(0);
	
}