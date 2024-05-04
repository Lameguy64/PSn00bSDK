#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#include <winbase.h>
#else
#include <unistd.h>
#endif /* _WIN32 */
#include "exec.h"
#include "cmdefs.h"
#include "main.h"

#ifdef _WIN32
const char *com_port = "COM1";
#define delay(t)	Sleep(t)
#else
const char *com_port = "/dev/ttyUSB0";
#define delay(t)	usleep(1000*t);
#endif
int com_baud = 115200;

static const char *msg_minargs     = "Not enough arguments.\n";

int UploadExec(const char *exefile)
{
	EXEC exec;
	char *exebuff;
	
	/* load the PS-EXE */
	printf("Loading executable file... ");
	fflush(stdout);
	exebuff = loadExecutable(exefile, &exec);
	if( !exebuff )
		return(1);
	printf("Done.\n");
	
	/* stop the target */
	printf("Target stop... ");
	fflush(stdout);
	if( dbSetExec(CMD_EXEC_STOP) )
		return(1);
	delay(100);
	printf("Done.\n");
	
	/* modify target's registers for new program execution */
	printf("Setup target (pc0=%08X)... ", exec.pc0);
	fflush(stdout);
	delay(100);
	if( dbSetReg("pc", exec.pc0) )
		return(1);
	delay(100);
	if( dbSetReg("sr", 0x00000400) )
		return(1);
	delay(100);
	if( dbSetReg("sp", 0x801FFFF0) )
		return(1);
	delay(100);
	printf("Done.\n");
	
	/* upload program text */
	printf("Program upload (t_addr=%08X)...\n", exec.t_addr);
	if( dbUploadMem(exec.t_addr, exebuff, exec.t_size) )
		return(1);
	
	printf("Target ready.\n");
	
	return(0);
	
} /* UploadExec */

int main( int argc, const char *argv[] )
{
    int i;
	const char *psexe;
    
    printf( "PSn00bDBG-mk2 Monitor Test Utility\n\n" );

	psexe = NULL;
    for( i=1; i<argc; i++ )
    {
        if( strcasecmp( "/p", argv[i] ) == 0 )      /* specify serial port */
        {
            if( ( argc-i ) < 2 )
            {
                puts( msg_minargs );
                return( 1 );
            }
            i++;
            com_port = argv[i];
        }
        else if( ( strcasecmp( "/?", argv[i] ) == 0 ) ||
            ( strcasecmp( "/h", argv[i] ) == 0 ) )
        {
            printf( "%s [params] [exe]\n\n", argv[0] );
            printf( "Parameters:\n" );
            printf( "   /p <COMx> - Specify serial port (default: COM1)\n" );
            printf( "   /? or /h  - Show help\n\n" );
            return( 0 );
        }
        else
        {
            psexe = argv[i];
        }
    }

    if( commOpen( com_port, com_baud ) )
        return( 1 );

    if( dbGetInfo() )
    {
        printf( "Unable to confirm connection with debug monitor.\n" );
        return( 1 );
    }
	
	/* perform PS-EXE upload */
	if( psexe )
	{
		UploadExec(psexe);
		promptMode();
	}
	else
	{
		promptMode();
	}

    commClose();
    
    return( 0 );
    
} /* main */
