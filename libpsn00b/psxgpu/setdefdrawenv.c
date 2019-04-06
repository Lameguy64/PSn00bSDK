#include <psxgpu.h>

DRAWENV *SetDefDrawEnv(DRAWENV *draw, int x, int y, int w, int h) {

	draw->clip.x = x;
	draw->clip.y = y;
	draw->clip.w = w;
	draw->clip.h = h;

	draw->ofs[0] = 0;
	draw->ofs[1] = 0;

	draw->tw.x = 0;
	draw->tw.y = 0;
	draw->tw.w = 0;
	draw->tw.h = 0;

	draw->tpage = 0x0a;
	draw->dtd = 1;
	draw->dfe = 0;
	draw->isbg = 0;
	setRGB0( draw, 0, 0, 0 );

	return draw;

}
