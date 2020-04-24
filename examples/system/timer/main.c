#include <stdio.h>
#include <psxgpu.h>
#include <psxapi.h>
#include <psxetc.h>

/* OT and Packet Buffer sizes */
#define OT_LEN			256
#define PACKET_LEN		1024

/* Screen resolution */
#define SCREEN_XRES		320
#define SCREEN_YRES		240

/* Screen center position */
#define CENTERX			SCREEN_XRES>>1
#define CENTERY			SCREEN_YRES>>1


/* Double buffer structure */
typedef struct {
	DISPENV	disp;			/* Display environment */
	DRAWENV	draw;			/* Drawing environment */
} DB;

/* Double buffer variables */
DB		db[2];
int		db_active = 0;


/* Function declarations */
void init();
void display();


volatile int timer_calls = 0;
volatile short *timer2_ctrl = (short*)0x1F801124;
void timer_func()
{
	timer_calls++;
}

volatile int vsync_count = 0;
volatile int tick_count = 0;
volatile int tick_value = 0;

void vsync_func()
{
	vsync_count++;
	if( vsync_count > 60 )
	{
		tick_value = timer_calls-tick_count;
		tick_count = timer_calls;
		vsync_count = 0;
	}
}

/* Main function */
int main() {
	
	int counter;
	
	/* Init graphics and GTE */
	init();
	
	
	EnterCriticalSection();
	//SetRCnt(RCntCNT2, 0xF040, RCntMdINTR);
	
	// NTSC clock base
	counter = 4304000/560;
	
	// PAL clock base
	//counter = 5163000/560;
	
	SetRCnt(RCntCNT2, counter, RCntMdINTR);
	*timer2_ctrl = 0x1E58;
	InterruptCallback(6, timer_func);
	StartRCnt(RCntCNT2);
	ChangeClearRCnt(2, 0);
	ExitCriticalSection();
	
	VSyncCallback(vsync_func);
	
	/* Main loop */
	while( 1 ) {
		
		FntPrint(-1, "TIMER COUNT=%d\n", timer_calls);
		FntPrint(-1, "TICKS/SEC=%d\n", tick_value);
		
		/* Swap buffers and draw text */
		display();
		
	}
	
	return 0;
	
}

void init() {

	/* Reset the GPU, also installs a VSync event handler */
	ResetGraph( 0 );
	//SetVideoMode(MODE_PAL);
	
	/* Set display and draw environment areas */
	/* (display and draw areas must be separate, otherwise hello flicker) */
	SetDefDispEnv( &db[0].disp, 0, 0, SCREEN_XRES, SCREEN_YRES );
	SetDefDrawEnv( &db[0].draw, SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES );
	
	/* Enable draw area clear and dither processing */
	setRGB0( &db[0].draw, 63, 0, 127 );
	db[0].draw.isbg = 1;
	db[0].draw.dtd = 1;
	
	
	/* Define the second set of display/draw environments */
	SetDefDispEnv( &db[1].disp, SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES );
	SetDefDrawEnv( &db[1].draw, 0, 0, SCREEN_XRES, SCREEN_YRES );
	
	//db[0].disp.screen.y = 24;
	//db[1].disp.screen.y = 24;
	
	setRGB0( &db[1].draw, 63, 0, 127 );
	db[1].draw.isbg = 1;
	db[1].draw.dtd = 1;
	
	
	/* Apply the drawing environment of the first double buffer */
	PutDrawEnv( &db[0].draw );
	
	FntLoad(960, 0);
	FntOpen(0, 8, 320, 216, 0, 100);
	
}

void display() {
	
	FntFlush(-1);
	
	/* Wait for GPU to finish drawing and vertical retrace */
	DrawSync( 0 );
	VSync( 0 );
	
	/* Swap buffers */
	db_active ^= 1;
	
	/* Apply display/drawing environments */
	PutDrawEnv( &db[db_active].draw );
	PutDispEnv( &db[db_active].disp );
	
	/* Enable display */
	SetDispMask( 1 );
	
}