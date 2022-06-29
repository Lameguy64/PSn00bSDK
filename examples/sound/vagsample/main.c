/* 
 * LibPSn00b Example Programs
 *
 * VAG Playback Example
 * 2019-2021 Meido-Tek Productions / PSn00bSDK Project
 *
 * This example program demonstrates the basic use of the SPU; uploading sound
 * clips to SPU RAM and playing it back on one of 24 SPU voices (and possibly
 * leave you with ears ringing from the cacophony).
 *
 * The PS1 SPU only supports playing back of specially encoded ADPCM samples
 * natively and can play them at sample rates of up to 44.1KHz, so sound files
 * will have to be converted to 'VAG' format before it can be used on the PS1.
 * While it is possible to play plain PCM samples on the SPU, this requires
 * some special trickery that involves abusing the echo buffer and is not
 * supported by the (half-baked) SPU library of PSn00bSDK.
 *
 * Additionally, the SPU can only play ADPCM samples from its own local memory
 * called the SPU RAM, so sound samples will have to be uploaded to SPU RAM
 * before it can be played by the SPU.
 *
 * The included sound clips are by HighTreason610 (0proyt) and
 * Lameguy64 (threedeeffeggzz) respectively.
 *
 * Example by Lameguy64
 *
 *
 * Changelog:
 *
 *	  October 6, 2021 - Initial version
 *
 */
 
#include <stdio.h>
#include <sys/types.h>
#include <psxetc.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxapi.h>
#include <psxspu.h>

extern const unsigned char	proyt[];
extern const int			proyt_size;
extern const unsigned char	tdfx[];
extern const int			tdfx_size;

// Define display/draw environments for double buffering
DISPENV disp[2];
DRAWENV draw[2];
int db;

unsigned char pad_buff[2][34];

// SPU addresses of the uploaded sound clips
int proyt_addr;
int tdfx_addr;

// Init function
void init(void)
{
	int addr_temp;
	
	// This not only resets the GPU but it also installs the library's
	// ISR subsystem to the kernel
	ResetGraph(0);
	
	// Define display environments, first on top and second on bottom
	SetDefDispEnv(&disp[0], 0, 0, 320, 240);
	SetDefDispEnv(&disp[1], 0, 240, 320, 240);
	
	// Define drawing environments, first on bottom and second on top
	SetDefDrawEnv(&draw[0], 0, 240, 320, 240);
	SetDefDrawEnv(&draw[1], 0, 0, 320, 240);
	
	// Set and enable clear color
	setRGB0(&draw[0], 0, 96, 0);
	setRGB0(&draw[1], 0, 96, 0);
	draw[0].isbg = 1;
	draw[1].isbg = 1;
	
	// Clear double buffer counter
	db = 0;
	
	// Apply the GPU environments
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);
	
	// Load test font
	FntLoad(960, 0);
	
	// Open up a test font text stream of 100 characters
	FntOpen(0, 8, 320, 224, 0, 100);
	
	// Initialize the SPU
	SpuInit();
	
	// Set SPU transfer mode to DMA (only mode currently supported)
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	
	// Set SPU transfer address (start address for sample upload)
	addr_temp = 0x1000;
	SpuSetTransferStartAddr(addr_temp);
	
	// Upload first sound clip and wait for transfer to finish
	SpuWrite(((unsigned char*)proyt)+48, proyt_size-48);
	SpuWait();
	
	// Obtain the address of the sound and advance address for the next one
	// Samples are addressed in 8-byte units, so it'll have to be divided by 8
	proyt_addr = addr_temp/8;
	addr_temp += proyt_size-48;
	
	printf("proyt.vag\t= %02x\n", proyt_addr);
	
	// Upload second sound clip
	SpuSetTransferStartAddr(addr_temp);
	SpuWrite(((unsigned char*)tdfx)+48, tdfx_size-48);
	SpuWait();
	
	// Obtain the address of the second sound clip
	tdfx_addr = addr_temp/8;
	addr_temp += tdfx_size-48;
	
	printf("3dfx.vag\t= %02x\n", tdfx_addr);
	
	// Begin pad polling
	InitPAD( pad_buff[0], 34, pad_buff[1], 34 );
	StartPAD();
	ChangeClearPAD(0);
} /* init */

