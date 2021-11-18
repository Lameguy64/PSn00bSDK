#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <psxgpu.h>

typedef struct _fnt_stream {
	char *txtbuff;
	char *txtnext;
	char *pribuff;
	short x,y;
	short w,h;
	int bg;
	int maxchars;
} _fnt_stream;

static _fnt_stream _stream[8];
static int _nstreams = 0;

u_short _font_tpage;
u_short _font_clut;

extern u_char dbugfont[];

void FntLoad(int x, int y) {

	RECT pos;
	TIM_IMAGE tim;
	
	GetTimInfo( (u_long*)dbugfont, &tim );
	
	// Load font image
	pos = *tim.prect;
	pos.x = x;
	pos.y = y;
	
	_font_tpage = getTPage( 0, 0, pos.x, pos.y );
	
	LoadImage( &pos, tim.paddr );
	DrawSync(0);
	
	// Load font clut
	pos = *tim.crect;
	pos.x = x;
	pos.y = y+tim.prect->h;
	
	_font_clut = getClut( pos.x, pos.y );
	
	LoadImage( &pos, tim.caddr );
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
			setSprt8( sprt );
			setRGB0( sprt, 128, 128, 128 );
			setXY0( sprt, sx, sy );
			setUV0( sprt, (i%16)<<3, (i>>4)<<3 );
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
	DrawOTag((u_long*)_stream[id].pribuff);
	DrawSync(0);
	
	_stream[id].txtnext = _stream[id].txtbuff;
	_stream[id].txtbuff[0] = 0;
	
	return _stream[id].pribuff;
	
}