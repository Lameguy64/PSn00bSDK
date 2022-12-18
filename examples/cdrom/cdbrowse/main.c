/* 
 * LibPSn00b Example Programs
 *
 * CD File Browser Example
 * 2020 - 2021 Meido-Tek Productions / PSn00bSDK Project
 *
 * Demonstrates listing and browsing directory contents of a CD-ROM containing
 * an ISO9660 file system, using the directory query functions of the libpsxcd
 * library.
 *
 * The ISO9660 support in libpsxcd only supports the original ISO9660
 * version 1 file system specification, meaning it cannot query Rock Ridge
 * or Joliet extensions that allow for long file names and unicode, despite
 * the ISO9660 version 1 specification supporting names longer than 8.3 to
 * begin with. However, discs written with such extensions should still be
 * readable as the ISO9660 version should still be present for compatibility,
 * with file names truncated to 8.3.
 *
 *
 * Directory querying with libpsxcd is accomplished by using functions
 * CdOpenDir(), CdReadDir() and CdCloseDir(). These functions work more or
 * less the same as opendir(), readdir() and closedir() in *nix-like
 * environments. While the library can support directories containing
 * any number of files, the file browser in this example is limited to 40
 * entries, but this can be easily extended if need be.
 *
 * Differentiating file and directory entries is easily determined by whether
 * or not the file has a version number suffix (ie. MYFILE.DAT;1). Directory
 * records do not have a version number suffix.
 *
 * Currently, the only way to signal the isofs routines of libpsxcd that a
 * disc change has occured is by simply issuing a CdlNop command. In a file
 * browser that is aware of a disc change occuring at any moment a CdlNop
 * command shall be issued at regular intervals of at least once a second.
 * This also keeps CdStatus() up to date with disc presence.
 *
 *
 * Controls:
 *
 *	  Up/Down	- Move selection cursor.
 *	  Cross		- Enter directory.
 *	  Circle	- Go back to parent directory.
 *
 *
 * Example by Lameguy64
 *
 *
 * Changelog:
 *
 *	May 10, 2021: Variable types updated for psxgpu.h changes.
 *
 *	February 25, 2020: Initial version.
 *
 *  July 12, 2020: Updated CD-ROM directory query logic on disc change slightly.
 */
 
#include <stdint.h>
#include <stdbool.h>
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

/* Pad input buffer*/
char padbuff[2][34];


int query_dir(const char* path, CdlFILE* files, int max)
{
	CdlDIR*	dir;
	int		files_found;
	
	printf( "Querying directory %s... ", path );
	files_found = 0;
	
	/* Open the directory */
	dir = CdOpenDir( path );
	if( dir )
	{
		/* Read entries */
		while( CdReadDir( dir, &files[files_found] ) )
		{
			files_found++;
			if( files_found >= max )
				break;
		}
		
		/* Close directory after query */
		CdCloseDir( dir );
	}
	else
	{
		/* If directory path does not exist */
		printf( "Directory not found.\n" );
		return 0;
	}
	
	printf( "Done.\n" );
	return files_found;
}


void init()
{	
	int i;
	
	/* Uncomment to send tty messages to SIO */
	//AddSIO( 115200 );
	
	/* Reset GPU (also installs event handler for VSync) */
	printf("Init GPU... ");
	ResetGraph( 0 );
	printf("Done.\n");

	
	/* Uncomment to direct tty messages to serial */
	//AddSIO(115200);
	
	
	/* Initialize SPU and CD-ROM */
	printf("Initializing CD-ROM... ");
	//SpuInit();
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
	FntOpen(32, 32, 256, 176, 2, 400);
	
	
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
	EnterCriticalSection();
	
	InitPAD(padbuff[0], 34, padbuff[1], 34);
	StartPAD();
	ChangeClearPAD(0);
	
	ExitCriticalSection();
	
}


