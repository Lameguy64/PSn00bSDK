/*
 * LibPSn00b Example Programs
 *
 * CD-XA Audio Example
 * 2019 - 2021 Meido-Tek Productions / PSn00bSDK Project
 *
 * Demonstrates playback and looping of CD-XA audio using the
 * new libpsxcd library.
 *
 *
 * Since there is not yet an open source XA audio encoder yet, the
 * only way to create XA audio data is by using the official tools.
 * You will have to provide your own XA file to get this example
 * to work.
 *
 * You will also need MKPSXISO (https://github.com/lameguy64/mkpsxiso)
 * to build an ISO image with your XA audio data.
 *
 *
 * Theory of operation:
 *
 * CD-XA playback is accomplished by first locating the XA file in
 * the ISO9660 file system using CdSearchFile(). CD mode is then set
 * using the CdlSetmode command with CdlModeSpeed, CdlModeRT and
 * CdlModeSF flags to configure the CD controller for XA audio
 * streaming. Trying to play XA audio without CdlModeSF, which
 * enables filtering, will feed all XA channels to the SPU resulting
 * in a stuttery cacophony of sounds.
 *
 * The XA channel for playback is selected using  the CdlSetfilter
 * command and CdlFILTER struct. XA audio is usually comprised of
 * multiple audio streams interleaved together and this command
 * sets which channel to filter in from all the other channels.
 * CdlModeSF enables the filtering feature and is required when
 * playing back XA audio streams with multiple interleaved sound
 * channels.
 *
 * Playback is initiated by issuing CdlReadS with the XA file's
 * location specified as a parameter, which is internally issued
 * to the CD controller as the seek target before CdlReadS is
 * actually issued.
 *
 * Playback can be stopped by simply issuing CdlPause during playback.
 * It is not recommended to use CdlStop as it will stop the disc spinning
 * and result to much slower response once the disc has stopped.
 *
 * The most effective method of determining the end of an XA stream is
 * by hooking a callback routine with CdReadyCallback(), which is
 * triggered whenever a data sector has been read and checking the
 * header of the received sector if it belongs to the channel currently
 * being played.
 *
 *
 * Tips:
 *
 *	- For best efficiency, it is recommended to have all XA tracks in
 *    a single XA file have roughly the same length to one another,
 *    otherwise tracks that end short will be padded with empty sectors
 *	  which wastes potentially usable disc space.
 *
 *  - The CD filter can be changed during playback and the switchover is
 *    completely seamless. Use this trick to accomplish dynamic music
 *    effects that change seamlessly based on events during gameplay.
 *
 *	- With custom tools, it is possible to interleave data sectors with
 *	  a single XA audio stream which would permit music playback during
 *	  loading sessions (aside from using sequenced music). Alternatively,
 *	  CdlReadN can be used to begin playback but with more reliable data
 *	  reading. This is also how streaming FMV sequences are accomplished.
 *
 *
 * Pros over CD audio:
 *
 *	- Does not require changing disc speed when switching between data
 *	  access and audio playback.
 *
 *	- Permits switching between streams during playback without restart.
 *
 *	- Near 1:4 audio compression ratio.
 *
 *	- Data sectors can be interleaved alongside XA audio.
 *
 *
 * Cons compared to CD audio:
 *
 *	- Skips more often on a poor condition disc or optical pick-up.
 *
 *	- Cannot be played with any CD player.
 *
 *	- Lower audio sample rate (37.8KHz whereas CD audio is 44.1KHz, not
 *	  good for audiophools).
 *
 *
 * Controls:
 *
 *	  Up/Down	- Select channel.
 *	  Cross		- Play selected channel.
 *	  Circle	- Stop selected channel.
 *	  Right		- Switch channel (without restarting playback).
 *
 *
 * Example by Lameguy64
 * (TODO: clean up/rewrite this example)
 *
 * Changelog:
 *
 *	May 10, 2021		- Variable types updated for psxgpu.h changes.
 *
 *	November 22, 2019	- Initial version
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psxetc.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxapi.h>
#include <psxpad.h>
#include <psxsio.h>
#include <psxspu.h>
#include <psxcd.h>

#define MAX_BALLS	1536		/* Number of balls to display */

