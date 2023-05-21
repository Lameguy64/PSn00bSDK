/*
 * PSn00bSDK C++ basic graphics example
 * (C) 2020-2023 Lameguy64, spicyjpeg - MPL licensed
 *
 * A C++ variant of the beginner/hello example showcasing the use of classes and
 * templates in place of structures, making the code more readable and less
 * error-prone. The OT and primitive buffer are now allocated on the heap and
 * automatically freed when the RenderContext class is destroyed or goes out of
 * scope.
 *
 * See the original example for more details.
 */

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <psxgpu.h>

static constexpr size_t DEFAULT_OT_LENGTH     = 15;
static constexpr size_t DEFAULT_BUFFER_LENGTH = 8192;

/* RenderBuffer class */

class RenderBuffer {
private:
	DISPENV _disp_env;
	DRAWENV _draw_env;

	std::uint32_t *_ot;
	std::uint8_t  *_buffer;
	std::size_t   _ot_length, _buffer_length;

public:
	RenderBuffer(std::size_t ot_length, std::size_t buffer_length);
	~RenderBuffer(void);
	void setup(int x, int y, int w, int h, int r, int g, int b);

	inline uint8_t *buffer_start(void) const {
		return _buffer;
	}
	inline uint8_t *buffer_end(void) const {
		return &_buffer[_buffer_length];
	}
	inline uint32_t *ot_entry(int z) const {
		//assert((z >= 0) && (z < _ot_length));
		return &_ot[z];
	}

	inline void clear_ot(void) {
		ClearOTagR(_ot, _ot_length);
	}
	inline void draw(void) {
		DrawOTagEnv(&_ot[_ot_length - 1], &_draw_env);
	}
	inline void display(void) const {
		PutDispEnv(&_disp_env);
	}
};

RenderBuffer::RenderBuffer(std::size_t ot_length, std::size_t buffer_length)
: _ot_length(ot_length), _buffer_length(buffer_length) {
	// Initializing the OT in a constructor is unsafe, since ClearOTagR()
	// requires DMA to be enabled and may fail if called before ResetGraph() or
	// ResetCallback() (which can easily happen as constructors can run before
	// main()). Thus, this constructor is only going to allocate the buffers and
	// clearing is deferred to RenderContext::setup().
	_ot     = new uint32_t[ot_length];
	_buffer = new uint8_t[buffer_length];

	assert(_ot && _buffer);

	//std::printf("Allocated buffer, ot=0x%08x, buffer=0x%08x\n", ot, buffer);
}

RenderBuffer::~RenderBuffer(void) {
	delete[] _ot;
	delete[] _buffer;

	//std::printf("Freed buffer, ot=0x%08x, buffer=0x%08x\n", ot, buffer);
}

void RenderBuffer::setup(int x, int y, int w, int h, int r, int g, int b) {
	// Set the framebuffer's VRAM coordinates.
	SetDefDrawEnv(&_draw_env, x, y, w, h);
	SetDefDispEnv(&_disp_env, x, y, w, h);

	// Set the default background color and enable auto-clearing.
	setRGB0(&_draw_env, r, g, b);
	_draw_env.isbg = 1;
}

/* RenderContext class */

class RenderContext {
private:
	RenderBuffer _buffers[2];
	std::uint8_t *_next_packet;
	int          _active_buffer;

	// These functions are simply shorthands for _buffers[_active_buffer] and
	// _buffers[_active_buffer ^ 1] respectively. They are only used internally.
	inline RenderBuffer &_draw_buffer(void) {
		return _buffers[_active_buffer];
	}
	inline RenderBuffer &_disp_buffer(void) {
		return _buffers[_active_buffer ^ 1];
	}

public:
	RenderContext(
		std::size_t ot_length     = DEFAULT_OT_LENGTH,
		std::size_t buffer_length = DEFAULT_BUFFER_LENGTH
	);
	void setup(int w, int h, int r, int g, int b);
	void flip(void);

	// This is a "factory function" that allocates a new primitive within the
	// currently active buffer. It is a template method, meaning T will get
	// replaced at compile time by the type of the primitive we are going to
	// allocate (and sizeof(T) will change accordingly!).
	template<typename T> inline T *new_primitive(int z = 0) {
		// Place the primitive after all previously allocated primitives, then
		// insert it into the OT and bump the allocation pointer.
		auto prim = reinterpret_cast<T *>(_next_packet);

		addPrim(_draw_buffer().ot_entry(z), prim);
		_next_packet += sizeof(T);

		// Make sure we haven't yet run out of space for future primitives.
		assert(_next_packet <= _draw_buffer().buffer_end());

		return prim;
	}

	// A simple helper for drawing text using PSn00bSDK's debug font API. Note
	// that FntSort() requires the debug font texture to be uploaded to VRAM
	// beforehand by calling FntLoad().
	inline void draw_text(int x, int y, int z, const char *text) {
		_next_packet = reinterpret_cast<uint8_t *>(
			FntSort(_draw_buffer().ot_entry(z), _next_packet, x, y, text)
		);

		assert(_next_packet <= _draw_buffer().buffer_end());
	}
};

RenderContext::RenderContext(std::size_t ot_length, std::size_t buffer_length)
: _buffers{
		RenderBuffer(ot_length, buffer_length),
		RenderBuffer(ot_length, buffer_length)
} {}

void RenderContext::setup(int w, int h, int r, int g, int b) {
	// Place the two framebuffers vertically in VRAM.
	_buffers[0].setup(0, 0, w, h, r, g, b);
	_buffers[1].setup(0, h, w, h, r, g, b);

	// Initialize the first buffer and clear its OT so that it can be used for
	// drawing.
	_active_buffer = 0;
	_next_packet   = _draw_buffer().buffer_start();
	_draw_buffer().clear_ot();

	// Turn on the video output.
	SetDispMask(1);
}

void RenderContext::flip(void) {
	// Wait for the GPU to finish drawing, then wait for vblank in order to
	// prevent screen tearing.
	DrawSync(0);
	VSync(0);

	// Display the framebuffer the GPU has just finished drawing and start
	// rendering the display list that was filled up in the main loop.
	_disp_buffer().display();
	_draw_buffer().draw();

	// Switch over to the next buffer, clear it and reset the packet allocation
	// pointer.
	_active_buffer ^= 1;
	_next_packet    = _draw_buffer().buffer_start();
	_draw_buffer().clear_ot();
}

/* Main */

static constexpr int SCREEN_XRES = 320;
static constexpr int SCREEN_YRES = 240;

int main(int argc, const char **argv) {
	// Initialize the GPU and load the default font texture provided by
	// PSn00bSDK at (960, 0) in VRAM.
	ResetGraph(0);
	FntLoad(960, 0);

	// Set up our rendering context.
	RenderContext ctx;
	ctx.setup(SCREEN_XRES, SCREEN_YRES, 63, 0, 127);

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

		// Draw the square by allocating a TILE (i.e. untextured solid color
		// rectangle) primitive at Z = 1.
		auto tile = ctx.new_primitive<TILE>(1);

		setTile(tile);
		setXY0 (tile, x, y);
		setWH  (tile, 64, 64);
		setRGB0(tile, 255, 255, 0);

		// Draw some text in front of the square (Z = 0, primitives with higher
		// Z indices are drawn first).
		ctx.draw_text(8, 16, 0, "Hello from C++!");

		ctx.flip();
	}

	return 0;
}
