#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxetc.h>

#define PAL

#ifdef PAL

#define SCREEN_XRES	320
#define SCREEN_YRES 256

#else

#define SCREEN_XRES	320
#define SCREEN_YRES 240

#endif

typedef struct DB
{
	DISPENV disp;
	DRAWENV draw;
} DB;

DB db[2];
int db_active;

void init(void)
{
	ResetGraph(0);
	
	SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDispEnv(&db[1].disp, 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);
	
	SetDefDrawEnv(&db[0].draw, 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&db[1].draw, 0, 0, SCREEN_XRES, SCREEN_YRES);
	
	db[0].draw.isbg = 1;
	setRGB0(&db[0].draw, 63, 0, 127);
	db[1].draw.isbg = 1;
	setRGB0(&db[1].draw, 63, 0, 127);
	
#ifdef PAL
	SetVideoMode(MODE_PAL);
	db[0].disp.screen.y = 18;
	db[1].disp.screen.y = 18;
	db[0].disp.screen.h = 256;
	db[1].disp.screen.h = 256;
#endif
	
	PutDispEnv(&db[0].disp);
	PutDrawEnv(&db[0].draw);
	db_active = 0;
	
	FntLoad(960, 0);
	FntOpen(0, 8, SCREEN_XRES, SCREEN_YRES-16, 0, 100);
}

void display(void)
{
	FntFlush(-1);
	DrawSync(0);
	VSync(0);
	
	db_active = !db_active;
	PutDispEnv(&db[db_active].disp);
	PutDrawEnv(&db[db_active].draw);
	SetDispMask(1);
}

int main(int argc, const char *argv[])
{
	int count = 0;
	
	init();
	
	while(1)
	{
		FntPrint(-1, "HELLO WORLD\n");
		FntPrint(-1, "COUNTER=%d\n", count);
		display();
		count++;
	}
}