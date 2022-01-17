/* 
 * LibPSn00b Example Programs
 *
 * Drawing Tile-maps with Assembler Routines
 * 2022 Meido-Tek Productions / PSn00bSDK Project
 *
 * Example by John "Lameguy" Wilbert Villamor (Lameguy64)
 *
 * Demonstrates the use of assembler code to write a high-speed graphics
 * routine that draws a 2D tile-map by generating SPRT_16 primitives.
 * The assembler routine is called from a compiled C module in a manner no
 * different to calling a C routine... A program written in this manner is
 * known as mixed-language programming which includes other programming
 * languages, not just assembly.
 *
 * This example also demonstrates using assembler files to include data files
 * into a program as arrays (see 'data.s'). Arguably much more elegant and
 * convenient than converting files to C headers like in the old days.
 *
 * The tile data and drawing is handled in a manner similar to that of the
 * tile-map drawing functions in libgs where the map data consists of 16-bit
 * words with 0xFFFF being a transparent tile, with each tile entry defined
 * by an array of structs specifying the tpage, clut and u,v coordinates for
 * each tile number. This provides flexibility with how the tile data will be
 * organized in the framebuffer.
 *
 * Per-pixel clipping of the tile-map is actually not performed by the
 * DrawTiles function but rather with DR_AREA primitives. This method
 * eliminates code overhead from performing the clipping operations in
 * software and thus, yields the fastest possible performance for tile
 * sorting. Use of the DR_AREA primitive must be handled with care, as
 * setting it beyond the display buffers could overwrite textures and
 * CLUT data.
 *
 * Changelog:
 *	
 *	January 16, 2022 - Initial version.
 *
 * Controls:
 *
 *	D-pad		- Scroll tile-map
 *	L1 (hold)	- Move tile-map window with D-pad (must be shrunk first)
 *	L2 (hold)	- Resize tile-map window with D-pad
 *
 */
 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <psxetc.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxapi.h>
#include <psxpad.h>

/*
 * Constants for convenience
 */
#define OT_LEN		4		/* Number of ordering table entries */
#define PKTBUFF_LEN	32768	/* Size of packet buffer (in bytes) */

/* Define display/draw environments for double buffering */
DISPENV disp[2];
DRAWENV draw[2];
int db;

/* Define the ordering table and packet buffer arrays */
long ot[2][OT_LEN];
char pkt[2][PKTBUFF_LEN];

/* Next packet pointer for primitive generation */
char *pkt_addr;

/* Pad data buffer */
char pad_buff[2][34];

/*
 * Tile definition structure (to define VRAM coordinates of tiles)
 */
typedef struct _TILEDEF
{
	u_char	u,v;		// Texture coordinates of tile
	u_short	clut;		// CLUT number of tile
	u_short	pad;		// Padding
	u_short	tpage;		// Texture page number
} TILEDEF;

/*
 * Tile info structure (for drawing function)
 */
typedef struct _TILEINFO
{
	RECT	window;		// Drawing window of tile info (only cull, not clip)
	TILEDEF	*tiles;		// Pointer to TILEDEF array
	u_short	*mapdata;	// Pointer to 16-bit map data
	u_short	map_w;		// Tile map dimensions
	u_short	map_h;
} TILEINFO;

/* Define array of tiledefs */
TILEDEF tiles[256];

/*
 * Declarations for data.s
 */
extern u_long tim_tileset[];

/*
 * Declarations for drawtiles.s
 */
extern u_char *DrawTiles(int scroll_x, int scroll_y,
	TILEINFO *info, long *ot, u_char *pri);

/*
 * Init function
 */
