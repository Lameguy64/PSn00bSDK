#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <psxgpu.h>
#include <psxsio.h>
#include "psxcd.h"

// Uncommend to enable debug output
//#define DEBUG

#pragma pack(push, 1)

/// Structure of a double-endian unsigned short word
typedef struct ISO_USHORT_PAIR
{
	unsigned short	lsb;	/// LSB format 16-bit word
	unsigned short	msb;	/// MSB format 16-bit word
} ISO_USHORT_PAIR;

/// Structure of a double-endian unsigned int word
typedef struct ISO_UINT_PAIR
{
	unsigned int	lsb;		/// LSB format 32-bit word
	unsigned int	msb;		/// MSB format 32-bit word
} ISO_UINT_PAIR;

/// ISO descriptor header structure
typedef struct ISO_DESCRIPTOR_HEADER
{
	unsigned char	type;		/// Volume descriptor type (1 is descriptor, 255 is descriptor terminator)
	char			id[5];		/// Volume descriptor ID (always CD001)
	unsigned short	version;	/// Volume descriptor version (always 0x01)
} ISO_DESCRIPTOR_HEADER;

/// Structure of a date stamp for ISO_DIR_ENTRY structure
typedef struct ISO_DATESTAMP
{
	unsigned char	year;		/// number of years since 1900
	unsigned char	month;		/// month, where 1=January, 2=February, etc.
	unsigned char	day;		/// day of month, in the range from 1 to 31
	unsigned char	hour;		/// hour, in the range from 0 to 23
	unsigned char	minute;		/// minute, in the range from 0 to 59
	unsigned char	second;		/// Second, in the range from 0 to 59
	unsigned char	GMToffs;	/// Greenwich Mean Time offset
} ISO_DATESTAMP;

/// Structure of an ISO path table entry (specifically for the cd::IsoReader class)
typedef struct ISO_PATHTABLE_ENTRY
{
	unsigned char nameLength;	/// Name length (or 1 for the root directory)
	unsigned char extLength;	/// Number of sectors in extended attribute record
	unsigned int dirOffs;		/// Number of the first sector in the directory, as a double word
	unsigned short dirLevel;	/// Index of the directory record's parent directory
								/// If nameLength is odd numbered, a padding byte will be present after the identifier text.
} ISO_PATHTABLE_ENTRY;

typedef struct ISO_DIR_ENTRY
{
	unsigned char entryLength;			// Directory entry length (variable, use for parsing through entries)
	unsigned char extLength;			// Extended entry data length (always 0)
	ISO_UINT_PAIR entryOffs;			// Points to the LBA of the file/directory entry
	ISO_UINT_PAIR entrySize;			// Size of the file/directory entry
	ISO_DATESTAMP entryDate;			// Date & time stamp of entry
	unsigned char flags;				// File flags (0x02 for directories, 0x00 for files)
	unsigned char fileUnitSize;			// Unit size (usually 0 even with Form 2 files such as STR/XA)
	unsigned char interleaveGapSize;	// Interleave gap size (usually 0 even with Form 2 files such as STR/XA)
	ISO_USHORT_PAIR volSeqNum;			// Volume sequence number (always 1)
	unsigned char identifierLen;		// Identifier (file/directory name) length in bytes
} ISO_DIR_ENTRY;

typedef struct ISO_ROOTDIR_HEADER
{
	unsigned char entryLength;			// Always 34 bytes
	unsigned char extLength;			// Always 0
	ISO_UINT_PAIR entryOffs;			// Should point to LBA 22
	ISO_UINT_PAIR entrySize;			// Size of entry extent
	ISO_DATESTAMP entryDate;			// Record date and time
	unsigned char flags;				// File flags
	unsigned char fileUnitSize;
	unsigned char interleaveGapSize;
	ISO_USHORT_PAIR volSeqNum;
	unsigned char identifierLen;		// 0x01
	unsigned char identifier;			// 0x00
} ISO_ROOTDIR_HEADER;

