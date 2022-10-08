#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psxgpu.h>
#include <psxsio.h>
#include "psxcd.h"
#include "isofs.h"

// Uncommend to enable debug output
//#define DEBUG

#ifdef DEBUG
#define _LOG(...) printf(__VA_ARGS__)
#else
#define _LOG(...)
#endif

#define DEFAULT_PATH_SEP	'\\'
#define IS_PATH_SEP(ch)		(((ch) == '/') || ((ch) == '\\'))

typedef struct _CdlDIR_INT
{
	uint32_t	_pos;
	uint32_t	_len;
	uint8_t		*_dir;
} CdlDIR_INT;

extern int _cd_media_changed;


static int		_cd_iso_last_dir_lba;
static uint8_t	_cd_iso_descriptor_buff[2048];
static uint8_t	*_cd_iso_pathtable_buff=NULL;
static uint8_t	*_cd_iso_directory_buff=NULL;
static int		_cd_iso_directory_len;
static int		_cd_iso_error=0;

static int _CdReadIsoDescriptor(int session_offs)
{
	int i;
	CdlLOC loc;
	ISO_DESCRIPTOR *descriptor;
	
	// Check if the lid had been opened
	if( !_cd_media_changed )
	{
		CdControl(CdlNop, 0, 0);
		if( (CdStatus()&0x10) )
		{
			// Check if lid is still open
			CdControl(CdlNop, 0, 0);
			if( (CdStatus()&0x10) )
			{
				_LOG("psxcd: Lid is still open.\n");

				_cd_iso_error = CdlIsoLidOpen;
				return -1;
			}
			// Reparse the file system
			_cd_media_changed = 1;
		}
	}
	
	if( !_cd_media_changed )
	{
		return 0;
	}

	_LOG("psxcd: Parsing ISO file system.\n");

	// Seek to volume descriptor
	CdIntToPos(16+session_offs, &loc);
	if( !CdControl(CdlSetloc, (uint8_t*)&loc, 0) )
	{
		_LOG("psxcd: Could not set seek destination.\n");

		_cd_iso_error = CdlIsoSeekError;
		return -1;
	}

	_LOG("psxcd: Read sectors.\n");

	// Read volume descriptor
	CdRead(1, (uint32_t*)_cd_iso_descriptor_buff, CdlModeSpeed);
	
	if( CdReadSync(0, 0) )
	{
		_LOG("psxcd: Error reading ISO volume descriptor.\n");

		_cd_iso_error = CdlIsoReadError;
		return -1;
	}

	_LOG("psxcd: Read complete.\n");

	// Verify if volume descriptor is present
	descriptor = (ISO_DESCRIPTOR*)_cd_iso_descriptor_buff;
	if( strncmp("CD001", descriptor->header.id, 5) )
	{
		_LOG("psxcd: Disc does not contain a ISO9660 file system.\n");

		_cd_iso_error = CdlIsoInvalidFs;
		return -1;
	}

	_LOG("psxcd: Path table LBA = %d\n", descriptor->pathTable1Offs);
	_LOG("psxcd: Path table len = %d\n", descriptor->pathTableSize.lsb);

	// Allocate path table buffer
	i = ((2047+descriptor->pathTableSize.lsb)>>11)<<11;
	if( _cd_iso_pathtable_buff )
	{
		free(_cd_iso_pathtable_buff);
	}
	_cd_iso_pathtable_buff = (uint8_t*)malloc(i);

	_LOG("psxcd: Allocated %d bytes for path table.\n", i);

	// Read path table
	CdIntToPos(descriptor->pathTable1Offs, &loc);
	CdControl(CdlSetloc, (uint8_t*)&loc, 0);
	CdRead(i>>11, (uint32_t*)_cd_iso_pathtable_buff, CdlModeSpeed);
	if( CdReadSync(0, 0) )
	{
		_LOG("psxcd: Error reading ISO path table.\n");

		_cd_iso_error = CdlIsoReadError;
		return -1;
	}
	
	_cd_iso_last_dir_lba	= 0;
	_cd_iso_error			= CdlIsoOkay;
	
	_cd_media_changed		= 0;
	
	return 0;
}

