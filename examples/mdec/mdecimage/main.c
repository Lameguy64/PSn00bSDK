/*
 * PSn00bSDK MDEC static image example
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * This is a modified version of the graphics/rgb24 example showing how to feed
 * run-length encoded data into the MDEC and retrieve a decoded 24bpp image. To
 * keep the example simple no additional compression is applied (usually MDEC
 * data would be Huffman encoded to save more space, with the initial
 * decompression being done in software). A Python script is included to encode
 * an image into the format expected by the MDEC; quality and file size can be
 * tweaked by changing the quantization scales with the -y and -c arguments.
 *
 * Using the MDEC to decode static images can be useful for e.g. menu
 * backgrounds or loading screens, where smaller file sizes are desirable even
 * if quality is sacrificed.
 */

#include <stdint.h>
#include <stddef.h>
#include <psxgpu.h>
#include <psxpress.h>
#include <hwregs_c.h>

extern const uint32_t	mdec_image[];
extern const size_t		mdec_image_size;

#define SCREEN_XRES 640
#define SCREEN_YRES 480

//#define BLOCK_SIZE 8	// Monochrome (8x8), 15bpp display
//#define BLOCK_SIZE 12	// Monochrome (8x8), 24bpp display
//#define BLOCK_SIZE 16	// Color (16x16), 15bpp display
#define BLOCK_SIZE 24	// Color (16x16), 24bpp display

int main(int argc, const char* argv[]) {
	DISPENV disp;

	ResetGraph(0);
	DecDCTReset(0);

	// Set up the GPU for 640x480 interlaced 24bpp output.
	SetDefDispEnv(&disp, 0, 0, SCREEN_XRES, SCREEN_YRES);
	disp.isrgb24 = 1;
	disp.isinter = 1;

	PutDispEnv(&disp);
	SetDispMask(1);

	// Start feeding image data to the MDEC. This doesn't immediately start the
	// decoding, instead the MDEC will wait until a destination buffer is also
	// set up.
	MDEC0 = 0x30000000 | (mdec_image_size / 4); // 0x38000000 for 15bpp
	DecDCTinRaw(mdec_image, mdec_image_size / 4);

	// Fetch decoded data from the MDEC in vertical 8x480 or 16x480 "slices".
	// This is necessary as the MDEC doesn't buffer an entire frame but only
	// returns a series of square macroblocks, which can't be placed into VRAM
	// with a single LoadImage() call.
	//for (uint32_t x = 0; x < SCREEN_XRES; x += BLOCK_SIZE) {			// 15bpp
	for (uint32_t x = 0; x < (SCREEN_XRES * 3 / 2); x += BLOCK_SIZE) {	// 24bpp
		RECT     rect;
		uint32_t slice[BLOCK_SIZE * SCREEN_YRES / 2];

		rect.x = x;
		rect.y = 0;
		rect.w = BLOCK_SIZE;
		rect.h = SCREEN_YRES;

		// Configure the MDEC to output to the slice buffer and let it finish
		// decoding a slice, then upload it to the framebuffer.
		DecDCTout(slice, BLOCK_SIZE * SCREEN_YRES / 2);
		DecDCToutSync(0);

		LoadImage(&rect, slice);
		DrawSync(0);
	}

	for (;;)
		__asm__ volatile("");

	return 0;
}
