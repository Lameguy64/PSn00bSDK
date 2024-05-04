#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif /* _WIN32 */
#include "cmdefs.h"
#include "main.h"
#include "mips_disassembler.h"

#ifdef _WIN32                           /* platform specific macros */

#define SIO_DELAY()   ( Sleep( 10 ) )

#else

#define SIO_DELAY()   ( usleep( 10000 ) )

#endif /* _WIN32 */

static const char *msg_sendfail    = "Failed to send command.\n";
static const char *msg_recfail     = "Error on receive.\n";
static const char *msg_noack       = "No acknowledgement from target.\n";

static char *regnames_param[] =
{
	"r0", "at", "v0", "v1",
	"a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3",
	"t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3",
	"s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1",
	"gp", "sp", "fp", "ra",
	"pc", "hi", "lo", "sr"
};

int dbGetInfo( void )
{
    int i;

    if( commWriteByte( CMD_DB_GETINFO ) > 0 )
    {
        printf( "Debug monitor info string: " );
        while( i = commReadByte() )
        {
            if( i < 0 )
            {
                puts( msg_recfail );
                return( 1 );
            }
            putchar( i );
        }
        putchar( '\n' );
    }
    else
    {
        puts( msg_sendfail );
        return( 1 );
    }

    return( 0 );
    
} /* dbGetInfo */

void dbGetStatus( void )
{
    int i;
    
    if( commWriteByte( CMD_DB_GETSTAT ) > 0 )
    {
        if( (i = commReadByte()) < 0 )
        {
            puts( msg_recfail );
        }
        else
        {
            printf( "Target status: %d (", i );
            
            switch( i )
            {
            case DB_STAT_STOP:
                printf( "stopped" );
                break;
            case DB_STAT_BREAK:
                printf( "breakpoint" );
                break;
            case DB_STAT_EXCEPT:
                printf( "unhandled exception" );
                break;
            case DB_STAT_RUN:
                printf( "running" );
                break;
            default:
                printf( "undefined" );
            }
            printf( ")\n" );
        }
    }
    else
    {
        puts( msg_sendfail );
    }

} /* dbGetStatus */

int dbGetStatusPoll(void)
{
    int i;
    
    if( commWriteByte( CMD_DB_GETSTAT ) > 0 )
    {
        if( (i = commReadByte()) < 0 )
        {
            return(-1);
        }
    }
    else
    {
        return(-1);
    }
	
	return(i);

} /* dbGetStatus */

int dbSetExec( int exec )
{
    if( commWriteByte( CMD_DB_SETEXEC ) < 0 )   /* send command */
    {
        puts( msg_sendfail );
        return(1);
    }

    if( commWriteByte( exec) < 0 )  /* send exec value */
    {
        puts( msg_sendfail );
        return(1);
    }

    if( commReadByte() < 0 )        /* check for acknowledgment */
    {
        puts( msg_noack );
        return(1);
    }
	
	return(0);
   
} /* dbSetExec */

void dbRunTo(unsigned int addr, int flag)
{
	if( commWriteByte(CMD_DB_RUNTO) < 0 )	/* send command */
    {
        puts( msg_sendfail );
        return;
    }
	
	if( commWriteByte(flag) < 0 )			/* send flag */
    {
        puts( msg_sendfail );
        return;
    }
	
	if( commWriteBytes( &addr, 4 ) < 4 )    /* send runto address */
    {
        puts( msg_sendfail );
        return;
    }
	
	if( commReadByte() < 0 )        /* check for acknowledgment */
    {
        puts( msg_noack );
        return;
    }
	
} /* dbRunTo */

void dbSetBreak(unsigned int addr, unsigned int flag, int count)
{
	int ret;
	
	if( commWriteByte(CMD_DB_SETBRK) < 0 )	/* send command */
    {
        puts(msg_sendfail);
        return;
    }
	
	if( commWriteBytes(&addr, 4) < 4 )		/* send breakpoint address */
    {
        puts(msg_sendfail);
        return;
    }
	
	SIO_DELAY();	/* delay otherwise last byte doesn't get sent somehow */
	
	ret = (flag&0xFF) | (count<<16);		/* send flag value */
	if( commWriteBytes(&ret, 4) < 4 )
    {
        puts(msg_sendfail);
        return;
    }
	
	SIO_DELAY();
	
	if( (ret = commReadByte()) < 0 )        /* check for acknowledgment */
    {
        puts( msg_noack );
        return;
    }
	
	if( ret == 0 )
	{
		printf("Bad breakpoint address.\n");
	}
	else if( ret == 1 )
	{
		printf("Breakpoint set/update.\n");
	}
	else
	{
		printf("Unknown response value: %d\n", ret);
	}

} /* dbSetBreak */

