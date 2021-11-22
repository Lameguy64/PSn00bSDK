#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psxetc.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxapi.h>
#include <siofs.h>

#define OT_LEN 		8

#define SCREEN_XRES	320
#define SCREEN_YRES	240

#define CENTER_X	SCREEN_XRES/2
#define CENTER_Y	SCREEN_YRES/2


/* Display and drawing environments */
DISPENV disp[2];
DRAWENV draw[2];

char pribuff[2][16384];			/* Primitive packet buffers */
unsigned int ot[2][OT_LEN];		/* Ordering tables */
char *nextpri;					/* Pointer to next packet buffer offset */
int db = 0;						/* Double buffer index */


void init() {
	
	int i;
	
	/* Reset GPU (also installs event handler for VSync) */
	printf("Init GPU... ");
	ResetGraph( 0 );
	printf("Done.\n");
	
	
	printf("Set video mode... ");
	
	/* Set display and draw environment parameters */
	SetDefDispEnv( &disp[0], 0, 0, SCREEN_XRES, SCREEN_YRES );
	SetDefDrawEnv( &draw[0], 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES );
	SetDefDispEnv( &disp[1], 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES );
	SetDefDrawEnv( &draw[1], 0, 0, SCREEN_XRES, SCREEN_YRES );
	
	//disp[0].isinter = 1; /* Enable interlace (required for hires) */
	//disp[1].isinter = 1;
	
	/* Set clear color, area clear and dither processing */
	setRGB0( &draw[0], 63, 0, 127 );
	draw[0].isbg = 1;
	draw[0].dtd = 1;
	
	setRGB0( &draw[1], 63, 0, 127 );
	draw[1].isbg = 1;
	draw[1].dtd = 1;
	
	/* Apply the display and drawing environments */
	PutDispEnv( &disp[1] );
	PutDrawEnv( &draw[1] );
	
	/* Enable video output */
	SetDispMask( 1 );
	
	printf("Done.\n");
	
	
	FntLoad( 960, 0 );
	FntOpen( 8, 16, 304, 216, 0, 100 );
	
}

extern void _InitPadDirect(void);
extern int _PadReadDirect(int port, unsigned char *data, int len);
extern int _CardRead(int port, unsigned char *data, int lba);
extern int _CardWrite(int port, unsigned char *data, int lba);
extern int _PadExchng(int byte);
extern void _PadSetPort(int port);

extern unsigned char _pad_mot_values[];

unsigned char padbuff[2][34];


char *hex_tbl = "0123456789ABCDEF";

void byte2str(unsigned char *in, int bytes, char *out)
{
	int i;
	
	for( i=0; i<bytes; i++ )
	{
		*out = hex_tbl[in[i]>>4];	out++;
		*out = hex_tbl[in[i]&0xF];	out++;
	}
	
	*out = 0x0;
}

unsigned char card_buff[128];

void dump_data(unsigned char *buff)
{
	int ix,iy;
	
	for( iy=0; iy<8; iy++ )
	{
		for( ix=0; ix<16; ix++ )
		{
			printf( "%02x", *buff );
			buff++;
		}
		printf( "\n" );
	}
}

void TurnOnVibrators(void)
{
	_PadSetPort(0);
	
	if( _PadExchng(0x01) & 0x100 )
	{
		ExitCriticalSection();
		return;
	}
	
	if( _PadExchng(0x43) & 0x100 )
	{
		ExitCriticalSection();
		return;
	}
	
	_PadExchng(0x00);
	_PadExchng(0x01);
	
	while( !(_PadExchng(0) & 0x100) );
	
	_PadSetPort(2);
	
	ExitCriticalSection();
	VSync(0);
	EnterCriticalSection();
	
	// Set analog state
	_PadSetPort(0);
	
	_PadExchng(0x01);
	_PadExchng(0x4D);
	_PadExchng(0x00);
	
	_PadExchng(0x00);
	_PadExchng(0x01);
	_PadExchng(0xFF);
	_PadExchng(0xFF);
	_PadExchng(0xFF);
	_PadExchng(0xFF);
	
	_PadSetPort(2);
	
	ExitCriticalSection();
	VSync(0);
	EnterCriticalSection();
	
	// Exit configuration mode
	_PadSetPort(0);
		
	_PadExchng(0x01);
	_PadExchng(0x43);
	_PadExchng(0x00);
	_PadExchng(0x00);
	
	while( !(_PadExchng(0) & 0x100) );
	
	_PadSetPort(2);
	
	ExitCriticalSection();
	VSync(0);
}

