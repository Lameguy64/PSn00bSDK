#include <stdio.h>
#include <ctype.h>
#include <psxgpu.h>

extern unsigned short _font_tpage;
extern unsigned short _font_clut;

char *FntSort(unsigned int *ot, char *pri, int x, int y, const char *text) {
	
	DR_TPAGE *tpage;
	SPRT_8 *sprt = (SPRT_8*)pri;
	int	i;
	
	while( *text != 0 ) {
	
		i = toupper( *text )-32;
	
		if( i > 0 ) {
			
			i--;
			setSprt8( sprt );
			setRGB0( sprt, 128, 128, 128 );
			setXY0( sprt, x, y );
			setUV0( sprt, (i%16)<<3, (i>>4)<<3 );
			sprt->clut = _font_clut;
			addPrim( ot, sprt );
			sprt++;
			
		}
	
		x += 8;
		text++;
		
	}
	
	pri = (char*)sprt;
	
	tpage = (DR_TPAGE*)pri;
	tpage->code[0] = _font_tpage;
	setlen( tpage, 1 );
	setcode( tpage, 0xe1 );
	addPrim( ot, pri );
	pri += sizeof(DR_TPAGE);
	
	return pri;
	
}