void dbClearBreaks()
{
	int ret;
	
	if( commWriteByte(CMD_DB_CLRBRK) < 0 )	/* send command */
    {
        puts(msg_sendfail);
        return;
    }
	
	if( (ret = commReadByte()) < 0 )        /* check for acknowledgment */
    {
        puts( msg_noack );
        return;
    }
	
} /* dbClearBreaks */

void dbReboot( void )
{
    if( commWriteByte( CMD_REBOOT ) < 0 )   /* send command */
    {
        puts( msg_sendfail );
        return;
    }
   
} /* dbReboot */

void dbUploadMemFile( unsigned int addr, const char *infile, size_t len )
{
    FILE *fp;
    int i,progress,olen;
    unsigned char buffer[100];
    size_t file_sz;

    fp = fopen( infile, "rb" );
    
    if( !fp )
    {
        printf( "File not found.\n" );
        return;
    }

    fseek( fp, 0, SEEK_END );                   /* determine file size */
    file_sz = ftell( fp );
    fseek( fp, 0, SEEK_SET );

    if( len > file_sz )
        len = file_sz;
    
    if( commWriteByte( CMD_DB_GETMEM ) < 0 )    /* send getmem command */
    {
        puts( msg_sendfail );
        fclose( fp );
        return;
    }

    if( commWriteByte( 1 ) < 1 )            /* send operation */
    {
        puts( msg_sendfail );
        fclose( fp );
        return;
    }

    SIO_DELAY();

    if( commWriteBytes( &addr, 4 ) < 4 )    /* send write address */
    {
        puts( msg_sendfail );
        fclose( fp );
        return;
    }

    SIO_DELAY();

    if( commWriteBytes( &len, 4 ) < 4 )     /* send write length */
    {
        puts( msg_sendfail );
        fclose( fp );
        return;
    }

    putchar( ' ' );                         /* draw a progress bar */
    for( i=0; i<70; i++ )
    {
        putchar( '.' );
    }
    putchar( ']' );
    putchar( '\r' );
    putchar( '[' );
    fflush( stdout );
    
    progress = 0;
    olen = len;
    
    while( len > 0 )                        /* send the incoming bytes */
    {
        i = len;
        if( i > 10 )
            i = 10;

        i = fread( buffer, 1, i, fp );
        if( i <= 0 )
        {
            break;
        }
            
        if( ( i = commWriteBytes( buffer, i ) ) < 0 )
        {
            puts( msg_recfail );
            break;
        }
        len -= i;

        i = (70*((1024*((olen-len)>>2))/    /* draw out progress */
            (olen>>2)))/1024;
        if( i > progress )
        {
            progress = i;
            putchar( '#' );
            fflush( stdout );
        }
    }

    for( i=0; i<(70-progress); i++ )
        putchar( '#' );
    
    putchar( '\n' );

    fclose( fp );
    
} /* dbUploadMemFile */

int dbUploadMem( unsigned int addr, char *buff, size_t len )
{
    int i,progress,olen;
    
    if( commWriteByte( CMD_DB_GETMEM ) < 0 )    /* send getmem command */
    {
        puts( msg_sendfail );
        return(1);
    }

    if( commWriteByte( 1 ) < 1 )            /* send operation */
    {
        puts( msg_sendfail );
        return(1);
    }

    SIO_DELAY();

    if( commWriteBytes( &addr, 4 ) < 4 )    /* send write address */
    {
        puts( msg_sendfail );
        return(1);
    }

    SIO_DELAY();

    if( commWriteBytes( &len, 4 ) < 4 )     /* send write length */
    {
        puts( msg_sendfail );
        return(1);
    }

    putchar( ' ' );                         /* draw a progress bar */
    for( i=0; i<70; i++ )
    {
        putchar( '.' );
    }
    putchar( ']' );
    putchar( '\r' );
    putchar( '[' );
    fflush( stdout );
    
    progress = 0;
    olen = len;
    
    while( len > 0 )                        /* send the incoming bytes */
    {
        i = len;
        if( i > 10 )
            i = 10;
            
        if( ( i = commWriteBytes( buff, i ) ) < 0 )
        {
            puts( msg_recfail );
            break;
        }
        len -= i;
		buff += i;

        i = (70*((1024*((olen-len)>>2))/    /* draw out progress */
            (olen>>2)))/1024;
        if( i > progress )
        {
            progress = i;
            putchar( '#' );
            fflush( stdout );
        }
    }

    for( i=0; i<(70-progress); i++ )
        putchar( '#' );
    
    putchar( '\n' );
	
	return(0);
    
} /* dbUploadMem */