static int _CdReadIsoDirectory(int lba)
{
	int i;
	CdlLOC loc;
	ISO_DIR_ENTRY *direntry;
	
	if( lba == _cd_iso_last_dir_lba )
	{
		return 0;
	}
	
	CdIntToPos(lba, &loc);
	i = CdPosToInt(&loc);

	_LOG("psxcd_dbg: Seek to sector %d\n", i);

	if( !CdControl(CdlSetloc, (uint8_t*)&loc, 0) )
	{
		_LOG("psxcd: Could not set seek destination.\n");

		_cd_iso_error = CdlIsoSeekError;
		return -1;
	}
	
	if( _cd_iso_directory_buff )
	{
		free(_cd_iso_directory_buff);
	}
	
	// Read first sector of directory record
	_cd_iso_directory_buff = (uint8_t*)malloc(2048);
	CdRead(1, (uint32_t*)_cd_iso_directory_buff, CdlModeSpeed);
	if( CdReadSync(0, 0) )
	{
		_LOG("psxcd: Error reading initial directory record.\n");

		_cd_iso_error = CdlIsoReadError;
		return -1;
	}
	
	direntry = (ISO_DIR_ENTRY*)_cd_iso_directory_buff;
	_cd_iso_directory_len = direntry->entrySize.lsb;

	_LOG("psxcd: Location of directory record = %d\n", direntry->entryOffs.lsb);
	_LOG("psxcd: Size of directory record = %d\n", _cd_iso_directory_len);

	if( _cd_iso_directory_len > 2048 )
	{
		if( !CdControl(CdlSetloc, (uint8_t*)&loc, 0) )
		{
			_LOG("psxcd: Could not set seek destination.\n");

			_cd_iso_error = CdlIsoSeekError;
			return -1;
		}
	
		free(_cd_iso_directory_buff);
		i = ((2047+_cd_iso_directory_len)>>11)<<11;
		_cd_iso_directory_buff = (uint8_t*)malloc(i);

		_LOG("psxcd: Allocated %d bytes for directory record.\n", i);

		CdRead(i>>11, (uint32_t*)_cd_iso_directory_buff, CdlModeSpeed);
		if( CdReadSync(0, 0) )
		{
			_LOG("psxcd: Error reading remaining directory record.\n");

			_cd_iso_error = CdlIsoReadError;
			return -1;
		}
	}
	
	_cd_iso_last_dir_lba = lba;
	_cd_iso_error = CdlIsoOkay;
	
	return 0;
}

#ifdef DEBUG

static void dump_directory(void)
{
	int i;
	int dir_pos;
	ISO_DIR_ENTRY *dir_entry;
	char namebuff[16];
	
	_LOG("psxcd: Cached directory record contents:\n");
	
	i = 0;
	dir_pos = 0;
	while(1)
	{
		dir_entry = (ISO_DIR_ENTRY*)(_cd_iso_directory_buff+dir_pos);
		
		strncpy(namebuff, 
			_cd_iso_directory_buff+dir_pos+sizeof(ISO_DIR_ENTRY), dir_entry->identifierLen);
			
		_LOG("P:%d L:%d %s\n", dir_pos, dir_entry->identifierLen, namebuff);
		
		dir_pos += dir_entry->entryLength;
		i++;
		
		// Check if padding is reached (end of record sector)
		if( _cd_iso_directory_buff[dir_pos] == 0 )
		{
			// Snap it to next sector
			dir_pos = ((dir_pos+2047)>>11)<<11;
			
			// Break if exceeds length of directory buffer (end)
			if( dir_pos >= _cd_iso_directory_len )
			{
				break;
			}
		}
	}
	
	_LOG("--\n");
	
}

