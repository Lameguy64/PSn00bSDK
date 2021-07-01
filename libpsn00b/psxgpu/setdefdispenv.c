#include <sys/types.h>
#include <psxgpu.h>

DISPENV *SetDefDispEnv(DISPENV *disp, int x, int y, int w, int h) {

	disp->disp.x = x;
	disp->disp.y = y;
	disp->disp.w = w;
	disp->disp.h = h;

	disp->screen.x = 0;
	disp->screen.y = 0;
	disp->screen.w = 0;
	disp->screen.h = 0;

	disp->isinter = 0;
	disp->isrgb24 = 0;
	disp->reverse = 0;

	return disp;

}
