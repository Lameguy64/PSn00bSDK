/* Work in progress example, need to add comments. 
 *
 * Basically a quick little example that showcases C++ classes are
 * functioning in PSn00bSDK. - Lameguy64
 *
 * First written in December ‎18, ‎2020.
 *
 * Changelog:
 *
 *	May 10, 2021		- Variable types updated for psxgpu.h changes.
 *
 */
 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <psxgte.h>
#include <psxgpu.h>

class GraphClass
{
	u_long	*_ot[2];
	u_char	*_pri[2];
	u_char	*_nextpri;

	int _ot_count;
	int _db;
	
	DISPENV	_disp[2];
	DRAWENV _draw[2];
	
public:

	GraphClass( int ot_len = 8, int pri_len = 8192 )
	{
		_ot[0] = (u_long*)malloc( sizeof(u_long)*ot_len );
		_ot[1] = (u_long*)malloc( sizeof(u_long)*ot_len );
		
		_db = 0;
		_ot_count = ot_len;
		ClearOTagR( _ot[0], _ot_count );
		ClearOTagR( _ot[1], _ot_count );
		
		_pri[0] = (u_char*)malloc( pri_len );
		_pri[1] = (u_char*)malloc( pri_len );
		
		_nextpri = _pri[0];
		
		printf( "GraphClass::GraphClass: Buffers allocated.\n" );
		
	} /* GraphClass */
	
	virtual ~GraphClass()
	{
		/* free the OTs and primitive buffers */
		free( _ot[0] );
		free( _ot[1] );
		
		free( _pri[0] );
		free( _pri[1] );
		
		printf( "GraphClass::GraphClass: Buffers freed.\n" );
		
	} /* ~GraphClass */
	
	void SetRes( int w, int h )
	{
		SetDefDispEnv( &_disp[0], 0, h, w, h );
		SetDefDispEnv( &_disp[1], 0, 0, w, h );
		
		SetDefDrawEnv( &_draw[0], 0, 0, w, h );
		SetDefDrawEnv( &_draw[1], 0, h, w, h );
		
		setRGB0( &_draw[0], 63, 0, 127 );
		_draw[0].isbg = 1;
		_draw[0].dtd = 1;
		setRGB0( &_draw[1], 63, 0, 127 );
		_draw[1].isbg = 1;
		_draw[1].dtd = 1;
		
		PutDispEnv( &_disp[0] );
		PutDrawEnv( &_draw[0] );
		
	} /* SetRes */
	
	void IncPri( int bytes )
	{
		_nextpri += bytes;
		
	} /* IncPri */
	
	void SetPri( u_char *ptr )
	{
		_nextpri = ptr;
		
	} /* SetPri */
	
	u_char *GetNextPri( void )
	{
		return( _nextpri );
		
	} /* GetNextPri */
	
	u_long *GetOt( void )
	{
		return( _ot[_db] );
		
	} /* GetOt */
	
	void Display( void )
	{
		VSync( 0 );
		DrawSync( 0 );
		SetDispMask( 1 );
		
		_db = !_db;
		
		PutDispEnv( &_disp[_db] );
		PutDrawEnv( &_draw[_db] );
		
		DrawOTag( _ot[!_db]+(_ot_count-1) );
		
		ClearOTagR( _ot[_db], _ot_count );
		_nextpri = _pri[_db];
		
	} /* Display */
	
}; /* GraphClass */

GraphClass *otable;

int main( int argc, const char *argv[] )
{
	TILE *tile;
	
	ResetGraph( 0 );
	
	otable = new GraphClass();
	
	otable->SetRes( 320, 240 );
	
	while( 1 )
	{
		tile = (TILE*)otable->GetNextPri();
		setTile( tile );
		setXY0( tile, 32, 32 );
		setWH( tile, 128, 128 );
		setRGB0( tile, 255, 255, 0 );
		addPrim( otable->GetOt(), tile );
		otable->IncPri( sizeof(TILE) );
		
		otable->Display();
	}
	
	return( 0 );
	
} /* main */