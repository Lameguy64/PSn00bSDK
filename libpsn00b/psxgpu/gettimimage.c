#include <sys/types.h>
#include <psxgpu.h>

int GetTimInfo(const u_long *tim, TIM_IMAGE *timimg) {

	u_long *rtim;

	// Check ID
	if( ( tim[0]&0xff ) != 0x10 ) {
		return 1;
	}

	// Check version
	if( ( (tim[0]>>8)&0xff ) != 0x0 ) {
		return 2;
	}

	timimg->mode = tim[1];
	rtim = tim+2;

	// Clut present?
	if( timimg->mode & 0x8 ) {

		timimg->crect = (RECT*)(rtim+1);
		timimg->caddr = (u_long*)(rtim+3);

		rtim += rtim[0]>>2;

	} else {
	
		timimg->caddr = 0;
		
	}

	timimg->prect = (RECT*)(rtim+1);
	timimg->paddr = (u_long*)(rtim+3);

	return 0;

}