void TurnOnAnalog(void)
{
	EnterCriticalSection();
	
	// Enter configuration mode
	_PadSetPort(0);
	
	if( _PadExchng(0x01) & 0x100 )
	{
		ExitCriticalSection();
		return;
	}
	
	if( _PadExchng(0x43) & 0x100 )
	{
		ExitCriticalSection();
		return;
	}
	
	_PadExchng(0x00);
	_PadExchng(0x01);
	
	while( !(_PadExchng(0) & 0x100) );
	
	_PadSetPort(2);
	
	ExitCriticalSection();
	VSync(0);
	EnterCriticalSection();
	
	// Set analog state
	_PadSetPort(0);
	
	_PadExchng(0x01);
	_PadExchng(0x44);
	_PadExchng(0x00);
	_PadExchng(0x01);	// 0 - analog off, 1 - analog on
	_PadExchng(0x02);

	while( !(_PadExchng(0) & 0x100) );

	_PadSetPort(2);
	
	ExitCriticalSection();
	VSync(0);
	EnterCriticalSection();
	
	// Exit configuration mode
	_PadSetPort(0);
		
	_PadExchng(0x01);
	_PadExchng(0x43);
	_PadExchng(0x00);
	_PadExchng(0x00);
	
	while( !(_PadExchng(0) & 0x100) );
	
	_PadSetPort(2);
	
	ExitCriticalSection();
	VSync(0);
}

int main(int argc, const char* argv[]) {
	
	DR_TPAGE	*tpri;
	
	int i,j,counter=0,fd;
	char textbuff[64];
	char *memcard_image,*p;
	
	/* Init graphics and stuff before doing anything else */
	init();
	
	_InitPadDirect();
	
	
	/*
	if( fsInit(115200) >= 0 )
	{
		printf("Loading memory card image... ");
		
		fd = fsOpen("cardimage.bin", FS_READ|FS_BINARY);
		
		if( fd < 0 )
		{
			printf("File not found\n");
		}
		else
		{
			memcard_image = (char*)malloc(131072);
		
			fsRead(fd, memcard_image, 131072);
			
			fsClose(fd);
		
			printf("Done.\n");
			
			p = memcard_image;
			for( j=0; j<1024; j++ )
			{
				i = _CardWrite(0, p, j);
				printf("Writing memory card (%d/1024)... \r", j);
				if( i )
					printf("\nError %d at sector %d\n", i, j);
				p += 128;
				// Memory cards need at least 2 vsyncs between each write
				// apparently
				VSync(2);
			}
			
		}
	}
	else
	{
		fd = -1;
		memcard_image = NULL;
	}
	*/
	
	/*
	for( j=0; j<1024; j++ )
	{
		memset( card_buff, 0xff, 128 );
		i = _CardRead( 0, card_buff, j );
	
		printf("Reading memory card (%d/1024)... \r", j);
	
		if( i != 0 )
		{
			printf("\nPotential read error at sector %d\n", j);
		}
		
		if( fd >= 0 )	
			fsWrite(fd, (u_char*)card_buff, 128);
		
		if( i )
			break;
	}
	
	if( fd >= 0 )
		fsClose(fd);
	
	printf("\nSuccessfully read entire memory card\n");
	*/
	
	/*
	i = _CardRead( 0, card_buff, j );
	printf("Clearing memory card FAT.\n");
	memset( card_buff, 0xff, 128 );
	for(j=0; j<4; j++)
	{
		i = _CardWrite( 0, card_buff, 4 );
		printf("_CardWrite returned %d\n", i);
		VSync(2);
	}
	*/
	
	
	TurnOnAnalog();
	TurnOnVibrators();
	
	/* Main loop */
	printf("Entering loop...\n");
	
	while(1) {
	
		/*
		if((counter>>4)&0x1)
		{
			_pad_mot_values[0] = 0xff;
		}
		else
		{
			_pad_mot_values[0] = 0x0;
		}
		*/
		
		i = _PadReadDirect(0, padbuff[0], 34);
		//_PadReadDirect(1, padbuff[1], 34);
		
		FntPrint( -1, "HELLO WORLD %d\n", counter );
		
		byte2str( padbuff[0], i, textbuff );
		FntPrint( -1, "P1:%s\n", textbuff );
		//byte2str( padbuff[1], 8, textbuff );
		//FntPrint( -1, "P2:%s\n", textbuff );
		
		/* Clear ordering table and set start address of primitive */
		/* buffer for next frame */
		ClearOTagR( ot[db], OT_LEN );
		nextpri = pribuff[db];
		
		
		
		/* Wait for GPU and VSync */
		FntFlush(-1);
		DrawSync( 0 );
		VSync( 0 );
		
		/* Since draw.isbg is non-zero this clears the screen */
		PutDispEnv( &disp[db] );
		PutDrawEnv( &draw[db] );
		
		/* Begin drawing the new frame */
		DrawOTag( ot[db]+(OT_LEN-1) );
		
		/* Alternate to the next buffer */
		db = !db;
		
		/* Increment counter for the snake animation */
		counter++;
		
	}
		
	return 0;

}
