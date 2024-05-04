#ifndef _MAIN_H
#define _MAIN_H

/* globals */
extern const char *com_port;
extern int com_baud;

/* comms.c */
int dbGetInfo( void );
void dbGetStatus( void );
int dbGetStatusPoll(void);
int dbSetExec( int exec );
void dbRunTo(unsigned int addr, int flag);
void dbSetBreak(unsigned int addr, unsigned int flag, int count);
void dbClearBreaks(void);
void dbGetBreaks(void);
void dbGetRegs( void );
int dbSetReg(const char *regname, unsigned int value);
void dbReboot( void );
void dbGetMem( unsigned int addr, int len, const char *outfile );
int dbUploadMem( unsigned int addr, char *buff, size_t len );
void dbUploadMemFile( unsigned int addr, const char *infile, size_t len );
unsigned int dbReadValue( int wordsz, unsigned int addr );
void dbWriteValue( int wordsz, unsigned int addr, unsigned int val );

/* prompt.c */
void promptMode( void );

/* serial.c */
int commOpen( const char *port, int baud );
int commWriteByte( int value );
int commWriteBytes( void *buff, int len );
int commReadByte( void );
int commReadBytes( void *buff, int len );
int commPendingBytes(void);
void commClose( void );

#endif /* _MAIN_H */
