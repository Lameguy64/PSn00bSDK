#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <psxgte.h>
#include <psxgpu.h>

/* mdec.s */
extern void mdec_reset( void );
extern void mdec_setscale( void );
extern void mdec_setquants( void );
extern void mdec_cmd( unsigned int cmd );
extern void mdec_in( void *src, int blocks );
extern void mdec_out( void *dst, int blocks );

/* vlc.c */
int DecDCTvlc( unsigned short *mdec_bs, unsigned short *mdec_rl );

/* data.s */
extern unsigned short bs_data[];

DISPENV disp[2];
DRAWENV draw[2];
int db;

void memory_browser(unsigned int addr)
{
	int i,j,key;
	unsigned char *ptr,*pptr;
	
	while(1)
	{
		/* Set cursor position to top-left */
		printf("\033[1;1H");
		printf("MEMVIEW   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  0123456789ABCDEF");

		ptr = (unsigned char*)addr;
	
		/* Print contents from current location of memory */
		for(j=0; j<23; j++)
		{
			printf("\n%04X: ", (unsigned int)ptr);
			pptr = ptr;
			for(i=0; i<16; i++)
			{
				printf("%02X ", *ptr);
				ptr++;
			}
			printf(" ");
			for(i=0; i<16; i++)
			{
				if(( *pptr < 32 ) || ( *pptr > 127 ) )
				{
					printf(".");
				}
				else
				{
					printf("%c", *pptr);
				}
				pptr++;
			}
		}
	
		/* Parse input */
		while(1)
		{
			key = getchar();
			if( key == 0x1B )
			{
				key = getchar();
				
				if( key == 0x5B )
				{
					key = getchar();
					if( key == 0x41 )		// Up
					{
						addr -= 16;
						break;
					}
					else if( key == 0x42 )	// Down
					{
						addr += 16;
						break;
					}
					if( key == 0x35 )		// Page up
					{
						addr -= 16*23;
						break;
					}
					else if( key == 0x36 )	// Page down
					{
						addr += 16*23;
						break;
					}
				}
			}
			
		}
	}
	
} /* memory_browser */

void init( void )
{
	ResetGraph( 0 );
	
	SetDefDispEnv( &disp[0], 0, 0, 320, 240 );
	SetDefDispEnv( &disp[1], 0, 240, 320, 240 );
	
	SetDefDrawEnv( &draw[0], 0, 240, 320, 240 );
	SetDefDrawEnv( &draw[1], 0, 0, 320, 240 );
	
	draw[0].isbg = draw[1].isbg = 1;
	setRGB0( &draw[0], 0, 63, 0 );
	setRGB0( &draw[1], 0, 63, 0 );
	
	PutDispEnv( &disp[1] );
	PutDrawEnv( &draw[1] );
	db = 0;
	
} /* init */

void display( void )
{
	DrawSync( 0 );
	VSync( 0 );
	
	PutDispEnv( &disp[db] );
	PutDrawEnv( &draw[db] );
	SetDispMask( 1 );
	
	db = !db;
	
} /* display */

void decode_test( void )
{
	unsigned short *out_buff;
	unsigned short *dec_buff;
	int dec_len;
	RECT rect;
	int xx;
	
	out_buff = (unsigned short*)malloc( 131072 );
	dec_buff = (unsigned short*)malloc( 7680 );
	
	memset( out_buff, 0, 131072 );
	DecDCTvlc( bs_data, out_buff );
	
	dec_len = ((unsigned short*)out_buff)[0];
	
	printf( "out_buff=%p len=%d\n", out_buff, dec_len );
	
	//memory_browser( (unsigned int)(out_buff+2) );
	mdec_cmd( 0x38000000|dec_len );
	
	mdec_in( out_buff+2, 8 );
	
	rect.x = 320;	rect.y = 0;
	rect.w = 16;	rect.h = 240;
	xx = 0;
	//while( xx < 320 )
	{
		mdec_out( dec_buff, 120 );
		LoadImage( &rect, (unsigned int*)dec_buff );
		DrawSync( 0 );
		xx += 16;
		rect.x += 16;
	}
	
} /* decode_test */

int main( int argc, const char *argv[] )
{
	init();
	
	/* init MDEC */
	mdec_reset();
	mdec_setscale();
	mdec_setquants();
	
	decode_test();
	
	while( 1 )
	{
		display();
	}
	
	return( 0 );
	
} /* main */