// ISO descriptor structure
typedef struct ISO_DESCRIPTOR
{

	// ISO descriptor header
	ISO_DESCRIPTOR_HEADER header;
	// System ID (always PLAYSTATION)
	char systemID[32];
	// Volume ID (or label, can be blank or anything)
	char volumeID[32];
	// Unused null bytes
	unsigned char pad2[8];
	// Size of volume in sector units
	ISO_UINT_PAIR volumeSize;
	// Unused null bytes
	unsigned char pad3[32];
	// Number of discs in this volume set (always 1 for single volume)
	ISO_USHORT_PAIR volumeSetSize;
	// Number of this disc in volume set (always 1 for single volume)
	ISO_USHORT_PAIR volumeSeqNumber;
	// Size of sector in bytes (always 2048 bytes)
	ISO_USHORT_PAIR sectorSize;
	// Path table size in bytes (applies to all the path tables)
	ISO_UINT_PAIR pathTableSize;
	// LBA to Type-L path table
	unsigned int	pathTable1Offs;
	// LBA to optional Type-L path table (usually a copy of the primary path table)
	unsigned int	pathTable2Offs;
	// LBA to Type-L path table but with MSB format values
	unsigned int	pathTable1MSBoffs;
	// LBA to optional Type-L path table but with MSB format values (usually a copy of the main path table)
	unsigned int	pathTable2MSBoffs;
	// Directory entry for the root directory (similar to a directory entry)
	ISO_ROOTDIR_HEADER	rootDirRecord;
	// Volume set identifier (can be blank or anything)
	char	volumeSetIdentifier[128];
	// Publisher identifier (can be blank or anything)
	char	publisherIdentifier[128];
	// Data preparer identifier (can be blank or anything)
	char	dataPreparerIdentifier[128];
	// Application identifier (always PLAYSTATION)
	char	applicationIdentifier[128];
	// Copyright file in the file system identifier (can be blank or anything)
	char	copyrightFileIdentifier[37];
	// Abstract file in the file system identifier (can be blank or anything)
	char	abstractFileIdentifier[37];
	// Bibliographical file identifier in the file system (can be blank or anything)
	char	bibliographicFilelIdentifier[37];
	// Volume create date (in text format YYYYMMDDHHMMSSMMGG)
	char	volumeCreateDate[17];
	// Volume modify date (in text format YYYYMMDDHHMMSSMMGG)
	char	volumeModifyDate[17];
	// Volume expiry date (in text format YYYYMMDDHHMMSSMMGG)
	char	volumeExpiryDate[17];
	// Volume effective date (in text format YYYYMMDDHHMMSSMMGG)
	char	volumeEffeciveDate[17];
	// File structure version (always 1)
	unsigned char	fileStructVersion;
	// Padding
	unsigned char	dummy0;
	// Application specific data (says CD-XA001 at [141], the rest are null bytes)
	unsigned char	appData[512];
	// Padding
	unsigned char	pad4[653];

} ISO_DESCRIPTOR;

// Leave non-aligned structure packing
#pragma pack(pop)

extern char _cd_media_changed;
int _cd_iso_last_dir_lba;

u_char _cd_iso_descriptor_buff[2048];
u_char *_cd_iso_pathtable_buff=NULL;
u_char *_cd_iso_directory_buff=NULL;
int _cd_iso_directory_len;

int _CdReadIsoDescriptor(int session_offs)
{
	int i;
	CdlLOC loc;
	ISO_DESCRIPTOR *descriptor;
	
	// Seek to volume descriptor location
	CdIntToPos(16+session_offs, &loc);
	if( !CdControl(CdlSetloc, (u_char*)&loc, 0) )
	{
#ifdef DEBUG
		printf("psxcd: Could not set seek destination.\n");
#endif
		return -1;
	}
	
	// Read volume descriptor
	CdRead(1, (u_int*)_cd_iso_descriptor_buff, CdlModeSpeed);
	if( CdReadSync(0, 0) )
	{
#ifdef DEBUG
		printf("psxcd: Error reading ISO volume descriptor.\n");
#endif
		return -1;
	}
	
	
	// Verify if volume descriptor is present
	descriptor = (ISO_DESCRIPTOR*)_cd_iso_descriptor_buff;
	if( strncmp("CD001", descriptor->header.id, 5) )
	{
#ifdef DEBUG
		printf("psxcd: Disc does not have a ISO9660 file system.\n");
#endif
		return -1;
	}
	
#ifdef DEBUG
	printf("psxcd_dbg: Path table LBA = %d\n", descriptor->pathTable1Offs);
	printf("psxcd_dbg: Path table len = %d\n", descriptor->pathTableSize.lsb);
#endif
	
	// Allocate path table buffer
	i = ((2047+descriptor->pathTableSize.lsb)>>11)<<11;
	if( _cd_iso_pathtable_buff )
	{
		free(_cd_iso_pathtable_buff);
	}
	_cd_iso_pathtable_buff = (u_char*)malloc(i);
	
#ifdef DEBUG
	printf("psxcd_dbg: Allocated %d bytes for path table.\n", i);
#endif
	
	// Read path table
	CdIntToPos(descriptor->pathTable1Offs, &loc);
	CdControl(CdlSetloc, (u_char*)&loc, 0);
	CdRead(i>>11, (u_int*)_cd_iso_pathtable_buff, CdlModeSpeed);
	if( CdReadSync(0, 0) )
	{
#ifdef DEBUG
		printf("psxcd: Error reading ISO path table.\n");
#endif
		return -1;
	}
	
	_cd_iso_last_dir_lba = 0;
	
	return 0;
}

