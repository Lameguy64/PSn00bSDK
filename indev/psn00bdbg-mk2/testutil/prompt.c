#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32

#include <windows.h>
#include <conio.h>

#else

#include <sys/time.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

#endif /* _WIN32 */

#include "cmdefs.h"
#include "main.h"

static int prompt_quit;
int UploadExec(const char *exefile);

#ifdef _WIN32

#define _kbhit		kbhit

#else
	
struct termios orig_term;

void enable_raw_mode()
{
    struct termios term;
    tcgetattr(0, &orig_term);
	
	term = orig_term;
    term.c_lflag &= ~(ICANON | ECHO); // Disable echo as well
    tcsetattr(0, TCSANOW, &term);
}

void disable_raw_mode()
{
    tcsetattr(0, TCSANOW, &orig_term);
}

int _kbhit()
{    
	struct timeval tv = { 0L, 0L };
    
	fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
	
    return select(1, &fds, NULL, NULL, &tv);
	
} /* _kbhit */

void term_func(int signum)
{
	disable_raw_mode();
	exit( 0 );
	
} /* term_func */

#endif /* _WIN32 */

static void promptHelp( void )
{
    printf("?                         View help\n");
    printf("up <addr> <len> <file>    Upload memory from <file>\n");
    printf("down <addr> <len> <file>  Download memory to <file>\n");
    printf("rv <8|16|32> <addr>       Read a value from <addr>\n");
    printf("wv <8|16|32> <addr> <val> Write a value to <addr>\n");
	printf("sb <addr> <mode> [count]  Set program breakpoint at <addr>\n");
	printf("cb                        Clear all breakpoints\n");
	printf("lb                        List breakpoints\n");
	printf("sr <regname> <value>      Set register value\n");
    printf("stop                      Stop target\n");
    printf("cont                      Resume target\n");
    printf("stat                      Show target status\n");
    printf("r / regs                  Show registers\n");
    printf("t / trace                 Trace\n");
	printf("rt / runto <addr>         Run until <addr>\n");
	printf("tt / traceto <addr>       Run until <addr> or jump/branch\n");
    printf("reboot                    Reboot target and quit\n");
    printf("q / quit                  Quit\n\n");
    printf("Commands can be stacked as one keystroke like so:\n");
    printf("trace regs - Performs a trace operation then shows registers\n\n");
    
} /* promptHelp */

static char *getarg( const char *msg )
{
    char *arg;
    
    arg = strtok( NULL, " " );
    
    if( !arg )
    {
        printf( "Missing %s argument.\n", msg );
        return( NULL );
    }

    return( arg );
    
} /* getarg */