#define OT_LEN 		8			/* Ordering table length */

/* Screen coordinates */
#define SCREEN_XRES	320
#define SCREEN_YRES	240

#define CENTER_X	SCREEN_XRES/2
#define CENTER_Y	SCREEN_YRES/2


/* Display and drawing environments */
DISPENV disp[2];
DRAWENV draw[2];

char pribuff[2][65536];			/* Primitive packet buffers */
uint32_t ot[2][OT_LEN];		/* Ordering tables */
char *nextpri;					/* Pointer to next packet buffer offset */
int db = 0;						/* Double buffer index */


/* Ball struct and array */
typedef struct BALL_TYPE
{
	short x,y;
	short xdir,ydir;
	unsigned char r,g,b,p;
} BALL_TYPE;

BALL_TYPE balls[MAX_BALLS];

/* Ball texture reference */
extern const uint32_t ball16c[];

/* TIM image parameters for loading the ball texture and drawing sprites */
TIM_IMAGE tim;


/* XA audio handling stuff */
volatile int num_loops=0;			/* Loop counter */
volatile int xa_play_channel;		/* Currently playing channel number */
CdlLOC xa_loc;						/* XA data start location */

/* Pad input buffer*/
char padbuff[2][34];


// https://problemkaputt.de/psx-spx.htm#cdromxasubheaderfilechannelinterleave
typedef struct {
	uint8_t file, channel;
	uint8_t submode, coding_info;
} XA_Header;

typedef enum {
	XA_END_OF_RECORD = 1 << 0,
	XA_TYPE_VIDEO    = 1 << 1,
	XA_TYPE_AUDIO    = 1 << 2,
	XA_TYPE_DATA     = 1 << 3,
	XA_TRIGGER       = 1 << 4,
	XA_FORM2         = 1 << 5,
	XA_REAL_TIME     = 1 << 6,
	XA_END_OF_FILE   = 1 << 7
} XA_SubmodeFlag;

// https://problemkaputt.de/psx-spx.htm#cdromsectorencoding
typedef struct {
	CdlLOC     pos;
	XA_Header  xa_header[2];
	uint8_t    data[2048];
	uint32_t   edc;
	uint8_t    ecc[276];
} Sector;

// This buffer is used by cd_event_handler() as a temporary area for sectors
// read from the CD. Due to DMA limitations it can't be allocated on the stack
// (especially not in the interrupt callbacks' stack, whose size is very
// limited).
Sector sector;

void cd_event_handler(CdlIntrResult event, uint8_t *payload) {
	// Ignore all events other than a sector being ready.
	if (event != CdlDataReady)
		return;

	// Fetch the sector and check if it is part of the audio file by checking
	// its XA flags. If it isn't an audio sector, restart playback.
	CdGetSector(&sector, sizeof(Sector) / 4);

	if (
		!(sector.xa_header[0].submode & XA_TYPE_AUDIO) &&
		!(sector.xa_header[1].submode & XA_TYPE_AUDIO)
	) {
		// Seek back to the beginning of the file.
		CdControlF(CdlReadS, &xa_loc);

		num_loops++;
	}
}