static void dump_pathtable(void)
{
	uint8_t *tbl_pos;
	ISO_PATHTABLE_ENTRY *tbl_entry;
	ISO_DESCRIPTOR *descriptor;
	char namebuff[16];
	
	_LOG("psxcd: Path table entries:\n");
	
	descriptor = (ISO_DESCRIPTOR*)_cd_iso_descriptor_buff;
	
	tbl_pos = _cd_iso_pathtable_buff;
	tbl_entry = (ISO_PATHTABLE_ENTRY*)tbl_pos;
	
	while( (int)(tbl_pos-_cd_iso_pathtable_buff) <
		descriptor->pathTableSize.lsb )
	{
		strncpy(namebuff, 
			tbl_pos+sizeof(ISO_PATHTABLE_ENTRY), 
			tbl_entry->nameLength);
		
		_LOG("psxcd: %s\n", namebuff);
		
		// Advance to next entry
		tbl_pos += sizeof(ISO_PATHTABLE_ENTRY)
			+(2*((tbl_entry->nameLength+1)/2));
			
		tbl_entry = (ISO_PATHTABLE_ENTRY*)tbl_pos;
	}
	
}

#endif

static int get_pathtable_entry(int entry, ISO_PATHTABLE_ENTRY *tbl, char *namebuff)
{
	int i;
	uint8_t *tbl_pos;
	ISO_PATHTABLE_ENTRY *tbl_entry;
	ISO_DESCRIPTOR *descriptor;
	
	descriptor = (ISO_DESCRIPTOR*)_cd_iso_descriptor_buff;
	
	tbl_pos = _cd_iso_pathtable_buff;
	tbl_entry = (ISO_PATHTABLE_ENTRY*)tbl_pos;
	
	i = 0;
	while( (int)(tbl_pos-_cd_iso_pathtable_buff) <
		descriptor->pathTableSize.lsb )
	{
		if( i == (entry-1) )
		{
			if( namebuff )
			{
				strncpy(namebuff, 
					tbl_pos+sizeof(ISO_PATHTABLE_ENTRY), 
					tbl_entry->nameLength);
			}
			
			if( tbl )
			{
				*tbl = *tbl_entry;
			}
			
			return 0;
		}
		
		// Advance to next entry
		tbl_pos += sizeof(ISO_PATHTABLE_ENTRY)
			+(2*((tbl_entry->nameLength+1)/2));
			
		tbl_entry = (ISO_PATHTABLE_ENTRY*)tbl_pos;
		i++;
	}
	
	if( entry <= 0 )
	{
		return i+1;
	}
	
	return -1;
}

static char* resolve_pathtable_path(int entry, char *rbuff)
{
	char namebuff[16];
	ISO_PATHTABLE_ENTRY tbl_entry;
	
	*rbuff = 0;
	
	do
	{
		if( get_pathtable_entry(entry, &tbl_entry, namebuff) )
		{
			return NULL;
		}
		
		rbuff -= tbl_entry.nameLength;
		memcpy(rbuff, namebuff, tbl_entry.nameLength);
		rbuff--;
		*rbuff = DEFAULT_PATH_SEP;
	
		// Parse to the parent
		entry = tbl_entry.dirLevel;
	
	} while( entry > 1 );
	
	return rbuff;
}

static int find_dir_entry(const char *name, ISO_DIR_ENTRY *dirent)
{
	int i;
	int dir_pos;
	ISO_DIR_ENTRY *dir_entry;
	char namebuff[16];

	_LOG("psxcd: Locating file %s.\n", name);

	i = 0;
	dir_pos = 0;
	while(dir_pos < _cd_iso_directory_len)
	{
		dir_entry = (ISO_DIR_ENTRY*)(_cd_iso_directory_buff+dir_pos);

		if( !(dir_entry->flags & 0x2) )
		{
			strncpy(namebuff, 
				_cd_iso_directory_buff+dir_pos+sizeof(ISO_DIR_ENTRY), 
				dir_entry->identifierLen);
			
			if( strcmp(namebuff, name) == 0 )
			{
				*dirent = *dir_entry;
				return 0;
			}
		}
		
		dir_pos += dir_entry->entryLength;
		i++;
		
		// Check if padding is reached (end of record sector)
		if( _cd_iso_directory_buff[dir_pos] == 0 )
		{
			// Snap it to next sector
			dir_pos = ((dir_pos+2047)>>11)<<11;
			
		}
	}
	
	return -1;
}

static char* get_pathname(char *path, const char *filename)
{
	const char *c = 0;
	for (const char *i = filename; *i; i++) {
		if (IS_PATH_SEP(*i))
			c = i;
	}
	
	if(( c == filename ) || ( !c ))
	{
		path[0] = DEFAULT_PATH_SEP;
		path[1] = 0;
		return NULL;
	}
	
	strncpy(path, filename, (int)(c-filename));
	return path;
}