void dbGetMem( unsigned int addr, int len, const char *outfile )
{
    FILE *fp;
    int i,progress,olen;
    unsigned char buffer[100];

    fp = fopen( outfile, "wb" );
    
    if( !fp )
    {
        printf( "Cannot create output file.\n" );
        return;
    }
    
    if( commWriteByte( CMD_DB_GETMEM ) < 0 )    /* send getmem command */
    {
        puts( msg_sendfail );
        fclose( fp );
        return;
    }

    if( commWriteByte( 0 ) < 1 )                /* send operation */
    {
        puts( msg_sendfail );
        fclose( fp );
        return;
    }

    SIO_DELAY();

    if( commWriteBytes( &addr, 4 ) < 4 )    /* send read address */
    {
        puts( msg_sendfail );
        fclose( fp );
        return;
    }

    SIO_DELAY();

    if( commWriteBytes( &len, 4 ) < 4 )     /* send read length */
    {
        puts( msg_sendfail );
        fclose( fp );
        return;
    }

    putchar( ' ' );                         /* draw a progress bar */
    for( i=0; i<70; i++ )
    {
        putchar( '.' );
    }
    putchar( ']' );
    putchar( '\r' );
    putchar( '[' );
    fflush( stdout );
    
    progress = 0;
    olen = len;
    
    while( len > 0 )                        /* receive the incoming bytes */
    {
        i = len;
        if( i > 10 )
            i = 10;
            
        if( ( i = commReadBytes( buffer, i ) ) < 0 )
        {
            puts( msg_recfail );
            break;
        }

        fwrite( buffer, 1, i, fp );
        len -= i;

        i = (70*((1024*((olen-len)>>2))/    /* draw out progress */
            (olen>>2)))/1024;
        if( i > progress )
        {
            progress = i;
            putchar( '#' );
            fflush( stdout );
        }
    }
        
    for( i=0; i<(70-progress); i++ )
        putchar( '#' );
    
    putchar( '\n' );

    fclose( fp );
    
} /* dbGetMem */

unsigned int dbReadValue( int wordsz, unsigned int addr )
{
    unsigned int val;
    
    if( commWriteByte( CMD_DB_WORD ) < 0 )    /* send value command */
    {
        puts( msg_sendfail );
        return( 0 );
    }

    if( commWriteByte( 0 ) < 0 )            /* perform read operation */
    {
        puts( msg_sendfail );
        return( 0 );
    }
    
    if( commWriteByte( wordsz ) < 0 )               /* send word size */
    {
        puts( msg_sendfail );
        return( 0 );
    }

    if( commWriteBytes( &addr, 4 ) < 4 )              /* send address */
    {
        puts( msg_sendfail );
        return( 0 );
    }

    if( commReadBytes( &val, 4 ) < 4 )              /* retrieve value */
    {
        puts( msg_recfail );
        return( 0 );
    }

    return( val );
    
} /* dbReadValue */

void dbWriteValue( int wordsz, unsigned int addr, unsigned int val )
{
    if( commWriteByte( CMD_DB_WORD ) < 0 )    /* send value command */
    {
        puts( msg_sendfail );
        return;
    }

    if( commWriteByte( 1 ) < 0 )            /* perform write operation */
    {
        puts( msg_sendfail );
        return;
    }

    if( commWriteByte( wordsz ) < 0 )               /* send word size */
    {
        puts( msg_sendfail );
        return;
    }

    if( commWriteBytes( &addr, 4 ) < 4 )              /* send address */
    {
        puts( msg_sendfail );
        return;
    }

    SIO_DELAY();

    if( commWriteBytes( &val, 4 ) < 4 )                 /* send value */
    {
        puts( msg_sendfail );
        return;
    }

    if( commReadByte() < 0 )
    {
        puts( msg_noack );
    }
    
} /* dbWriteValue */