int _CdReadIsoDirectory(int lba)
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
#ifdef DEBUG
	printf("psxcd_dbg: Seek to sector %d\n", i);
#endif
	if( !CdControl(CdlSetloc, (u_char*)&loc, 0) )
	{
#ifdef DEBUG
		printf("psxcd: Could not set seek destination.\n");
#endif
		return -1;
	}
	
	if( _cd_iso_directory_buff )
	{
		free(_cd_iso_directory_buff);
	}
	
	// Read first sector of directory record
	_cd_iso_directory_buff = (u_char*)malloc(2048);
	CdRead(1, (u_int*)_cd_iso_directory_buff, CdlModeSpeed);
	if( CdReadSync(0, 0) )
	{
#ifdef DEBUG
		printf("psxcd: Error reading initial directory record.\n");
#endif
		return -1;
	}
	
	direntry = (ISO_DIR_ENTRY*)_cd_iso_directory_buff;
	_cd_iso_directory_len = direntry->entrySize.lsb;
	
#ifdef DEBUG
	printf("psxcd_dbg: Location of directory record = %d\n", direntry->entryOffs.lsb);
	printf("psxcd_dbg: Size of directory record = %d\n", _cd_iso_directory_len);
#endif

	if( _cd_iso_directory_len > 2048 )
	{
		if( !CdControl(CdlSetloc, (u_char*)&loc, 0) )
		{
#ifdef DEBUG
			printf("psxcd: Could not set seek destination.\n");
#endif
			return -1;
		}
	
		free(_cd_iso_directory_buff);
		i = ((2047+_cd_iso_directory_len)>>11)<<11;
		_cd_iso_directory_buff = (u_char*)malloc(i);
#ifdef DEBUG
		printf("psxcd_dbg: Allocated %d bytes for directory record.\n", i);
#endif

		CdRead(i>>11, (u_int*)_cd_iso_directory_buff, CdlModeSpeed);
		if( CdReadSync(0, 0) )
		{
#ifdef DEBUG
			printf("psxcd: Error reading initial directory record.\n");
#endif
			return -1;
		}
	}
	
	_cd_iso_last_dir_lba = lba;
	
	return 0;
}

#ifdef DEBUG

