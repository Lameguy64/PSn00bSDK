#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <psxgpu.h>
#include <psxpress.h>
#include <hwregs_c.h>

#define SCREEN_XRES		320
#define SCREEN_YRES		240

//#define BLOCK_SIZE 8	// Monochrome (8x8), 15bpp display
//#define BLOCK_SIZE 12	// Monochrome (8x8), 24bpp display
//#define BLOCK_SIZE 16	// Color (16x16), 15bpp display
#define BLOCK_SIZE 24	// Color (16x16), 24bpp display

/* vlc.c */
int DecDCTvlc( unsigned short *mdec_bs, unsigned short *mdec_rl );

/* data.s */
extern unsigned short bs_data[];

DISPENV disp;

void init( void )
{
	ResetGraph(0);
	DecDCTReset(0);
	
	SetDefDispEnv(&disp, 0, 0, SCREEN_XRES, SCREEN_YRES);
	
	disp.isrgb24 = disp.isrgb24 = 1;
	
	PutDispEnv(&disp);
	SetDispMask(1);
	
} /* init */


void decode_test( void )
{
	uint16_t *out_buff;
	uint16_t dec_len;
	
	// Allocate and decode VLC encoded BS data
	printf("Decode start...\n");
	out_buff = (uint16_t*)malloc(4*((*(long*)bs_data)+1));
	DecDCTvlc(bs_data, out_buff);
	
	// Print out decompression results
	dec_len = ((uint16_t*)out_buff)[0];
	printf("out_buff=%p len=%d\n", out_buff, dec_len);
	
	// Initialize MDEC data input
	MDEC0 = 0x30000000 | dec_len; // 0x38000000 for 15bpp
	DecDCTinRaw((const uint32_t*)(out_buff+1), dec_len);
	
	// Decode image slice-by-slice
	for (uint32_t x = 0; x < (SCREEN_XRES * 3 / 2); x += BLOCK_SIZE)
	{	// 24bpp
		RECT     rect;
		uint32_t slice[BLOCK_SIZE * SCREEN_YRES / 2];
		
		rect.x = x;
		rect.y = 0;
		rect.w = BLOCK_SIZE;
		rect.h = SCREEN_YRES;

		// Configure the MDEC to output to the slice buffer and let it finish
		// decoding a slice, then upload it to the framebuffer.
		DecDCTout(slice, BLOCK_SIZE * SCREEN_YRES / 2);
		DecDCToutSync(0);

		LoadImage(&rect, (u_long *) slice);
		DrawSync(0);
	}
	
	free(out_buff);
	
} /* decode_test */

int main( int argc, const char *argv[] )
{
	init();
	
	decode_test();
	
	for (;;)
		__asm__ volatile("");
	
	return 0;
	
} /* main */