void promptMode( void )
{
	int		inkey;
	int		bytebuff;
    char    inbuff[100];
	int		inpos;
    int     inlen;
    char    *inptr;
    char    *arg;

    unsigned int addr;
    int len;
    const char *file;

    int wordsz;
    unsigned int value;
	
	int firsttime;
	int pollcount;
	int laststat;
	int pollstat;
	
#ifdef _WIN32
#else
	struct sigaction st;
#endif /* _WIN32 */
    
    printf( "Enter ? for list of directives.\n\n" );

    prompt_quit = 0;
	
#ifdef _WIN32
#else
	st.sa_handler = term_func;
	sigemptyset( &st.sa_mask );
	st.sa_flags = SA_RESTART;
	sigaction( SIGINT, &st, NULL );
	
	enable_raw_mode();
#endif /* _WIN32 */

	if( (laststat = dbGetStatusPoll()) < 0 )
	{
		printf("Error polling target status.\n");
		laststat = 0;
	}
    
    while( !prompt_quit )
    {
        memset( inbuff, 0, 100 );
#if 1//def _WIN32
        //gets( inbuff );
#else
        fgets( inbuff, 100, stdin );
        
        while( arg = strchr( inbuff, '\n' ) )  /* remove newlines from input */
        {
            *arg = 0;
        }
#endif

		printf("nDB>");
		inpos = 0;
		fflush(stdout);
		firsttime = 1;
		pollcount = 0;
		while(1)
		{
			if( commPendingBytes() > 0 )
			{
				/* routine that simply flushes the serial interface */
				bytebuff = commReadByte();
				printf("\rReceived byte: %02x\n", bytebuff);
				
				printf("nDB>%s", inbuff);
				fflush(stdout);
				pollcount = 0;
			}
			
			if( _kbhit() )
			{
#ifdef _WIN32
				inkey = getch();
				if( inkey == 13 )
#else
				inkey = getchar();
				if( inkey == '\n' )
#endif
				{
					fputc('\n', stdout);
					fflush(stdout);
					break;
				}
#ifdef _WIN32
				else if( inkey == 8 )
#else
				else if( inkey == 127 )	/* linux backspace */
#endif /* _WIN32 */
				{
					if( inpos > 0 )
					{
						fputc('\b', stdout);
						fputc(' ', stdout);
						fputc('\b', stdout);
						fflush(stdout);
						inpos--;
						inbuff[inpos] = 0;
						continue;
					}
				}
				else
				{
					if( inpos < 100 )
					{
						fputc(inkey, stdout);
						fflush(stdout);
						inbuff[inpos] = inkey;
						inpos++;
						inbuff[inpos] = 0;
						continue;
					}
				}
			}
#ifdef _WIN32
			Sleep(10);
#else
			usleep(1000);
#endif /* _WIN32 */

			pollcount++;
			if( pollcount > 10 )
			{
				if( (pollstat = dbGetStatusPoll()) < 0 )
				{
					printf("Error polling target status.\n");
				}
				else
				{
					if( pollstat != laststat )
					{
						laststat = pollstat;
						switch(pollstat)
						{
						  case DB_STAT_STOP:
						    printf("\rTarget stopped.\n");
						    break;
						  case DB_STAT_BREAK:
							printf("\rBreakpoint/trace complete.\n");
							break;
						  case DB_STAT_EXCEPT:
							printf("\rUnhandled exception.\n");
							break;
						  default:
							printf("\rUndefined target state (%d).\n", pollstat);
						}
						dbGetRegs();
						printf("nDB>%s", inbuff);
						fflush(stdout);
					}
				}
				pollcount = 0;
			}
		}

        if( ( inlen = strlen( inbuff ) ) == 0 )
            continue;

        inptr = inbuff;
        while( arg = strtok( inptr, " " ) )
        {
            inptr = NULL;
            
            /* quit parsing when end of string is reached */
            
            if( strcasecmp( "?", arg ) == 0 )
            {
                promptHelp();
            }
            else if( strcasecmp( "rv", arg ) == 0 ) /* read value */
            {
                if( !(arg = getarg( "word size" ) ) ) /* get word size arg */
                    break;
                sscanf( arg, "%d", &wordsz );

                if( !(arg = getarg( "address" ) ) )  /* get address arg */
                    break;
                sscanf( arg, "%x", &addr );

                switch( wordsz )
                {
                case 8:
                    wordsz = 0;
                    break;
                case 16:
                    wordsz = 1;
                    break;
                case 32:
                    wordsz = 2;
                    break;
                default:
                    printf( "Unknown word size: %d\n", wordsz );
                    wordsz = -1;
                }

                if( wordsz >= 0 )
                {
                    value = dbReadValue( wordsz, addr );
                    switch( wordsz )
                    {
                    case 0:
                        printf( "%08X=%02X\n", addr, value );
                        break;
                    case 1:
                        printf( "%08X=%04X\n", addr, value );
                        break;
                    case 2:
                        printf( "%08X=%08X\n", addr, value );
                        break;
                    }
                }
            }
            else if( strcasecmp( "wv", arg ) == 0 ) /* read value */
            {
                if( !(arg = getarg( "word size" ) ) ) /* get word size arg */
                    break;
                sscanf( arg, "%d", &wordsz );

                if( !(arg = getarg( "address" ) ) )  /* get address arg */
                    break;
                sscanf( arg, "%x", &addr );

                if( !(arg = getarg( "value" ) ) )  /* get value */
                    break;
                sscanf( arg, "%x", &value );

                switch( wordsz )
                {
                case 8:
                    wordsz = 0;
                    break;
                case 16:
                    wordsz = 1;
                    break;
                case 32:
                    wordsz = 2;
                    break;
                default:
                    printf( "Unknown word size: %d\n", wordsz );
                    wordsz = -1;
                }

                dbWriteValue( wordsz, addr, value );
            }
			else if( strcasecmp( "exe", arg ) == 0 ) /* upload executable */
            {
                if( !(file = getarg( "file name" ) ) )  /* get file arg */
                    break;
                    
                UploadExec(file);
				
				laststat = DB_STAT_STOP;
            }
            else if( strcasecmp( "up", arg ) == 0 ) /* upload memory */
            {
                if( !(arg = getarg( "address" ) ) ) /* get address arg */
                    break;
                sscanf( arg, "%x", &addr );
                
                if( !(arg = getarg( "length" ) ) )  /* get length arg */
                    break;
                sscanf( arg, "%d", &len );

                if( !(file = getarg( "file name" ) ) )  /* get file arg */
                    break;

                printf( "Performing memory upload of %d byte(s) to %08X from %s\n",
                    len, addr, file );
                    
                dbUploadMemFile( addr, file, len );
            }
            else if( strcasecmp( "down", arg ) == 0 ) /* download memory */
            {
                if( !(arg = getarg( "address" ) ) ) /* get address arg */
                    break;
                sscanf( arg, "%x", &addr );
                
                if( !(arg = getarg( "length" ) ) )  /* get length arg */
                    break;
                sscanf( arg, "%d", &len );

                if( !(file = getarg( "file name" ) ) )  /* get file arg */
                    break;

                printf( "Performing memory dump of %d byte(s) from %08X to %s\n",
                    len, addr, file );
                    
                dbGetMem( addr, len, file );
            }
			else if( strcasecmp("sb", arg) == 0 ) /* set breakpoint */
			{
				if( !(arg = getarg("address") ) ) /* get address arg */
                    break;
                sscanf(arg, "%x", &addr);
				if( !(arg = getarg("flag") ) )	/* get flag arg */
                    break;
                sscanf(arg, "%d", &value);
				if( value == 2 )
				{
					if( !(arg = getarg("count") ) )	/* get flag arg */
						break;
					sscanf(arg, "%d", &len);
				}
				else
				{
					len = 0;
				}
				dbSetBreak(addr, value, len);
			}
			else if( strcasecmp("cb", arg) == 0 ) /* clear breakpoints */
			{
				dbClearBreaks();
			}
			else if( strcasecmp("lb", arg) == 0 )
			{
				dbGetBreaks();
			}
            else if( strcasecmp( "stat", arg ) == 0 ) /* get target status */
            {
                dbGetStatus();
            }
            else if( strcasecmp( "stop", arg ) == 0 ) /* stop target */
            {
                dbSetExec( CMD_EXEC_STOP );
            }
            else if( strcasecmp( "cont", arg ) == 0 ) /* continue target */
            {
                dbSetExec( CMD_EXEC_RESUME );
				laststat = DB_STAT_RUN;
            }
            else if( ( strcasecmp( "t", arg ) == 0 ) ||
                ( strcasecmp( "trace", arg ) == 0 ) )
            {
                dbSetExec( CMD_EXEC_STEP );
				laststat = DB_STAT_RUN;
            }
			else if( ( strcasecmp( "rt", arg ) == 0 ) || /* runto address */
                ( strcasecmp( "runto", arg ) == 0 ) )
            {
                if( !(arg = getarg( "program address" ) ) ) /* get address arg */
                    break;
                sscanf( arg, "%x", &addr );
				dbRunTo(addr, 0);
				laststat = DB_STAT_RUN;
            }
			else if( ( strcasecmp( "tt", arg ) == 0 ) || /* traceto address */
                ( strcasecmp( "traceto", arg ) == 0 ) )
            {
                if( !(arg = getarg( "program address" ) ) ) /* get address arg */
                    break;
                sscanf( arg, "%x", &addr );
				dbRunTo(addr, 1);
				laststat = DB_STAT_RUN;
            }
            else if( ( strcasecmp( "r", arg ) == 0 ) || /* get registers */
                ( strcasecmp( "regs", arg ) == 0 ) )
            {
                dbGetRegs();
            }
			else if( strcasecmp("sr", arg) == 0 )	/* set register value */
			{
				if( !(arg = getarg("register") ) ) /* get address arg */
                    break;
                file = arg;
				if( !(arg = getarg("value") ) )	/* get flag arg */
                    break;
                sscanf(arg, "%x", &value);
				dbSetReg(file, value);
			}
            else if( strcasecmp( "reboot", arg ) == 0 )
            {
                dbReboot();
                prompt_quit = 1;
                break;
            }
            else if( ( strcasecmp( "q", arg ) == 0 ) || /* quit program */
                ( strcasecmp( "quit", arg ) == 0 ) )
            {
                prompt_quit = 1;
                break;
            }
            else
            {
                printf( "Unknown directive: %s\n", arg );
            }
			/* delay before executing next directive */
#ifdef _WIN32
			Sleep(100);
#else
			usleep(1000);
#endif /* _WIN32 */
        }
    }

#ifdef _WIN32
#else	
	disable_raw_mode();
#endif /* _WIN32 */
    
} /* promptMode */