int main(int argc, const char* argv[])
{

	SPRT*		spr;				// Primitive stuff
	SPRT_16*	sprt;
	DR_TPAGE*	tpri;
	
	PADTYPE*	pad;				// Pad stuff
	
	CdlFILE		files[40];			// To store file/directory entries
	char		path[128];			// Directory path buffer
	char		disc_label[32];		// Disc label
	
	int i,counter=0;
	int files_found;
	int sel_channel=0;
	int list_scroll;
	int p_up=0,p_down=0,p_cross=0,p_circle=0;
	int disc_present,update_listing;
	
	/* Init graphics and stuff before doing anything else */
	init();

	/* Updates CdStatus() */
	CdControl( CdlNop, 0, 0 );
		
	/* Main loop */
	printf("Entering loop...\n");
	
	list_scroll = 0;
	disc_present = false;
	update_listing = true;
	
	printf( "Calling CdStatus()...\n" );
	CdStatus();
	printf( "Call done.\n" );
	
	while(1) {
		
		/* Set flag if disc has been removed */
		if( (CdStatus()&0x12) != 0x2 )
		{
			disc_present = false;
		}
		
		
		/* Get pad stats */
		pad = ((PADTYPE*)padbuff[0]);

		if( pad->stat == 0 )
		{
			if(( pad->type == 0x4 )||( pad->type == 0x5 )||( pad->type == 0x7 ))
			{
				/* Menu controls */
				if( disc_present )
				{
					if( !(pad->btn&PAD_UP) )
					{
						if( !p_up )
						{
							if( sel_channel > 0 )
							{
								sel_channel--;
								
								if( (sel_channel-list_scroll) < 0 )
								{
									list_scroll = sel_channel;
								}
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
							if( sel_channel < files_found-1 )
							{
								sel_channel++;
								
								if( (sel_channel-list_scroll) > 15 )
								{
									list_scroll = sel_channel-15;
								}
							}
							p_down = 1;
						}
					}
					else
					{
						p_down = 0;
					}
				}
				
				/* Enter a selected directory */
				if( !(pad->btn&PAD_CROSS) )
				{
					if( !p_cross )
					{
						if( disc_present )
						{
							if( strchr( files[sel_channel].name, ';' ) == 0 )
							{
								if( strcmp( files[sel_channel].name, ".." ) 
									== 0 )
								{
									*(strrchr( path, '\\' )+1) = 0x0;
									if( strlen( path ) > 1 )
									{
										path[strlen( path )-1] = 0x0;
									}
								}
								else if( strcmp( files[sel_channel].name, "." ) )
								{
									if( strlen( path ) > 1 )
									{
										strcat( path, "\\" );
									}
									strcat( path, files[sel_channel].name );
								}
								update_listing = true;
							}
						}
						else
						{
							strcpy( path, "\\" );
							update_listing = true;
						}
						p_cross = 1;
					}
				}
				else
				{
					p_cross = 0;
				}
				
				/* Return to root directory */
				if( !(pad->btn&PAD_CIRCLE) )
				{
					if( !p_circle )
					{
						strcpy( path, "\\" );
						update_listing = true;
						p_circle = 1;
					}
				}
				else
				{
					p_circle = 0;
				}				
			}
		}
		
		
		/* Updates directory listing */
		if( update_listing )
		{
			/* Reset path and update label if new disc inserted */
			if( !disc_present )
			{
				strcpy( path, "\\" );
				CdGetVolumeLabel(disc_label);
				disc_present = true;
			}
				
			/* Query directory */
			files_found = query_dir( path, files, 40 );
			sel_channel = 0;
			list_scroll = 0;
			
			update_listing = false;
		}
		
		
		/* Display information */
		FntPrint( -1, "\n PSN00BSDK CD BROWSER EXAMPLE\n\n" );
		
		if( disc_present )
		{
			FntPrint( -1, " FILES:%d LABEL:%s\n", files_found, disc_label );
			FntPrint( -1, " %s\n", path );
			
			for(i=0; i<16; i++)
			{
				if( (i+list_scroll) > (files_found-1) )
					break;
				
				if( (i+list_scroll) == sel_channel )
				{
					FntPrint( -1, " ->%s\n", files[i+list_scroll].name );
				}
				else
				{
					FntPrint( -1, "   %s\n", files[i+list_scroll].name );
				}
				
			}
		}
		else
		{
			if( (CdStatus()&0x12) == 0x2 )
			{
				FntPrint( -1, "\n PRESS <X> TO LIST CONTENTS %x\n", 
					CdStatus() );
			}
			else
			{
				FntPrint( -1, "\n NO DISC INSERTED %x\n", 
					CdStatus() );
			}
		}
		
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
		
		/* Periodically issue a CdlNop to keep CdStatus() updated */
		counter++;
		if( (counter%60) == 59 )
		{
			CdControl(CdlNop, 0, 0);
		}
	}
	
	return 0;

}