void init(void)
{
	TIM_IMAGE tim;
	
	int i,tx,ty;
	
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
	
	// Setup buffer counter, packet pointer and ordering tables
	db = 0;
	pkt_addr = pkt[db];
	
	// Clear ordering tables
	ClearOTagR(ot[0], OT_LEN);
	ClearOTagR(ot[1], OT_LEN);
	
	// Load test font
	FntLoad(960, 0);
	
	// Open up a test font text stream of 100 characters
	FntOpen(0, 8, 320, 224, 0, 100);
	
	// Upload tileset TIM
	GetTimInfo((u_long*)tim_tileset, &tim);	/* Get TIM parameters */
	LoadImage(tim.prect, tim.paddr);		/* Upload texture to VRAM */
	if( tim.mode & 0x8 )					/* Upload CLUT if present */
		LoadImage(tim.crect, tim.caddr);
	
	// Initialize tiledefs with coords to the tileset TIM
	i = 0;
	for(ty=0; ty<16; ty++)
	{
		for(tx=0; tx<16; tx++)
		{
			tiles[i].u = tx<<4;
			tiles[i].v = ty<<4;
			tiles[i].tpage = getTPage(tim.mode & 3, 0, 
				tim.prect->x, tim.prect->y);
			tiles[i].clut = getClut(tim.crect->x, tim.crect->y);
			i++;
		}
	}
	
	// Initialize pads
	InitPAD(&pad_buff[0][0], 34, &pad_buff[1][0], 34);
	StartPAD();
	ChangeClearPAD(0);
	
} /* init */

/*
 * Display function
 */
void display(void)
{
	// Wait for all drawing to complete
	DrawSync(0);
	
	// Wait for vertical sync to cap the logic to 60fps (or 50 in PAL mode)
	// and avoid screen tearing
	VSync(0);

	// Switch pages	
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);
	
	// Begin drawing of the ordering table
	DrawOTag(ot[db]+(OT_LEN-1));
	
	// Toggle buffer index
	db = !db;
	
	// Clear next ordering table array
	ClearOTagR(ot[db], OT_LEN);
	
	// Reset packet pointer
	pkt_addr = pkt[db];
	
	// Enable display output, ResetGraph() disables it by default
	SetDispMask(1);
	
} /* display */

/*
 * Initializes a randomly generated tile map
 */
void initdata(u_short *tiledata, int w, int h)
{
	int tx,ty;
	
	for(ty=0; ty<h; ty++)
	{
		srand(200+ty);
		for(tx=0; tx<w; tx++)
		{
			*tiledata = rand() & 0xFF;
			tiledata++;
		}
	}
	
} /* initdata */

/*
 * Simple line sorting function
 */
void sortLine(int x1, int y1, int x2, int y2, long *ot)
{
	LINE_F2 *line_pkt;
	
	line_pkt = (LINE_F2*)pkt_addr;
	setLineF2(line_pkt);
	setXY2(line_pkt, x1, y1, x2, y2);
	setRGB0(line_pkt, 255, 255, 255);
	addPrim(ot, line_pkt);
	line_pkt++;
	
	pkt_addr = (char*)line_pkt;
	
} /* sortLine */

/*
 * Simple box sorting function
 */
void sortBox(RECT *rect)
{
	sortLine(rect->x, rect->y,
		rect->x, rect->y+rect->h, 
		ot[db]);
	sortLine(rect->x+rect->w, rect->y,
		rect->x+rect->w, rect->y+rect->h, 
		ot[db]);
	sortLine(rect->x, rect->y,
		rect->x+rect->w, rect->y,
		ot[db]);
	sortLine(rect->x, rect->y+rect->h,
		rect->x+rect->w, rect->y+rect->h, ot[db]);
		
} /* sortBox */

/*
 * Sorts a draw area primitive for hardware clipping
 */
void sortDrawEnv(RECT *drawarea)
{
	DR_AREA *drawpkt;
	
	drawpkt = (DR_AREA*)pkt_addr;
	setDrawArea(drawpkt, drawarea);
	addPrim(ot[db], drawpkt);
	pkt_addr += sizeof(DR_AREA);
	
} /* sortDrawEnv */

/*
 * Main function
 */