static char* get_filename(char *name, const char *filename)
{
	const char *c = 0;
	for (const char *i = filename; *i; i++) {
		if (IS_PATH_SEP(*i))
			c = i;
	}
	
	if (!c) {
		strcpy(name, filename);
		return name;
	}
	if (c == filename) {
		strcpy(name, filename+1);
		return name;
	}
	
	c++;
	strcpy(name, c);
	return name;
}

CdlFILE *CdSearchFile(CdlFILE *fp, const char *filename)
{
	int i,j,found_dir,num_dirs;
	int dir_len;
	char tpath_rbuff[128];
	char search_path[128];
	char *rbuff;
	ISO_PATHTABLE_ENTRY tbl_entry;
	ISO_DIR_ENTRY dir_entry;
	
	// Read ISO descriptor if changed flag is set
	//if( _cd_media_changed )
	//{
		// Read ISO descriptor and path table
	if( _CdReadIsoDescriptor(0) )
	{
		_LOG("psxcd: Could not read ISO file system.\n");
		return NULL;
	}

	//	_LOG("psxcd: ISO file system cache updated.\n");
	//	_cd_media_changed = 0;
	//}
	
	// Get number of directories in path table
	num_dirs = get_pathtable_entry(0, NULL, NULL);
	
#ifdef DEBUG
	_LOG("psxcd: Directories in path table: %d\n", num_dirs);
	
	rbuff = resolve_pathtable_path(num_dirs-1, tpath_rbuff+127);

	if( !rbuff )
	{
		_LOG("psxcd: Could not resolve path.\n");
	}
	else
	{
		_LOG("psxcd: Longest path: %s|\n", rbuff);
	}
#endif
	
	if( get_pathname(search_path, filename) )
	{
		_LOG("psxcd: Search path = %s|\n", search_path);
	}
	
	// Search the pathtable for a matching path
	found_dir = 0;
	for(i=1; i<num_dirs; i++)
	{
		rbuff = resolve_pathtable_path(i, tpath_rbuff+127);
		_LOG("psxcd: Found = %s|\n", rbuff);

		if( rbuff )
		{
			if( strcmp(search_path, rbuff) == 0 )
			{
				found_dir = i;
				break;
			}
		}
	}
	
	if( !found_dir )
	{
		_LOG("psxcd: Directory path not found.\n");
		return NULL;
	}

	_LOG("psxcd: Found directory at record %d!\n", found_dir);

	get_pathtable_entry(found_dir, &tbl_entry, NULL);
	_LOG("psxcd: Directory LBA = %d\n", tbl_entry.dirOffs);

	_CdReadIsoDirectory(tbl_entry.dirOffs);
	get_filename(fp->name, filename);
	
	// Add version number if not specified
	if( !strchr(fp->name, ';') )
	{
		strcat(fp->name, ";1");
	}
	
#ifdef DEBUG
	dump_directory();
#endif
	
	if( find_dir_entry(fp->name, &dir_entry) )
	{
		_LOG("psxcd: Could not find file.\n");

		return NULL;
	}

	_LOG("psxcd: Located file at LBA %d.\n", dir_entry.entryOffs.lsb);

	CdIntToPos(dir_entry.entryOffs.lsb, &fp->pos);
	fp->size = dir_entry.entrySize.lsb;
	
	return fp;
}

