/* 
 * LibPSn00b Example Programs
 *
 * Hello World Example
 * 2019-2020 Meido-Tek Productions / PSn00bSDK Project
 *
 * The obligatory hello world example normally included in nearly every
 * SDK package. This example should also get you started in how to manage
 * the display using psxgpu.
 *
 * Example by Lameguy64
 *
 *
 * Changelog:
 *
 *	  January 1, 2020 - Initial version
 */
 
#include <stdio.h>
#include <sys/types.h>
#include <psxetc.h>
#include <psxgte.h>
#include <psxgpu.h>


// Define display/draw environments for double buffering
DISPENV disp[2];
DRAWENV draw[2];
int db;


// Init function
void init(void)
{
	// This not only resets the GPU but it also installs the library's
	// ISR subsystem to the kernel
	ResetGraph(0);
	
	// Define display environments, first on top and second on bottom
	SetDefDispEnv(&disp[0], 0, 0, 320, 240);
	SetDefDispEnv(&disp[1], 0, 240, 320, 240);
	
	// Define drawing environments, first on bottom and second on top
	SetDefDrawEnv(&draw[0], 0, 240, 320, 240);
	SetDefDrawEnv(&draw[1], 0, 0, 320, 240);
	
	// Set and enable clear color
	setRGB0(&draw[0], 0, 96, 0);
	setRGB0(&draw[1], 0, 96, 0);
	draw[0].isbg = 1;
	draw[1].isbg = 1;
	
	// Clear double buffer counter
	db = 0;
	
	// Apply the GPU environments
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);
	
	// Load test font
	FntLoad(960, 0);
	
	// Open up a test font text stream of 100 characters
	FntOpen(0, 8, 320, 224, 0, 100);
}

// Display function
void display(void)
{
	// Flip buffer index
	db = !db;
	
	// Wait for all drawing to complete
	DrawSync(0);
	
	// Wait for vertical sync to cap the logic to 60fps (or 50 in PAL mode)
	// and prevent screen tearing
	VSync(0);

	// Switch pages	
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);
	
	// Enable display output, ResetGraph() disables it by default
	SetDispMask(1);
	
}

// Main function, program entrypoint
int main(int argc, const char *argv[])
{
	int counter;

	// Init stuff	
	init();
	
	// Main loop
	counter = 0;
	while(1)
	{
	
		// Print the obligatory hello world and counter to show that the
		// program isn't locking up to the last created text stream
		FntPrint(-1, "HELLO WORLD\n");
		FntPrint(-1, "COUNTER=%d\n", counter);
		
		// Draw the last created text stream
		FntFlush(-1);
		
		// Update display
		display();
		
		// Increment the counter
		counter++;
	}
	
	return 0;
}