static const char *regnames[] =
{
    "r0=","at=","v0=","v1=",
    "a0=","a1=","a2=","a3=",
    "t0=","t1=","t2=","t3=",
    "t4=","t5=","t6=","t7=",
    "s0=","s1=","s2=","s3=",
    "s4=","s5=","s6=","s7=",
    "t8=","t9=","k0=","k1=",
    "gp=","sp=","fp=","ra=",
    "pc=","hi=","lo="
};

static const char *cop0names[] =
{
    "SR        =",
    "CAUSE     =",
    "BADVADDR  =",
    "JUMPDEST  =",
    "DCIC      =",
    "OPCODE[0] =",
	"OPCODE[1] ="
};

void dbGetRegs( void )
{
    int i;
    unsigned int regval;
	unsigned int pc;
	char textbuff[32];
    
    if( commWriteByte( CMD_DB_GETREGS ) < 0 )    /* send getmem command */
    {
        puts( msg_sendfail );
        return;
    }

    for( i=0; i<35; i++ )
    {
        if( commReadBytes( &regval, 4 ) < 4 )
        {
            puts( msg_recfail );
            return;
        }
        
        printf( "%s%08X  ", regnames[i], regval );
        if( (i&0x3) == 3 )
            printf( "\n" );
			
		if( i == 32 )
			pc = regval;
    }

    printf( "\n\n" );
    
    for( i=0; i<7; i++ )
    {
        if( commReadBytes( &regval, 4 ) < 4 )
        {
            puts( msg_recfail );
            return;
        }
        printf( "%s%08X", cop0names[i], regval );
		if( i < 5 )
			putchar( '\n' );
		if( i >= 5 )
		{
			mips_Decode( regval, pc+((i>5)?4:0), textbuff, 0 );
			printf( " %s\n", textbuff );
		}
    }

    printf( "\n" );
    
} /* dbGetRegs */

void dbGetBreaks(void)
{
    unsigned int regval;
    
    if( commWriteByte(CMD_DB_GETBRK) < 0 )    /* send get breaks command */
    {
        puts(msg_sendfail);
        return;
    }
	
	while(1)
	{
		if( commReadBytes(&regval, 4) < 4 )
		{
			puts(msg_recfail);
			return;
		}
		
		if( regval == 0xFFFFFFFF )
		{
			break;
		}
		
		printf("PC=%08X ", regval);
		
		if( commReadBytes(&regval, 4) < 4 )
		{
			puts(msg_recfail);
			return;
		}
		
		printf("FLAG=%08X ", regval);
		
		if( commReadBytes(&regval, 4) < 4 )
		{
			puts(msg_recfail);
			return;
		}
		
		printf("CNT=%08X\n", regval);
	}
	
} /* dbGetBreaks */

int dbSetReg(const char *regname, unsigned int value)
{
	int regnum;
	int found;
	
	found = 0;
	for(regnum=0; regnum<36; regnum++)
	{
		if( strcmp(regname, regnames_param[regnum]) == 0 )
		{
			found = 1;
			break;
		}
	}
	
	if( found == 0 )
	{
		printf("Invalid register name.\n");
		return(1);
	}
	
	if( commWriteByte(CMD_DB_SETREG) < 0 )	/* send set register command */
    {
        puts(msg_sendfail);
        return(1);
    }
	
	if( commWriteByte(regnum) < 0 )			/* send register byte */
    {
        puts(msg_sendfail);
        return(1);
    }
	
	if( commWriteBytes(&value, 4) < 4 )		/* send new register value */
	{
		puts(msg_sendfail);
        return(1);
	}
	
	SIO_DELAY();
	
	if( commReadByte() < 0 )
    {
        puts(msg_noack);
    }
	
	return(0);
	
} /* dbSetReg */

int dbSetBDregs(unsigned int addr, unsigned int mask, unsigned char flags)
{
	/* send set data break command */
	if( commWriteByte(CMD_DB_SETBDREG) < 0 )
    {
        puts(msg_sendfail);
        return 1;
    }
	
	if( commWriteBytes(&addr, 4) < 4 )
	{
		puts(msg_sendfail);
        return 1;
	}
	
	if( commWriteBytes(&mask, 4) < 4 )
	{
		puts(msg_sendfail);
        return 1;
	}
	
	if( commWriteByte(flags) < 0 )
    {
        puts(msg_sendfail);
        return 1;
    }
	
	SIO_DELAY();
	
	if( commReadByte() < 0 )
    {
        puts(msg_noack);
    }
	
	return 0;
	
} /* dbSetDBregs */
