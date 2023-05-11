#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <psxgpu.h>

typedef struct _fnt_stream {
	char	*txtbuff;
	char	*txtnext;
	char	*pribuff;
	int16_t	x, y;
	int16_t	w, h;
	int		bg, maxchars;
} _fnt_stream;

static _fnt_stream _stream[8];
static int _nstreams = 0;

uint16_t _font_tpage;
uint16_t _font_clut;

extern uint8_t _gpu_debug_font[];

void FntLoad(int x, int y) {
	_sdk_validate_args_void((x >= 0) && (y >= 0) && (x < 1024) && (y < 1024));

	RECT pos;
	TIM_IMAGE tim;
	
	GetTimInfo((const uint32_t *) _gpu_debug_font, &tim);
	
	// Load font image
	pos = *tim.prect;
	pos.x = x;
	pos.y = y;
	
	_font_tpage = getTPage(0, 0, pos.x, pos.y);
	
	LoadImage(&pos, tim.paddr);
	DrawSync(0);
	
	// Load font clut
	pos = *tim.crect;
	pos.x = x;
	pos.y = y+tim.prect->h;
	
	_font_clut = getClut(pos.x, pos.y);
	
	LoadImage(&pos, tim.caddr);
	DrawSync(0);
	
	// Clear previously opened text streams
	if( _nstreams ) {
		
		int i;
		
		for( i=0; i<_nstreams; i++ ) {
			free(_stream[i].txtbuff);
			free(_stream[i].pribuff);
		}
		
		_nstreams = 0;
		
	}
	
}

int FntOpen(int x, int y, int w, int h, int isbg, int n) {
	_sdk_validate_args((w > 0) && (h > 0) && (n > 0), -1);

	int i;
	
	// Initialize a text stream
	_stream[_nstreams].x = x;
	_stream[_nstreams].y = y;
	_stream[_nstreams].w = w;
	_stream[_nstreams].h = h;
	
	_stream[_nstreams].txtbuff = (char*)malloc(n+1);
	
	i = (sizeof(SPRT_8)*n)+sizeof(DR_TPAGE);
	
	if( isbg ) {
		i += sizeof(TILE);
	}
	
	_stream[_nstreams].pribuff = (char*)malloc(i);
	_stream[_nstreams].maxchars = n;
	
	_stream[_nstreams].txtbuff[0] = 0x0;
	_stream[_nstreams].txtnext = _stream[_nstreams].txtbuff;
	_stream[_nstreams].bg = isbg;
	
	n = _nstreams;
	_nstreams++;
	
	return n;
	
}

int FntPrint(int id, const char *fmt, ...) {
	_sdk_validate_args((id < _nstreams) && fmt, -1);

	int n;
	va_list ap;

	if( id < 0 )
		id = _nstreams-1;
	
	n = strlen(_stream[id].txtbuff);
	
	if( n >= _stream[id].maxchars ) {
		return n;
	}
	
	va_start(ap, fmt);
	
	n = vsnprintf(_stream[id].txtnext, _stream[id].maxchars-n, fmt, ap);
	
	_stream[id].txtnext += n;
	
	va_end(ap);
	
	return strlen(_stream[id].txtbuff);
	
}

char *FntFlush(int id) {
	_sdk_validate_args(id < _nstreams, 0);

	char		*opri;
	SPRT_8		*sprt;
	DR_TPAGE	*tpage;
	char		*text;
	int			 i,sx,sy;
	
	if( id < 0 )
		id = _nstreams-1;
	
	sx = _stream[id].x;
	sy = _stream[id].y;
	
	text = _stream[id].txtbuff;
	
	opri = _stream[id].pribuff;
	
	// Create TPage primitive
	tpage = (DR_TPAGE*)opri;
	setDrawTPage(tpage, 0, 0, _font_tpage);
	
	// Create a black rectangle background when enabled
	if( _stream[id].bg ) {
		
		TILE *tile;
		opri += sizeof(DR_TPAGE);
		tile = (TILE*)opri;
		
		setTile(tile);
		
		if( _stream[id].bg == 2 )
			setSemiTrans(tile, 1);
		
		setXY0(tile, _stream[id].x, _stream[id].y);
		setWH(tile, _stream[id].w, _stream[id].h);
		setRGB0(tile, 0, 0, 0);
		setaddr(tpage, tile);
		opri = (char*)tile;
		
		sprt = (SPRT_8*)(opri+sizeof(TILE));
		
	} else {
		
		sprt = (SPRT_8*)(opri+sizeof(DR_TPAGE));
		
	}
	
	// Create the sprite primitives
	while( *text != 0 ) {
	
		if( ( *text == '\n' ) || ( ( sx-_stream[id].x ) > _stream[id].w-8 ) ) {
			sx = _stream[id].x;
			sy += 8;
			
			if( *text == '\n' )
				text++;
			
			continue;
		}
		
		if( ( sy-_stream[id].y ) > _stream[id].h-8 ) {
			break;
		}
		
		i = toupper( *text )-32;
	
		if( i > 0 ) {
			
			i--;
			setSprt8(sprt);
			setShadeTex(sprt, 1);
			setSemiTrans(sprt, 1);
			setXY0(sprt, sx, sy);
			setUV0(sprt, (i % 16) * 8, (i / 16) * 8);
			sprt->clut = _font_clut;
			setaddr(opri, sprt);
			opri = (char*)sprt;
			sprt++;
			
		}
	
		sx += 8;
		text++;
		
	}
	
	// Set a terminator value to the last primitive
	termPrim(opri);
	
	// Draw the primitives
	DrawSync(0);
	DrawOTag((uint32_t*)_stream[id].pribuff);
	DrawSync(0);
	
	_stream[id].txtnext = _stream[id].txtbuff;
	_stream[id].txtbuff[0] = 0;
	
	return _stream[id].pribuff;
	
}

char *FntSort(uint32_t *ot, char *pri, int x, int y, const char *text) {
	_sdk_validate_args(ot && pri, 0);

	DR_TPAGE *tpage;
	SPRT_8 *sprt = (SPRT_8*)pri;
	int	i;
	
	while( *text != 0 ) {
	
		i = toupper( *text )-32;
	
		if( i > 0 ) {
			
			i--;
			setSprt8(sprt);
			setShadeTex(sprt, 1);
			setSemiTrans(sprt, 1);
			setXY0(sprt, x, y);
			setUV0(sprt, (i % 16) * 8, (i / 16) * 8);
			sprt->clut = _font_clut;
			addPrim(ot, sprt);
			sprt++;
			
		}
	
		x += 8;
		text++;
		
	}
	
	pri = (char*)sprt;
	
	tpage = (DR_TPAGE*)pri;
	tpage->code[0] = _font_tpage;
	setlen(tpage, 1);
	setcode(tpage, 0xe1);
	addPrim(ot, pri);
	pri += sizeof(DR_TPAGE);
	
	return pri;
	
}
