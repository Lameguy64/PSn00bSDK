#ifndef _TIMREADER_H
#define _TIMREADER_H

typedef struct {
	struct {
		unsigned int pmode:3;		// Pixel mode (0: 4-bit, 1: 8-bit, 2: 16-bit, 3: 24-bit)
		unsigned int cf:1;			// CLUT flag (if 1, CLUT is present)
		unsigned int reserved:28;
	} flag;
	struct {
		unsigned int length;
		unsigned short px,py;
		unsigned short pw,ph;
	} clutdata;
	struct {
		unsigned int length;
		unsigned short px,py;
		unsigned short pw,ph;
	} pixdata;
} TIM_COORDS;

int GetTimCoords(const char *fileName, TIM_COORDS *coords);

unsigned short GetClut(int cx, int cy);

unsigned short GetTPage(int tp, int abr, int x, int y);

#endif // _TIMREADER_H
