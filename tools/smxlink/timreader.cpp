#include <stdio.h>
#include <string.h>
#include "timreader.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

int GetTimCoords(const char* fileName, TIM_COORDS *coords) {

    FILE* fp = fopen(fileName, "rb");

	if (fp == NULL)
		return false;


    unsigned int id;

	fread(&id, 4, 1, fp);

	if (id != 0x00000010) {

		fclose(fp);
		return false;

	}

    fread(&coords->flag, 4, 1, fp);

	if (coords->flag.cf) {

        fread(&coords->clutdata, 12, 1, fp);
        fseek(fp, coords->clutdata.length-12, SEEK_CUR);

	} else {

		memset(&coords->clutdata, 0x00, 12);

	}

    fread(&coords->pixdata, 12, 1, fp);

	fclose(fp);

	return true;

}

unsigned short GetClut(int cx, int cy) {

	unsigned short clut = (cx/16)&0x3f;
	clut |= (cy&0x1ff)<<6;

	return clut;

}

unsigned short GetTPage(int tp, int abr, int x, int y) {

	unsigned short tpage = (x/64)&0xf;	// Set X
	tpage |= ((y/256)&0x1)<<4;			// Set Y

	tpage |= (abr&0x3)<<5;				// Set blend mode
    tpage |= (tp&0x3)<<7;				// Set page mode
	tpage |= 1<<9;						// Set dither processing bit

	return tpage;

}
