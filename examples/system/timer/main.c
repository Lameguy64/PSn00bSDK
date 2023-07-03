/*
 * PSn00bSDK hardware timer example
 * (C) 2023 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <psxgpu.h>
#include <psxetc.h>
#include <psxapi.h>
#include <hwregs_c.h>

/* Display/GPU context utilities */

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define BGCOLOR_R 48
#define BGCOLOR_G 24
#define BGCOLOR_B  0

typedef struct {
	DISPENV disp;
	DRAWENV draw;
} Framebuffer;

typedef struct {
	Framebuffer db[2];
	int         db_active;
} RenderContext;

void init_context(RenderContext *ctx) {
	Framebuffer *db;

	ResetGraph(0);
	ctx->db_active = 0;

	db = &(ctx->db[0]);
	SetDefDispEnv(&(db->disp),           0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), BGCOLOR_R, BGCOLOR_G, BGCOLOR_B);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	db = &(ctx->db[1]);
	SetDefDispEnv(&(db->disp), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw),           0, 0, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), BGCOLOR_R, BGCOLOR_G, BGCOLOR_B);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	PutDrawEnv(&(db->draw));
	//PutDispEnv(&(db->disp));

	// Create a text stream at the top of the screen.
	FntLoad(960, 0);
	FntOpen(8, 16, 304, 208, 2, 512);
}

void display(RenderContext *ctx) {
	Framebuffer *db;

	DrawSync(0);
	VSync(0);
	ctx->db_active ^= 1;

	db = &(ctx->db[ctx->db_active]);
	PutDrawEnv(&(db->draw));
	PutDispEnv(&(db->disp));
	SetDispMask(1);
}

/* Interrupt handlers */

typedef struct {
	int irq_count, last_irq_count, irqs_per_sec;
} TimerState;

static volatile TimerState timer_state[3];

static void timer0_handler(void) {
	timer_state[0].irq_count++;
}

static void timer1_handler(void) {
	timer_state[1].irq_count++;
}

static void timer2_handler(void) {
	timer_state[2].irq_count++;
}

static void vblank_handler(void) {
	int refresh_rate = (GetVideoMode() == MODE_PAL) ? 50 : 60;

	// Only update once per second (every 50 or 60 vblanks).
	if (VSync(-1) % refresh_rate)
		return;

	for (int i = 0; i < 3; i++) {
		TimerState *state = &timer_state[i];

		int count             = state->irq_count;
		state->irqs_per_sec   = count - state->last_irq_count;
		state->last_irq_count = count;
	}
}

/* Main */

static RenderContext ctx;

int main(int argc, const char* argv[]) {
	init_context(&ctx);

	// Set up the timers and register callbacks for their interrupts.
	TIMER_CTRL(0) = 0x0160; // Dotclock input, repeated IRQ on overflow
	TIMER_CTRL(1) = 0x0160; // Hblank input, repeated IRQ on overflow
	TIMER_CTRL(2) = 0x0260; // CLK/8 input, repeated IRQ on overflow

	__builtin_memset(timer_state, 0, sizeof(timer_state));

	EnterCriticalSection();
	InterruptCallback(IRQ_TIMER0, &timer0_handler);
	InterruptCallback(IRQ_TIMER1, &timer1_handler);
	InterruptCallback(IRQ_TIMER2, &timer2_handler);
	VSyncCallback(&vblank_handler);
	ExitCriticalSection();

	while (1) {
		FntPrint(-1, "HARDWARE TIMER EXAMPLE\n\n");

		for (int i = 0; i < 3; i++) {
			TimerState *state = &timer_state[i];

			FntPrint(-1, "TIMER %d:\n", i);
			FntPrint(-1, " VALUE:  %d\n", TIMER_VALUE(i));
			FntPrint(-1, " IRQS:   %d\n", state->irq_count);
			FntPrint(-1, " IRQS/S: %d\n\n", state->irqs_per_sec);
		}

		FntPrint(-1, "VBLANK COUNTER:\n");
		FntPrint(-1, " VALUE:  %d\n\n", VSync(-1));

		FntFlush(-1);
		display(&ctx);
	}

	return 0;
}