void init()
{
	int i;

	/* Uncomment to direct tty messages to serial */
	//AddSIO(115200);

	/* Reset GPU (also installs event handler for VSync) */
	printf("Init GPU... ");
	ResetGraph( 0 );
	printf("Done.\n");


	/* Initialize SPU and CD-ROM */
	printf("Initializing CD-ROM... ");
	SpuInit();
	CdInit();
	printf("Done.\n");


	/* Set display and draw environment parameters */
	SetDefDispEnv(&disp[0], 0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDispEnv(&disp[1], 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);

	SetDefDrawEnv(&draw[0], 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&draw[1], 0, 0, SCREEN_XRES, SCREEN_YRES);


	/* Set clear color, area clear and dither processing */
	setRGB0(&draw[0], 63, 0, 127);
	draw[0].isbg = 1;
	draw[0].dtd = 1;
	setRGB0(&draw[1], 63, 0, 127);
	draw[1].isbg = 1;
	draw[1].dtd = 1;


	/* Load and open font stream */
	FntLoad(960, 0);
	FntOpen(32, 32, 256, 176, 2, 200);


	/* Upload the ball texture */
	GetTimInfo(ball16c, &tim); /* Get TIM parameters */
	LoadImage(tim.prect, tim.paddr);		/* Upload texture to VRAM */
	if( tim.mode & 0x8 )
	{
		LoadImage(tim.crect, tim.caddr);	/* Upload CLUT if present */
	}


	/* Calculate ball positions */
	printf("Calculating balls... ");

	for(i=0; i<MAX_BALLS; i++)
	{
		balls[i].x = (rand()%304);
		balls[i].y = (rand()%224);
		balls[i].xdir = 1-(rand()%3);
		balls[i].ydir = 1-(rand()%3);
		if( !balls[i].xdir )
			balls[i].xdir = 1;
		if( !balls[i].ydir )
			balls[i].ydir = 1;
		balls[i].xdir *= 2;
		balls[i].ydir *= 2;
		balls[i].r = (rand()%256);
		balls[i].g = (rand()%256);
		balls[i].b = (rand()%256);
	}

	printf("Done.\n");


	/* Initialize pad */
	InitPAD(padbuff[0], 34, padbuff[1], 34);
	StartPAD();
	ChangeClearPAD(0);

}


int main(int argc, const char* argv[])
{

	SPRT *spr;
	SPRT_16 *sprt;
	DR_TPAGE *tpri;
	PADTYPE *pad;

	CdlFILE	file;
	CdlFILTER filter;

	int i,counter=0;
	int sel_channel=0;
	int p_up=0,p_down=0,p_right=0,p_cross=0,p_circle=0;


	/* Init graphics and stuff before doing anything else */
	init();

	/* Locate the XA file */
	if( !CdSearchFile(&file, "\\XASAMPLE.XA") )
	{
		printf("Unable to find file.\n");
		return 0;
	}
	else
	{
		int sec;
		sec = CdPosToInt(&file.pos);
		printf("XA located at sector %d size %d.\n", sec, file.size);
	}

	/* Save file location as XA location */
	xa_loc = file.pos;

	/* Hook XA callback function to CdReadyCallback (for auto stop/loop */
	EnterCriticalSection();
	CdReadyCallback(&cd_event_handler);
	ExitCriticalSection();

	/* Set CD mode for XA streaming (2x speed, send XA to SPU, enable filter */
	i = CdlModeSpeed|CdlModeRT|CdlModeSF;
	CdControl(CdlSetmode, &i, 0);

	/* Set file 1 on filter for channels 0-31 */
	filter.file = 1;

	/* Main loop */
	printf("Entering loop...\n");

	while(1) {

		pad = ((PADTYPE*)padbuff[0]);

		if( pad->stat == 0 )
		{
			if(( pad->type == 0x4 )||( pad->type == 0x5 )||( pad->type == 0x7 ))
			{
				/* Menu selection controls */
				if( !(pad->btn&PAD_UP) )
				{
					if( !p_up )
					{
						if( sel_channel > 0 )
						{
							sel_channel--;
						}
						p_up = 1;
					}
				}
				else
				{
					p_up = 0;
				}

				if( !(pad->btn&PAD_DOWN) )
				{
					if( !p_down )
					{
						if( sel_channel < 31 )
						{
							sel_channel++;
						}
						p_down = 1;
					}
				}
				else
				{
					p_down = 0;
				}

				/* Play selected XA channel from start */
				if( !(pad->btn&PAD_CROSS) )
				{
					if( !p_cross )
					{
						filter.chan = sel_channel;
						CdControl(CdlSetfilter, &filter, 0);
						CdControl(CdlReadS, &xa_loc, 0);
						xa_play_channel = sel_channel;
						p_cross = 1;
					}
				}
				else
				{
					p_cross = 0;
				}

				/* Stop playback */
				if( !(pad->btn&PAD_CIRCLE) )
				{
					if( !p_circle )
					{
						CdControl(CdlPause, 0, 0);
						p_circle = 1;
					}
				}
				else
				{
					p_circle = 0;
				}

				/* Change XA channel */
				if( !(pad->btn&PAD_RIGHT) )
				{
					if( !p_right )
					{
						filter.chan = sel_channel;
						CdControl(CdlSetfilter, &filter, 0);
						xa_play_channel = sel_channel;
						p_right = 1;
					}
				}
				else
				{
					p_right = 0;
				}

			}
		}


		/* Display information */
		FntPrint(-1, "\n PSN00BSDK XA AUDIO EXAMPLE\n\n");
		FntPrint(-1, " CHANNEL:\n");

		for(i=0; i<8; i++)
		{
			if( i == sel_channel )
			{
				FntPrint(-1, " -->%d\n", i);
			}
			else
			{
				FntPrint(-1, "    %d\n", i);
			}
		}

		FntPrint(-1, "\n CURRENT=%d STATUS=%x LOOPS=%d\n",
			xa_play_channel, CdStatus(), num_loops);
		FntPrint(-1, "\n <X>-PLAY (START) <O>-STOP\n <R>-SET CHANNEL\n");


		/* Clear ordering table and set start address of primitive buffer */
		ClearOTagR(ot[db], OT_LEN);
		nextpri = pribuff[db];


		/* Sort the balls */
		sprt = (SPRT_16*)nextpri;
		for( i=0; i<MAX_BALLS; i++ ) {

			setSprt16(sprt);
			setXY0(sprt, balls[i].x, balls[i].y);
			setRGB0(sprt, balls[i].r, balls[i].g, balls[i].b);
			setUV0(sprt, 0, 0);
			setClut(sprt, tim.crect->x, tim.crect->y);

			addPrim(ot[db]+(OT_LEN-1), sprt);
			sprt++;

			balls[i].x += balls[i].xdir;
			balls[i].y += balls[i].ydir;

			if( ( balls[i].x+16 ) > SCREEN_XRES ) {
				balls[i].xdir = -2;
			} else if( balls[i].x < 0 ) {
				balls[i].xdir = 2;
			}

			if( ( balls[i].y+16 ) > SCREEN_YRES ) {
				balls[i].ydir = -2;
			} else if( balls[i].y < 0 ) {
				balls[i].ydir = 2;
			}

		}
		nextpri = (char*)sprt;


		/* Sort a TPage primitive so the sprites will draw pixels from */
		/* the correct texture page in VRAM */
		tpri = (DR_TPAGE*)nextpri;
		setDrawTPage(tpri, 0, 0, getTPage(0, 0, tim.prect->x, tim.prect->y));
		addPrim(ot[db]+(OT_LEN-1), tpri);
		nextpri += sizeof(DR_TPAGE);

		/* Draw font */
		FntFlush(-1);

		/* Wait for GPU and VSync */
		DrawSync(0);
		VSync(0);

		/* Since draw.isbg is non-zero this clears the screen */
		PutDispEnv(&disp[db]);
		PutDrawEnv(&draw[db]);
		SetDispMask(1);

		/* Begin drawing the new frame */
		DrawOTag( ot[db]+(OT_LEN-1) );

		/* Alternate to the next buffer */
		db = !db;

		/* Periodically issue CdlNop every second to update CdStatus() */
		counter++;
		if( (counter%60) == 59 )
		{
			CdControl(CdlNop, 0, 0);
		}

	}

	return 0;

}