void dump_directory(void)
{
	int i;
	int dir_pos;
	ISO_DIR_ENTRY *dir_entry;
	char namebuff[16];
	
	printf("psxcd_dbg: Cached directory record contents:\n");
	
	i = 0;
	dir_pos = 0;
	while(1)
	{
		dir_entry = (ISO_DIR_ENTRY*)(_cd_iso_directory_buff+dir_pos);
		
		strncpy(namebuff, 
			_cd_iso_directory_buff+dir_pos+sizeof(ISO_DIR_ENTRY), dir_entry->identifierLen);
			
		printf("P:%d L:%d %s\n", dir_pos, dir_entry->identifierLen, namebuff);
		
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
	
	printf("--\n");
	
}

void dump_pathtable(void)
{
	u_char *tbl_pos;
	ISO_PATHTABLE_ENTRY *tbl_entry;
	ISO_DESCRIPTOR *descriptor;
	char namebuff[16];
	
	printf("psxcd_dbg: Path table entries:\n");
	
	descriptor = (ISO_DESCRIPTOR*)_cd_iso_descriptor_buff;
	
	tbl_pos = _cd_iso_pathtable_buff;
	tbl_entry = (ISO_PATHTABLE_ENTRY*)tbl_pos;
	
	while( (int)(tbl_pos-_cd_iso_pathtable_buff) <
		descriptor->pathTableSize.lsb )
	{
		strncpy(namebuff, 
			tbl_pos+sizeof(ISO_PATHTABLE_ENTRY), 
			tbl_entry->nameLength);
		
		printf("psxcd_dbg: %s\n", namebuff);
		
		// Advance to next entry
		tbl_pos += sizeof(ISO_PATHTABLE_ENTRY)
			+(2*((tbl_entry->nameLength+1)/2));
			
		tbl_entry = (ISO_PATHTABLE_ENTRY*)tbl_pos;
	}
	
}

#endif

int get_pathtable_entry(int entry, ISO_PATHTABLE_ENTRY *tbl, char *namebuff)
{
	int i;
	u_char *tbl_pos;
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

char *resolve_pathtable_path(int entry, char *rbuff)
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
		*rbuff = '\\';
	
		// Parse to the parent
		entry = tbl_entry.dirLevel;
	
	} while( entry > 1 );
	
	return rbuff;
}

int find_dir_entry(const char *name, ISO_DIR_ENTRY *dirent)
{
	int i;
	int dir_pos;
	ISO_DIR_ENTRY *dir_entry;
	char namebuff[16];
	
#ifdef DEBUG
	printf("psxcd_dbg: Locating file %s.\n", name);
#endif

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

char *get_pathname(char *path, const char *filename)
{
	char *c;
	c = strrchr(filename, '\\');
	
	if(( c == filename ) || ( !c ))
	{
		path[0] = '\\';
		path[1] = 0;
		return NULL;
	}
	
	strncpy(path, filename, (int)(c-filename));
	return path;
}

char *get_filename(char *name, const char *filename)
{
	char *c;
	c = strrchr(filename, '\\');
	
	if(( c == filename ) || ( !c ))
	{
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
	if( _cd_media_changed )
	{
		// Read ISO descriptor and path table
		if( _CdReadIsoDescriptor(0) )
		{
#ifdef DEBUG
			printf("psxcd: Could not read ISO file system.\n");
#endif
			return NULL;
		}
#ifdef DEBUG
		printf("psxcd: ISO file system cache updated.\n");
#endif
		_cd_media_changed = 0;
	}
	
	// Get number of directories in path table
	num_dirs = get_pathtable_entry(0, NULL, NULL);
	
#ifdef DEBUG
	printf("psxcd_dbg: Directories in path table: %d\n", num_dirs);
	
	rbuff = resolve_pathtable_path(num_dirs-1, tpath_rbuff+127);

	if( !rbuff )
	{
		printf("psxcd_dbg: Could not resolve path.\n");
	}
	else
	{
		printf("psxcd_dbg: Longest path: %s|\n", rbuff);
	}
#endif
	
	if( get_pathname(search_path, filename) )
	{
#ifdef DEBUG
		printf("psxcd_dbg: Search path = %s|\n", search_path);
#endif
	}
	
	// Search the pathtable for a matching path
	found_dir = 0;
	for(i=1; i<num_dirs; i++)
	{
		rbuff = resolve_pathtable_path(i, tpath_rbuff+127);
#ifdef DEBUG
		printf("psxcd_dbg: Found = %s|\n", rbuff);
#endif
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
#ifdef DEBUG
		printf("psxcd_dbg: Directory path not found.\n");
#endif
		return NULL;
	}
	
#ifdef DEBUG
	printf("psxcd_dbg: Found directory at record %d!\n", found_dir);
#endif

	get_pathtable_entry(found_dir, &tbl_entry, NULL);

#ifdef DEBUG	
	printf("psxcd_dbg: Directory LBA = %d\n", tbl_entry.dirOffs);
#endif

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
#ifdef DEBUG
		printf("psxcd: Could not find file.\n");
#endif
		return NULL;
	}
	
#ifdef DEBUG
	printf("psxcd_dbg: Located file at LBA %d.\n", dir_entry.entryOffs.lsb);
#endif

	CdIntToPos(dir_entry.entryOffs.lsb, &fp->loc);
	fp->size = dir_entry.entrySize.lsb;
	
	return fp;
}
