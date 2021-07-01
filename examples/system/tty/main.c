/* 
 * LibPSn00b Example Programs
 *
 * Teletype Example
 * 2020 - 2021 Meido-Tek Productions / PSn00bSDK Project
 *
 * This example showcases the uses of tty through stdio facilities. If you've
 * written text console applications before, this one is not too dissimilar to
 * that. Escape codes for formatting and such should work as this is more 
 * dependant on the terminal program used than the PS1 console itself.
 *
 *
 * Example by Lameguy64
 *
 * Changelog:
 *
 *	May 10, 2021		- Variable types updated for psxgpu.h changes.
 *
 *  April 23, 2020		- Initial version.
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <psxgpu.h>

/* Memory viewer thing, you may use this in your own applications
 * for testing or analysis */
void memory_browser(unsigned int addr)
{
	int i,j,key;
	unsigned char *ptr,*pptr;
	
	while(1)
	{
		/* Set cursor position to top-left */
		printf("\033[1;1H");
		printf("MEMVIEW   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  0123456789ABCDEF");

		ptr = (unsigned char*)addr;
	
		/* Print contents from current location of memory */
		for(j=0; j<23; j++)
		{
			printf("\n%04X: ", (unsigned int)ptr);
			pptr = ptr;
			for(i=0; i<16; i++)
			{
				printf("%02X ", *ptr);
				ptr++;
			}
			printf(" ");
			for(i=0; i<16; i++)
			{
				if(( *pptr < 32 ) || ( *pptr > 127 ) )
				{
					printf(".");
				}
				else
				{
					printf("%c", *pptr);
				}
				pptr++;
			}
		}
	
		/* Parse input */
		while(1)
		{
			key = getchar();
			if( key == 0x1B )
			{
				key = getchar();
				
				if( key == 0x5B )
				{
					key = getchar();
					if( key == 0x41 )		// Up
					{
						addr -= 16;
						break;
					}
					else if( key == 0x42 )	// Down
					{
						addr += 16;
						break;
					}
					if( key == 0x35 )		// Page up
					{
						addr -= 16*23;
						break;
					}
					else if( key == 0x36 )	// Page down
					{
						addr += 16*23;
						break;
					}
				}
			}
			
		}
	}
	
}


int main(int argc, const char *argv[])
{
	int i;
	char strbuff[32];

	/* Mostly to get interrupts going for this example */
	ResetGraph( 0 );

	/* Uncomment if you don't have an environment that provides tty access 
	 * by default */
	//AddSIO(115200);
		
	/* A standby loop until 'Y' is entered */
	while(1)
	{
		/* Print banner */
		printf("Hello world!\n");
		printf("Press 'Y' to proceed with this demonstration.\n");
		
		/* Get input for a Y character */
		i = getchar();
		if( tolower(i) == 'y' )
			break;
	}
	
	/* Do a classic text input prompt and display the inputted text */
	printf("Enter a string, any string (no more than 32 characters):\n");
	gets(strbuff);
	
	printf("You've entered: %s\n\n", strbuff);
	
	/* Prompt entering into the memory browser */
	printf("Press a key to enter a memory browser demo...\n");
	printf("Make sure your terminal or text console supports vt100 escape codes!\n");
	getchar();
	
	/* Start the memory browser interface */
	memory_browser(0x80010000);
	
	return 0;
}
