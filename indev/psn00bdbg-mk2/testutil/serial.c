#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#else
#include <sys/file.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif /* _WIN32 */

#ifdef _WIN32
HANDLE  hComm;
#else
int		hComm;
#endif /* _WIN32 */

int commOpen( const char *port, int baud )
{

#ifdef _WIN32

    DCB             dcbParams;
    COMMTIMEOUTS    timeouts;
    
    hComm = CreateFile( port,                   /* open serial port */
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL );
    
    if( hComm == INVALID_HANDLE_VALUE )
    {
        printf( "Cannot open port %s.\n", port );
        return( 1 );
    }

    if( !GetCommState( hComm, &dcbParams ) )    /* get DCB parameters */
    {
        CloseHandle( hComm );
        printf( "Cannot get comm state.\n" );
        return( 2 );
    }

    dcbParams.BaudRate          = baud;     /* adjust DCB parameters */
    dcbParams.ByteSize          = 8;
    dcbParams.StopBits          = ONESTOPBIT;
    dcbParams.Parity            = NOPARITY;
    dcbParams.fOutxCtsFlow      = TRUE;
    dcbParams.fOutxDsrFlow      = TRUE;
    dcbParams.fDtrControl       = DTR_CONTROL_ENABLE;
    dcbParams.fDsrSensitivity   = FALSE;
    dcbParams.fOutX             = FALSE;
    dcbParams.fInX              = FALSE;
    dcbParams.fErrorChar        = FALSE;
    dcbParams.fNull             = FALSE;
    dcbParams.fRtsControl       = RTS_CONTROL_ENABLE;//RTS_CONTROL_HANDSHAKE;
    dcbParams.fAbortOnError     = FALSE;

    if( !SetCommState( hComm, &dcbParams ) )    /* apply DCB parameters */
    {
        CloseHandle( hComm );
        printf( "Cannot set desired comm state.\n" );
        return( 3 );
    }

    timeouts.ReadIntervalTimeout         = 100;
    timeouts.ReadTotalTimeoutConstant    = 50;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.WriteTotalTimeoutConstant   = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if( !SetCommTimeouts( hComm, &timeouts ) )
    {
        CloseHandle( hComm );
        printf( "Cannot set desired timeouts.\n" );
        return( 4 );
    }

    Sleep( 50 );    /* delay for the PS1 to receive the DSR signal */
	
#else

	struct termios tty;
	
	memset( &tty, 0, sizeof(struct termios) );

	hComm = open( port, O_RDWR|O_NOCTTY );
	
	if( hComm == -1 )
	{
		printf( "Cannot open serial device %s.\n", port );
		return( 1 );
	}
	
	cfsetspeed( &tty, (speed_t)baud );
	
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= (CS8 | CREAD | CLOCAL);
	//tty.c_cflag |= (CS8 | CREAD | CLOCAL | CRTSCTS);
	
	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_iflag |= IGNPAR;
	
	tty.c_oflag &= ~OPOST;
	
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 5;
	
	if ( tcsetattr( hComm, TCSANOW, &tty ) != 0 )
	{
		printf( "Cannot set desired comms parameters.\n" );
        return( 3 );
	}

#endif /* _WIN32 */

    return( 0 );
    
} /* commOpen */

int commWriteByte( int value )
{
#ifdef _WIN32

    DWORD nwritten;
    
    if( !WriteFile( hComm, (LPCVOID)&value, 1, &nwritten, NULL ) )
    {
        return( -1 );
    }

    if( nwritten == 0 )
    {
        return( -1 );
    }

#else
	
	if( write( hComm, &value, 1 ) < 1 )
	{
		return( -1 );
	}

#endif /* _WIN32 */

    return( 1 );
    
} /* commWriteByte */

int commWriteBytes( void *buff, int len )
{
#ifdef _WIN32

    DWORD nwritten;

    if( !WriteFile( hComm, (LPCVOID)buff, len, &nwritten, NULL ) )
    {
        return( -1 );
    }

    return( nwritten );
	
#else

	int nwritten;
	
	nwritten = write( hComm, buff, len );
	
	return( nwritten );

#endif /* _WIN32 */
    
} /* commWriteBytes */

int commReadByte( void )
{
    int recval;

#ifdef _WIN32

    DWORD nread;

    recval = 0;
    if( !ReadFile(hComm, (LPVOID)&recval, 1, &nread, NULL) )
    {
        return(-1);
    }

    if( nread == 0 )
    {
        return(-1);
    }
	
#else

	recval = 0;
	if( read(hComm, &recval, 1) < 1 )
		return(-1);

#endif /* _WIN32 */

    return( recval );
    
} /* commReadByte */

int commPendingBytes(void)
{
#ifdef _WIN32
	
	DWORD dwErrorFlags;
	COMSTAT ComStat;

	ClearCommError(hComm, &dwErrorFlags, &ComStat);

	return( (int)ComStat.cbInQue );
	
#else
	
	int bytes;
	ioctl(hComm, FIONREAD, &bytes);
	
	return( bytes );
	
#endif /* _WIN32 */
	
} /* commPendingBytes */

int commReadBytes( void *buff, int len )
{
#ifdef _WIN32

    DWORD nread;

    if( !ReadFile( hComm, (LPVOID)buff, len, &nread, NULL ) )
    {
        return( -1 );
    }

#else

	int nread;
	
	nread = read( hComm, buff, len );
	
#endif /* _WIN32 */

	return( nread );
    
} /* commReadBytes */

void commClose( void )
{
#ifdef _WIN32

    Sleep( 50 );
    
    CloseHandle( hComm );

#else

	close( hComm );

#endif /* _WIN32 */
    
} /* commClose */
