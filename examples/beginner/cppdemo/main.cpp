/* Work in progress example, need to add comments.
 *
 * Basically a quick little example that showcases C++ classes are
 * functioning in PSn00bSDK. - Lameguy64
 *
 * First written in December 18, 2020.
 *
 * Changelog:
 *
 *  May 11, 2023 - Updated the example to use C++ standard library headers,
 *                 renamed the class and cleaned up some methods.
 *  May 10, 2021 - Variable types updated for psxgpu.h changes.
 *
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <psxgte.h>
#include <psxgpu.h>

/* Example class */

class RenderContext {
private:
	std::uint32_t *_ot[2];
	std::uint8_t  *_pri[2];
	std::uint8_t  *_nextpri;

	int _ot_count, _db;

	DISPENV _disp[2];
	DRAWENV _draw[2];

public:
	RenderContext(std::size_t ot_len = 8, std::size_t pri_len = 8192);
	~RenderContext(void);
	void setupBuffers(int w, int h, int r, int g, int b);
	void flip(void);

	template<typename T> inline T *addPrimitive(void) {
		auto pri = reinterpret_cast<T *>(_nextpri);
		addPrim(_ot[_db], pri);

		_nextpri += sizeof(T);
		return pri;
	}
};

RenderContext::RenderContext(std::size_t ot_len, std::size_t pri_len) {
	_ot[0] = new std::uint32_t[ot_len];
	_ot[1] = new std::uint32_t[ot_len];

	_db = 0;
	_ot_count = ot_len;
	ClearOTagR(_ot[0], _ot_count);
	ClearOTagR(_ot[1], _ot_count);

	_pri[0] = new std::uint8_t[pri_len];
	_pri[1] = new std::uint8_t[pri_len];

	_nextpri = _pri[0];

	std::printf("RenderContext::RenderContext: Buffers allocated.\n");
}

RenderContext::~RenderContext(void) {
	delete[] _ot[0];
	delete[] _ot[1];

	delete[] _pri[0];
	delete[] _pri[1];

	std::printf("RenderContext::RenderContext: Buffers freed.\n");
}

void RenderContext::setupBuffers(int w, int h, int r, int g, int b) {
	SetDefDispEnv(&_disp[0], 0, h, w, h);
	SetDefDispEnv(&_disp[1], 0, 0, w, h);
	SetDefDrawEnv(&_draw[0], 0, 0, w, h);
	SetDefDrawEnv(&_draw[1], 0, h, w, h);

	setRGB0(&_draw[0], r, g, b);
	_draw[0].isbg = 1;
	_draw[0].dtd  = 1;

	setRGB0(&_draw[1], r, g, b);
	_draw[1].isbg = 1;
	_draw[1].dtd  = 1;

	PutDispEnv(&_disp[0]);
	PutDrawEnv(&_draw[0]);
}

void RenderContext::flip(void) {
	DrawSync(0);
	VSync(0);

	_db ^= 1;

	PutDispEnv(&_disp[_db]);
	PutDrawEnv(&_draw[_db]);

	DrawOTag(_ot[_db ^ 1] + _ot_count - 1);
	ClearOTagR(_ot[_db], _ot_count);

	_nextpri = _pri[_db];
}

/* Main */

static constexpr int SCREEN_XRES = 320;
static constexpr int SCREEN_YRES = 240;

static constexpr int BGCOLOR_R =  63;
static constexpr int BGCOLOR_G =   0;
static constexpr int BGCOLOR_B = 127;

int main(int argc, const char **argv) {
	ResetGraph(0);
	SetDispMask(1);

	RenderContext ctx;
	ctx.setupBuffers(SCREEN_XRES, SCREEN_YRES, BGCOLOR_R, BGCOLOR_G, BGCOLOR_B);

	int x  = 0, y  = 0;
	int dx = 1, dy = 1;

	for (;;) {
		// Update the position and velocity of the bouncing square.
		if (x < 0 || x > (SCREEN_XRES - 64))
			dx = -dx;
		if (y < 0 || y > (SCREEN_YRES - 64))
			dy = -dy;

		x += dx;
		y += dy;

		// Draw the square.
		auto tile = ctx.addPrimitive<TILE>();
		setTile(tile);
		setXY0 (tile, x, y);
		setWH  (tile, 64, 64);
		setRGB0(tile, 255, 255, 0);

		ctx.flip();
	}

	return 0;
}
