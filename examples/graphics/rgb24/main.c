/* LibPSn00b Example Programs
 * Part of the PSn00bSDK Project
 *
 * RGB24 Example by Lameguy64
 *
 *
 * This example demonstrates the 24-bit color mode of the PS1. This mode is
 * not practical for gameplay as the GPU can only draw graphics primitives
 * in 16-bit color depth so this feature would normally be used only for
 * fullscreen graphic illustrations or FMV sequences.
 *
 *
 * Changelog:
 *
 *	May 10, 2021		- Variable types updated for psxgpu.h changes.
 *
 *	May 3, 2019			- Initial version.
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <psxgte.h>
#include <psxgpu.h>

// So data from tim.s can be accessed
extern u_long tim_image[];

int main() {

	DISPENV		disp;
	TIM_IMAGE	tim;
	
	// Reset GPU
	ResetGraph(0);
	
	// Setup 640x480 24-bit video mode
	SetDefDispEnv(&disp, 0, 0, 640, 480);
	disp.isrgb24 = 1;
	disp.isinter = 1;
	
	// Apply and enable display
	PutDispEnv(&disp);
	SetDispMask(1);
	
	// Upload image to VRAM
	GetTimInfo(tim_image, &tim);
	LoadImage(tim.prect, tim.paddr);
	DrawSync(0);
	
	while(1) {
	}
	
	return 0;
}