CdlDIR *CdOpenDir(const char* path)
{
	CdlDIR_INT*	dir;
	int			num_dirs;
	int			i,found_dir;
	char		tpath_rbuff[128];
	char*		rbuff;
	
	ISO_PATHTABLE_ENTRY tbl_entry;
	
	// Read ISO descriptor if changed flag is set
//	if( _cd_media_changed )
//	{
	// Read ISO descriptor and path table
	if( _CdReadIsoDescriptor( 0 ) )
	{
		_LOG( "psxcd: Could not read ISO file system.\n" );
		return NULL;
	}

//		_LOG( "psxcd: ISO file system cache updated.\n" );
//		_cd_media_changed = 0;
//	}
	
	num_dirs = get_pathtable_entry( 0, NULL, NULL );

	found_dir = 0;
	for( i=1; i<num_dirs; i++ )
	{
		rbuff = resolve_pathtable_path( i, tpath_rbuff+127 );
		_LOG( "psxcd_dbg: Found = %s|\n", rbuff );

		if( rbuff )
		{
			if( strcmp( path, rbuff ) == 0 )
			{
				found_dir = i;
				break;
			}
		}
	}
	
	if( !found_dir )
	{
		_LOG( "psxcd_dbg: Directory path not found.\n" );
		return NULL;
	}

	_LOG( "psxcd_dbg: Found directory at record %d!\n", found_dir );

	get_pathtable_entry( found_dir, &tbl_entry, NULL );
	_LOG( "psxcd_dbg: Directory LBA = %d\n", tbl_entry.dirOffs );

	_CdReadIsoDirectory( tbl_entry.dirOffs );

	dir = (CdlDIR_INT*)malloc( sizeof(CdlDIR_INT) );
	
	dir->_len = _cd_iso_directory_len;
	dir->_dir = malloc( _cd_iso_directory_len );
	
	memcpy( dir->_dir, _cd_iso_directory_buff, _cd_iso_directory_len );
	
	dir->_pos = 0;
	
	if( found_dir == 1 )
	{
		ISO_DIR_ENTRY *dir_entry;
	
		for( i=0; i<2; i++ )
		{
			dir_entry = (ISO_DIR_ENTRY*)(dir->_dir+dir->_pos);
			dir->_pos += dir_entry->entryLength;
		}
	}
	
	return (CdlDIR)dir;
}

int CdReadDir(CdlDIR *dir, CdlFILE* file)
{
	CdlDIR_INT*		d_dir;
	ISO_DIR_ENTRY*	dir_entry;

	// Locks up in an infinite loop if directory record size is 6144 bytes
	
	d_dir = (CdlDIR_INT*)dir;
	
	if( d_dir->_pos >= _cd_iso_directory_len )
		return 0;
		
	// Some generated file systems have a premature NULL entry, consider this
	// the end of the directory record
	if( d_dir->_dir[d_dir->_pos] == 0 )
		return 0;
	
	dir_entry = (ISO_DIR_ENTRY*)(d_dir->_dir+d_dir->_pos);
		
	if( d_dir->_dir[d_dir->_pos+sizeof(ISO_DIR_ENTRY)] == 0 )
	{
		strcpy( file->name, "." );
	}
	else if( d_dir->_dir[d_dir->_pos+sizeof(ISO_DIR_ENTRY)] == 1 )
	{
		strcpy( file->name, ".." );
	}
	else
	{
		strncpy( file->name, 
			d_dir->_dir+d_dir->_pos+sizeof(ISO_DIR_ENTRY), 
			dir_entry->identifierLen );
	}
	
	CdIntToPos( dir_entry->entryOffs.lsb, &file->pos );
	
	file->size = dir_entry->entrySize.lsb;

	_LOG("dir_entry->entryLength = %d, ", dir_entry->entryLength);

	d_dir->_pos += dir_entry->entryLength;

	_LOG("d_dir->_pos = %d\n", d_dir->_pos);

	// Check if padding is reached (end of record sector)
	if( d_dir->_dir[d_dir->_pos] == 0 )
	{
		// Snap it to next sector
		d_dir->_pos = ((d_dir->_pos+2047)>>11)<<11;
	}
	
	return 1;
}

void CdCloseDir(CdlDIR *dir)
{
	CdlDIR_INT*	d_dir;
	
	d_dir = (CdlDIR_INT*)dir;
	
	free( d_dir->_dir );
	free( d_dir );
}

int CdIsoError()
{
	return _cd_iso_error;
}

int CdGetVolumeLabel(char* label)
{
	int i;
	ISO_DESCRIPTOR* descriptor;
	
	if( _CdReadIsoDescriptor(0) )
	{
		return -1;
	}
	
	descriptor = (ISO_DESCRIPTOR*)_cd_iso_descriptor_buff;
	
	i = 0;
	for( i=0; (descriptor->volumeID[i]!=0x20)&&(i<32); i++ )
	{
		label[i] = descriptor->volumeID[i];
	}
	
	label[i] = 0x00;
	
	return 0;
}