int main(int argc, const char *argv[])
{
	u_short		*tiledata;
	TILEINFO	tileinfo;
	PADTYPE		*pad;
	RECT		cliprect;
	
	int i,scroll_x,scroll_y;
	
	/* Init stuff */
	init();
	
	/* Allocate buffer for tile data */
	tiledata = (u_short*)malloc((256*256)<<1);
	
	/* Generate a random tilemap of values 0-255 */
	initdata(tiledata, 256, 256);
	
	/* Define the TILEINFO struct */
	tileinfo.window.x = 0;
	tileinfo.window.y = 0;
	tileinfo.window.w = 320;
	tileinfo.window.h = 240;
	tileinfo.tiles = tiles;
	tileinfo.mapdata = tiledata;
	tileinfo.map_w = 256;
	tileinfo.map_h = 256;
	
	/* Main loop */
	scroll_x = 0;
	scroll_y = 0;
	
	while(1)
	{
		/* Handle inputs */
		pad = (PADTYPE*)&pad_buff[0][0];
		
		if( pad->stat == 0 )
		{
			if( ( pad->type == 0x4 ) || 
				( pad->type == 0x5 ) || 
				( pad->type == 0x7 ) )
			{
				if( !(pad->btn&PAD_L1) )	/* Window resize */
				{
					if( !(pad->btn&PAD_UP) && ( tileinfo.window.y > 0 ) )
					{
						tileinfo.window.y--;
					}
					i = tileinfo.window.y+tileinfo.window.h;
					if( !(pad->btn&PAD_DOWN) && ( i < disp[db].disp.h ) )
					{
						tileinfo.window.y++;
					}
					if( !(pad->btn&PAD_LEFT) && ( tileinfo.window.x > 0 ) )
					{
						tileinfo.window.x--;
					}
					i = tileinfo.window.x+tileinfo.window.w;
					if( !(pad->btn&PAD_RIGHT) && ( i < disp[db].disp.w ) )
					{
						tileinfo.window.x++;
					}
				}
				else if( !(pad->btn&PAD_L2) )	/* Window move */
				{
					if( !(pad->btn&PAD_UP) && ( tileinfo.window.h > 0 ) )
					{
						tileinfo.window.h--;
					}
					i = tileinfo.window.y+tileinfo.window.h;
					if( !(pad->btn&PAD_DOWN) && ( i < disp[db].disp.h ) )
					{
						tileinfo.window.h++;
					}
					if( !(pad->btn&PAD_LEFT) && ( tileinfo.window.w > 0 ) )
					{
						tileinfo.window.w--;
					}
					i = tileinfo.window.y+tileinfo.window.h;
					if( !(pad->btn&PAD_RIGHT) && ( i < disp[db].disp.w ) )
					{
						tileinfo.window.w++;
					}
				}
				else							/* Scrolling */
				{
					if( !(pad->btn&PAD_UP) )
					{
						scroll_y-=2;
					}
					if( !(pad->btn&PAD_DOWN) )
					{
						scroll_y+=2;
					}
					if( !(pad->btn&PAD_LEFT) )
					{
						scroll_x-=2;
					}
					if( !(pad->btn&PAD_RIGHT) )
					{
						scroll_x+=2;
					}
				}
			}
		}
		
		/* Draw a box around the tile-map window */
		sortBox(&tileinfo.window);
		
		/* Sort default clipping (this is processed last) */
		sortDrawEnv(&draw[db].clip);
		
		/* Sort the tiles */
		pkt_addr = DrawTiles(scroll_x, scroll_y, 
			&tileinfo, ot[db], pkt_addr);
	
		/* Sort clipping to the tile window */
		cliprect = tileinfo.window;
		cliprect.y += draw[db].clip.y;
		sortDrawEnv(&cliprect);
		
		/* Print stats */
		FntPrint(-1, "X=%d Y=%d\n", scroll_x, scroll_y);
		FntPrint(-1, "WINDOW POS. (%d,%d)\n",
			tileinfo.window.x, tileinfo.window.y);
		FntPrint(-1, "WINDOW SIZE (%d,%d)\n",
			tileinfo.window.w, tileinfo.window.h);
		FntFlush(-1);
		
		/* Refresh display */
		display();
	}
	
	return 0;
	
} /* main */