// Display function
void display(void)
{
	// Flip buffer index
	db = !db;
	
	// Wait for all drawing to complete
	DrawSync(0);
	
	// Wait for vertical sync to cap the logic to 60fps (or 50 in PAL mode)
	// and prevent screen tearing
	VSync(0);

	// Switch pages	
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);
	
	// Enable display output, ResetGraph() disables it by default
	SetDispMask(1);
	
} /* main */

// Main function, program entrypoint
int main(int argc, const char *argv[])
{
	int counter,nextchan;
	int cross_pressed;
	int circle_pressed;
	
	PADTYPE *pad;
	SpuVoiceRaw voice;

	// Init stuff	
	init();
	
	// Set common values for the SpuVoiceRaw stuct
	// Technically one struct can be used to play all sounds as the
	// parameters are copied to the SPU registers
	
	voice.vol.left		= 0x3FFE;		// Left voice volume, 3FFEh = max
	voice.vol.right		= 0x3FFE;		// Right voice volume, 3FFEh = max
	voice.adsr_param	= 0xdff18087;	// ADSR parameters
	
	// Main loop
	counter = 0;
	nextchan = 0;
	cross_pressed = 0;
	circle_pressed = 0;
	
	while(1)
	{
		pad = (PADTYPE*)&pad_buff[0][0];
		
		if( pad->stat == 0 )
		{	
			// For digital pad, dual-analog and dual-shock
			if( ( pad->type == 0x4 ) || ( pad->type == 0x5 ) || ( pad->type == 0x7 ) )
			{
				// Plays the first sound
				if( !(pad->btn&PAD_CROSS) )
				{
					if( !cross_pressed )
					{
						// Voice frequency 
						// (400h = 11.25KHz, 1000h = 44.1KHz)
						voice.freq		= 0x800;
						// Voice start playback address
						// (transfer address / 8)
						voice.addr		= proyt_addr;
						// Voice loop address
						// (transfer address / 8)
						voice.loop_addr	= proyt_addr;
										
						// Set voice to key-off to allow restart
						SpuSetKey(0, 1<<nextchan);
						// Set voice parameters
						SpuSetVoiceRaw(nextchan, &voice);
						// Set voice to key-on
						SpuSetKey(1, 1<<nextchan);
						
						// Advance to next voice
						nextchan++;
						if( nextchan > 23 )
							nextchan = 0;
						
						cross_pressed = 1;
					}
				}
				else
				{
					cross_pressed = 0;
				}
				
				// Plays the second sound
				if( !(pad->btn&PAD_CIRCLE) )
				{
					if( !circle_pressed )
					{
						// Voice frequency 
						// (400h = 11.25KHz, 1000h = 44.1KHz)
						voice.freq		= 0x1000;
						// Voice start playback address
						// (transfer address / 8)
						voice.addr		= tdfx_addr;
						// Voice loop address
						// (transfer address / 8)
						voice.loop_addr	= tdfx_addr;
										
						// Set voice to key-off to allow restart
						SpuSetKey(0, 1<<nextchan);
						// Set voice parameters
						SpuSetVoiceRaw(nextchan, &voice);
						// Set voice to key-on
						SpuSetKey(1, 1<<nextchan);
						
						// Advance to next voice
						nextchan++;
						if( nextchan > 23 )
							nextchan = 0;
						
						circle_pressed = 1;
					}
				}
				else
				{
					circle_pressed = 0;
				}
			}
		}
		else
		{
			cross_pressed = 0;
			circle_pressed = 0;
		}
	
		// Print the obligatory hello world and counter to show that the
		// program isn't locking up to the last created text stream
		FntPrint(-1, "VAG SAMPLE - PRESS X OR O TO PLAY\n");
		FntPrint(-1, "COUNTER=%d\n", counter);
		
		// Draw the last created text stream
		FntFlush(-1);
		
		// Update display
		display();
		
		// Increment the counter
		counter++;
	}
	
	return 0;
	
} /* main */