// Session load routine

void _cd_control(unsigned char com, unsigned char *param, int plen);

static volatile unsigned int _ready_oldcb;

static volatile int _ses_scanfound;
static volatile int _ses_scancount;
static volatile int _ses_scancomplete;
//static volatile char _ses_scan_resultbuff[8];
static volatile char *_ses_scanbuff;

static void _scan_callback(int status, unsigned char *result)
{
	if( status == CdlDataReady )
	{
		CdGetSector((void*)_ses_scanbuff, 512);
		
		if( _ses_scanbuff[0] == 0x1 )
		{
			if( strncmp((const char*)_ses_scanbuff+1, "CD001", 5) == 0 )
			{
				_cd_control(CdlPause, 0, 0);
				_ses_scancomplete = 1;
				_ses_scanfound = 1;
				return;
			}
		}
		_ses_scancount++;
		if( _ses_scancount >= 512 )
		{
			_cd_control(CdlPause, 0, 0);
			_ses_scancomplete = 1;
			return;
		}
	}
	
	if( status == CdlDiskError )
	{
		_cd_control(CdlPause, 0, 0);
		_ses_scancomplete = 1;
	}
}

int CdLoadSession(int session)
{
	CdlLOC *loc;
	unsigned int ready_oldcb;
	char scanbuff[2048];
	char resultbuff[16];
	int i;

	// Seek to specified session	
	_LOG("psxcd: CdLoadSession(): Seeking to session %d...\n", session);
	CdControl(CdlSetsession, (unsigned char*)&session, 
		(unsigned char*)&resultbuff);
	
	if( CdSync(0, 0) == CdlDiskError )
	{
		_LOG("psxcd: CdLoadSession(): Session seek failed, "
			"session does not exist.\n");
		_LOG("psxcd: CdLoadSession(): Restarting CD-ROM...\n");

		// Restart CD-ROM on session seek failure
		CdControl(CdlNop, 0, 0);
		CdControl(CdlInit, 0, 0);
		CdSync(0, 0);
		
		return -1;
	}
	
	// Set search routine callback
	ready_oldcb = CdReadyCallback(_scan_callback);
	
	_ses_scanfound = 0;
	_ses_scancount = 0;
	_ses_scancomplete = 0;
	_ses_scanbuff = scanbuff;
	
	// Begin scan for an ISO volume descriptor
	_LOG("psxcd: CdLoadSession(): Scanning for ISO9660 volume descriptor.\n");

	i = CdlModeSpeed;
	CdControl(CdlSetmode, (unsigned char*)&i, 0);
	CdControl(CdlReadN, 0, (unsigned char*)resultbuff);
	
	// Wait until scan complete
	while(!_ses_scancomplete);
		
	CdReadyCallback((void*)_ready_oldcb);
	
	if( !_ses_scanfound )
	{
		_LOG("psxcd: CdLoadSession(): Did not find volume descriptor.\n");

		_cd_iso_error = CdlIsoInvalidFs;
		CdReadyCallback((CdlCB)ready_oldcb);
		return -1;
	}
	
	// Restore old callback if any
	CdReadyCallback((CdlCB)ready_oldcb);
	
	// Wait until CD-ROM has completely stopped reading, to get a consistent
	// fix of the CD-ROM pickup's current location
	do
	{
		VSync(2);
		CdControl(CdlNop, 0, 0);
	} while(CdStatus()&0xE0);

	// Get location of volume descriptor
	CdControl(CdlGetlocL, 0, (unsigned char*)resultbuff);
	CdSync(0, 0);
	
	loc = (CdlLOC*)resultbuff;

	_LOG("psxcd: CdLoadSession(): Session found in %02d:%02d:%02d (LBA=%d)\n",
		btoi(loc->minute), btoi(loc->second), btoi(loc->sector), CdPosToInt(loc));

	i = CdPosToInt(loc)-17;
	_LOG("psxcd: CdLoadSession(): Session starting at LBA=%d\n", i);

	_cd_media_changed = 1;
	
	if( _CdReadIsoDescriptor(i) )
	{
		return -1;
	}

	return 0;
}
